#include <IO/interrupts.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::IO;
using namespace crystalos::util;

InterruptManager::GateDescriptor InterruptManager::InterruptDescriptorTable[256];
InterruptHandler *InterruptManager::handlers[256];

InterruptManager* InterruptManager::ActiveInterruptManager = 0;

InterruptHandler::InterruptHandler(InterruptManager *intManager, uint8_t interruptNumber)
{
    this->interruptNumber = interruptNumber;
    intManager->handlers[interruptNumber] = this;
}

InterruptHandler::~InterruptHandler()
{
    
}

uint32_t InterruptHandler::HandleInterrupt(uint32_t esp)
{
    return esp;
}

void InterruptManager::SetIDTEntry(uint8_t interruptNumber, void (*handler)())
{
    InterruptDescriptorTable[interruptNumber].handlerAddressLowBits = (uint32_t)handler & 0xFFFF;
    InterruptDescriptorTable[interruptNumber].gdt_codeSegmentSelector = 0x08;
    InterruptDescriptorTable[interruptNumber].reserved = 0;
    InterruptDescriptorTable[interruptNumber].access = 0x8E;
    InterruptDescriptorTable[interruptNumber].handlerAddressHighBits = (uint32_t)handler >> 16;
}

InterruptManager::InterruptManager()
: PICMasterCommand(0x20),
  PICMasterData(0x21),
  PICSlaveCommand(0xA0),
  PICSlaveData(0xA1)
{
    for (uint16_t i = 0; i<256; i++)
    {
        SetIDTEntry(i, &IgnoreInterruptRequest);
        handlers[i] = 0;
    }

	SetIDTEntry(0x00, &HandleException0x00);
	SetIDTEntry(0x01, &HandleException0x01);
	SetIDTEntry(0x02, &HandleException0x02);
	SetIDTEntry(0x03, &HandleException0x03);
	SetIDTEntry(0x04, &HandleException0x04);
	SetIDTEntry(0x05, &HandleException0x05);
	SetIDTEntry(0x06, &HandleException0x06);
	SetIDTEntry(0x07, &HandleException0x07);
	SetIDTEntry(0x08, &HandleException0x08);
	SetIDTEntry(0x09, &HandleException0x09);
	SetIDTEntry(0x0A, &HandleException0x0A);
	SetIDTEntry(0x0B, &HandleException0x0B);
	SetIDTEntry(0x0C, &HandleException0x0C);
	SetIDTEntry(0x0D, &HandleException0x0D);
	SetIDTEntry(0x0E, &HandleException0x0E);
	SetIDTEntry(0x0F, &HandleException0x0F);
	SetIDTEntry(0x10, &HandleException0x10);
	SetIDTEntry(0x11, &HandleException0x11);
	SetIDTEntry(0x12, &HandleException0x12);
	SetIDTEntry(0x13, &HandleException0x13);
	SetIDTEntry(0x14, &HandleException0x14);
	SetIDTEntry(0x15, &HandleException0x15);
	SetIDTEntry(0x16, &HandleException0x16);
	SetIDTEntry(0x17, &HandleException0x17);
	SetIDTEntry(0x18, &HandleException0x18);
	SetIDTEntry(0x19, &HandleException0x19);
	SetIDTEntry(0X1A, &HandleException0x1A);
	SetIDTEntry(0x1B, &HandleException0x1B);
	SetIDTEntry(0x1C, &HandleException0x1C);
	SetIDTEntry(0x1D, &HandleException0x1D);
	SetIDTEntry(0x1E, &HandleException0x1E);
	SetIDTEntry(0x1F, &HandleException0x1F);
	
    SetIDTEntry(0x20, &HandleInterruptRequest0x00);
    SetIDTEntry(0x21, &HandleInterruptRequest0x01);
    SetIDTEntry(0x22, &HandleInterruptRequest0x02);
    SetIDTEntry(0x23, &HandleInterruptRequest0x03);
    SetIDTEntry(0x24, &HandleInterruptRequest0x04);
    SetIDTEntry(0x25, &HandleInterruptRequest0x05);
    SetIDTEntry(0x26, &HandleInterruptRequest0x06);
    SetIDTEntry(0x27, &HandleInterruptRequest0x07);
    SetIDTEntry(0x28, &HandleInterruptRequest0x08);
    SetIDTEntry(0x29, &HandleInterruptRequest0x09);
    SetIDTEntry(0x2A, &HandleInterruptRequest0x0A);
    SetIDTEntry(0x2B, &HandleInterruptRequest0x0B);
    SetIDTEntry(0x2C, &HandleInterruptRequest0x0C);
    SetIDTEntry(0x2D, &HandleInterruptRequest0x0D);
    SetIDTEntry(0x2E, &HandleInterruptRequest0x0E);
    SetIDTEntry(0x2F, &HandleInterruptRequest0x0F);
    SetIDTEntry(0x30, &HandleInterruptRequest0x10);
    SetIDTEntry(0x31, &HandleInterruptRequest0x11);
    SetIDTEntry(0x32, &HandleInterruptRequest0x12);
    SetIDTEntry(0x33, &HandleInterruptRequest0x13);
    SetIDTEntry(0x34, &HandleInterruptRequest0x14);
    SetIDTEntry(0x35, &HandleInterruptRequest0x15);
    SetIDTEntry(0x36, &HandleInterruptRequest0x16);
    SetIDTEntry(0x37, &HandleInterruptRequest0x17);
    SetIDTEntry(0x38, &HandleInterruptRequest0x18);
    SetIDTEntry(0x39, &HandleInterruptRequest0x19);
    SetIDTEntry(0x3A, &HandleInterruptRequest0x1A);
    SetIDTEntry(0x3B, &HandleInterruptRequest0x1B);
    SetIDTEntry(0x3C, &HandleInterruptRequest0x1C);
    SetIDTEntry(0x3D, &HandleInterruptRequest0x1D);
    SetIDTEntry(0x3E, &HandleInterruptRequest0x1E);
    SetIDTEntry(0x3F, &HandleInterruptRequest0x1F);
    SetIDTEntry(0x40, &HandleInterruptRequest0x20);
    SetIDTEntry(0x41, &HandleInterruptRequest0x21);
    SetIDTEntry(0x42, &HandleInterruptRequest0x22);
    SetIDTEntry(0x43, &HandleInterruptRequest0x23);
    SetIDTEntry(0x44, &HandleInterruptRequest0x24);
    SetIDTEntry(0x45, &HandleInterruptRequest0x25);
    SetIDTEntry(0x46, &HandleInterruptRequest0x26);
    SetIDTEntry(0x47, &HandleInterruptRequest0x27);
    SetIDTEntry(0x48, &HandleInterruptRequest0x28);
    SetIDTEntry(0x49, &HandleInterruptRequest0x29);
    SetIDTEntry(0x4A, &HandleInterruptRequest0x2A);
    SetIDTEntry(0x4B, &HandleInterruptRequest0x2B);
    SetIDTEntry(0x4C, &HandleInterruptRequest0x2C);
    SetIDTEntry(0x4D, &HandleInterruptRequest0x2D);
    SetIDTEntry(0x4E, &HandleInterruptRequest0x2E);
    SetIDTEntry(0x4F, &HandleInterruptRequest0x2F);
    SetIDTEntry(0x50, &HandleInterruptRequest0x30);

    PICMasterCommand.Write(0x11);
    PICSlaveCommand.Write(0x11);

    PICMasterData.Write(0x20);
    PICSlaveData.Write(0x28);

    PICMasterData.Write(0x04);
    PICSlaveData.Write(0x02);

    PICMasterData.Write(0x01);
    PICSlaveData.Write(0x01);

    PICMasterData.Write(0x00);
    PICSlaveData.Write(0x00);

    IDTPointer idt_pointer;
    idt_pointer.size = 256 * sizeof(GateDescriptor) - 1;
    idt_pointer.base = (uint32_t)InterruptDescriptorTable;

    if (ActiveInterruptManager == 0) ActiveInterruptManager = this;

    asm volatile("lidt %0" : : "m"(idt_pointer));
}

InterruptManager::~InterruptManager()
{

}

void InterruptManager::Activate()
{
    if (ActiveInterruptManager == 0)
        ActiveInterruptManager = this;
    asm("sti");
}

void InterruptManager::Deactivate()
{
    asm("cli");
}

uint32_t InterruptManager::DoHandleInterrupt(uint8_t interruptNumber, uint32_t esp)
{
    uint32_t stack_pointer = esp;

	if(interruptNumber >= 0 && interruptNumber<= 0x1F)
	{
		print("Exception ");
		printHex8(interruptNumber);
        asm("hlt");
	}

    if (interruptNumber == 0x20)
    {
        //timer
    }

    if (interruptNumber >= 0x20 && interruptNumber <= 0x40)
    {
        if (handlers[interruptNumber] != 0)
            stack_pointer = handlers[interruptNumber]->HandleInterrupt(esp);
    }

    PICMasterCommand.Write(0x20);
        if (interruptNumber >= 0x28)
            PICSlaveCommand.Write(0x20);

    return stack_pointer;
}

uint32_t InterruptManager::HandleInterrupt(uint8_t interruptNumber, uint32_t esp)
{
    return ActiveInterruptManager->DoHandleInterrupt(interruptNumber, esp);
}