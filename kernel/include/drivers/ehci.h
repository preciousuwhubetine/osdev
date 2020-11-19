#ifndef CRYSTALOS__DRIVERS__EHCI_H
#define CRYSTALOS__DRIVERS__EHCI_H

#include <common/types.h>
#include <drivers/driver.h>
#include <IO/interrupts.h>
#include <IO/pci.h>
#include <util/screen.h>

/*
    This is the EHCI driver for USB 2.0 drives.
    
*/

namespace crystalos
{
    namespace drivers
    {
        struct EHCICapRegs
        {
            common::uint32_t CAPLEN_HCIVERSION;
            common::uint32_t HCSPARAMS;
            common::uint32_t HCCPARAMS;
            common::uint32_t HCSP_PORTROUTE[2];
        }__attribute__((packed));

        struct EHCIOperationalRegs
        {
            common::uint32_t USBCMD;
            common::uint32_t USBSTS;
            common::uint32_t USBINTR;
            common::uint32_t FRINDEX;
            common::uint32_t CTRLDSSEGMENT;
            common::uint32_t PERIODICLISTBASE;
            common::uint32_t ASYNCLISTADDR;
            common::uint8_t Reserved[0x40-0x1c];
            common::uint32_t CONFIGFLAG;
            common::uint32_t PORTSC[16];
        }__attribute__((packed));

        struct QueueHead
        {
            common::uint32_t DWORD1;
            common::uint32_t DWORD2;
            common::uint32_t DWORD3;
            common::uint32_t DWORD4;
            common::uint32_t DWORD5;
            common::uint32_t DWORD6;
            common::uint32_t DWORD7;
            common::uint32_t DWORD8;
            common::uint32_t DWORD9;
            common::uint32_t DWORD10;
            common::uint32_t DWORD11;
            common::uint32_t DWORD12;
            common::uint32_t DWORD13;
            common::uint32_t DWORD14;
            common::uint32_t DWORD15;
            common::uint32_t DWORD16;
            common::uint32_t DWORD17;
        }__attribute__((packed));

        struct USBDeviceDescriptorRequest
        {
            common::uint8_t bmRequestType;
            common::uint8_t bRequest;
            common::uint16_t wValue;
            common::uint16_t wIndex;
            common::uint16_t wLength;
        }__attribute__((packed));

        struct USBDeviceDescriptor
        {
            common::uint8_t Length;
            common::uint8_t descriptorType;
            common::uint16_t bcdUSB;
            common::uint8_t deviceClass;
            common::uint8_t deviceSubClass;
            common::uint8_t deviceProtocol;
            common::uint8_t maxPacketSize;
            common::uint16_t vendorID;
            common::uint16_t productID;
            common::uint16_t bcdDevice;
            common::uint8_t iManufacturer;
            common::uint8_t iProduct;
            common::uint8_t iSerialNumber;
            common::uint8_t numConfigurations;
        }__attribute__((packed));

        struct USBConfigDescriptor
        {
            common::uint8_t Length;
            common::uint8_t descriptorType;
            common::uint16_t totalLength;
            common::uint8_t numInterfaces;
            common::uint8_t configurationValue;
            common::uint8_t iConfiguration;
            common::uint8_t bmAttributes;
            common::uint8_t maxPower;
        }__attribute__((packed));

        struct USBInterfaceDescriptor
        {
            common::uint8_t Length;
            common::uint8_t descriptorType;
            common::uint8_t interfaceNumber;
            common::uint8_t alternateSetting;
            common::uint8_t numEndpoints;
            common::uint8_t Class;
            common::uint8_t SubClass;
            common::uint8_t Protocol;
            common::uint8_t iInterface;
        }__attribute__((packed));

        struct USBEndpointDescriptor
        {
            common::uint8_t Length;
            common::uint8_t descriptorType;
            common::uint8_t endpointAddress;
            common::uint8_t bmAttributes;
            common::uint16_t maxPacketSize;
            common::uint8_t bInterval;
        }__attribute__((packed));

        class EHCIDriver : public Driver, IO::InterruptHandler
        {
            private:
                common::uint32_t baseAddress;
                EHCICapRegs* capabilities;
                int numQueueHeads;  //Number of queue heads in the async schedule
                int ActivePort;

            public:
                int numPorts;
                bool portCanChange;
                bool USBInterrupt;
                static EHCIDriver* OSEHCIDriver;
                EHCIOperationalRegs* opregs;
                QueueHead* defaultControlQueueHead;

                common::uint8_t SendCommand(USBDeviceDescriptorRequest* request, QueueHead* qh, common::uint8_t* buffer, int numStages);
                
				void Activate();
                int Reset();

                void EnableAsyncSchedule();
                void DisableAsyncSchedule();
                void CheckPort(int portno);
                QueueHead* AddAsyncScheduleQueueHead(common::uint16_t deviceAddress, common::uint16_t endpointNumber, common::uint16_t maxPacketSize, bool);

                bool DevicePresent(int);
                static void HandlePortChange();
                common::uint32_t ResetPort(int portno);
                common::uint32_t HandleInterrupt(common::uint32_t);
                
                EHCIDriver(IO::PeripheralComponentInterconnectController*, IO::PCIDeviceDescriptor*, IO::InterruptManager*);
                ~EHCIDriver();
        };
    }
}

#endif