#include <memory_manager.h>
#include <util/screen.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;

MemoryManager* MemoryManager::ActiveMemoryManager;

MemoryManager::MemoryManager(uint32_t start, size_t size)
{
	first = (MemoryChunk*)start;
	first->prev = 0;
	first->next = 0;
	first->allocated = false;
	first->size = size - sizeof(MemoryChunk);

	if(ActiveMemoryManager == 0)
		ActiveMemoryManager = this;
}

MemoryManager::~MemoryManager()
{
	if (ActiveMemoryManager == this)
		ActiveMemoryManager = 0;
}

void* MemoryManager::malloc(size_t size)
{
	if (size == 0) return 0;
	MemoryChunk* result = 0;
	for(MemoryChunk* chunk = first; chunk != 0 && result == 0; chunk = chunk->next)
	{
		if(!chunk->allocated && chunk->size > (sizeof(MemoryChunk) + size))
		{
			result = chunk;
		}
	}
	
	if (result == 0) 
	{
		return 0;
	}
	MemoryChunk* newChunk = (MemoryChunk*)((uint32_t)result+size+sizeof(MemoryChunk));
	
	newChunk->next = result->next;
	newChunk->prev = result;
	newChunk->next->prev = newChunk;
	newChunk->allocated = false;
	newChunk->size = result->size - (size + sizeof(MemoryChunk));
	
	result->next = newChunk;
	result->allocated = true;
	result->size = size;
	
	return (void*)(((size_t)result) + sizeof(MemoryChunk));
}

void MemoryManager::free(void *ptr)
{
	MemoryChunk* chunk = (MemoryChunk*)((uint32_t)ptr - sizeof(MemoryChunk));
	chunk->allocated = false;
	
	if(chunk->prev != 0 && !chunk->prev->allocated)
	{
		chunk->prev->next = chunk->next;
		chunk->prev->size += chunk->size + sizeof(MemoryChunk);
		
		if(chunk->next != 0)
			chunk->next->prev = chunk->prev;
		
		chunk = chunk->prev;
	}
	
	if(chunk->next != 0 && !chunk->next->allocated)
	{
		chunk->size += chunk->next->size + sizeof(MemoryChunk);
		chunk->next = chunk->next->next;
		if(chunk->next != 0)
			chunk->next->prev = chunk;
	}
	
}

void MemoryManager::memcpy(uint8_t* source, uint8_t* destination, int length)
{
	for (int i = 0; i < length; i++)
	{
		destination[i] = source[i];
	}
}

void MemoryManager::memset(uint8_t* address, uint8_t value, int size)
{
	for (int i = 0; i < size; i++) address[i] = value;
}

void* operator new(unsigned long size)
{
	if(crystalos::MemoryManager::ActiveMemoryManager == 0)
		return 0;
	return crystalos::MemoryManager::ActiveMemoryManager->malloc(size);
}

void* operator new[](unsigned long size)
{
	if(crystalos::MemoryManager::ActiveMemoryManager == 0)
		return 0;
	return crystalos::MemoryManager::ActiveMemoryManager->malloc(size);
}

void* operator new(unsigned long size, void *ptr)
{
	return ptr;
}

void* operator new[](unsigned long size, void *ptr)
{
	return ptr;
}

void operator delete(void *ptr)
{
	if(crystalos::MemoryManager::ActiveMemoryManager != 0)
		crystalos::MemoryManager::ActiveMemoryManager->free(ptr);
}

void operator delete[](void *ptr)
{
	if(crystalos::MemoryManager::ActiveMemoryManager != 0)
		crystalos::MemoryManager::ActiveMemoryManager->free(ptr);
}