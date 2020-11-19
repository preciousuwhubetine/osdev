#ifndef CRYSTALOS__DRIVERS__AHCI_H
#define CRYSTALOS__DRIVERS__AHCI_H

/*
    This is the SATA disk driver
    
    Features:
        Supports SATA hard disks connected via AHCI ports.
        Read Hard disks (max 64 sectors at a time).
        No support for ATAPI drives yet.
*/

#include <IO/pci.h>
#include <IO/interrupts.h>
#include <drivers/driver.h>

#define AHCI_BASE 0x600000 //6M

#define SATA_SIG_ATA 0x00000101 // SATA drive 
#define SATA_SIG_ATAPI 0xEB140101 // SATAPI drive 
#define SATA_SIG_SEMB 0xC33C0101 // Enclosure management bridge 
#define SATA_SIG_PM 0x96690101 // Port multiplier   

//Disk types
#define AHCI_DEV_NULL 0 
#define AHCI_DEV_SATA 1 
#define AHCI_DEV_SEMB 2 
#define AHCI_DEV_PM 3 
#define AHCI_DEV_SATAPI 4

//Port commands
#define HBA_PxCMD_ST 0x0001
#define HBA_PxCMD_FRE 0x0010
#define HBA_PxCMD_FR 0x4000
#define HBA_PxCMD_CR 0x8000

#define HBA_PxCMD_SUD 0x0002
#define HBA_PxCMD_POD 0x0004
#define HBA_PxCMD_ICC 0x10000000

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3 

//Identity Info
#define ATA_IDENT_MODEL        54
#define ATA_IDENT_CAPABILITIES 98
#define ATA_IDENT_MAX_LBA      120
#define ATA_IDENT_COMMANDSETS  164
#define ATA_CMD_READ_DMA_EXT      0x25
#define ATA_IDENT_MAX_LBA_EXT  200

//COmmands
#define ATA_CMD_READ_PIO          0x20
#define ATA_CMD_READ_PIO_EXT      0x24
#define ATA_CMD_IDENTIFY          0xEC

namespace crystalos
{
    namespace drivers
    {
        
        typedef enum
        {
            FIS_TYPE_REG_H2D = 0x27, // Register FIS - host to device
            FIS_TYPE_REG_D2H = 0x34, // Register FIS - device to host
            FIS_TYPE_DMA_ACT = 0x39, // DMA activate FIS - device to host
            FIS_TYPE_DMA_SETUP = 0x41, // DMA setup FIS - bidirectional  
            FIS_TYPE_DATA  = 0x46, // Data FIS - bidirectional  
            FIS_TYPE_BIST  = 0x58, // BIST activate FIS - bidirectional  
            FIS_TYPE_PIO_SETUP = 0x5F, // PIO setup FIS - device to host  
            FIS_TYPE_DEV_BITS = 0xA1, // Set device bits FIS - device to host 
        } FIS_TYPE;

        //Declare some data structures required to communicate with AHCI.
        struct FIS_REG_H2D
        {  
            // DWORD 0  
            common::uint8_t  fis_type; // FIS_TYPE_REG_H2D    
            common::uint8_t  pmport:4; // Port multiplier  
            common::uint8_t  rsv0:3;  // Reserved  
            common::uint8_t  c:1;  // 1: Command, 0: Control    
            common::uint8_t  command; // Command register  
            common::uint8_t  featurel; // Feature register, 7:0    
            
            // DWORD 1 
            common::uint8_t  lba0;  // LBA low register, 7:0  
            common::uint8_t  lba1;  // LBA mid register, 15:8  
            common::uint8_t  lba2;  // LBA high register, 23:16  
            common::uint8_t  device;  // Device register   
            
            // DWORD 2  
            common::uint8_t  lba3;  // LBA register, 31:24  
            common::uint8_t  lba4;  // LBA register, 39:32  
            common::uint8_t  lba5;  // LBA register, 47:40  
            common::uint8_t  featureh; // Feature register, 15:8    
            
