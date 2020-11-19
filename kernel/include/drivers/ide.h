#ifndef CRYSTALOS__DRIVERS__IDE_H
#define CRYSTALOS__DRIVERS__IDE_H

/*
	This is the OS IDE driver.
	For reading sectors from and writing sectors to IDE hard disks.
	ATAPI drives are not yet supported.
*/

//Drive status
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DF      0x20    // Drive write fault
#define ATA_SR_DSC     0x10    // Drive seek complete
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_CORR    0x04    // Corrected data
#define ATA_SR_IDX     0x02    // Index
#define ATA_SR_ERR     0x01    // Error

//Drive error
#define ATA_ER_BBK      0x80    // Bad block
#define ATA_ER_UNC      0x40    // Uncorrectable data
#define ATA_ER_MC       0x20    // Media changed
#define ATA_ER_IDNF     0x10    // ID mark not found
#define ATA_ER_MCR      0x08    // Media change request
#define ATA_ER_ABRT     0x04    // Command aborted
#define ATA_ER_TK0NF    0x02    // Track 0 not found
#define ATA_ER_AMNF     0x01    // No address mark

//Device Identity offsets
#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    2
#define ATA_IDENT_HEADS        6
#define ATA_IDENT_SECTORS      12
#define ATA_IDENT_SERIAL       20
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_FIELDVALID   106
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_IDENT_MAX_LBA_EXT  200

//Commands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_READ_DMA          0xC8
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_CMD_WRITE_PIO         0x30
#define ATA_CMD_WRITE_PIO_EXT     0x34
#define ATA_CMD_WRITE_DMA         0xCA
#define ATA_CMD_WRITE_DMA_EXT     0x35
#define ATA_CMD_CACHE_FLUSH       0xE7
#define ATA_CMD_CACHE_FLUSH_EXT   0xEA
#define ATA_CMD_PACKET            0xA0
#define ATA_CMD_IDENTIFY_PACKET   0xA1
#define ATA_CMD_IDENTIFY          0xEC

#define ATAPI_CMD_READ 0xA8
#define ATAPI_CMD_EJECT 0x1B

#include <common/types.h>
#include <drivers/driver.h>
#include <IO/port.h>
#include <IO/interrupts.h>
#include <memory_manager.h>

namespace crystalos
{
	namespace drivers
	{		
		class IDEChannel;

		enum DriveMode
		{
			Bit28 = 0, Bit48 = 1
		};
		
		class IDEDevice	//Represents an IDE drive (ATA or ATAPI)
		{
			friend class IDEChannel;
			private:
				IDEChannel* Parent;
				bool Master;
			
			protected:
				IO::Port16Bit dataPort;
				IO::Port16Bit errorPort;
				IO::Port8Bit sectorCountPort;
				IO::Port8Bit lbaLowPort;
				IO::Port8Bit lbaMidPort;
				IO::Port8Bit lbaHiPort;
				IO::Port8Bit devicePort;
				IO::Port8Bit commandPort;
				IO::Port8Bit controlPort;

				bool Empty;
			
			public:
				IDEDevice(common::uint16_t portBase, bool master);
				~IDEDevice();

				common::uint32_t numSectors;
				DriveMode Mode;
				char ManufacturerModel[41];
				common::uint16_t SectorSize;
				common::uint32_t SizeInGB;

				void Identify();
				bool IsEmpty();
				void Read(common::uint32_t sector, int numSectors, common::uint8_t* buffer);
				void Write(common::uint32_t sector, common::uint8_t* data);

				void Write28(common::uint32_t sector, common::uint8_t* data);
				void Write48(common::uint32_t sector, common::uint8_t* data);

				void Read28(common::uint32_t sector, common::uint16_t numSectors, common::uint8_t* buffer);
				void Read48(common::uint32_t sector, common::uint16_t numSectors, common::uint8_t* buffer);

				void Flush();
			};

		class IDEChannel : public IO::InterruptHandler	//Represents an IDE Channel (Primary and Secondary)
		{
			public:
				IDEChannel(common::uint16_t portBase, IO::InterruptManager* interrupts, common::uint8_t interruptNumber);
				~IDEChannel();

				IDEDevice masterDrive;
				IDEDevice slaveDrive;
				bool IRQ_Invoked;

				common::uint32_t HandleInterrupt(common::uint32_t esp);
		};

		class IDEDriver : public Driver	//The main IDE Driver class
		{
			public:
				IDEChannel primaryChannel;
				IDEChannel secondaryChannel;
			public:
				static IDEDriver* KernelIDEDriver;
				IDEDriver(common::uint16_t primaryChannelPortBase, common::uint16_t secondaryChannelPortBase, 
						IO::InterruptManager* interrupts,  common::uint8_t primaryChannelInterrupt, common::uint8_t secondaryChannelInterrupt);
				~IDEDriver();

				void Activate();
				void Initialize();
		};
	}
}

#endif
