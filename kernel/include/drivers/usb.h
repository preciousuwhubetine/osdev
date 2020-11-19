#ifndef CRYSTALOS__DRIVERS__USB_H
#define CRYSTALOS__DRIVERS__USB_H

#include <common/types.h>
#include <drivers/driver.h>
#include <util/screen.h>
#include <util/system.h>
#include <drivers/ehci.h>

/* This is the USB Host Driver.
    All Companion Host controller drivers (EHCI, OHCI and UHCI) report here
        No support for xHCI
*/

namespace crystalos
{
    namespace drivers
    {
        struct EHCIPortChangeStatus
        {
            int portNumber;
            int deviceSpeed;
            EHCIDriver* controller;
        }__attribute__((packed));

        class USBDriver
        {
            private:
                common::uint32_t ddrAddress;
                common::uint32_t requestBuffer;
                common::uint16_t nextFreeAddress;
                bool deviceAddresses[128];
            public:
                static USBDriver* OSUSBDriver;
                USBDriver();
                ~USBDriver();

                void HandlePortChange(EHCIPortChangeStatus*);
                common::uint16_t getFreeAddress();
        };
    }
}


#endif