#include <IO/pci.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::IO;

void print(char* str);
void printHex32(uint32_t);

PeripheralComponentInterconnectController::PeripheralComponentInterconnectController()
: dataPort(0xCFC),
  commandPort(0xCF8)
{
	
}

PeripheralComponentInterconnectController::~PeripheralComponentInterconnectController()
{
	
}

uint32_t PeripheralComponentInterconnectController::Read(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset)
{
	uint32_t id = 
		0x1 << 31
		| ((bus & 0xFF) << 16)
		| ((device & 0x1F) << 11)
		| ((function & 0X07) << 8)
		| (registeroffset & 0xFC);
	
	commandPort.Write(id);
	uint32_t result = dataPort.Read();
	return result >> (8* (registeroffset % 4));
}

void PeripheralComponentInterconnectController::Write(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t value)
{
	uint32_t id = 
		0x1 << 31
		| ((bus & 0xFF) << 16)
		| ((device & 0x1F) << 11)
		| ((function & 0X07) << 8)
		| (registeroffset & 0xFC);
	
	commandPort.Write(id);
	dataPort.Write(value);
}

bool PeripheralComponentInterconnectController::DeviceHasFunctions(uint16_t bus, uint16_t device)
{
	return Read(bus, device, 0, 0x0E) & (1<<7);
}

void PeripheralComponentInterconnectController::GetDescriptor(uint16_t class_id, uint16_t subclass_id, PCIDeviceDescriptor* result)
{
	MemoryManager::ActiveMemoryManager->memset((uint8_t*)result, 0, sizeof(PCIDeviceDescriptor));
	for (int bus = 0; bus < 8; bus++)
	{
		for (int device = 0; device < 32; device++)
		{
			int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;
			for (int function = 0; function < numFunctions; function++)
			{
				PCIDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);

				if (dev.class_id == class_id && dev.subclass_id == subclass_id)
				{
					MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)&dev, (uint8_t*)result, sizeof(PCIDeviceDescriptor));
					return;
				}
			}
		}
	}
}

void PeripheralComponentInterconnectController::GetDescriptor(uint16_t class_id, uint16_t subclass_id, uint16_t interface_id, PCIDeviceDescriptor* result)
{
	MemoryManager::ActiveMemoryManager->memset((uint8_t*)result, 0, sizeof(PCIDeviceDescriptor));
	for (int bus = 0; bus < 8; bus++)
	{
		for (int device = 0; device < 32; device++)
		{
			int numFunctions = DeviceHasFunctions(bus, device) ? 8 : 1;
			for (int function = 0; function < numFunctions; function++)
			{
				PCIDeviceDescriptor dev = GetDeviceDescriptor(bus, device, function);

				if (dev.class_id == class_id && dev.subclass_id == subclass_id && dev.interface_id == interface_id)
				{
					MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)&dev, (uint8_t*)result, sizeof(PCIDeviceDescriptor));
					return;
				}
			}
		}
	}
}

uint32_t PeripheralComponentInterconnectController::GetBaseAddressRegister(uint16_t bus, uint16_t device, uint16_t function, uint16_t barNum)
{
	uint8_t headerType = Read(bus, device, function, 0x0E) & 0x7F;
	int maxBARS = 6 - (4*headerType);
	if (barNum > maxBARS)
		return 0;
	
	uint32_t bar_value = Read(bus, device, function, 0x10 + (4*barNum));

	if (bar_value & 0x1)	//IO
	{
		return (bar_value & ~0x3);
	}
	else	//Memory mapping
	{
		return (bar_value & ~0x7);
	}
}

PCIDeviceDescriptor PeripheralComponentInterconnectController::GetDeviceDescriptor(uint16_t bus, uint16_t device, uint16_t function)
{
	PCIDeviceDescriptor result;
	
	result.bus = bus;
	result.device = device;
	result.function = function;
	
	result.vendor_id = Read(bus, device, function, 0x00);
	result.device_id = Read(bus, device, function, 0x02);
	
	result.command = Read(bus, device, function, 0x04);
	result.status = Read(bus, device, function, 0x06);
	
	result.class_id = Read(bus, device, function, 0x0b);
	result.subclass_id = Read(bus, device, function, 0x0a);
	result.interface_id = Read(bus, device, function, 0x09);
	
	result.revision = Read(bus, device, function, 0x08);
	result.capabilities = Read(bus, device, function, 0x34);
	
	result.interrupt_line = Read(bus, device, function, 0x3c);
	result.interrupt_pin = Read(bus, device, function, 0x3d);

	result.bar0 = GetBaseAddressRegister(bus, device, function, 0);
	result.bar1 = GetBaseAddressRegister(bus, device, function, 1);
	result.bar2 = GetBaseAddressRegister(bus, device, function, 2);
	result.bar3 = GetBaseAddressRegister(bus, device, function, 3);
	result.bar4 = GetBaseAddressRegister(bus, device, function, 4);
	result.bar5 = GetBaseAddressRegister(bus, device, function, 5);
	
	return result;
}