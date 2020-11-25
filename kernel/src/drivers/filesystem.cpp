#include <drivers/filesystem.h>
#include <graphics/desktop.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::drivers;
using namespace crystalos::util;
using namespace crystalos::graphics;

FileSystem* FileSystem::BootDiskFileSystem = 0;

FileSystem::FileSystem(IDEDevice* disk, OSBootSector* bs)
{
    ide_disk = disk;
    sata_disk = 0;
    usb_disk = 0;

    uint8_t* buffer = (uint8_t*)MemoryManager::ActiveMemoryManager->malloc(513);
    
    if (BootDiskFileSystem == 0)
    {
        ide_disk->Read(bs->partition2.StartSector, 1, buffer);
        boot_sector = (FAT32BootSector*)buffer;
        
        //Get starting sector of the first FAT
        FirstFATSector = boot_sector->HiddenSectors + boot_sector->RsvdSectorsCount;
        
        FAT = (uint32_t*)MemoryManager::ActiveMemoryManager->malloc(boot_sector->BytesPerSector);
        
        FirstDataSector = FirstFATSector + (boot_sector->NumFATs*boot_sector->FATSz32);
        
        RootDirectoryFirstCluster = boot_sector->RootCluster;
        BootDiskFileSystem = this;
    }
    else
    {
        ide_disk->Read(bs->partition2.StartSector, 1, buffer);
        boot_sector = (FAT32BootSector*)buffer;
        
        //Get starting sector of the first FAT
        FirstFATSector = boot_sector->HiddenSectors + boot_sector->RsvdSectorsCount;
        
        FAT = (uint32_t*)MemoryManager::ActiveMemoryManager->malloc(boot_sector->BytesPerSector);
        
        FirstDataSector = FirstFATSector + (boot_sector->NumFATs*boot_sector->FATSz32);
        
        RootDirectoryFirstCluster = boot_sector->RootCluster;
    } 
}

FileSystem::FileSystem(SATADISK* disk, OSBootSector* bs)
{
    ide_disk = 0;
    sata_disk = disk;
    usb_disk = 0;

    uint8_t* buffer = (uint8_t*)MemoryManager::ActiveMemoryManager->malloc(513);

    if (BootDiskFileSystem == 0)
    {
        sata_disk->Read(bs->partition2.StartSector, 1, buffer);
        boot_sector = (FAT32BootSector*)buffer;
        
        //Get starting sector of the first FAT
        FirstFATSector = boot_sector->HiddenSectors + boot_sector->RsvdSectorsCount;
        
        FAT = (uint32_t*)MemoryManager::ActiveMemoryManager->malloc(boot_sector->BytesPerSector);
        
        FirstDataSector = FirstFATSector + (boot_sector->NumFATs*boot_sector->FATSz32);
        
        RootDirectoryFirstCluster = boot_sector->RootCluster;
        BootDiskFileSystem = this;
    }
    else
    {
        sata_disk->Read(bs->partition2.StartSector, 1, buffer);
        boot_sector = (FAT32BootSector*)buffer;
        
        //Get starting sector of the first FAT
        FirstFATSector = boot_sector->HiddenSectors + boot_sector->RsvdSectorsCount;
        
        FAT = (uint32_t*)MemoryManager::ActiveMemoryManager->malloc(boot_sector->BytesPerSector);
        
        FirstDataSector = FirstFATSector + (boot_sector->NumFATs*boot_sector->FATSz32);
        
        RootDirectoryFirstCluster = boot_sector->RootCluster;
    }
}

FileSystem::FileSystem(USBMassStorage* disk, OSBootSector* bs)
{
    ide_disk = 0;
    sata_disk = 0;
    usb_disk = disk;

    uint8_t* buffer = (uint8_t*)MemoryManager::ActiveMemoryManager->malloc(513);

    if (BootDiskFileSystem == 0)
    {
        usb_disk->portStatus->controller->EnableAsyncSchedule();
        usb_disk->Read(bs->partition2.StartSector, 1, buffer);
        usb_disk->portStatus->controller->DisableAsyncSchedule();
        boot_sector = (FAT32BootSector*)buffer;

    
        
        //Get starting sector of the first FAT
        FirstFATSector = boot_sector->HiddenSectors + boot_sector->RsvdSectorsCount;
        
        FAT = (uint32_t*)MemoryManager::ActiveMemoryManager->malloc(boot_sector->BytesPerSector);
        
        FirstDataSector = FirstFATSector + (boot_sector->NumFATs*boot_sector->FATSz32);
        
        RootDirectoryFirstCluster = boot_sector->RootCluster;
        BootDiskFileSystem = this;
    }
    else
    {

    }
}

