#pragma once
#include "InodeTable.h"
#include "Inode.h"
#include "FileSystem.h"

//负责内存 Inode 的分配和释放
class InodeTable {
public:
	//内存 Inode 的数量
	static const int NINODE = 100;

	//内存 Inode 数组，每个打开文件都会占用一个内存Inode
	Inode m_Inode[NINODE];

	//对全局对象 g_FileSystem 的引用
	FileSystem* m_FileSystem;

public:
	InodeTable ();
	~InodeTable ();

	//初始化对 g_FileSystem 对象的引用
	void Initialize();

	//根据指定设备号 dev，外存 Inode 编号获取对应 Inode:
	//如果该 Inode 已经在内存中，对其上锁并返回该内存 Inode
	//如果不在内存中，则将其读入内存后上锁并返回该内存 Inode
	Inode* IGet(int inumber);

	//减少该内存 Inode 的引用计数，如果此 Inode 已经没有目录项指向它，
	//且无进程引用该 Inode，则释放此文件占用的磁盘块
	void IPut(Inode* pNode);

	//将所有被修改过的内存 Inode 更新到对应外存 Inode 中
	void UpdateInodeTable();
	
	//检查设备 dev 上编号为 inumber 的外存 inode 是否有内存拷贝
	//如果有则返回该内存 Inode 在内存 Inode 表中的索引
	int IsLoaded(int inumber);

	//在内存 Inode 表中寻找一个空闲的内存 Inode
	Inode* GetFreeInode();

};

InodeTable ::InodeTable ()
{
}

InodeTable ::~InodeTable ()
{
}
