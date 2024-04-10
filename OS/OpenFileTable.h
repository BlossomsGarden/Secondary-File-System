#pragma once
#include "OpenFileTable.h"
#include "File.h"

//进程的打开文件结构
class OpenFileTable {
public:
	static const int NFILE = 100; /* 打开文件控制块 File 结构的数量 */

public:
	OpenFileTable ();
	~OpenFileTable ();

	//在系统打开文件表中分配一个空闲的 File 结构
	File* FAlloc();
	
	//对打开文件控制块 File 结构的引用计数 f_count 减 1,若引用计数 f_count 为 0，则释放 File 结构。
	void CloseF(File* pFile);

	//系统打开文件表，为所有进程共享，进程打开文件描述符表中包含指向打开文件表中对应 File 结构的指针
	File m_File[NFILE];
};

OpenFileTable ::OpenFileTable ()
{
}

OpenFileTable ::~OpenFileTable ()
{
}