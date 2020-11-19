#ifndef CRYSTALOS__UTIL__NUMBER_H
#define CRYSTALOS__UTIL__NUMBER_H

#include <common/types.h>
#include <IO/interrupts.h>

namespace crystalos
{
    namespace util
    {
        void itoa(int number, common::uint16_t base, char* out);
        int atoi(char* string);
        int len(char* string);
        common::uint32_t bigE(common::uint32_t value);
    }
}

#endif