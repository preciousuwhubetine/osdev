#ifndef CRYSTALOS__IO__PCI_H
#define CRYSTALOS__IO__PCI_H

#include <common/types.h>
#include <IO/port.h>
#include <memory_manager.h>

namespace crystalos
{
	namespace IO
	{		
		struct PCIDeviceDescriptor
		{
			common::uint8_t interrupt_pin;
			common::uint8_t interrupt_line;
			
			common::uint16_t bus;
			common::uint16_t device;
			common::uint16_t function;
			
			common::uint16_t vendor_id;
			common::uint16_t device_id;
			
			common::uint16_t command;
			common::uint16_t status;
			
			common::uint8_t class_id;
			common::uint8_t subclass_id;
			common::uint8_t interface_id;
			
			common::uint8_t revision;
			common::uint8_t capabilities;

			common::uint32_t bar0;
			common::uint32_t bar1;
			common::uint32_t bar2;
			common::uint32_t bar3;
			common::uint32_t bar4;
			common::uint32_t bar5;
		}__attribute__((packed));

		class PeripheralComponentInterconnectController
		{
			Port32Bit dataPort;
			Port32Bit commandPort;
			
			public:
				PeripheralComponentInterconnectController();
				~PeripheralComponentInterconnectController();

				common::uint32_t Read(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint32_t registeroffset);
				void Write(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint32_t registeroffset, common::uint32_t value);
				bool DeviceHasFunctions(common::uint16_t bus, common::uint16_t device);

				void GetDescriptor(common::uint16_t, common::uint16_t, PCIDeviceDescriptor*);
				void GetDescriptor(common::uint16_t, common::uint16_t, common::uint16_t, PCIDeviceDescriptor*);
				PCIDeviceDescriptor GetDeviceDescriptor(common::uint16_t bus, common::uint16_t device, common::uint16_t function);
				
				common::uint32_t GetBaseAddressRegister(common::uint16_t bus, common::uint16_t device, common::uint16_t function, common::uint16_t bar);
		};
	}
}
#endif