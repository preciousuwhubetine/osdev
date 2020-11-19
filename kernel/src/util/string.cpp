#include <util/string.h>

using namespace crystalos;
using namespace crystalos::common;
using namespace crystalos::util;

void util::split(char* string, char delimeter, char* out)
{
    int numDelimeters = 0;
    for (int i = 0; i < len(string); i++)
        if(string[i] == delimeter) numDelimeters++;
    for (int i = 0; i < len(string); i++)
    {
        if(string[i] == delimeter) out[i] = 0;
        else out[i] = string[i];
    }
    out[len(string)] = 0;
}

char* util::getStringAtIndex(char* string, int index)
{
    int tmp = 0;
    if (index == 0)
        return string;
    for (int i = 0;  ; i++)
    {
        if (string[i] == 0) tmp++;
        if (tmp == index) return string+i+1;
    }
    return string;
}

char* util::subString(char* str, int pos, int length)
{
    if (pos > len(str) - 1) return str;
    char* res = (char*)(str+pos);
    if (length > len(str) - pos) return res;
    res[length] = 0;
    return res;
}

char* util::insertChar(char c, char* str, int index)
{
    char* result = str;
	char* empty = "";
	if (index < 0) return empty;
	if (index > len(result)) return result;
	else
	{
		for (int i = len(result); i >= index; i--)
		{
			result[i+1] = result[i];
		}
		result[index] = c;
		return result;
	}
}
