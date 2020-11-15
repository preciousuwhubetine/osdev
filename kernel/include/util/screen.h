#ifndef CRYSTALOS__UTIL__SCREEN_H
#define CRYSTALOS__UTIL__SCREEN_H

#include <common/types.h>

namespace crystalos
{
    namespace util
    {
        void clearScreen();
        void print(char*);
        void printHex32(common::uint32_t);  //prints a 32 bit hexadecimal value in ascii format
        void printHex8(common::uint8_t);    //prints an 8 bit hexadecimal value in ascii format
    }
}

#endif