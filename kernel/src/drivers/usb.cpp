#include <drivers/usb.h>
#include <drivers/usb_mass_storages.h>

/*
    Written on the 24th of August 2020 12:14am - 6:30am
*/

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::drivers;
using namespace crystalos::util;

USBDriver* USBDriver::OSUSBDriver = 0;

USBDriver::USBDriver()
{
    if (OSUSBDriver == 0) OSUSBDriver = this;
    ddrAddress = 0x810000;
    requestBuffer = 0x820000;

    for (int i = 0; i < 127; i++)
        deviceAddresses[i] = false;
}

USBDriver::~USBDriver()
{
    if (OSUSBDriver == this) OSUSBDriver = 0;
}

void USBDriver::HandlePortChange(EHCIPortChangeStatus* status)
{
    if (status->deviceSpeed == 0x01) print("High speed device attached!\n");
    if (status->deviceSpeed == 0x02) print("Full speed device attached!\n");

    if (status->deviceSpeed == 0x1 || status->deviceSpeed == 0x2)
    {
        status->controller->EnableAsyncSchedule();
        //Step 1: Get the device descriptor
        USBDeviceDescriptorRequest* request = (USBDeviceDescriptorRequest*)ddrAddress;
        request->bmRequestType = 0x80;
        request->bRequest = 0x6;
        request->wValue = 0x100;
        request->wIndex = 0;
        request->wLength = 18;

        //////////////////////////////////////////
        if (!status->controller->DevicePresent(status->portNumber))
        {
            print("USB device state was changed during processing.\n");
            return;
        }
        
        uint8_t stat = status->controller->SendCommand(request, status->controller->defaultControlQueueHead, (uint8_t*)requestBuffer, 3);
        if (stat != 0) 
        {
            print("USB Device Error!\n");
            return;
        }
        /////////////////////////////////////

        USBDeviceDescriptor* usbDescriptor = (USBDeviceDescriptor*)MemoryManager::ActiveMemoryManager->malloc(sizeof(USBDeviceDescriptor));
        if (usbDescriptor == 0)
        { 
            print("USB Memory error.\n");
            return;
        }

        MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)requestBuffer, (uint8_t*)usbDescriptor, sizeof(USBDeviceDescriptor));

        if (!status->controller->DevicePresent(status->portNumber))
        {
            print("USB device state was changed during processing.\n");
            MemoryManager::ActiveMemoryManager->free(usbDescriptor);
            return;
        }

        //Step 2: Get 9 byte configuration descriptor
        request->bmRequestType = 0x80;
        request->bRequest = 0x6;
        request->wValue = 0x200;
        request->wIndex = 0;
        request->wLength = 9;

        /////////////////////////////////////
        stat = status->controller->SendCommand(request, status->controller->defaultControlQueueHead, (uint8_t*)requestBuffer, 3);
        if (stat != 0) 
        {
            print("USB Device Error!\n");
            MemoryManager::ActiveMemoryManager->free(usbDescriptor);
            return;
        }
        ///////////////////////////////////

        USBConfigDescriptor* usbConfig = (USBConfigDescriptor*)MemoryManager::ActiveMemoryManager->malloc(sizeof(1*1024*1024));
        if (usbConfig == 0)
        {
            print("USB Memory error.\n");

            //Just for debugging purposes...
            for (MemoryChunk* chunk = (MemoryChunk*)(10*1024*1024); chunk->next != 0; chunk = chunk->next)
            {
                char bhh[13];
                itoa((uint32_t)chunk, 16, bhh);
                print(bhh);
                itoa((uint32_t)chunk->size, 16, bhh);
                print(bhh);
            }
            ///////////////////////////////
            
            MemoryManager::ActiveMemoryManager->free(usbDescriptor);
            return;
        }
        
        MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)requestBuffer, (uint8_t*)usbConfig, sizeof(USBConfigDescriptor));

        if (!status->controller->DevicePresent(status->portNumber))
        {
            print("USB device state was changed during processing.\n");
            MemoryManager::ActiveMemoryManager->free(usbDescriptor);
            MemoryManager::ActiveMemoryManager->free(usbConfig);
            return;
        }

        //Step 3: Get the config descriptor, all interface descriptors
        // ... and all endpoint descriptors in one shot.
        request->bmRequestType = 0x80;
        request->bRequest = 0x6;
        request->wValue = 0x200;
        request->wIndex = 0;
        request->wLength = usbConfig->totalLength;

        stat = status->controller->SendCommand(request, status->controller->defaultControlQueueHead, (uint8_t*)requestBuffer, 3);
        MemoryManager::ActiveMemoryManager->free(usbConfig);
        if (stat != 0)
        { 
            print("USB Device Error!\n");
            MemoryManager::ActiveMemoryManager->free(usbDescriptor);
            return;
        }

        if (!status->controller->DevicePresent(status->portNumber))
        {
            print("USB device state was changed during processing.\n");
            MemoryManager::ActiveMemoryManager->free(usbDescriptor);
            return;
        }

        usbConfig = (USBConfigDescriptor*)requestBuffer;
        uint8_t* otherDeviceData = (uint8_t*)requestBuffer;
        //Step 4: Some few checks
        
        if (usbConfig->numInterfaces != 0x1)
        {
            print("No support for USB devices that have more than one interface yet!.\n");
            MemoryManager::ActiveMemoryManager->free(usbDescriptor);
            return;
        }

        for (int index = 0; index < usbConfig->totalLength; )
        {
            if (otherDeviceData[index+1] == 0x04)  //Interface descriptor
            {
                USBInterfaceDescriptor* id = (USBInterfaceDescriptor*)(otherDeviceData + index);

                //Handle Mass storage interfaces
                if (id->Class == 0x08 && id->SubClass == 0x06 && id->Protocol == 0x50)
                {
                    //Mass storage device
                    //Protocol : Bulk only (0x50)
                    index += id->Length;
                    //Get endpoint descriptors...
                    //One OUT endpoint descriptor and one IN endpoint descriptor
                    int inEndpoint, outEndpoint, deviceAddress;
                    USBEndpointDescriptor* epin = (USBEndpointDescriptor*)MemoryManager::ActiveMemoryManager->malloc(sizeof(USBEndpointDescriptor));
                    USBEndpointDescriptor* epout = (USBEndpointDescriptor*)MemoryManager::ActiveMemoryManager->malloc(sizeof(USBEndpointDescriptor));

                    for (index; index < usbConfig->totalLength;)
                    {
                        USBEndpointDescriptor* ep = (USBEndpointDescriptor*)(otherDeviceData + index);
                        if (ep->descriptorType == 0x05) //Endpoint descriptor
                        {
                            if (ep->endpointAddress & 0x80)
                            {
                                inEndpoint = otherDeviceData[index+2] & 0x7;
                                MemoryManager::ActiveMemoryManager->memcpy((otherDeviceData + index), (uint8_t*)epin, sizeof(USBEndpointDescriptor));
                            }
                            if (!(ep->endpointAddress & 0x80))
                            {
                                outEndpoint = otherDeviceData[index+2] & 0x7;
                                MemoryManager::ActiveMemoryManager->memcpy((otherDeviceData + index), (uint8_t*)epout, sizeof(USBEndpointDescriptor));
                            }
                        }
                        index += ep->Length;
                    }

                    if (inEndpoint == 0 || outEndpoint == 0 || (!status->controller->DevicePresent(status->portNumber)))
                    {
                        print("This device does not contain valid data endpoints!\n");
                        MemoryManager::ActiveMemoryManager->free(usbDescriptor);
                        MemoryManager::ActiveMemoryManager->free(epin);
                        MemoryManager::ActiveMemoryManager->free(epout);
                        return;
                    }

                    deviceAddress = getFreeAddress();
                    //Step 5: Set Device address
                    request->bmRequestType = 0x00;
                    request->bRequest = 0x5;
                    request->wValue = deviceAddress;	//Device address
                    request->wIndex = 0;
                    request->wLength = 0;

                    stat = status->controller->SendCommand(request, status->controller->defaultControlQueueHead, (uint8_t*)requestBuffer, 2);
                    if (stat != 0) 
                    { 
                        print("USB Device Error!\n");
                        MemoryManager::ActiveMemoryManager->free(usbDescriptor);
                        MemoryManager::ActiveMemoryManager->free(epin);
                        MemoryManager::ActiveMemoryManager->free(epout);
                        deviceAddresses[deviceAddress] = false;                        
                        return;
                    }
                    //Step 6: Add queue heads to the async schedule
                    status->controller->DisableAsyncSchedule();

                    QueueHead* deviceControl = status->controller->AddAsyncScheduleQueueHead(deviceAddress, 0, usbDescriptor->maxPacketSize, true);
                    QueueHead* bulkIn = status->controller->AddAsyncScheduleQueueHead(deviceAddress, inEndpoint, epin->maxPacketSize, false);
                    QueueHead* bulkOut = status->controller->AddAsyncScheduleQueueHead(deviceAddress, outEndpoint, epout->maxPacketSize, false);
                    
                    status->controller->EnableAsyncSchedule();

                    //Step 7: Set device configuration
                    request->bmRequestType = 0x00;
                    request->bRequest = 0x9;
                    request->wValue = 0x1;
                    request->wIndex = 0;
                    request->wLength = 0;

                    stat = status->controller->SendCommand(request, deviceControl, (uint8_t*)requestBuffer, 2);
                    if (stat != 0 || (!status->controller->DevicePresent(status->portNumber))) 
                    { 
                        print("USB Device Error!\n");
                        MemoryManager::ActiveMemoryManager->free(usbDescriptor);
                        MemoryManager::ActiveMemoryManager->free(epin);
                        MemoryManager::ActiveMemoryManager->free(epout);
                        deviceAddresses[deviceAddress] = false;
                        //TODO: Remove deviceControl, bulkIn and bulkOut queue heads
                        return; 
                    }

                    //At this point, A USB Mass storage device has been successfully 
                    // assigned an address and configured.
                    // Create a new USBMassStorage object and pass some params.

                    USBMassStorage* massStorage = (USBMassStorage*)MemoryManager::ActiveMemoryManager->malloc(sizeof(USBMassStorage));
                    if (massStorage == 0 || (!status->controller->DevicePresent(status->portNumber)))
                    {
                        if (massStorage == 0) print("USB Memory error 3.\n");
                        else print("USB device was removed during processing .\n");
                        MemoryManager::ActiveMemoryManager->free(usbDescriptor);
                        MemoryManager::ActiveMemoryManager->free(epin);
                        MemoryManager::ActiveMemoryManager->free(epout);
                        deviceAddresses[deviceAddress] = false;
                        //Remove deviceControl, bulkIn and bulkOut queue heads
                        return;
                    }

                    new (massStorage) USBMassStorage(status, usbDescriptor, usbConfig, deviceControl, bulkIn, bulkOut);
                    if (massStorage->Initialize() != 0)
                    {
                        print("USB Mass Storage Init error.\n");
                        MemoryManager::ActiveMemoryManager->free(usbDescriptor);
                        MemoryManager::ActiveMemoryManager->free(epin);
                        MemoryManager::ActiveMemoryManager->free(epout);
                        MemoryManager::ActiveMemoryManager->free(massStorage);
                        deviceAddresses[deviceAddress] = false;
                        //Remove deviceControl, bulkIn and bulkOut queue heads
                        return;
                    }
                    break;
                }

                //TODO: Handle some other interfaces
                else
                {
                    print("Unknown USB Interface.\n");                    
                }
            }
            index += otherDeviceData[index]; //Get length field
        }
    }
    else if (status->deviceSpeed == 0x0) //Device removed
    {
        print("USB Device Removed.\n");
    }
}

uint16_t USBDriver::getFreeAddress()
{
    for (uint16_t i = 1; i < 127; i++)
        if (!deviceAddresses[i]) 
        {
            deviceAddresses[i] = true;
            return i;
        }
    return 0;
}