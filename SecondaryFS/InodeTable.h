#pragma once
#include"Inode.h"
#include "FileSystem.h"

/*
 * 内存Inode表(class InodeTable)
 * 负责内存Inode的分配和释放。
 */
class InodeTable{
public:
	static const int NINODE = 100;	/* 内存Inode的数量 */

	Inode m_Inode[NINODE];		/* 内存Inode数组，每个打开文件都会占用一个内存Inode */

	FileSystem* m_FileSystem;	/* 对全局对象g_FileSystem的引用 */

public:
	InodeTable();
	~InodeTable();

	/*
	 * @comment 根据指定设备号dev，外存Inode编号获取对应
	 * Inode。如果该Inode已经在内存中，对其上锁并返回该内存Inode，
	 * 如果不在内存中，则将其读入内存后上锁并返回该内存Inode
	 */
	Inode* IGet(int inumber);
	/*
	 * @comment 减少该内存Inode的引用计数，如果此Inode已经没有目录项指向它，
	 * 且无进程引用该Inode，则释放此文件占用的磁盘块。
	 */
	void IPut(Inode* pNode);
	/*
	 * @comment 将所有被修改过的内存Inode更新到对应外存Inode中
	 */
	void UpdateInodeTable();
	/*
	 * @comment 检查设备dev上编号为inumber的外存inode是否有内存拷贝，
	 * 如果有则返回该内存Inode在内存Inode表中的索引
	 */
	int IsLoaded(int inumber);
	/*
	 * @comment 在内存Inode表中寻找一个空闲的内存Inode
	 */
	Inode* GetFreeInode();
	/*
	 * @comment 将所有内存Inode初始化一遍
	 */
	void Format();
};