            // DWORD 3  
            common::uint8_t  countl;  // Count register, 7:q0  
            common::uint8_t  counth;  // Count register, 15:8  
            common::uint8_t  icc;  // Isochronous command completion  
            common::uint8_t  control; // Control register

            // DWORD 4  
            common::uint8_t  rsv1[4]; // Reserved 
        }__attribute__((packed)); 

        struct FIS_REG_D2H {  
            // DWORD 0 
            common::uint8_t  fis_type;    // FIS_TYPE_REG_D2H    
            common::uint8_t  pmport:4;    // Port multiplier  
            common::uint8_t  rsv0:2;      // Reserved  
            common::uint8_t  i:1;         // Interrupt bit  
            common::uint8_t  rsv1:1;      // Reserved   
            common::uint8_t  status;      // Status register  
            common::uint8_t  error;       // Error register    
            
            // DWORD 1  
            common::uint8_t  lba0;        // LBA low register, 7:0  
            common::uint8_t  lba1;        // LBA mid register, 15:8  
            common::uint8_t  lba2;        // LBA high register, 23:16  
            common::uint8_t  device;      // Device register    
            
            // DWORD 2  
            common::uint8_t  lba3;        // LBA register, 31:24  
            common::uint8_t  lba4;        // LBA register, 39:32  
            common::uint8_t  lba5;        // LBA register, 47:40  
            common::uint8_t  rsv2;        // Reserved    
            
            // DWORD 3  
            common::uint8_t  countl;      // Count register, 7:0  
            common::uint8_t  counth;      // Count register, 15:8  
            common::uint8_t  rsv3[2];     // Reserved    
            
            // DWORD 4  
            common::uint8_t  rsv4[4];     // Reserved 
        }__attribute__((packed));

        struct FIS_DATA
        {  
            // DWORD 0  
            common::uint8_t  fis_type; // FIS_TYPE_DATA   
            common::uint8_t  pmport:4; // Port multiplier  
            common::uint8_t  rsv0:4;  // Reserved    
            common::uint8_t  rsv1[2]; // Reserved    
            
            // DWORD 1 ~ N  
            common::uint32_t data[30]; // Payload
        }__attribute__((packed)); 

        struct FIS_PIO_SETUP 
        {
            // DWORD 0  
            common::uint8_t  fis_type; // FIS_TYPE_PIO_SETUP    
            common::uint8_t  pmport:4; // Port multiplier  
            common::uint8_t  rsv0:1;  // Reserved  
            common::uint8_t  d:1;  // Data transfer direction, 1 - device to host  
            common::uint8_t  i:1;  // Interrupt bit 
            common::uint8_t  rsv1:1; 

            common::uint8_t  status;  // Status register  
            common::uint8_t  error;  // Error register 
            
            // DWORD 1  
            common::uint8_t  lba0;  // LBA low register, 7:0  
            common::uint8_t  lba1;  // LBA mid register, 15:8  
            common::uint8_t  lba2;  // LBA high register, 23:16  
            common::uint8_t  device;  // Device register    
            
            // DWORD 2 
            common::uint8_t  lba3;  // LBA register, 31:24  
            common::uint8_t  lba4;  // LBA register, 39:32  
            common::uint8_t  lba5;  // LBA register, 47:40  
            common::uint8_t  rsv2;  // Reserved    
            
            // DWORD 3  
            common::uint8_t  countl;  // Count register, 7:0  
            common::uint8_t  counth;  // Count register, 15:8  
            common::uint8_t  rsv3;  // Reserved  
            common::uint8_t  e_status; // New value of status register    
            
            // DWORD 4  
            common::uint16_t tc;  // Transfer count  
            common::uint8_t  rsv4[2]; // Reserved 
        }__attribute__((packed));

