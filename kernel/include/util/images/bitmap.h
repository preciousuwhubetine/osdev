
#ifndef CRYSTALOS__UTIL__IMAGE__BITMAP_H
#define CRYSTALOS__UTIL__IMAGE__BITMAP_H

#include <common/types.h>
#include <memory_manager.h>

namespace crystalos
{
    namespace util
    {
        namespace images
        {
            struct ImageHeader
            {
                common::uint16_t Signature;
                common::uint32_t FileSize;
                common::uint32_t Reserved;
                common::uint32_t DataOffset;

                common::uint32_t Size;
                common::uint32_t Width;
                common::uint32_t Height;
                common::uint16_t Planes;
                common::uint16_t BitsPerPixel;
                common::uint32_t CompressionMethod;
                common::uint32_t ImageSize;
                common::uint32_t XPixelsPerM;
                common::uint32_t YPixelsPerM;
                common::uint32_t ColorsUsed;
                common::uint32_t ImportantColors;
                
            }__attribute__((packed));

            class Bitmap
            {
                protected:
                    common::uint8_t* Buffer;
                    common::uint8_t BitsPerPixel;
                    common::uint8_t* Palette;
                    common::uint8_t* Data;
                    int bytesPerScanline;

                public:
                    int Width;
                    int Height;
                    common::uint8_t* ImageRGB;
                    Bitmap(common::uint8_t* buffer);
                    ~Bitmap();
            };
        }
    }
}


#endif