#include <drivers/usb.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::drivers;
using namespace crystalos::IO;
using namespace crystalos::util;

EHCIDriver* EHCIDriver::OSEHCIDriver = 0;

EHCIDriver::EHCIDriver(PeripheralComponentInterconnectController* pci, PCIDeviceDescriptor* dev, InterruptManager* man)
: InterruptHandler(man, 0x20 + dev->interrupt_line)
{
    baseAddress = dev->bar0;

     //Enable PCI BUS Master (bit 2) and Memory space (bit 1)
    uint32_t command = pci->Read(dev->bus, dev->device, dev->function, 4);
    command |= 0x6;
    pci->Write(dev->bus, dev->device, dev->function, 4, command);

    capabilities = (EHCICapRegs*)baseAddress;
    //Get extended capabilities pointer
    uint8_t ext_cap_offset = (capabilities->HCCPARAMS & 0x0000FF00) >> 8;
    int i;
    if (ext_cap_offset > 0)
    {
        uint32_t ext_cap0 = pci->Read(dev->bus, dev->device, dev->function, ext_cap_offset);
        ext_cap0 |= 0x1000000;  //Get ownership of EHCI Controller from BIOS
        pci->Write(dev->bus, dev->device, dev->function, ext_cap_offset, ext_cap0);
        for (i = 0; i < 10; i++)
        {
            ext_cap0 = pci->Read(dev->bus, dev->device, dev->function, ext_cap_offset);
            if (((ext_cap0 & 0x1000000) == 0x1000000) && ((ext_cap0 & 0x10000) == 0))
                break;
        }
        if (i == 10) 
        {
            print("USB Init error!\n");   
            return;
        }
    }

    uint32_t val = capabilities->CAPLEN_HCIVERSION;  //Get Capabilities regs length
    opregs = (EHCIOperationalRegs*)(baseAddress + (val & 0xff));    //Add to base address to get Operational registers start
    
    val = Reset();  //Reset host controller

    capabilities = (EHCICapRegs*)baseAddress;

    val = capabilities->CAPLEN_HCIVERSION;  //Get Capabilities regs length
    opregs = (EHCIOperationalRegs*)(baseAddress + (val & 0xff));    //Add to base address to get Operational registers start

    //Set up async list for control transfers
    //Async list starts at 0x800000

    //First queue head - Control transfers at 0x800000
    QueueHead* qh0 = (QueueHead*)0x800000;
    //DWORD1 - Horizontal link pointer
    qh0->DWORD1 = 0x800002;
    ///DWORD2 - Endpoint characteristics
    /*
        31:28   - NAK reload counter (5h)
        27      - Control endpoint flag (low speed)
        26:16   - Max packet size (0x40 for control transfers)
        15      - Head of reclaim list (1 - This is the first QH)
        14      - Data toggle (1 = Toggle from TD)
        13:12   - Endpoint speed (2 = HS)
        11:8    - Endpoint number (0 for control transfers)
        7       - Inactive on next transaction (For periodic schedule)
        6:0     - Device Address (0 for control transfers before SetAddress)
    */
    qh0->DWORD2 = 0x5040e000;
    //DWORD3 - Endpoint capabilities
    //0x40 - 1 transaction per microframe
    //PortNum = HubAddr = c-Mask = s-Mask = 0
    qh0->DWORD3 = 0x40000000;
    //DWORD4 - DWORD12 : QueueHead overlay
    qh0->DWORD4 = 0;    //Current qTD pointer
    qh0->DWORD5 = 1;    //Next qTD pointer (set the T bit)
    qh0->DWORD6 = 0;    //Alternate next qTD pointer
    qh0->DWORD7 = 0;
    qh0->DWORD8 = 0;
    qh0->DWORD9 = 0;
    qh0->DWORD10 = 0;
    qh0->DWORD11 = 0;
    qh0->DWORD12 = 0;
    //DWORD13 - DWORD17
    qh0->DWORD13 = 0;   //Extended buffer pointer page 0
    qh0->DWORD14 = 0;
    qh0->DWORD15 = 0;
    qh0->DWORD16 = 0;
    qh0->DWORD17 = 0;   //Extended buffer pointer page 4

    portCanChange = true;
    numQueueHeads = 1;
    defaultControlQueueHead = qh0;

    if (OSEHCIDriver == 0) OSEHCIDriver = this;
}

EHCIDriver::~EHCIDriver()
{
}