        struct FIS_DMA_SETUP 
        {
            // DWORD 0  
            common::uint8_t  fis_type; // FIS_TYPE_DMA_SETUP    
            common::uint8_t  pmport:4; // Port multiplier  
            common::uint8_t  rsv0:1;  // Reserved  
            common::uint8_t  d:1;  // Data transfer direction, 1 - device to host  
            common::uint8_t  i:1;  // Interrupt bit  
            common::uint8_t  a:1;  // Auto-activate. Specifies if DMA Activate FIS is needed           
            
            common::uint8_t  rsved[2];  // Reserved    
            
            //DWORD 1&2           
            common::uint64_t DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory          
            
            //DWORD 3         
            common::uint32_t rsvd;           //More reserved           
            
            //DWORD 4         
            common::uint32_t DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0           
            
            //DWORD 5         
            common::uint32_t TransferCount;  //Number of bytes to transfer. Bit 0 must be 0           
            
            //DWORD 6         
            common::uint32_t resvd;          //Reserved   
        }__attribute__((packed));

         struct HBA_FIS
        {
            // 0x00  
            FIS_DMA_SETUP dsfis;  // DMA Setup FIS  
            common::uint8_t pad0[4];    
            // 0x20  
            FIS_PIO_SETUP psfis;  // PIO Setup FIS  
            common::uint8_t pad1[12];    
            // 0x40
            FIS_REG_D2H rfis;  // Register – Device to Host FIS
            common::uint8_t pad2[4];    
            // 0x58
            common::uint8_t pad3[0x60-0x58];
            // FIS_DEV_BITS sdbfis;  // Set Device Bit FIS
            // 0x60  
            common::uint8_t ufis[64];
            // 0xA0
            common::uint8_t rsv[0x100-0xA0];
        }__attribute__((packed)); 

        struct HBA_CMD_HEADER 
        {
            // DW0
            common::uint8_t  cfl:5;  // Command FIS length in DWORDS, 2 ~ 16
            common::uint8_t  a:1;  // ATAPI  
            common::uint8_t  w:1;  // Write, 1: H2D, 0: D2H  
            common::uint8_t  p:1;  // Prefetchable
            common::uint8_t  r:1;  // Reset  
            common::uint8_t  b:1;  // BIST  
            common::uint8_t  c:1;  // Clear busy upon R_OK  
            common::uint8_t  rsv0:1;  // Reserved  
            common::uint8_t  pmp:4;  // Port multiplier port    
            common::uint16_t prdtl;  // Physical region descriptor table length in entries    
            
            // DW1  
            common::uint32_t prdbc;  // Physical region descriptor byte count transferred    
            
            // DW2, 3  
            common::uint32_t ctba;  // Command table descriptor base address  
            common::uint32_t ctbau;  // Command table descriptor base address upper 32 bits    
            
            // DW4 - 7  
            common::uint32_t rsv1[4]; // Reserved
        }__attribute__((packed));
        
         struct HBA_PORT {
            common::uint32_t clb;  // 0x00, command list base address, 1K-byte aligned  
            common::uint32_t clbu;  // 0x04, command list base address upper 32 bits  
            common::uint32_t fb;  // 0x08, FIS base address, 256-byte aligned  
            common::uint32_t fbu;  // 0x0C, FIS base address upper 32 bits  
            common::uint32_t is;  // 0x10, interrupt status  
            common::uint32_t ie;  // 0x14, interrupt enable  
            common::uint32_t cmd;  // 0x18, command and status  
            common::uint32_t rsv0;  // 0x1C, Reserved  
            common::uint32_t tfd;  // 0x20, task file data  
            common::uint32_t sig;  // 0x24, signature  
            common::uint32_t ssts;  // 0x28, SATA status (SCR0:SStatus)  
            common::uint32_t sctl;  // 0x2C, SATA control (SCR2:SControl)  
            common::uint32_t serr;  // 0x30, SATA error (SCR1:SError)  
            common::uint32_t sact;  // 0x34, SATA active (SCR3:SActive)  
            common::uint32_t ci;  // 0x38, command issuxe  
            common::uint32_t sntf;  // 0x3C, SATA notification (SCR4:SNotification)  
            common::uint32_t fbs;  // 0x40, FIS-based switch control  
            common::uint32_t devslp; //0x44 - 0x47
            common::uint32_t rsv1[10]; // 0x48 ~ 0x6F, Reserved  
            common::uint32_t vendor[4]; // 0x70 ~ 0x7F, vendor specific 
        }__attribute__((packed));

