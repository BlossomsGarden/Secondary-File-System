#pragma once
#include "OpenFiles.h"
#include "File.h"

class OpenFiles {
public:
	//进程允许打开的最大文件数
	static const int NOFILES = 15;

private:
	//File对象的指针数组，指向系统打开文件表中的File对象
	File* ProcessOpenFileTable[NOFILES];

public:
	OpenFiles ();
	~OpenFiles ();

	//进程请求打开文件时，在打开文件描述符表中分配一个空闲表项
	int AllocFreeSlot();

	//根据用户系统调用提供的文件描述符参数 fd，找到对应的打开文件控制块 File 结构
	File* GetF(int fd);

	//为已分配到的空闲描述符 fd 和已分配的打开文件表中空闲 File 对象建立勾连关系
	void SetF(int fd, File* pFile);
};

OpenFiles ::OpenFiles ()
{
}

OpenFiles ::~OpenFiles ()
{
}