int EHCIDriver::Reset()
{
    //Halt the Controller
    uint32_t val = opregs->USBCMD;   //Read USBCMD reg
    val &= 0xfffffffe;      //Clear bit 0
    opregs->USBCMD = val;   //Write it back

    //Wait for controller to get halted
    val = opregs->USBSTS;
    while((val & 0x1000) != 0x1000) val = opregs->USBSTS;

    val = opregs->USBCMD;
    val |= 0x2;
    opregs->USBCMD = val;    //Reset Host controller

    Sleep(2);   //100ms
    val = opregs->USBCMD;
    while ((val & 0x2) == 0x2)
    {
        val = opregs->USBCMD;
    }
    
    val = capabilities->HCSPARAMS;
    val &= 0xf;
    numPorts = val;

    opregs->CTRLDSSEGMENT = 0;
    opregs->USBINTR = 0x0000003f;   //Enable all interrupts

    return 0;
}

void EHCIDriver::Activate()
{
    int i;

    //Halt the Controller
    uint32_t val = opregs->USBCMD;   //Read USBCMD reg
    val &= 0xfffffffe;      //Clear bit 0
    opregs->USBCMD = val;   //Write it back

    //Wait for controller to get halted
    val = opregs->USBSTS;
    while((val & 0x1000) != 0x1000) val = opregs->USBSTS;

    // Program the rate at which interrupts are raised
    // Max: every 4ms
    val = opregs->USBCMD;
    val &= 0xff00ffff;
    val |= 0x00200000;
    opregs->USBCMD = val;

    //Start the controller
    val = opregs->USBCMD;
    val |= 1;
    opregs->USBCMD = val;

    opregs->CONFIGFLAG = 1; //Route all ports to EHCI
    Sleep(1);

    print("EHCI Driver activated!\n");
}

uint32_t EHCIDriver::ResetPort(int portno)
{
    uint32_t val = opregs->PORTSC[portno];
    val |= 0x100;
    val &= 0xfffffffb;
    opregs->PORTSC[portno] = val;
    //Send reset signal to port
    
    Sleep(4);  //Sleep for 200ms
    val &= ~0x100;
    opregs->PORTSC[portno] = val;   //Terminate reset sequence

    Sleep(2);

    val = opregs->PORTSC[portno];
    opregs->PORTSC[portno] = opregs->PORTSC[portno];

    if (val & 0x4)
    {
        return 0x01;    //HS Device
    }
    else
    {
        return 0x02;   //FS Device
    }
}

bool EHCIDriver::DevicePresent(int portno)
{
    uint32_t portsc = opregs->PORTSC[portno];
    if (portsc & 0x1) //Device present
        return true;
    else
        return false;
}

uint32_t EHCIDriver::HandleInterrupt(uint32_t esp)
{
    uint32_t int_val = opregs->USBSTS;
    if (int_val & 0x1)  //USB Interrupt
    {
        USBInterrupt = true;
    }
	if (int_val & 0x2)
	{
		print("***USB ERROR***\n");
		// printHex32(opregs->USBSTS);
	}
    if (int_val & 0x4)  //Port change Detected
    {
        for (int i = 0; i < numPorts; i++)
        {
            uint32_t val = opregs->PORTSC[i];
            if (val & 0x2)  //This port experienced a change
            {
                ActivePort = i;
                opregs->PORTSC[i] = opregs->PORTSC[i];
                if (portCanChange)
                {
                    portCanChange = false;
                    void (*handler)() = (void (*)())&HandlePortChange;
                    Schedule(handler, 1);
                }
                break;
            }
        }
    }
    opregs->USBSTS = opregs->USBSTS;
    return esp;
}

void EHCIDriver::CheckPort(int portno)
{
    uint32_t portsc = OSEHCIDriver->opregs->PORTSC[portno];
    uint32_t result = 0;
    if (OSEHCIDriver->DevicePresent(portno))
    {
        if (((portsc >> 10) & 0x3) != 1) //Full or high speed device
        {
            result = OSEHCIDriver->ResetPort(portno);
            EHCIPortChangeStatus* status = (EHCIPortChangeStatus*)MemoryManager::ActiveMemoryManager->malloc(sizeof(EHCIPortChangeStatus));
            status->portNumber = portno;
            status->deviceSpeed = result;
            status->controller = OSEHCIDriver;

            //Check again for device presence on port
            if (!OSEHCIDriver->DevicePresent(portno)) 
            {
                print("USB device was changed during processing.\n");
                MemoryManager::ActiveMemoryManager->free(status);
                return;
            }
            USBDriver::OSUSBDriver->HandlePortChange(status);
        }
        else
        {
            print("Low speed device attached!\n");
            //TODO: Pass control to a companion controller
        }
    }
    else
    {
        print("USB Device removed!\n");
        EHCIPortChangeStatus* status = (EHCIPortChangeStatus*)MemoryManager::ActiveMemoryManager->malloc(sizeof(EHCIPortChangeStatus));
        status->portNumber = portno;
        status->deviceSpeed = result;
        status->controller = OSEHCIDriver;
        USBDriver::OSUSBDriver->HandlePortChange(status);
    }
    OSEHCIDriver->DisableAsyncSchedule();
}