        struct HBA_MEM
        {
            // 0x00 - 0x2B, Generic Host Control  
            common::uint32_t cap;  // 0x00, Host capability  
            common::uint32_t ghc;  // 0x04, Global host control  
            common::uint32_t is;  // 0x08, Interrupt status  
            common::uint32_t pi;  // 0x0C, Port implemented  
            common::uint32_t vs;  // 0x10, Version  
            common::uint32_t ccc_ctl; // 0x14, Command completion coalescing control  
            common::uint32_t ccc_pts; // 0x18, Command completion coalescing ports  
            common::uint32_t em_loc;  // 0x1C, Enclosure management location  
            common::uint32_t em_ctl;  // 0x20, Enclosure management control  
            common::uint32_t cap2;  // 0x24, Host capabilities extended  
            common::uint32_t bohc;  // 0x28, BIOS/OS handoff control and status    
            
            // 0x2C - 0x9F, Reserved  
            common::uint8_t  rsv[0xA0-0x2C];    
            
            // 0xA0 - 0xFF, Vendor specific registers  
            common::uint8_t  vendor[0x100-0xA0];    
            
            // 0x100 - 0x10FF, Port control registers  
            HBA_PORT ports[32]; // 1 ~ 32 
        }__attribute__((packed));
        
        struct HBA_PRDT_ENTRY 
        {
            common::uint32_t dba;  // Data base address  
            common::uint32_t dbau;  // Data base address upper 32 bits  
            common::uint32_t rsv0;  // Reserved    
            
            common::uint32_t dbc:22;  // Byte count, 4M max  
            common::uint32_t rsv1:9;  // Reserved 
            common::uint32_t i:1;  // Interrupt on completion 
        }__attribute__((packed));

        struct HBA_CMD_TBL
        {
            // 0x00  
            common::uint8_t  cfis[64]; // Command FIS    
            // 0x40  
            common::uint8_t  acmd[16]; // ATAPI command, 12 or 16 bytes    
            // 0x50  
            common::uint8_t  rsv[48]; // Reserved    
            // 0x80  
            HBA_PRDT_ENTRY prdt_entry[1]; // Physical region descriptor table entries, 0 ~ 65535
        }__attribute__((packed)); 

        class SATADISK; //This class represents any hard disk connected via an AHCI port.
        class AHCIDriver : public drivers::Driver, IO::InterruptHandler
        {
            friend class SATADISK;
            private:
                common::uint32_t base_memory;
                common::uint32_t Drives[32];
                HBA_MEM* hba_memory;
                int FindCommandSlot(HBA_PORT* port);

            public:
                int numDisks;
                SATADISK* disks[256];
                static AHCIDriver* KernelAHCI;
                void Initialize();
                void Activate();
                AHCIDriver(IO::PeripheralComponentInterconnectController*, IO::InterruptManager* interrupts, IO::PCIDeviceDescriptor* dev);
                ~AHCIDriver();

                 int GetType(HBA_PORT *port);
                void RebasePort(HBA_PORT* port, int portno);
                void StartCmd(HBA_PORT* port);
                void StopCmd(HBA_PORT* port);
                common::uint32_t HandleInterrupt(common::uint32_t esp);
        };

        class SATADISK
        {
            private:
            public:
                int portnum;
                char ManufacturerModel[41];
                common::uint32_t numSectors;
                common::uint32_t SizeInGB;
                bool isDiskATA;
                SATADISK(char* model, int numSectors, bool isDiskATA);
                ~SATADISK();
                void Read(common::uint32_t sector, int numSectors, common::uint8_t* buffer);
        };
    }
}

#endif