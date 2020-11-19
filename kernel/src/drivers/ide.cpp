#include <drivers/ide.h>

using namespace crystalos;
using namespace crystalos::IO;
using namespace crystalos::drivers;
using namespace crystalos::common;

void print(char*);
IDEDriver* IDEDriver::KernelIDEDriver = 0;

IDEDevice::IDEDevice(uint16_t portBase, bool master)
:dataPort(portBase),
 errorPort(portBase+1),
 sectorCountPort(portBase+2),
 lbaLowPort(portBase+3),
 lbaMidPort(portBase+4),
 lbaHiPort(portBase+5),
 devicePort(portBase+6),
 commandPort(portBase+7),
 controlPort(portBase+0x206)
{
    Master = master;
    Empty = true;
}

IDEDevice::~IDEDevice()
{

}

void IDEDevice::Identify()
{
    uint8_t status;

    devicePort.Write(Master ? 0xA0 : 0xB0);
    for (int i = 0; i < 4; i++) status = controlPort.Read();
    controlPort.Write(0);

    devicePort.Write(0xA0);
    for (int i = 0; i < 4; i++) status = controlPort.Read();
    status = commandPort.Read();
    if (status == 0xFF) {
        return;
    }

    devicePort.Write(Master ? 0xA0 : 0xB0);
    for (int i = 0; i < 4; i++) status = controlPort.Read();

    sectorCountPort.Write(0);
    lbaLowPort.Write(0);
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
    commandPort.Write(ATA_CMD_IDENTIFY);
    for (int i = 0 ; i < 65536; i++);

    status = commandPort.Read();
    if (status == 0x00)  return; //No device
    while(((status & ATA_SR_BSY) == ATA_SR_BSY) && ((status & ATA_SR_ERR) != ATA_SR_ERR))
        status = commandPort.Read();

    if (status & ATA_SR_ERR)
    {
        return ; //ATAPI device
    }

    uint8_t* configSpace = (uint8_t*)MemoryManager::ActiveMemoryManager->malloc(512);
    for (int i = 0; i < 512; i++) configSpace[i] = 0;

    for (uint16_t i = 0; i < 512; i += 2)
    {
        uint16_t data = dataPort.Read();
        configSpace[i] = (data >> 8) & 0xFF;
        configSpace[i+1] = data & 0xFF;
    }

    for (uint8_t i = ATA_IDENT_MODEL; i < ATA_IDENT_MODEL+40; i++)
        ManufacturerModel[i-ATA_IDENT_MODEL] = configSpace[i];
    ManufacturerModel[40] = 0;

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
        Mode = Bit48;
    }
    else
    {
        // Device uses CHS or 28-bit Addressing:
        numSectors = *((uint32_t*)(configSpace + ATA_IDENT_MAX_LBA));
        Mode = Bit28;
    }
    
    l1 = (numSectors & 0x00ff0000) >> 16;
    l2 = (numSectors & 0xff000000) >> 24;
    l3 = numSectors & 0x000000ff;
    l4 = (numSectors & 0x0000ff00) >> 8;

    numSectors = ((uint32_t)l1 << 24) | ((uint32_t)l2 << 16) | ((uint32_t)l3 << 8) | (uint32_t)l4;

    SectorSize = 512;
    SizeInGB = numSectors/(1024*1024*2);

    Empty = false;
	MemoryManager::ActiveMemoryManager->free(configSpace);
}

void IDEDevice::Read(uint32_t sector, int numSectors, uint8_t* buffer)
{
    switch(Mode)
    {
        case Bit28:
            Read28(sector, numSectors, buffer);
            break;
        case Bit48:
            Read48(sector, numSectors, buffer);
            break;
    }
    
}

void IDEDevice::Write(uint32_t sector, uint8_t* data)
{
    switch (Mode)
    {
        case Bit28:
            Write28(sector, data);
            break;
        case Bit48:
            Write48(sector, data);
            break;
    }
}

