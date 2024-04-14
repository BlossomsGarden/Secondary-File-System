#pragma once
#include "File.h"


class OpenFileTable;

//来一个全局打开文件表
extern OpenFileTable g_OpenFileTable;


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


//定义了目录项的结构(Inode编号+路径名组成)
//目录文件保存在磁盘上，一个盘块可保存 16 个目录项
//根据目录项的 Inode 编号找到目录中的文件
class DirectoryEntry {
public:
	//目录项中路径部分的最大字符串长度
	static const int DIRSIZ = 28;

	//目录项中 Inode 编号部分
	int m_ino;
	//目录项中路径名部分
	char m_name[DIRSIZ];

public:
	DirectoryEntry() {};
	~DirectoryEntry() {};
};