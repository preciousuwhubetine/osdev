#include <drivers/ahci.h>

using namespace crystalos;
using namespace crystalos::IO;
using namespace crystalos::drivers;
using namespace crystalos::common;

AHCIDriver* AHCIDriver::KernelAHCI = 0;
void printHex32(uint32_t);

AHCIDriver::AHCIDriver(PeripheralComponentInterconnectController* pci, InterruptManager* interrupts, PCIDeviceDescriptor* dev)
: Driver(),
 InterruptHandler(interrupts, dev->interrupt_line + 0x20)
{
	for (int i = 0; i < 32;i++) Drives[i] = AHCI_DEV_NULL;
    base_memory = dev->bar5;

    uint32_t command = pci->Read(dev->bus, dev->device, dev->function, 4);
    command |= (1 << 9) | (1 << 8) | (1 << 4) | (1 << 2) | (1 << 1);
    pci->Write(dev->bus, dev->device, dev->function, 4, command);    

    hba_memory = (HBA_MEM*)base_memory;

    uint32_t pi = hba_memory->pi;
    int i = 0;
    while (i<32)
    {
        if (pi & 1)
        {
            int dt = GetType(&hba_memory->ports[i]);
            if (dt == AHCI_DEV_SATA)
            {
                Drives[i] = AHCI_DEV_SATA;
            }else if (dt == AHCI_DEV_SATAPI)
            {
                Drives[i] = AHCI_DEV_SATAPI;
            }else if (dt == AHCI_DEV_SEMB)
            {
                Drives[i] = AHCI_DEV_SEMB;
            }else if (dt == AHCI_DEV_PM)
            {
                Drives[i] = AHCI_DEV_PM;
            }else
            {
                Drives[i] = AHCI_DEV_NULL;
            }
        }
        pi >>= 1;
        i ++;
    }

    if (KernelAHCI == 0) KernelAHCI = this;
    numDisks = 0;
    for (int i = 0; i < 256; i++) disks[i] = 0;
}

AHCIDriver::~AHCIDriver()
{
    if (KernelAHCI == this) KernelAHCI = 0;
}

uint32_t AHCIDriver::HandleInterrupt(uint32_t esp)
{
    for (int i = 0; i < 32; i++)
    {
        if((hba_memory->is >> i) & 0x1 == 0x1)
        {
            if (hba_memory->ports[i].is & 0x40)
            {
                for (int j = 0; j < 200000000; j++);
                hba_memory->ports[i].sctl = 0x700;
            }
            hba_memory->ports[i].is = hba_memory->ports[i].is;
            hba_memory->ports[i].serr = hba_memory->ports[i].serr;
        }
    }
    hba_memory->is = hba_memory->is;
    return esp;
}

void AHCIDriver::Activate()
{

}

int AHCIDriver::FindCommandSlot(HBA_PORT* port)
{
	uint32_t slots = port->sact | port->ci;
	for (int i = 0; i < 32 ; i++)
	{
		if ((slots&1) == 0) return i;
        slots >> 1;
	}
    return -1;
}

