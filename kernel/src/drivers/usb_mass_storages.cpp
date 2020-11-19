#include <drivers/usb_mass_storages.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::drivers;
using namespace crystalos::util;

USBMassStorageDevices* USBMassStorageDevices::Devices = 0;

USBMassStorage::USBMassStorage(EHCIPortChangeStatus* estatus, USBDeviceDescriptor* usbDescriptor, USBConfigDescriptor* usbConfig, QueueHead* deviceControl, QueueHead* bulkIn, QueueHead* bulkOut)
{
    memory = (uint8_t*)0x830000;
    statusBuffer = 0x860000;
    dataBuffer = 0x850000;
    portStatus = estatus;

    deviceControlQueueHead = deviceControl;
    bulkInQueueHead = bulkIn;
    bulkOutQueueHead = bulkOut;
}

int USBMassStorage::Initialize()
{
    int retries = 0; uint8_t status;
    ((uint8_t*)statusBuffer)[12] = 0xff;
    while (((uint8_t*)statusBuffer)[12] != 0 && retries < 5)  //There was an error
    {
        retries++;
        status = Inquiry();
        if (status != 0)
        {
            print("Mass storage Inquiry Error!\n");
            continue;
        }
        MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)(dataBuffer + 8), (uint8_t*)ProductID, 8);
        ProductID[8] = 0;
        MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)(dataBuffer + 16), (uint8_t*)DeviceID, 8);
        DeviceID[8] = 0;
    }

    if (retries >= 5) return status;

    retries = 0;
    ((uint8_t*)statusBuffer)[12] = 0xff;
    while (((uint8_t*)statusBuffer)[12] != 0 && retries < 5)  //There was am error
    {
        retries++;
        status = TestUnitReady();
        if (status != 0)
        {
            print("Mass storage Test Unit Ready Error!\n");
            continue;
        }
        
        uint8_t tmp = ((uint8_t*)statusBuffer)[12];
        if (((uint8_t*)statusBuffer)[12] != 0)
        {       
            status = RequestSense();
            if (status != 0)
            {
                print("Mass storage Request sense Error!\n");
                continue;
            }
        }
        ((uint8_t*)statusBuffer)[12] = tmp;
    }

    if (retries >= 5) return status;

    retries = 0;
    ((uint8_t*)statusBuffer)[12] = 0xff;
    while (((uint8_t*)statusBuffer)[12] != 0 && retries < 5)  //There was am error
    {
        retries++;
        status = ReadCapacity();
        if (status != 0)
        {
            print("Mass storage Read Capacity Error!\n");
            continue;
        }
        
        uint8_t tmp = ((uint8_t*)statusBuffer)[12];
        if (((uint8_t*)statusBuffer)[12] != 0)
        {       
            status = RequestSense();
            if (status != 0)
            {
                print("Mass storage Request sense Error!\n");
                continue;
            }
        }
        ((uint8_t*)statusBuffer)[12] = tmp;
    }
    
    if (retries >= 5) return status;

    uint8_t* buf = (uint8_t*)dataBuffer;
    bytesPerSector = buf[7];
    bytesPerSector |= ((uint32_t)buf[6] << 8);
    bytesPerSector |= ((uint32_t)buf[5] << 16);
    bytesPerSector |= ((uint32_t)buf[4] << 24);

    uint32_t max_lba = buf[3];
    max_lba |= ((uint32_t)buf[2] << 8);
    max_lba |= ((uint32_t)buf[1] << 16);
    max_lba |= ((uint32_t)buf[0] << 24);

    numSectors = max_lba + 1;
    sizeInGB = numSectors / (1024*1024*2);

    status = Read(0, 1, (uint8_t*)dataBuffer);  //try the read command
    if (status != 0)
    {
        print("Drive Read error!\n"); 
        // for (int i = 0; i < 13; i++) printHex8(((uint8_t*)statusBuffer)[i]);
        return status;
    }

    if (!USBMassStorageDevices::Devices->AddDevice(this)) return 0xff;
    print("New USB Mass Storage Initialized.\n");

    status = Read(0, 1, (uint8_t*)dataBuffer);  //try the read command again
    if (status != 0)
    {
        print("Drive Read error!\n"); 
        // for (int i = 0; i < 13; i++) printHex8(((uint8_t*)statusBuffer)[i]);
        return status;
    }

    print("Dumping first sector ::: \n");
    for (int i = 0; i < 512; i++)
    {
        char* str = " \0";
        str[0] = ((uint8_t*)dataBuffer)[i];
        print(str);
    }
    print("\n***END OF DATA***\n");

    return 0;
}

