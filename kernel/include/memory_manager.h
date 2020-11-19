#ifndef CRYSTALOS__MEMORYMANAGER_H
#define CRYSTALOS__MEMORYMANAGER_H

#include <common/types.h>

//Pre kernel: 0x100000
//Stack: 0x100000 - 0x200000
//Page tables - 0x200000 - 0x600000
//AHCI: 0x600000 - 0x700000
//Kernel: 0x100000 - 0x200000
//EHCI: 0x800000 - 0x900000
//Free: 0x900000 - 0xA00000
//kernel screen buffer: 0x900000 - 0x901000
//Kernel memory manager: 0xA00000 - 

namespace crystalos
{
	struct MemoryMapEntry
	{
		common::uint64_t baseAddress;
		common::uint64_t length;
		common::uint32_t type;
		common::uint32_t acpi_extended;
	}__attribute__((packed));
	
	struct MemoryChunk
	{
		MemoryChunk* prev;
		MemoryChunk* next;
		bool allocated;
		common::size_t size;
	}__attribute__((packed));
	
	class MemoryManager
	{
		private:
			MemoryChunk* first;
			
		public:
			void* malloc(common::size_t size);
			void free(void *ptr);

			void memcpy(common::uint8_t* source, common::uint8_t* destination, int length);
			void memset(common::uint8_t* address, common::uint8_t value, int size);
			static MemoryManager* ActiveMemoryManager;
			
			MemoryManager(common::uint32_t start, common::size_t size);
			~MemoryManager();
	};
}

void* operator new(unsigned long size);
void* operator new[](unsigned long size);

void* operator new(unsigned long size, void *ptr);
void* operator new[](unsigned long size, void *ptr);

void operator delete(void *ptr);
void operator delete[](void *ptr);
	

#endif