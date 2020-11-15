#include <common/types.h>
#include <util/screen.h>
#include <IO/port.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;
using namespace crystalos::IO;

extern "C" void kernel_main(uint32_t kernel_info_block)
{

    clearScreen();
    print("Hello Welcome to CrystalOS!\n");
    while(1);
}