
#include <util/images/bitmap.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;
using namespace crystalos::util::images;

void printHex32(uint32_t);
void printHex8(uint8_t);

Bitmap::Bitmap(uint8_t* buffer)
{
    ImageHeader* header = (ImageHeader*)buffer;
    Width = header->Width;
    Height = header->Height;
    Palette = (uint8_t*)(buffer + 40 + 14);
    Data = (uint8_t*)(buffer + header->DataOffset);
    BitsPerPixel = header->BitsPerPixel;
    ImageRGB = (uint8_t*)MemoryManager::ActiveMemoryManager->malloc(Width*Height*3);

    uint8_t red, green, blue;
    int xpos = 0, ypos = 0, x, y;

    if (BitsPerPixel == 4)
    {
        int max = Width >> 1;
        bytesPerScanline = max;

        if (Width % 2 != 0)
        {
            bytesPerScanline++;
            max++;
        }
        
        if ((bytesPerScanline % 4) != 0)
            bytesPerScanline = bytesPerScanline - (bytesPerScanline % 4) + 4;

        for (y = bytesPerScanline * (Height - 1); y >= 0; y -= bytesPerScanline)
        {
            for (int x = 0; x < bytesPerScanline ; x++)
            {
                uint8_t palette_index = Data[y+x] >> 4;
                ImageRGB[ypos + xpos] =  Palette[(palette_index << 2) + 2];
                ImageRGB[ypos + xpos + 1] =  Palette[(palette_index << 2) + 1];
                ImageRGB[ypos + xpos + 2] =  Palette[palette_index << 2];
                xpos += 3;

                if (xpos/3 == Width) break;

                palette_index = Data[y+x] & 0x0F;
                ImageRGB[ypos + xpos] =  Palette[(palette_index << 2) + 2];
                ImageRGB[ypos + xpos + 1] =  Palette[(palette_index << 2) + 1];
                ImageRGB[ypos + xpos + 2] =  Palette[palette_index << 2];
                xpos += 3;
            }
            xpos = 0;
            ypos += Width * 3;
        }
    } 
    else if (BitsPerPixel == 24)
    {
        bytesPerScanline = Width * 3;
        if ((bytesPerScanline % 4) != 0) 
            bytesPerScanline = bytesPerScanline - (bytesPerScanline % 4) + 4;
        for (y = bytesPerScanline * (Height - 1); y >= 0; y -= bytesPerScanline)
        {
            for (x = 0; x < Width * 3; x+= 3)
            {
                ImageRGB[ypos + xpos] =  Data[y+x+2];
                ImageRGB[ypos + xpos + 1] =  Data[y+x+1];
                ImageRGB[ypos + xpos + 2] =  Data[y+x];
                xpos += 3;
            }
            xpos = 0;
            ypos += Width * 3;
        }
    }

    
    MemoryManager::ActiveMemoryManager->free(buffer);
}

Bitmap::~Bitmap()
{

}
