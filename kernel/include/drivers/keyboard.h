#ifndef CRYSTALOS__DRIVERS__KEYBOARD_H
#define CRYSTALOS__DRIVERS__KEYBOARD_H

#include <common/types.h>
#include <IO/port.h>
#include <IO/interrupts.h>
#include <drivers/driver.h>
#include <util/screen.h>

/*
    The keyboard driver
*/

namespace crystalos
{
    namespace drivers
    {
        enum Modifier
        {
            NOKEY = 0x00,
            SHIFTKEY = 0x01,
            CONTROLKEY = 0x02,
            ALTKEY = 0x03,
            WINKEY = 0x04
        };
        
        class KeyboardDriver;
        class KeyboardEventHandler
        {
            public:
                KeyboardDriver* base;
                KeyboardEventHandler();

                virtual void OnKeyDown(char);
                virtual void OnKeyUp(char);
        };

        class KeyboardDriver : public Driver, IO::InterruptHandler
        {            
            protected:
                IO::Port8BitSlow commandPort;
                IO::Port8BitSlow dataPort;

            public:
                common::uint8_t Modifiers[5];
                KeyboardEventHandler* kb_handler;

                virtual void Activate();
                KeyboardDriver(IO::InterruptManager* manager, KeyboardEventHandler* kbhandler);
                static KeyboardDriver* Main;
                
                virtual common::uint32_t HandleInterrupt(common::uint32_t esp);
        };
        
        class ConsoleKeyboardHandler : public KeyboardEventHandler
        {
            private:
                common::uint8_t arr[1];
                char out[3];
                
            public:
                void OnKeyDown(char c);
                void OnKeyUp(char c);
        };

    }
}

#endif