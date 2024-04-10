#pragma once
#include "File.h"
#include "Inode.h"

//记录进程打开文件的读、写请求类型、文件读写位置等
class File{
public:
	enum FileFlags {
		//读请求
		FREAD = 0x1,
		//写请求
		FWRITE = 0x2,
		//管道
		FPIPE=0x4
	};

	//该文件的读写操作限制
	unsigned int f_flag;
	//引用该文件控制块的进程数量
	int f_count;
	//指向打开文件的内存Inode指针
	Inode* f_inode;
	//文件读写位置指针
	int f_offset;


public:
	File();
	~File();

};

File::File()
{
}

File::~File()
{
}