USBMassStorage::~USBMassStorage()
{

}

uint8_t USBMassStorage::SendCommand(uint32_t request, uint8_t* buffer, int numStages)
{
    //Command transport
    uint32_t dataSize = ((uint32_t*)request)[2];
    uint32_t* td = (uint32_t*)0x8f0000;
    td[0] = 1;  //Terminate - Next qTD pointer
    td[1] = 1;  //Terminate - Alternate next qTD pointer
    /*
        DWORD2:
        bulkToggle: 0
        Total bytes to transfer: 31;
        Interrupt on complete: 1;
        Current page: 0;
        Cerr: 0;
        PID_Code: 0x0 (OUT)
        Status: 0x80; (ACTIVE)
    */
    td[2] = 0x001F8080;

    //DWORDS 3 - 12 (Buffer page pointers)
    td[3] = request;
    td[4] = request + 0x1000;
    td[5] = request + 0x2000;
    td[6] = request + 0x3000;
    td[7] = request + 0x4000;
    td[8] = 0;
    td[9] = 0;
    td[10] = 0;
    td[11] = 0;
    td[12] = 0;

    portStatus->controller->USBInterrupt = false;
    bulkOutQueueHead->DWORD5 = (uint32_t)td;

    while(!portStatus->controller->USBInterrupt);
    uint8_t status = td[2] & 0xff;
    if (status != 0) 
    {
        return status;
    }

    if (numStages == 3)
    {
        //Data transport
        td[0] = 1;  //Terminate
        td[1] = 1;  //Terminate
        /*
            DWORD2:
            dataToggle: 0
            Total bytes to transfer: ;
            Interrupt on complete: 1;
            Current page: 0;
            Cerr: 0;
            PID_Code: 0x1 (IN)
            Status: 0x80; (ACTIVE)
        */
        td[2] = 0x00008180;
        td[2] |= dataSize << 16;
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

        portStatus->controller->USBInterrupt = false;
        bulkInQueueHead->DWORD5 = (uint32_t)td;

        while(!portStatus->controller->USBInterrupt);
        status = td[2] & 0xff;
        if (status != 0) 
        {
            return status;
        }
    }    

    //Status transport
    td[0] = 1;  //Terminate
    td[1] = 1;  //Terminate
    /*
        DWORD2:
        controlToggle: 0
        Total bytes to transfer: 13;
        Interrupt on complete: 1;
        Current page: 0;
        Cerr: 0;
        PID_Code: 0x1 (IN)
        Status: 0x80; (ACTIVE)
    */
    td[2] = 0x000d8180;
    td[3] = statusBuffer;
    td[4] = statusBuffer + 0x1000;
    td[5] = statusBuffer + 0x2000;
    td[6] = statusBuffer + 0x3000;
    td[7] = statusBuffer + 0x4000;
	td[8] = 0;
    td[9] = 0;
    td[10] = 0;
    td[11] = 0;
    td[12] = 0;

    portStatus->controller->USBInterrupt = false;
    bulkInQueueHead->DWORD5 = (uint32_t)td;

    while(!portStatus->controller->USBInterrupt);
    status = td[2] & 0xff;
    return status;
}

uint8_t USBMassStorage::Inquiry()
{
    MassStorageInquiryRequest* request = (MassStorageInquiryRequest*)memory;
    request->CBWSignature = 0x43425355;
    request->CBWTag = 0xaabbccdd;
    request->CBWDataTransferLength = 0x24;
    request->CBWFlags = 0x80;
    request->CBWLun = 0;
    request->CBWCBLength = 6;
    
    request->SCSIOpCode = 0x12;
    request->rsvd0 = 0;
    request->SCSIPage = 0;
    request->rsvd1 = 0;
    request->SCSIAllocationLength = 0x24;
    request->SCSIControl = 0;

    for (int i = 0; i < 15; i++) request->Pad[i] = 0;
    return SendCommand((uint32_t)request, (uint8_t*)dataBuffer, 3);
}

uint8_t USBMassStorage::TestUnitReady()
{
    MassStorageInquiryRequest* request = (MassStorageInquiryRequest*)memory;
    
    request->CBWSignature = 0x43425355;
    request->CBWTag = 0xaabbccdd;
    request->CBWDataTransferLength = 0;
    request->CBWFlags = 0;
    request->CBWLun = 0;
    request->CBWCBLength = 6;
    
    request->SCSIOpCode = 0;
    request->rsvd0 = 0;
    request->SCSIPage = 0;
    request->rsvd1 = 0;
    request->SCSIAllocationLength = 0;
    request->SCSIControl = 0;
    for (int i = 0; i < 15; i++) request->Pad[i] = 0;

    return SendCommand((uint32_t)request, (uint8_t*)dataBuffer, 2);
}

