#include <common/types.h>
#include <util/screen.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;

extern "C" void kernel_main(uint32_t kernel_info_block)
{

    clearScreen();
    print("Hello Welcome to CrystalOS!\n");
    while(1);
}