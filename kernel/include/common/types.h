#ifndef CRYSTALOS__COMMON__TYPES_H
#define CRYSTALOS__COMMON__TYPES_H

//This contains all the types declaration needed by this OS.

namespace crystalos
{
    namespace common
    {
        typedef unsigned char uint8_t;
        typedef char int8_t;

        typedef unsigned short uint16_t;
        typedef short int16_t;

        typedef unsigned int uint32_t;
        typedef int int32_t;

        typedef unsigned long long uint64_t;
        typedef long long int64_t;
        
		typedef uint32_t size_t;
    }
}

#endif