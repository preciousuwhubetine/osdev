#ifndef CRYSTALOS__GRAPHICS__VESA_H
#define CRYSTALOS__GRAPHICS__VESA_H

#include <common/types.h>

namespace crystalos
{
    namespace graphics
    {
        struct VESAModeInfo
        {
            common::uint16_t ModeAttributes;
            common::uint8_t WinAAttributes;
            common::uint8_t WinBAttributes;
            common::uint16_t WinGranularity;
            common::uint16_t WinSize;
            common::uint16_t WinASegment; 
            common::uint16_t WinBSegment;
            common::uint32_t WinFuncPtr;
            common::uint16_t BytesPerScanLine;
            // Mandatory information for VBE 1.2 and above
            common::uint16_t XResolution; // horizontal resolution in pixels or characters
            common::uint16_t YResolution; //vertical resolution in pixels or characters
            common::uint8_t XCharSize; //character cell width in pixels
            common::uint8_t YCharSize; //character cell height in pixels
            common::uint8_t NumberOfPlanes; //number of memory planes
            common::uint8_t BitsPerPixel;
            common::uint8_t NumberOfBanks;
            common::uint8_t MemoryModel;
            common::uint8_t BankSize;	//In KB
            common::uint8_t NumberOfImagePages;
            common::uint8_t Reserved;
            common::uint8_t RedMaskSize; // size of direct color red mask in bits
            common::uint8_t RedFieldPosition; //bit position of lsb of red mask
            common::uint8_t GreenMaskSize; // size of direct color green mask in bits
            common::uint8_t GreenFieldPosition; // bit position of lsb of green mask
            common::uint8_t BlueMaskSize; // size of direct color blue mask in bits
            common::uint8_t BlueFieldPosition; // bit position of lsb of blue mask
            common::uint8_t RsvdMaskSize; // size of direct color reserved mask in bits
            common::uint8_t RsvdFieldPosition; //bit position of lsb of reserved mask
            common::uint8_t DirectColorModeInfo; //direct color mode attributes
            //; Mandatory information for VBE 2.0 and above
            common::uint32_t PhysBasePtr; //physical address for flat memory frame buffer
            common::uint32_t OffScreenMemOffset; //pointer to start of off screen memory
            common::uint16_t OffScreenMemSize; //amount of off screen memory in 1k units
            common::uint8_t Reserved1[206]; //remainder of ModeInfoBlock
        }__attribute__((packed));


        struct VESAControllerInfo
        {
            common::uint8_t VESASignature[4];
            common::uint16_t VBEVersion;
            common::uint16_t OemStringPtrSeg;
            common::uint16_t OemStringPtrOff;
            common::uint32_t Capabilities;
            common::uint16_t VideoModePtrSeg;
            common::uint16_t VideoModePtrOff;
            common::uint16_t TotalMemory;	
            common::uint16_t OEMSoftwareRev;
            common::uint32_t OEMVendorNamePtr;	
            common::uint32_t OEMProductNamePtr;
            common::uint32_t OEMProductRevisionPtr;
            common::uint8_t Reserved[222];
            common::uint8_t OEMData[256];
        }__attribute__((packed));
    }
}

#endif