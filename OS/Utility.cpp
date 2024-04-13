#include "Utility.h"
using namespace std;

//
void IOMove(unsigned char* from, unsigned char* to, int count) {
	while (count--)
	{
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