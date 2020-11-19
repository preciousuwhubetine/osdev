#ifndef CRYSTALOS__UTIL__STRING_H
#define CRYSTALOS__UTIL__STRING_H

#include <common/types.h>
#include <IO/interrupts.h>
#include <util/number.h>

namespace crystalos
{
    namespace util
    {
        void split(char* string, char delimeter, char* out);
        char* getStringAtIndex(char* string, int index);
        char* insertChar(char c, char* str, int index);
        char* subString(char* str, int pos, int length);
    }
}

#endif