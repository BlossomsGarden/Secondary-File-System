#include "Utility.h"
#include <string.h>


void IOMove(unsigned char* from, unsigned char* to, int count) {
	while (count--){
		*to++ = *from++;
	}
	return;
}

void DWordCopy(int* src, int* dst, int count)
{
	while (count--)
	{
		*dst++ = *src++;
	}
	return;
}

bool IsNumberString(std::string str){
    for (int i = 0; i < str.size(); i++){
        int tmp = (int)str[i];
        if (tmp >= 48 && tmp <= 57)
        {
            continue;
        }
        else
        {
            return false;
        }
    }
    return true;
}

void ConstStringCopy(char const* src, char* dst){
    while ((*dst++ = *src++) != 0);
}