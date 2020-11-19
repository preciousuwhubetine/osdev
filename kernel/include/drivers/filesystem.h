#ifndef CRYSTALOS__DRIVER__FILESYSTEM_H
#define CRYSTALOS__DRIVER__FILESYSTEM_H

#include <common/types.h>
#include <drivers/ide.h>
#include <drivers/ahci.h>
#include <drivers/usb_mass_storages.h>

/* 
    This is the filesystem class used to support reading and writing of files to disk.
    For now, only FAT32 is implemented.
    features:
        no support for long file names rn.
*/

namespace crystalos
{
    namespace drivers
    {
        struct DiskPartition
        {
            common::uint8_t rsvd0;
            common::uint8_t rsvd1;
            common::uint8_t rsvd2;
            common::uint8_t rsvd3;
            common::uint8_t FSType;
            common::uint8_t rsvd4;
            common::uint8_t rsvd5;
            common::uint8_t rsvd6;
            common::uint32_t StartSector;
            common::uint32_t NumSectors;
        }__attribute__((packed));
		
        struct OSBootSector //This is the expected format of our OS boot sector.
        {
            common::uint8_t BootCode[446];
            DiskPartition partition1;
            DiskPartition partition2;
            DiskPartition partition3;
            DiskPartition partition4;
        }__attribute__((packed));

        struct DirectoryEntry   //For FAT32
        {
            char FileName[8];
            common::uint8_t Extension[3];
            common::uint8_t Attributes;
            common::uint8_t Reserved0;
			common::uint8_t DirCreateTimeTenth;
			common::uint16_t CreationTime;
			common::uint16_t CreationDate;
			common::uint16_t LastAccessDate;
			common::uint16_t StartClusterHi;
            common::uint16_t LastUpdateTime;
            common::uint16_t LastUpdateDate;
			common::uint16_t StartClusterLo;
            common::uint32_t FileSize;
        }__attribute__((packed));

        struct FAT32BootSector
        {
            common::uint8_t jmp[3];
            common::uint8_t OEMName[8];
            common::uint16_t BytesPerSector;
            common::uint8_t SectorsPerCluster;
            common::uint16_t RsvdSectorsCount;
            common::uint8_t NumFATs;
            common::uint16_t RootEntryCount;
            common::uint16_t TotSec16;
            common::uint8_t Media;
            common::uint16_t FATSz16;
            common::uint16_t SectorsPerTrack;
            common::uint16_t NumHeads;
            common::uint32_t HiddenSectors;
            common::uint32_t TotSec32;
            common::uint32_t FATSz32;
            common::uint16_t ExtFlags;
            common::uint16_t FSVer;
            common::uint32_t RootCluster;
            common::uint16_t FSInfoSector;
            common::uint16_t BackupBootSector;
            common::uint32_t Reserved[3];
            common::uint8_t DrvNum;
            common::uint8_t Reserved1;
            common::uint8_t BootSig;
            common::uint32_t VolID;
            common::uint8_t VolLabel[11];
            common::uint8_t FSType[8];
        }__attribute__((packed));

        class File
        {
            public:
				common::uint16_t start_cluster_lo;
				common::uint16_t start_cluster_hi;
                char Name[12];
                common::uint32_t Size;
                common::uint8_t attrib;
                File();
                ~File();
        };

        class FileSystem   //FAT32
        {
            private:
                int FirstDataSector;
                common::uint32_t* FAT;
                common::uint8_t* ReadBuffer;
                common::uint32_t RootDirectoryFirstCluster;
                FAT32BootSector* boot_sector;
                IDEDevice* ide_disk;
                SATADISK* sata_disk;
                USBMassStorage* usb_disk;
				common::uint32_t FirstFATSector;

                common::uint32_t GetFATValue(common::uint32_t clusterNumber);
                void ReadCluster(int clusterNumber, common::uint8_t* buffer);

            public:
                FileSystem(SATADISK* disk, OSBootSector* bs);
                FileSystem(IDEDevice* disk, OSBootSector* bs);
                FileSystem(USBMassStorage* disk, OSBootSector* bs);

                static FileSystem* BootDiskFileSystem;

                File* GetFiles(char* directory, int* file_count);

                void ReadFile(File file, common::uint8_t* buffer);
                ~FileSystem();
        };
    }
}

#endif