void EHCIDriver::HandlePortChange()
{
    int portno = OSEHCIDriver->ActivePort;
    uint32_t portsc = OSEHCIDriver->opregs->PORTSC[portno];
    uint32_t result = 0;
    if (OSEHCIDriver->DevicePresent(portno))
    {
        if (((portsc >> 10) & 0x3) != 1) //Full or high speed device
        {
            result = OSEHCIDriver->ResetPort(portno);
            EHCIPortChangeStatus* status = (EHCIPortChangeStatus*)MemoryManager::ActiveMemoryManager->malloc(sizeof(EHCIPortChangeStatus));
            status->portNumber = portno;
            status->deviceSpeed = result;
            status->controller = OSEHCIDriver;

            //Check again for device presence on port
            if (!OSEHCIDriver->DevicePresent(portno)) 
            {
                print("USB device was changed during processing.\n");
                MemoryManager::ActiveMemoryManager->free(status);
                OSEHCIDriver->portCanChange = true;
                return;
            }
            USBDriver::OSUSBDriver->HandlePortChange(status);
        }
        else
        {
            print("Low speed device attached!\n");
            //TODO: Pass control to a companion controller
            OSEHCIDriver->portCanChange = true;
        }
    }
    else
    {
        print("USB Drive removed\n");
        EHCIPortChangeStatus* status = (EHCIPortChangeStatus*)MemoryManager::ActiveMemoryManager->malloc(sizeof(EHCIPortChangeStatus));
        status->portNumber = portno;
        status->deviceSpeed = result;
        status->controller = OSEHCIDriver;
        USBDriver::OSUSBDriver->HandlePortChange(status);
    }
    OSEHCIDriver->DisableAsyncSchedule();
    OSEHCIDriver->portCanChange = true;
}

uint8_t EHCIDriver::SendCommand(USBDeviceDescriptorRequest* request, QueueHead* qh, uint8_t* buffer, int numStages)
{
    //Command transport
    uint32_t* td = (uint32_t*)0x8f0000;
    td[0] = 1;  //Terminate - Next qTD pointer
    td[1] = 1;  //Terminate - Alternate next qTD pointer
    /*
        DWORD2:
        controlToggle: 0
        Total bytes to transfer: 8; //Sizeof Device descriptor request
        Interrupt on complete: 1;
        Current page: 0;
        Cerr: 0;
        PID_Code: 0x2 (SETUP)
        Status: 0x80; (ACTIVE)
    */
    td[2] = 0x00088280;

    //DWORDS 3 - 12 (Buffer page pointers)
    td[3] = (uint32_t)request;
    td[4] = (uint32_t)request + 0x1000;
    td[5] = (uint32_t)request + 0x2000;
    td[6] = (uint32_t)request + 0x3000;
    td[7] = (uint32_t)request + 0x4000;
    td[8] = 0;
    td[9] = 0;
    td[10] = 0;
    td[11] = 0;
    td[12] = 0;

    OSEHCIDriver->USBInterrupt = false;
    qh->DWORD5 = (uint32_t)td;     //Attach qTD to queue head for control transfer to begin

    while(!OSEHCIDriver->USBInterrupt);   //Wait for USBInterrupt to be raised
    uint8_t status = td[2] & 0xff;

    if (status != 0) return status;

    if (numStages == 3)
    {
        //Data transport
        td[0] = 1;  //Terminate
        td[1] = 1;  //Terminate
        /*
            DWORD2:
            dataToggle: 1
            Total bytes to transfer: request->wLength
            Interrupt on complete: 1;
            Current page: 0;
            Cerr: 0;
            PID_Code: 0x1 (IN)
            Status: 0x80; (ACTIVE)
        */
        td[2] = 0x80008180;
        td[2] |= (uint32_t)(request->wLength) << 16;
        td[3] = (uint32_t)buffer;
        td[4] = (uint32_t)buffer + 0x1000;
        td[5] = (uint32_t)buffer + 0x2000;
        td[6] = (uint32_t)buffer + 0x3000;
        td[7] = (uint32_t)buffer + 0x4000;
        td[8] = 0;
        td[9] = 0;
        td[10] = 0;
        td[11] = 0;
        td[12] = 0;

        OSEHCIDriver->USBInterrupt = false;
        qh->DWORD5 = (uint32_t)td;

        while(!OSEHCIDriver->USBInterrupt);   //Wait for USBInterrupt to be raised
        status = td[2] & 0xff;
        if (status != 0) return status;
    }

    //Status transport
    td[0] = 1;  //Terminate
    td[1] = 1;  //Terminate
    /* 
        DWORD2:
        controlToggle: 1
        Total bytes to transfer: 0;
        Interrupt on complete: 1;
        Current page: 0;
        Cerr: 0;
        PID_Code: (Data Stage?) 0x0 (OUT) (No Data Stage?) 0x1 (IN)
        Status: 0x80; (ACTIVE)
    */
    
    if (numStages == 3) td[2] = 0x80008080;
    else td[2] = 0x80008180;
    td[3] = 0;
    td[4] = 0;
    td[5] = 0;
    td[6] = 0;
    td[7] = 0;
    td[8] = 0;
    td[9] = 0;
    td[10] = 0;
    td[11] = 0;
    td[12] = 0;

    OSEHCIDriver->USBInterrupt = false;
    qh->DWORD5 = (uint32_t)td;
    
    while(!OSEHCIDriver->USBInterrupt);   //Wait for USBInterrupt to be raised
    status = td[2] & 0xff;

    return status;
}