void AHCIDriver::Initialize()
{
    hba_memory->ghc |= 0x80000001;
    while (hba_memory->ghc & 0x1)
    {
        for (int i = 0; i < 1000000; i++);
    }

    hba_memory->ghc |= 0x80000000;
    hba_memory->ghc |= 0x2;
    
    for (int i = 0; i < 1000000; i++);
    if ((hba_memory->cap2 & 0x1) && (hba_memory->bohc & 0x2))
    {
        hba_memory->bohc = (hba_memory->bohc & ~0x8) | 0x2;
        // print("Waiting for BIOS to give up AHCI Ownership!...\n");
        
        for (int i = 0; i < 1000000; i++);

        if (hba_memory->bohc & 0x1)
        {
            // print("Forcibly taking ownership...\n");
            hba_memory->bohc = 0x2;
            hba_memory->bohc |= 0x8;
        }
        else
        {
            // print("AHCI Ownership obtained...\n");
        }
    }
    else
    {
        // print("AHCI is already in OS mode...\n");
    }

    int i;
	for (i = 0; i < 32; i++)
	{
		if (Drives[i] == AHCI_DEV_SATA)
		{
            // print("Found a SATA port on slot %d...\n", i);
            HBA_PORT* port = (HBA_PORT*)&(hba_memory->ports[i]);

            StopCmd(port);
            RebasePort(port, i);
            StartCmd(port);

            port->is = port->is;
            port->ie = 0xffffffff;
        }
    }

    hba_memory->is = hba_memory->is;

    for (i = 0; i < 32; i++)
	{
		if (Drives[i] == AHCI_DEV_SATA)
		{
            HBA_PORT* port = (HBA_PORT*)&(hba_memory->ports[i]);

            if (port->devslp & 0x2)
            {
                port->devslp &= 0xfffffffe;
            }


            if (hba_memory->cap & (1 << 27))
            {
                port->cmd |= HBA_PxCMD_SUD | HBA_PxCMD_POD | HBA_PxCMD_ICC;
            }
            for (int j = 0; j < 1000000; j++);

            port->sctl = 0x701;
            port->sctl = 0x700;
            port->serr = port->serr;
            for (int j = 0; j < 1000000; j++);
            
            HBA_FIS* fises = (HBA_FIS*)port->fb;

            while(port->tfd & (0x80 | 0x08))
            {
                for (int j = 0; j < 1000000; j++);
            }

            MemoryManager::ActiveMemoryManager->memset((uint8_t*)fises, 0, sizeof(HBA_FIS));

            int slot = FindCommandSlot(port);
            HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)(port->clb + (slot*sizeof(HBA_CMD_HEADER)));
            cmdheader->cfl = 5;
            cmdheader->w = 0;
            cmdheader->c = 0;
            cmdheader->prdtl = 1;

            HBA_CMD_TBL* cmdtable = (HBA_CMD_TBL*)cmdheader->ctba;
            MemoryManager::ActiveMemoryManager->memset((uint8_t*)cmdtable, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl - 1)* sizeof(HBA_PRDT_ENTRY));

            cmdtable->prdt_entry[0].dba = 0x2F0000;
            cmdtable->prdt_entry[0].dbau = 0;
            cmdtable->prdt_entry[0].dbc = 511;
            cmdtable->prdt_entry[0].i = 1;

            MemoryManager::ActiveMemoryManager->memset((uint8_t*)(cmdtable->prdt_entry[0].dba), 'X', 512);

            // if PxSSTS.IPM == 2h || 6h - Port gets into Low power mode
            // if PxSSTS.IPM == 8h - Port gets into DevSleep mode

            FIS_REG_H2D* fis = (FIS_REG_H2D*)(&cmdtable->cfis);
            fis->fis_type = FIS_TYPE_REG_H2D;
            fis->command = ATA_CMD_IDENTIFY;
            fis->c = 1;

            port->cmd |= HBA_PxCMD_ST;

            int spin = 0;
            while((port->tfd & (0x08 | 0x80)) && spin < 3)
            {
                for (int j = 0; j < 1000000; j++);
                spin++;
            }
            if (spin == 3)
            {
                continue;
            }

            port->serr = port->serr;
            port->is = port->is;
            port->ci |= 1 << slot;      //Issue command

            while(1)    //Wait for command to complete
            {
                if ((port->ci & (1 << slot)) == 0) break;
            }

            if (port->tfd & 0x1)
            {
                continue;
            }

            port->serr = port->serr;
            for (int j = 0; j < 1000000; j++);

            port->serr = port->serr;

            for (int j = 0; j < 1000000; j++);

            uint16_t* data = (uint16_t*)(cmdtable->prdt_entry[0].dba);
            uint8_t* configSpace = (uint8_t*)MemoryManager::ActiveMemoryManager->malloc(512);
            for (uint16_t j = 0; j < 512; j += 2)
            {
                uint16_t tmp = data[j/2];
                configSpace[j] = (tmp >> 8) & 0xFF;
                configSpace[j+1] = tmp & 0xFF;
            }

            char Model[41];
            uint32_t numSectors;
            
            for (uint8_t k = ATA_IDENT_MODEL; k < ATA_IDENT_MODEL+40; k++)
                    Model[k-ATA_IDENT_MODEL] = configSpace[k];
            Model[40] = 0;

            SATADISK* disk = (SATADISK*)MemoryManager::ActiveMemoryManager->malloc(sizeof(SATADISK));
            if (disk != 0)
            {
                uint32_t CommandSets  = *((uint32_t*)(configSpace + ATA_IDENT_COMMANDSETS));
                uint8_t l1 = (CommandSets & 0x00ff0000) >> 16;
                uint8_t l2 = (CommandSets & 0xff000000) >> 24;
                uint8_t l3 = CommandSets & 0x000000ff;
                uint8_t l4 = (CommandSets & 0x0000ff00) >> 8;

                CommandSets = ((uint32_t)l1 << 24) | ((uint32_t)l2 << 16) | ((uint32_t)l3 << 8) | (uint32_t)l4;

                if (CommandSets & (1 << 26))
                {
                    // Device uses 48-Bit Addressing:
                    numSectors = *((uint32_t*)(configSpace + ATA_IDENT_MAX_LBA_EXT));
                }
                else
                {
                    // Device uses CHS or 28-bit Addressing:
                    numSectors = *((uint32_t*)(configSpace + ATA_IDENT_MAX_LBA));
                }

                l1 = (numSectors & 0x00ff0000) >> 16;
                l2 = (numSectors & 0xff000000) >> 24;
                l3 = numSectors & 0x000000ff;
                l4 = (numSectors & 0x0000ff00) >> 8;

                numSectors = ((uint32_t)l1 << 24) | ((uint32_t)l2 << 16) | ((uint32_t)l3 << 8) | (uint32_t)l4;
                new (disk) SATADISK(Model, numSectors, true);

                disk->portnum = i;
                disks[numDisks] = disk;
                numDisks++;
            }
        }
    }
}

