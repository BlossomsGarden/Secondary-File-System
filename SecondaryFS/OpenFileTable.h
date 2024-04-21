#pragma once
#include "File.h"

/*
 * 打开文件管理类(OpenFileManager)负责
 * 内核中对打开文件机构的管理，为进程
 * 打开文件建立内核数据结构之间的勾连
 * 关系。
 * 勾连关系指进程u区中打开文件描述符指向
 * 打开文件表中的File打开文件控制结构，
 * 以及从File结构指向文件对应的内存Inode。
 */
class OpenFileTable{
public:
	/* static consts */
	static const int NFILE = 100;	/* 打开文件控制块File结构的数量 */

	File m_File[NFILE];			/* 系统打开文件表，为所有进程共享，进程打开文件描述符表
								中包含指向打开文件表中对应File结构的指针。*/

public:
	OpenFileTable();
	~OpenFileTable();

	/*
	 * @comment 在系统打开文件表中分配一个空闲的File结构
	 */
	File* FAlloc();
	/*
	 * @comment 对打开文件控制块File结构的引用计数f_count减1，
	 * 若引用计数f_count为0，则释放File结构。
	 */
	void CloseF(File* pFile);
	/*
	 * @comment 初始化打开文件表
	 */
	void Format();
};
