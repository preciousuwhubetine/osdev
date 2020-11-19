#ifndef CRYSTALOS__UTIL__SYSTEM_H
#define CRYSTALOS__UTIL__SYSTEM_H

#include <common/types.h>
#include <IO/interrupts.h>

namespace crystalos
{
    namespace util
    {
        struct BootInformation
        {
            common::uint8_t vbeInfo[512];
            common::uint16_t vbeModes[128];
            common::uint16_t currentMode;
            common::uint8_t currentModeInfo[256];
            common::uint32_t no_of_memory_entries;
            common::uint32_t memory_entries;
            common::uint32_t page_directory_address;
        }__attribute__((packed));

        void Sleep(int ms);
        bool Schedule(void (*handler)(), int numFrames);
    }
}

#endif