uint8_t USBMassStorage::RequestSense()
{
    RequestSenseRequest* inq = (RequestSenseRequest*)memory;
    
    //Request Sense
    inq->CBWSignature = 0x43425355;
    inq->CBWTag = 0xaabbccdd;
    inq->CBWDataTransferLength = 18;
    inq->CBWFlags = 0x80;
    inq->CBWLun = 0;
    inq->CBWCBLength = 6;
    
    inq->SCSIOpCode = 0x03;
    inq->rsvd0 = 0;
    inq->SCSIPage = 0;
    inq->rsvd1 = 0;
    inq->SCSIAllocationLength = 18;
    inq->SCSIControl = 0;
    
    for (int id = 0; id < 6; id++) inq->Pad[id] = 0;
    
    return SendCommand((uint32_t)inq, (uint8_t*)dataBuffer, 3);
}

uint8_t USBMassStorage::ReadCapacity()
{
    MassStorageInquiryRequest* request = (MassStorageInquiryRequest*)memory;
    
    request->CBWSignature = 0x43425355;
    request->CBWTag = 0xaabbccdd;
    request->CBWDataTransferLength = 8;
    request->CBWFlags = 0x80;
    request->CBWLun = 0;
    request->CBWCBLength = 10;
    
    request->SCSIOpCode = 0x25;
    request->rsvd0 = 0;
    request->SCSIPage = 0;
    request->rsvd1 = 0;
    request->SCSIAllocationLength = 8;
    request->SCSIControl = 0;
    for (int i = 0; i < 15; i++) request->Pad[i] = 0;

    return SendCommand((uint32_t)request, (uint8_t*)dataBuffer, 3);
}

uint8_t USBMassStorage::Read(uint32_t lba, int count, uint8_t* buffer)
{
    if (lba > numSectors - 1 || count > 16 || (lba+count) > numSectors) return 0xff;

    ReadRequest* request = (ReadRequest*)memory;

    request->CBWSignature = 0x43425355;
    request->CBWTag = 0xaabbccdd;
    request->CBWDataTransferLength = count * bytesPerSector;
    request->CBWFlags = 0x80;
    request->CBWLun = 0;
    request->CBWCBLength = 10;

    request->SCSIOpCode = 0x28;
    request->ReadProtect = 0;
    request->lba0 = (lba >> 24) & 0xff;
    request->lba1 = (lba >> 16) & 0xff;
    request->lba2 = (lba >> 8) & 0xff;
    request->lba3 = lba & 0xff;
    request->groupnum = 0;
    request->TransferLengthHi = (count >> 8) & 0xff;
    request->TransferLengthLo = count & 0xff;
    request->control = 0;

    for (int i = 0; i < 6; i++) request->Pad[i] = 0;

    uint8_t status = SendCommand((uint32_t)request, (uint8_t*)dataBuffer, 3);

    if (status != 0) return status;

    if (dataBuffer != (uint32_t)buffer) MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)dataBuffer, (uint8_t*)buffer, count * bytesPerSector);
    
    return 0;
}




USBMassStorageDevices::USBMassStorageDevices()
{
    if (Devices == 0) Devices = this;
    
    for (int i = 0; i < 127; i++) MassStorageDevices[i] = 0;
    
}

USBMassStorageDevices::~USBMassStorageDevices()
{

}

bool USBMassStorageDevices::AddDevice(USBMassStorage* device)
{
    for (int i = 0; i < 127; i++)
    {
        if (MassStorageDevices[i] == 0) break;
        if (MassStorageDevices[i]->portStatus->portNumber == device->portStatus->portNumber 
            && MassStorageDevices[i]->portStatus->controller == device->portStatus->controller)
        {
            return true;
        }
    }
    for (int i = 0; i < 127; i++)
    {
        if (MassStorageDevices[i] == 0)
        {
            MassStorageDevices[i] = device;
            return true;
        }
    }
    return false;
}

void USBMassStorageDevices::RemoveDevice(USBMassStorage* device)
{
    for (int i = 0; i < 127; i++)
    {
        if (MassStorageDevices[i] == device) MassStorageDevices[i] = 0;
    }
}