void IDEDevice::Read28(uint32_t sector, uint16_t numSectors, uint8_t* buffer)
{
    if (sector > 0x0FFFFFFF) return;

    int count = 0;
    uint8_t status = 0;

    devicePort.Write((Master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    for (int i = 0; i < 4; i++) status = controlPort.Read();
	
    sectorCountPort.Write(numSectors);
    errorPort.Write(0);
    lbaLowPort.Write(sector & 0x000000FF);
    lbaMidPort.Write((sector & 0x0000FF00) >> 8);
    lbaHiPort.Write((sector & 0x00FF0000) >> 16);
    commandPort.Write(ATA_CMD_READ_PIO);
    for (int i = 0; i < 100; i++) status = controlPort.Read();

    for (int i = 0; i < numSectors; i++)
    {
        status = commandPort.Read();
        while(((status & ATA_SR_BSY) == ATA_SR_BSY) && ((status & ATA_SR_ERR) != ATA_SR_ERR))
            status = commandPort.Read();
        if (status & ATA_SR_ERR)
		{
			return;
		}        
        for (int j = count; j < count+512; j += 2)
        {
            uint16_t data = dataPort.Read();
            buffer[j+1] = (data >> 8) & 0xFF;
            buffer[j] = data & 0xFF;
        }
        count += 512;
    }
}

void IDEDevice::Read48(uint32_t sector, uint16_t numSectors, uint8_t* buffer)
{
    if (sector > 0xFFFFFFFF) return;

    int count = 0;
    uint8_t status = 0;

    devicePort.Write(Master ? 0xE0 : 0xF0);
    for (int i = 0; i < 4; i++) status = controlPort.Read();    //400 nanoseconds delay

    errorPort.Write(0);
    sectorCountPort.Write((numSectors >> 8) & 0xff);
    lbaLowPort.Write((sector & 0xFF000000) >> 24);
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
	sectorCountPort.Write(numSectors & 0xff);
    lbaLowPort.Write(sector & 0x000000FF);
    lbaMidPort.Write((sector & 0x0000FF00) >> 8);
    lbaHiPort.Write((sector & 0x00FF0000) >> 16);
    commandPort.Write(ATA_CMD_READ_PIO_EXT);

    for (int i = 0; i < 100; i++) status = controlPort.Read();  //400 nanaoseconds delay

    for (int i = 0; i < numSectors; i++)
    {
        status = commandPort.Read();
		while(((status & ATA_SR_BSY) == ATA_SR_BSY) && ((status & ATA_SR_ERR) != ATA_SR_ERR))
            status = commandPort.Read();
        if (status & ATA_SR_ERR)
		{
			return;
		}
        for (int j = count; j < count+512; j += 2)
        {
            uint16_t data = dataPort.Read();
            buffer[j+1] = (data >> 8) & 0xFF;
            buffer[j] = data & 0xFF;
        }
        count += 512;
    }
}

// Write 28 and 48 was done hurridely.
// Come back later to check and ensure working properly.
// 21st July, 2020 2:00:34 AM

void IDEDevice::Write28(uint32_t sector, uint8_t* data)
{
    if(sector > 0x0FFFFFFF) return;
    uint8_t status;

	devicePort.Write( (Master ? 0xE0 : 0xF0) | ((sector & 0x0F000000) >> 24));
    for (int i = 0; i < 4; i++) status = controlPort.Read();

	sectorCountPort.Write(1);
	errorPort.Write(0);
	lbaLowPort.Write(sector & 0x000000FF);
	lbaMidPort.Write((sector & 0x0000FF00) >> 8);
	lbaLowPort.Write((sector & 0x00FF0000) >> 16 );
	commandPort.Write(ATA_CMD_WRITE_PIO);

    for (int i = 0; i < 100; i++) status = controlPort.Read();

	for(int i = 0; i < 512; i += 2)
	{
		uint16_t wdata = data[i];
		wdata |= ((uint16_t)data[i+1]) << 8;
		dataPort.Write(wdata);
	}

    Flush();
}

void IDEDevice::Write48(uint32_t sector, uint8_t* data)
{
    if (sector > 0xFFFFFFFF) return;

    uint8_t status;

    devicePort.Write(Master ? 0xE0 : 0xF0);
    for (int i = 0; i < 4; i++) status = controlPort.Read();    //400 nanoseconds delay

    errorPort.Write(0);
    sectorCountPort.Write(0);
    lbaLowPort.Write((sector & 0xFF000000) >> 24);
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
	sectorCountPort.Write(1);
    lbaLowPort.Write(sector & 0x000000FF);
    lbaMidPort.Write((sector & 0x0000FF00) >> 8);
    lbaHiPort.Write((sector & 0x00FF0000) >> 16);
    commandPort.Write(ATA_CMD_WRITE_PIO_EXT);

    for (int i = 0; i < 100; i++) status = controlPort.Read();  //400 nanaoseconds delay

    for (int i = 0; i < 512; i += 2)
    {
        uint16_t wdata = data[i];
		wdata |= ((uint16_t)data[i+1]) << 8;
		dataPort.Write(wdata);
    }

    Flush();
}

void IDEDevice::Flush()
{
    devicePort.Write(Master ? 0xE0 : 0xF0);
	commandPort.Write(Mode == Bit28 ? ATA_CMD_CACHE_FLUSH : ATA_CMD_CACHE_FLUSH_EXT);
	uint8_t status = commandPort.Read();
	if(status == 0x00)
		return;
	while(((status & 0x80) == 0x80) && ((status & 0x01) != 0x01))
		status = commandPort.Read();
	if(status & 0x01)
	{
		return;
	}
}

bool IDEDevice::IsEmpty()
{
    return Empty;
}

IDEChannel::IDEChannel(uint16_t portBase, InterruptManager* interrupts, uint8_t interruptNumber)
: masterDrive(portBase, true),
  slaveDrive(portBase, false),
  InterruptHandler(interrupts, interruptNumber + 0x20)
{
    IRQ_Invoked = false;
    masterDrive.Parent = this;
    slaveDrive.Parent = this;
}

IDEChannel::~IDEChannel()
{
}

IDEDriver::IDEDriver(uint16_t primaryChannelPortBase, uint16_t secondaryChannelPortBase, InterruptManager* interrupts, uint8_t primaryChannelInterrupt, uint8_t secondaryChannelInterrupt)
: Driver(),
  primaryChannel(primaryChannelPortBase, interrupts, primaryChannelInterrupt),
  secondaryChannel(secondaryChannelPortBase, interrupts, secondaryChannelInterrupt)
{
    if (KernelIDEDriver == 0) KernelIDEDriver = this;
}

IDEDriver::~IDEDriver()
{
    if (KernelIDEDriver == this) KernelIDEDriver = 0;
}

void IDEDriver::Activate()
{
}

void IDEDriver::Initialize()
{
    primaryChannel.masterDrive.Identify();
    primaryChannel.slaveDrive.Identify();
    secondaryChannel.masterDrive.Identify();
    secondaryChannel.slaveDrive.Identify();
}

uint32_t IDEChannel::HandleInterrupt(uint32_t esp)
{
    if (!IRQ_Invoked) IRQ_Invoked = true;
    return esp;
}