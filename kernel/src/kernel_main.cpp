#include <common/types.h>
#include <IO/port.h>
#include <IO/interrupts.h>
#include <IO/pci.h>
#include <drivers/driver.h>
#include <drivers/ide.h>
#include <drivers/ahci.h>
#include <drivers/keyboard.h>
#include <drivers/usb.h>
#include <drivers/ehci.h>
#include <drivers/usb_mass_storages.h>
#include <graphics/vesa.h>
#include <util/system.h>
#include <util/screen.h>
#include <util/number.h>
#include <util/string.h>
#include <util/events.h>
#include <memory_manager.h>
#include <multitasking.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::IO;
using namespace crystalos::drivers;
using namespace crystalos::graphics;
using namespace crystalos::util;

extern "C" void kernel_main(uint32_t kernel_info_block)
{
	
	clearScreen();
	print("Hello World!\n");

	BootInformation* inf_struc = (BootInformation*)kernel_info_block;
	VESAModeInfo* currentVideoMode = (VESAModeInfo*)&(inf_struc->currentModeInfo);

	MemoryMapEntry memMapEntries[inf_struc->no_of_memory_entries];
	uint64_t* mem_list_pointer = (uint64_t*)inf_struc->memory_entries;
	uint32_t* mem_attrib_pointer = (uint32_t*)inf_struc->memory_entries;
	int index0 = 0;
	int index1 = 4;
	uint64_t amountofRAM = 0;

	for(uint32_t i = 0; i < inf_struc->no_of_memory_entries; i++)
	{
		MemoryMapEntry tmp_mem_entry;
		tmp_mem_entry.baseAddress = mem_list_pointer[index0];
		tmp_mem_entry.length = mem_list_pointer[index0+1];
		tmp_mem_entry.type = mem_attrib_pointer[index1];
		tmp_mem_entry.acpi_extended= mem_attrib_pointer[index1+1];

		memMapEntries[i] = tmp_mem_entry;
		if (tmp_mem_entry.type == 0x1) amountofRAM += tmp_mem_entry.length;
		index0 += 3;
		index1 += 6;
	}

	uint32_t amountofRAMinMB = amountofRAM / 1024 / 1024;
	if (amountofRAMinMB < 50) return;

	MemoryManager memoryManager(10*1024*1024, 70*1024*1024);

	InterruptManager* kernel_interrupts = (InterruptManager*)memoryManager.malloc(sizeof(InterruptManager));
	if (kernel_interrupts == 0) return;

	DriverManager* kernel_drivers = (DriverManager*)memoryManager.malloc(sizeof(DriverManager));
	if (kernel_drivers == 0) return;

	TaskManager* taskManager = (TaskManager*)memoryManager.malloc(sizeof(TaskManager));
	if (taskManager == 0) return;

	PeripheralComponentInterconnectController* pci = (PeripheralComponentInterconnectController*)memoryManager.malloc(sizeof(PeripheralComponentInterconnectController));
	if (pci == 0) return;

	KeyboardDriver* keyboard = (KeyboardDriver*)memoryManager.malloc(sizeof(KeyboardDriver));
	if (keyboard == 0) return;
	
	IDEDriver* ide_driver = (IDEDriver*)memoryManager.malloc(sizeof(IDEDriver));
	if (ide_driver == 0) return;

	AHCIDriver* ahci_driver = (AHCIDriver*)memoryManager.malloc(sizeof(AHCIDriver));
	if (ahci_driver == 0) return;

	USBDriver* usb_driver = (USBDriver*)memoryManager.malloc(sizeof(USBDriver));
	if (usb_driver == 0) return;

	USBMassStorageDevices* usbMassStorages = (USBMassStorageDevices*)memoryManager.malloc(sizeof(USBMassStorageDevices));
	if (usbMassStorages == 0) return;

	EHCIDriver* ehci_driver = (EHCIDriver*)memoryManager.malloc(sizeof(EHCIDriver));
	if (ehci_driver == 0) return;


	PCIDeviceDescriptor* ahci_descriptor = (PCIDeviceDescriptor*)memoryManager.malloc(sizeof(PCIDeviceDescriptor));
	if (ahci_descriptor == 0) return;
	PCIDeviceDescriptor* ide_descriptor = (PCIDeviceDescriptor*)memoryManager.malloc(sizeof(PCIDeviceDescriptor));
	if (ide_descriptor == 0) return;
	PCIDeviceDescriptor* ehci_descriptor = (PCIDeviceDescriptor*)memoryManager.malloc(sizeof(PCIDeviceDescriptor));
	if (ehci_descriptor == 0) return;

	new (taskManager) TaskManager();	
	new (kernel_interrupts) InterruptManager(); kernel_interrupts->Activate();	
	new (kernel_drivers) DriverManager();
	new (pci) PeripheralComponentInterconnectController();
	new (usbMassStorages) USBMassStorageDevices();

	pci->GetDescriptor(0x01, 0x01, ide_descriptor);
	if (ide_descriptor->class_id == 0x01 && ide_descriptor->subclass_id == 0x01)
	{
		print("Initializing IDE Driver\n");
		memoryManager.free(ide_descriptor);
		new (ide_driver) IDEDriver(0x1F0, 0x170, kernel_interrupts, 0x0E, 0x0F);
		ide_driver->Initialize();
	}
	else 
	{
		memoryManager.free(ide_driver);
		memoryManager.free(ide_descriptor);
		ide_driver = 0;
	}

	pci->GetDescriptor(0x01, 0x06, ahci_descriptor);
	if (ahci_descriptor->class_id == 0x01 && ahci_descriptor->subclass_id == 0x06)
	{
		print("Initializing AHCI Driver\n");
		new (ahci_driver) AHCIDriver(pci, kernel_interrupts, ahci_descriptor);
		ahci_driver->Initialize();
	}
	else 
	{
		memoryManager.free(ahci_driver);
		memoryManager.free(ahci_descriptor);
		ahci_driver = 0;
	}

	new (usb_driver) USBDriver();
	pci->GetDescriptor(0x0C, 0x03, 0x20, ehci_descriptor);
	if (ehci_descriptor->class_id == 0x0C && ehci_descriptor->subclass_id == 0x03 && ehci_descriptor->interface_id == 0x20)
	{
		print("Initializing USB EHCI Controller\n");
		new (ehci_driver) EHCIDriver(pci, ehci_descriptor, kernel_interrupts);
		kernel_drivers->Add(ehci_driver);
		ehci_driver->portCanChange = false;
	}
	else 
	{
		memoryManager.free(ehci_driver);
		memoryManager.free(ehci_descriptor);
		ehci_driver = 0;
	}

	IDEDevice* ide_disk = 0;
	SATADISK* ahci_disk = 0;
	int diskIndex = 0;

	uint32_t os_signature = *(uint32_t*)(0x7c00 + 442);		
	uint8_t* buffer = (uint8_t*)memoryManager.malloc(512);
	
	if (ide_driver != 0)
	{
		if (!(ide_driver->primaryChannel.masterDrive.IsEmpty()))
		{
			ide_driver->primaryChannel.masterDrive.Read(0, 1, buffer);
			if (*(uint32_t*)(buffer + 442) == os_signature) diskIndex = 1;
			ide_disk = &ide_driver->primaryChannel.masterDrive;
		}
		if (diskIndex == 0)
		{
			if (!(ide_driver->primaryChannel.slaveDrive.IsEmpty()))
			{
				ide_driver->primaryChannel.slaveDrive.Read(0, 1, buffer);
				if (*(uint32_t*)(buffer + 442) == os_signature) diskIndex = 2;
				ide_disk = &ide_driver->primaryChannel.slaveDrive;
			}
		}
		if (diskIndex == 0)
		{
			if (!(ide_driver->secondaryChannel.masterDrive.IsEmpty()))
			{
				ide_driver->secondaryChannel.masterDrive.Read(0, 1, buffer);
				if (*(uint32_t*)(buffer + 442) == os_signature) diskIndex = 3;
				ide_disk = &ide_driver->secondaryChannel.masterDrive;
			}
		}
		if (diskIndex == 0)
		{
			if (!(ide_driver->secondaryChannel.slaveDrive.IsEmpty()))
			{
				ide_driver->secondaryChannel.slaveDrive.Read(0, 1, buffer);
				if (*(uint32_t*)(buffer + 442) == os_signature) diskIndex = 4;
				ide_disk = &ide_driver->secondaryChannel.slaveDrive;
			}
		}
	}
	if (ahci_driver != 0)
	{
		if (diskIndex == 0)
		{
			for (int j = 0; j < ahci_driver->numDisks; j++)
			{
				ahci_disk = ahci_driver->disks[j];
				ahci_disk->Read(0, 1, buffer);
				if (*(uint32_t*)(buffer + 442) == os_signature)
				{
					diskIndex = 5 + j;
					break;
				}
			}
		}
	}

	kernel_drivers->ActivateAll();

	if (diskIndex == 0)
	{
		if (ehci_driver != 0)
		{
			print("Checking for USB Boot.. Please wait...\n");
			Sleep(40);	//2 seconds
			
			for (int i = 0; i < ehci_driver->numPorts; i++)
			{
				if (ehci_driver->DevicePresent(i))
				{
					ehci_driver->CheckPort(i);
				}
			}
			
			ehci_driver->EnableAsyncSchedule();
			for (int i = 0; i < 127; i++)
			{
				if (usbMassStorages->MassStorageDevices[i] == 0) continue;
				uint8_t status = usbMassStorages->MassStorageDevices[i]->Read(0, 1, buffer);
				if (*(uint32_t*)(buffer + 442) == os_signature)
				{
					diskIndex = 0x81 + i;
					break;
				}
			}
			ehci_driver->DisableAsyncSchedule();
		}
	}

	if (diskIndex == 0) {print("Boot disk not found! Please reboot your PC");return;}
	ehci_driver->portCanChange = true;


	ConsoleKeyboardHandler* kbhandler = (ConsoleKeyboardHandler*)memoryManager.malloc(sizeof(ConsoleKeyboardHandler));
	if (kbhandler != 0){
		new (kbhandler) ConsoleKeyboardHandler;
		if (keyboard != 0) new (keyboard) KeyboardDriver(kernel_interrupts, kbhandler); //keyboard driver
	}
	kernel_drivers->Add(keyboard);
	keyboard->Activate();

	print("OS Ready. Schedules running...\n");

	while(1)
	{
		if (kernel_interrupts->ScheduleEnabled){
			for (int int_num = 0; int_num < 256; int_num++)
			{
				if (kernel_interrupts->ScheduleEntries[int_num].handlerAddress == 0) break;
                if (kernel_interrupts->ScheduleEntries[int_num].free) continue;
				if (kernel_interrupts->ScheduleEntries[int_num].currentFrame >= kernel_interrupts->ScheduleEntries[int_num].lastFrame)
				{
					kernel_interrupts->ScheduleEnabled = false;
					kernel_interrupts->ScheduleEntries[int_num].free = true;
					kernel_interrupts->ScheduleEntries[int_num].handlerAddress();
					kernel_interrupts->ScheduleEnabled = true;
				}
			}
		}
	}
}