int AHCIDriver::GetType(HBA_PORT *port)
{
    uint32_t ssts = port->ssts;
    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;
    if (det != HBA_PORT_DET_PRESENT) // Check drive status
        return AHCI_DEV_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE)
       return AHCI_DEV_NULL;
    switch (port->sig)
    {
        case SATA_SIG_ATA: return AHCI_DEV_SATA;
        case SATA_SIG_ATAPI: return AHCI_DEV_SATAPI;
        case SATA_SIG_SEMB: return AHCI_DEV_SEMB;
        case SATA_SIG_PM: return AHCI_DEV_PM;
        default: return AHCI_DEV_NULL;
    }
}


void AHCIDriver::RebasePort(HBA_PORT* port, int portno)
{
    port->clb = AHCI_BASE + (portno << 10);
    port->clbu = 0;
    MemoryManager::ActiveMemoryManager->memset((uint8_t*)port->clb, 0, 1024);

    port->fb = AHCI_BASE + (32 << 10) + (portno << 8);
    port->fbu = 0;
    MemoryManager::ActiveMemoryManager->memset((uint8_t*)port->fb, 0, 256);

    HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)(port->clb);
    for (int i = 0; i < 32; i++)
    {
        cmdheader[i].prdtl = 8;
        cmdheader[i].ctba = AHCI_BASE + (i * 800);
        cmdheader[i].ctbau = 0;
        MemoryManager::ActiveMemoryManager->memset((uint8_t*)cmdheader[i].ctba, 0, 256);
    }
}

