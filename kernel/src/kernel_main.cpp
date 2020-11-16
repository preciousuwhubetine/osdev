#include <common/types.h>
#include <util/screen.h>
#include <IO/port.h>
#include <IO/interrupts.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;
using namespace crystalos::IO;

class Interrupt10Test : public InterruptHandler
{
    public:
        Interrupt10Test(InterruptManager* intman)
        : InterruptHandler(intman, 0x20 + 0x10) //since exceptions occupy the first 0x20(32) spots
        {

        }
        uint32_t HandleInterrupt(uint32_t esp)
        {
            print("Interrupt 0x10!");
            return esp;
        }
};

extern "C" void kernel_main(uint32_t kernel_info_block)
{

    clearScreen();
    print("Hello Welcome to CrystalOS!\n");

    InterruptManager kernel_interrupts; //init the interrupt manager class

    Interrupt10Test int10(&kernel_interrupts);

    kernel_interrupts.Activate();       //activate all interrupts

    asm("int $0x30"); // just to test the interrupt stuff
    while(1);
}