void EHCIDriver::EnableAsyncSchedule()
{
    opregs->ASYNCLISTADDR = 0x800000;
    opregs->USBCMD |= 0x20;
    while(!(opregs->USBSTS & 0x8000));
        Sleep(1);
}

void EHCIDriver::DisableAsyncSchedule()
{
    opregs->USBCMD &= (uint32_t)(~0x20);
    while(opregs->USBSTS & 0x8000);
        Sleep(1);
}

QueueHead* EHCIDriver::AddAsyncScheduleQueueHead(uint16_t deviceAddress, uint16_t endpointNumber, uint16_t maxPacketSize, bool toggleFromTD)
{ 
    QueueHead* qh1 = (QueueHead*)(0x800000 + (0x1000 * numQueueHeads));
    //DWORD1 - Horizontal link pointer
    qh1->DWORD1 = ((uint32_t)defaultControlQueueHead) | 0x2; //Point back to first queue head.
    ///DWORD2 - Endpoint characteristics
    /*
        31:28   - NAK reload counter (5h)
        27      - Control endpoint flag (low speed)
        26:16   - Max packet size
        15      - Head of reclaim list (0)
        14      - Data toggle
        13:12   - Endpoint speed (2 = HS)
        11:8    - Endpoint number 
        7       - Inactive on next transaction (For periodic schedule)
        6:0     - Device Address 
    */
    qh1->DWORD2 = 0x50002000;
    qh1->DWORD2 |= (deviceAddress & 0x7F); //Set device address
    qh1->DWORD2 |= (uint32_t)(endpointNumber) << 8; //Set endpoint number
    qh1->DWORD2 |=  (uint32_t)(maxPacketSize) << 16; //Max packet size
    if (toggleFromTD) qh1->DWORD2 |= (uint32_t)(1 << 14);

    //DWORD3 - Endpoint capabilities
    //0x40 - 1 transaction per microframe
    //PortNum = HubAddr = c-Mask = s-Mask = 0
    qh1->DWORD3 = 0x40000000;
    //DWORD4 - DWORD12 : QueueHead overlay
    qh1->DWORD4 = 0;    //Current qTD pointer
    qh1->DWORD5 = 1;    //Next qTD pointer (set the T bit)
    qh1->DWORD6 = 0;
    qh1->DWORD7 = 0;
    qh1->DWORD8 = 0;
    qh1->DWORD9 = 0;
    qh1->DWORD10 = 0;
    qh1->DWORD11 = 0;
    qh1->DWORD12 = 0;
    //DWORD13 - DWORD17
    qh1->DWORD13 = 0;   //Extended buffer pointer page 0
    qh1->DWORD14 = 0;
    qh1->DWORD15 = 0;
    qh1->DWORD16 = 0;
    qh1->DWORD17 = 0;   //Extended buffer pointer page 4

    ((QueueHead*)(0x800000 + (0x1000 * (numQueueHeads-1))))->DWORD1 = (uint32_t)qh1 | 0x2;
    numQueueHeads++;
    
    return qh1;
}