void AHCIDriver::StartCmd(HBA_PORT* port)
{
    //Wait until CR(bit 15) is cleared
    while(port->cmd & HBA_PxCMD_CR);
    port->cmd &= 0x0FFFFFFF;
    port->cmd |= 0x10000000;
    //Set FRE (Bit 14)
    port->cmd |= HBA_PxCMD_FRE;
    while(1)
    {
        if (!port->cmd & HBA_PxCMD_FR) continue;
        break;
    }

    for (int j = 0; j < 1000000; j++);

    while (!(port->ssts & 0x7) == 0x3)
    {
        port->ssts |= (~0x7);
        port->ssts |= 0x3;
        for (int j = 0; j < 1000000; j++);
    }

}

void AHCIDriver::StopCmd(HBA_PORT* port)
{
    port->cmd &= ~(HBA_PxCMD_ST | HBA_PxCMD_FRE);
    for (int j = 0; j < 10000000; j++);
    //Wait until FR(Bit 14) and CR(Bit 15) are cleared
    while(1)
    {
        if (port->cmd & HBA_PxCMD_FR) continue;
        if (port->cmd & HBA_PxCMD_CR) continue;
        break;
    }

    //Clear FRE(Bit 4)
    port->cmd &= ~HBA_PxCMD_FRE;
}


SATADISK::SATADISK(char* model, int sector_count, bool checkATA)
{
    MemoryManager::ActiveMemoryManager->memcpy((uint8_t*)model, (uint8_t*)ManufacturerModel, 41);
    numSectors = sector_count;
    SizeInGB = numSectors / (1024*1024*2);
    isDiskATA = checkATA;
}

void SATADISK::Read(uint32_t sector, int numSectors, uint8_t* buffer)
{
    if (sector > 0x0fffffff) return;
    if (numSectors > 16) return; //MAX: 8kb
    HBA_PORT* port = (HBA_PORT*)&(AHCIDriver::KernelAHCI->hba_memory->ports[portnum]);
    port->is = port->is;
    int slot = AHCIDriver::KernelAHCI->FindCommandSlot(port);

    if (slot == -1)
    {
        return;
    }

    HBA_CMD_HEADER* cmdheader = (HBA_CMD_HEADER*)(port->clb + (slot*sizeof(HBA_CMD_HEADER)));
    cmdheader->cfl = sizeof(FIS_REG_H2D) / sizeof(uint32_t);
    cmdheader->w = 0;
    cmdheader->prdtl = 1;

    HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
    MemoryManager::ActiveMemoryManager->memset((uint8_t*)cmdtbl, 0, sizeof(HBA_CMD_TBL) + (cmdheader->prdtl)*sizeof(HBA_PRDT_ENTRY));

    uint8_t* buf = (uint8_t*)(AHCI_BASE+0x0F0000);
    int count = numSectors;

    cmdtbl->prdt_entry[0].dba = (uint32_t)buf;
    cmdtbl->prdt_entry[0].dbc = (count << 9) - 1;
    cmdtbl->prdt_entry[0].i = 1;

    //Setup command
    FIS_REG_H2D* fis = (FIS_REG_H2D*)&(cmdtbl->cfis);
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c = 1;
    fis->command = ATA_CMD_READ_DMA_EXT;

    fis->lba0 = (uint8_t)sector;
    fis->lba1 = (uint8_t)(sector >> 8);
    fis->lba2 = (uint8_t)(sector >> 16);
    fis->device = 1 << 6;

    fis->lba3 = (uint8_t)(sector >> 24);
    fis->lba4 = 0;
    fis->lba5 = 0;

    fis->countl = count & 0xFF;
    fis->counth = (count>>8) & 0xFF;

    int spin = 0;
    while ((port->tfd & (0x08 | 0x80)) && spin < 1000000)
    {
        spin++;
    }  
    if (spin == 1000000)
    {
        return;
    }
    
    port->ci = 1 << slot;

    while(1)
    {
        if((port->ci  & (1 << slot)) == 0) break;
        if (port->tfd & 0x1)
        {
            return;
        }
    }

    if (port->tfd & 0x1)    //Error
    {
        return;
    }

    MemoryManager::ActiveMemoryManager->memcpy(buf, buffer, numSectors * 512);
}

SATADISK::~SATADISK()
{

}