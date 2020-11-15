#include <util/screen.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;

void util::clearScreen()
{
	//The video memory is located at address 0xb8000
	uint8_t* vidmem = (uint8_t*)0xb8000;
	for (int x = 0; x < 160; x++)
		for(int y = 0; y < 25; y++)
			vidmem[160*y+x] = 0;
}

void util::print(char* str)
{
	uint8_t* vidmem = (uint8_t*)0xb8000;
	static uint32_t x = 0, y = 0;

	for(uint16_t i=0; str[i] != '\0'; i++)
	{
		switch (str[i])
		{
			case '\n':
				y++;
				x = 0;
				break;
			default:
				vidmem[160*y+x] = str[i];
				vidmem[(160*y+x)+1] = 0x0F;
				x += 2;
				break;
		}
		if (x == 160)
		{
			x = 0;
			y++;
		}
		if (y >= 25)
		{
			x = 0;
			y = 24;
		}
	}
}

void util::printHex32(uint32_t hex_value)
{
	const char* hexArray = "0123456789ABCDEF";
	char* hex_out = "0x00000000 ";
	hex_out[9] = hexArray[hex_value&0x0000000F];
	hex_out[8] = hexArray[(hex_value>>4)&0x0000000F];
	hex_out[7] = hexArray[(hex_value>>8)&0x0000000F];
	hex_out[6] = hexArray[(hex_value>>12)&0x0000000F];
	hex_out[5] = hexArray[(hex_value>>16)&0x0000000F];
	hex_out[4] = hexArray[(hex_value>>20)&0x0000000F];
	hex_out[3] = hexArray[(hex_value>>24)&0x0000000F];
	hex_out[2] = hexArray[(hex_value>>28)&0x0000000F];
	print(hex_out);
}

void util::printHex8(uint8_t hex_value)
{
	const char* hexArray = "0123456789ABCDEF";
	char* hex_out = "00 ";
	hex_out[1] = hexArray[hex_value&0x0F];
	hex_out[0] = hexArray[(hex_value>>4)&0x0F];
	print(hex_out);
}