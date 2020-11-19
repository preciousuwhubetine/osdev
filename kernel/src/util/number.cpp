#include <util/number.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;

void util::itoa(int value, uint16_t base, char* out)
{
    if (value == 0) 
	{
		out[0] = '0';
		out[1] = 0;
		return;
	}
	int numChars = 0;
	uint32_t tmpval = value;
	while (tmpval)
	{
		tmpval /= base;
		numChars++;	
	}

	out[numChars] = 0;
	tmpval = value;
	int current = numChars - 1;
	
	while(tmpval)
	{
		int tmp = tmpval % base;
		if (tmp >= 10)
			out[current] = (char)(tmp + 55 - 0xE0);
		else
			out[current] = (char)(tmp + '0');
		current--;
		tmpval /= base;
	}
}

int util::atoi(char* string)
{

}

int util::len(char* str)
{
    int ret = 0;
    while(str[ret] != 0)
        ret++;
    return ret;
}


uint32_t util::bigE(uint32_t value)
{
	return ((value << 24) & 0xff000000)
             | ((value << 8) & 0x00ff0000) 
             | ((value >> 8) & 0x0000ff00)  
             | ((value >> 24) & 0x000000ff);
}