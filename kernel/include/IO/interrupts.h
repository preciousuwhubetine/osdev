#ifndef CRYSTALOS__IO__INTERRUPTS_H
#define CRYSTALOS__IO__INTERRUPTS_H

#include <common/types.h>
#include <IO/port.h>
#include <util/screen.h>

namespace crystalos
{
    namespace IO
    {
        class InterruptHandler;
        class InterruptManager
        {
            friend class InterruptHandler;
            protected:
				
                struct GateDescriptor
                {
                    common::uint16_t handlerAddressLowBits;
                    common::uint16_t gdt_codeSegmentSelector;
                    common::uint8_t reserved;
                    common::uint8_t access;
                    common::uint16_t handlerAddressHighBits;
                }__attribute__((packed));

                static struct GateDescriptor InterruptDescriptorTable[256];
                static InterruptHandler *handlers[256];

                Port8BitSlow PICMasterCommand;
                Port8BitSlow PICMasterData;
                Port8BitSlow PICSlaveCommand;
                Port8BitSlow PICSlaveData;

                struct IDTPointer
                {
                    common::uint16_t size;
                    common::uint32_t base;
                }__attribute__((packed));

                struct ScheduleEntry
                {
                    bool free;
                    void (*handlerAddress)();
                    common::uint32_t currentFrame;
                    common::uint32_t lastFrame;
                }__attribute__((packed));

            public:
                InterruptManager();
                ~InterruptManager();

				static InterruptManager* ActiveInterruptManager;
                void SetIDTEntry(common::uint8_t interruptNumber, void (*handler)());
                void Activate();
                void Deactivate();
                
                static common::uint32_t HandleInterrupt(common::uint8_t interruptNumber, common::uint32_t esp);
                common::uint32_t DoHandleInterrupt(common::uint8_t interruptNumber, common::uint32_t esp);

                static void IgnoreInterruptRequest();
                static void HandleInterruptRequest0x00();
                static void HandleInterruptRequest0x01();
                static void HandleInterruptRequest0x02();
                static void HandleInterruptRequest0x03();
                static void HandleInterruptRequest0x04();
                static void HandleInterruptRequest0x05();
                static void HandleInterruptRequest0x06();
                static void HandleInterruptRequest0x07();
                static void HandleInterruptRequest0x08();
                static void HandleInterruptRequest0x09();
                static void HandleInterruptRequest0x0A();
                static void HandleInterruptRequest0x0B();
                static void HandleInterruptRequest0x0C();
                static void HandleInterruptRequest0x0D();
                static void HandleInterruptRequest0x0E();
                static void HandleInterruptRequest0x0F();
                static void HandleInterruptRequest0x10();
                static void HandleInterruptRequest0x11();
                static void HandleInterruptRequest0x12();
                static void HandleInterruptRequest0x13();
                static void HandleInterruptRequest0x14();
                static void HandleInterruptRequest0x15();
                static void HandleInterruptRequest0x16();
                static void HandleInterruptRequest0x17();
                static void HandleInterruptRequest0x18();
                static void HandleInterruptRequest0x19();
                static void HandleInterruptRequest0x1A();
                static void HandleInterruptRequest0x1B();
                static void HandleInterruptRequest0x1C();
                static void HandleInterruptRequest0x1D();
                static void HandleInterruptRequest0x1E();
                static void HandleInterruptRequest0x1F();
                static void HandleInterruptRequest0x20();
                static void HandleInterruptRequest0x21();
                static void HandleInterruptRequest0x22();
                static void HandleInterruptRequest0x23();
                static void HandleInterruptRequest0x24();
                static void HandleInterruptRequest0x25();
                static void HandleInterruptRequest0x26();
                static void HandleInterruptRequest0x27();
                static void HandleInterruptRequest0x28();
                static void HandleInterruptRequest0x29();
                static void HandleInterruptRequest0x2A();
                static void HandleInterruptRequest0x2B();
                static void HandleInterruptRequest0x2C();
                static void HandleInterruptRequest0x2D();
                static void HandleInterruptRequest0x2E();
                static void HandleInterruptRequest0x2F();
                static void HandleInterruptRequest0x30();
				
				static void HandleException0x00();
				static void HandleException0x01();
				static void HandleException0x02();
				static void HandleException0x03();
				static void HandleException0x04();
				static void HandleException0x05();
				static void HandleException0x06();
				static void HandleException0x07();
				static void HandleException0x08();
				static void HandleException0x09();
				static void HandleException0x0A();
				static void HandleException0x0B();
				static void HandleException0x0C();
				static void HandleException0x0D();
				static void HandleException0x0E();
				static void HandleException0x0F();
				static void HandleException0x10();
				static void HandleException0x11();
				static void HandleException0x12();
				static void HandleException0x13();
				static void HandleException0x14();
				static void HandleException0x15();
				static void HandleException0x16();
				static void HandleException0x17();
				static void HandleException0x18();
				static void HandleException0x19();
				static void HandleException0x1A();
				static void HandleException0x1B();
				static void HandleException0x1C();
				static void HandleException0x1D();
				static void HandleException0x1E();
				static void HandleException0x1F();
        };

        class InterruptHandler
        { 
            protected:
                common::uint8_t interruptNumber;
                InterruptHandler(InterruptManager *intMan, common::uint8_t interruptNumber);
                ~InterruptHandler();

            public:
                virtual common::uint32_t HandleInterrupt(common::uint32_t esp);
        };
    }
}


#endif