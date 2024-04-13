#pragma once
#include "File.h"
#include "Inode.h"

//记录了对文件读、写时需用到的读写偏移量、字节数以及目标区域首地址参数
class IOParam {
public:
	//当前读、写用户目标区域的首地址
	unsigned char* m_Base;
	//当前读、写文件的字节偏移量
	int m_Offset;
	//当前还剩余的读、写字节数量
	int m_Count;

public:
	IOParam();
	~IOParam();
};




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




//OpenFiles 类是进程的 u 结构中包含的一个对象
// 维护了当前进程的所有打开文件
class OpenFiles {
public:
	//进程允许打开的最大文件数
	static const int MAXOPENFILES = 15;

private:
	//File对象的指针数组，指向系统打开文件表中的File对象
	File* ProcessOpenFileTable[MAXOPENFILES];

public:
	OpenFiles();
	~OpenFiles();

	//进程打开文件时，在其打开文件表中分配一个空闲表项插槽
	int AllocFreeSlot();

	//根据用户系统调用提供的文件描述符参数 fd，找到对应的打开文件控制块 File 结构
	File* GetFileByFd(int fd);

	//为已分配到的空闲描述符 fd 和已分配的打开文件表中空闲 File 对象建立勾连关系
	void SetFd2File(int fd, File* pFile);
};



