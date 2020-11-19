#ifndef CRYSTALOS__DRIVERS__USB_MASSSTORAGE__H
#define CRYSTALOS__DRIVERS__USB_MASSSTORAGE__H

#include <common/types.h>
#include <util/screen.h>
#include <util/number.h>
#include <drivers/usb.h>

/* This is the USB Mass storage Device Driver.
    Handles devices that have only one interface. Supports reading and writing sectors.
*/

namespace crystalos
{
    namespace drivers
    {
        struct MassStorageInquiryRequest
		{
			common::uint32_t CBWSignature;
			common::uint32_t CBWTag;
			common::uint32_t CBWDataTransferLength;
			common::uint8_t CBWFlags;
			common::uint8_t CBWLun;
			common::uint8_t CBWCBLength;
			
			common::uint8_t SCSIOpCode;
			common::uint8_t rsvd0;
			common::uint8_t SCSIPage;
			common::uint8_t rsvd1;
			common::uint8_t SCSIAllocationLength;
			common::uint8_t SCSIControl;
			common::uint8_t Pad[15];				
			
		}__attribute__((packed));
		
		struct RequestSenseRequest
		{
			common::uint32_t CBWSignature;
			common::uint32_t CBWTag;
			common::uint32_t CBWDataTransferLength;
			common::uint8_t CBWFlags;
			common::uint8_t CBWLun;
			common::uint8_t CBWCBLength;
			
			common::uint8_t SCSIOpCode;
			common::uint8_t rsvd0;
			common::uint32_t SCSIPage;
			common::uint8_t rsvd1;
			common::uint16_t SCSIAllocationLength;
			common::uint8_t SCSIControl;
			common::uint8_t Pad[6];	
		}__attribute__((packed));
		
		struct ReadRequest
		{
			common::uint32_t CBWSignature;
			common::uint32_t CBWTag;
			common::uint32_t CBWDataTransferLength;
			common::uint8_t CBWFlags;
			common::uint8_t CBWLun;
			common::uint8_t CBWCBLength;
			
			common::uint8_t SCSIOpCode;
			common::uint8_t ReadProtect;
			common::uint8_t lba0;
			common::uint8_t lba1;
			common::uint8_t lba2;
			common::uint8_t lba3;
			common::uint8_t groupnum;
			common::uint8_t TransferLengthHi;
			common::uint8_t TransferLengthLo;
			common::uint8_t control;
			common::uint8_t Pad[6];	
		}__attribute__((packed));
		
        class USBMassStorage
        {
            private:
                common::uint8_t* memory;
                common::uint32_t statusBuffer;
                common::uint32_t dataBuffer;
                common::uint8_t SendCommand(common::uint32_t request, common::uint8_t* buffer, int numStages);
                QueueHead* deviceControlQueueHead;
                QueueHead* bulkInQueueHead;
                QueueHead* bulkOutQueueHead;

            public:
                EHCIPortChangeStatus* portStatus;
                common::uint8_t Inquiry();
                common::uint8_t TestUnitReady();
                common::uint8_t RequestSense();
                common::uint8_t ReadCapacity();
                common::uint8_t Read(common::uint32_t lba, int count, common::uint8_t*);

                int Initialize();

				char ProductID[9];
				char DeviceID[9];
                common::uint32_t bytesPerSector;
                common::uint32_t numSectors;
				int sizeInGB;

                USBMassStorage(EHCIPortChangeStatus*, USBDeviceDescriptor* usbDescriptor, USBConfigDescriptor* usbConfig, QueueHead* deviceControl, QueueHead* bulkIn, QueueHead* bulkOut);
                ~USBMassStorage();
        };

		class USBMassStorageDevices
		{
			private:
				
			public:
				static USBMassStorageDevices* Devices;

				bool AddDevice(USBMassStorage*);
				void RemoveDevice(USBMassStorage*);

				USBMassStorage* MassStorageDevices[127];

				USBMassStorageDevices();
				~USBMassStorageDevices();
		};
    }
}


#endif