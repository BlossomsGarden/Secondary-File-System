#pragma once
#include "Utility.h"


//存放一些工具函数，待补充

/* 用于在读、写文件时，高速缓存与用户指定目标内存区域之间数据传送 */
static void IOMove(unsigned char* from, unsigned char* to, int count);

void DWordCopy(int* src, int* dst, int count);