File* FileSystem::GetFiles(char* directory, int* filecount)
{
    //Irrespective of the path supplied in "directory", this returns all files in the root for now...
    int FileCount = 0;
    uint8_t* RootDirectory = (uint8_t*)MemoryManager::ActiveMemoryManager->malloc(boot_sector->BytesPerSector * boot_sector->SectorsPerCluster);
    int currentCluster = RootDirectoryFirstCluster;
    
    //.. TODO: Add specific folder reading support...
    while (true)
    {
		ReadCluster(currentCluster, RootDirectory);
        DirectoryEntry* entries = (DirectoryEntry*)RootDirectory;
		
        for (int i = 0; i < boot_sector->BytesPerSector * boot_sector->SectorsPerCluster/sizeof(DirectoryEntry); i++)
        {
            if (((entries[i].FileName[0]) == 0x00) || ((entries[i].FileName[0] & 0xFF) == 0xE5)) continue;
            //blank entry or file deleted

			if ((entries[i].Attributes & 0x20) == 0x20) FileCount++;
        }
        int nextFATValue = GetFATValue(currentCluster);
        if (nextFATValue >= 0x0FFFFFF7) break;
        currentCluster = nextFATValue;
    }

    File* files = (File*)MemoryManager::ActiveMemoryManager->malloc(FileCount * sizeof(File));
    currentCluster = RootDirectoryFirstCluster;
    int index = 0;
    
    while (true)
    {
		ReadCluster(currentCluster, RootDirectory);
        DirectoryEntry* entries = (DirectoryEntry*)RootDirectory;
		
        for (int i = 0; i < boot_sector->BytesPerSector * boot_sector->SectorsPerCluster/sizeof(DirectoryEntry); i++)
        {
            if (((entries[i].FileName[0]) == 0x00) || ((entries[i].FileName[0] & 0xFF) == 0xE5)) continue;
			if ((entries[i].Attributes & 0x20) == 0x20)
            {
                MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)&(entries[i].FileName), (uint8_t*)&(files[index].Name), 11);
				files[index].Name[11] = 0;
				files[index].Size = entries[i].FileSize;
				files[index].attrib = entries[i].Attributes;
				files[index].start_cluster_lo = entries[i].StartClusterLo;
				files[index].start_cluster_hi = entries[i].StartClusterHi;
				index++;
            }
        }
        int nextFATValue = GetFATValue(currentCluster);
        if (nextFATValue >= 0x0FFFFFF7) break;
        currentCluster = nextFATValue;
    }
    *filecount = FileCount;
    return files;
}


uint32_t FileSystem::GetFATValue(uint32_t clusterNumber)
{
    uint32_t FATSector = clusterNumber / 128;
    uint32_t offset = clusterNumber % 128;
    FATSector += FirstFATSector;
    if (ide_disk != 0) ide_disk->Read(FATSector, 1, (uint8_t*)FAT);
    else if (sata_disk != 0) sata_disk->Read(FATSector, 1, (uint8_t*)FAT);
    else if (usb_disk != 0)
    {
        usb_disk->portStatus->controller->EnableAsyncSchedule();
        usb_disk->Read(FATSector, 1, (uint8_t*)FAT);
        usb_disk->portStatus->controller->DisableAsyncSchedule();
    }
    return FAT[offset];
}

void FileSystem::ReadCluster(int clusterNumber, uint8_t* buffer)
{
    if (ide_disk != 0)
        ide_disk->Read(FirstDataSector + ((clusterNumber - 2) * boot_sector->SectorsPerCluster), boot_sector->SectorsPerCluster, buffer);
    else if (sata_disk != 0)
        sata_disk->Read(FirstDataSector + ((clusterNumber - 2) * boot_sector->SectorsPerCluster), boot_sector->SectorsPerCluster, buffer);
    else if (usb_disk != 0)
    {
        usb_disk->portStatus->controller->EnableAsyncSchedule();
        usb_disk->Read(FirstDataSector + ((clusterNumber - 2) * boot_sector->SectorsPerCluster), boot_sector->SectorsPerCluster, buffer);
        usb_disk->portStatus->controller->DisableAsyncSchedule();
    }
}

void FileSystem::ReadFile(File file, uint8_t* buffer)
{
    int currentCluster = file.start_cluster_lo | ((uint32_t)(file.start_cluster_hi) << 16);
    int index = 0;
    while(1)
    {
        ReadCluster(currentCluster, (uint8_t*)(buffer+index));
        int nextFATValue = GetFATValue(currentCluster);
        if (nextFATValue >= 0x0FFFFFF7) break;
        currentCluster = nextFATValue;
        index += boot_sector->BytesPerSector * boot_sector->SectorsPerCluster;
    }
}

FileSystem::~FileSystem()
{

}

File::File()
{

}

File::~File()
{

}