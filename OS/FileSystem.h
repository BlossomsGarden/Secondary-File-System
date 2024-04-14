#pragma once
#include "Inode.h"
#include "SuperBlock.h"
#include "BufManager.h"



/*
 * 文件系统装配块(Mount)的定义。
 * 装配块用于实现子文件系统与
 * 根文件系统的连接。
 */
class Mount
{
	/* Functions */
public:
	/* Constructors */
	Mount();
	/* Destructors */
	~Mount();

	/* Members */
public:
	short m_dev;       /* 文件系统设备号 */
	SuperBlock* m_spb; /* 指向文件系统的Super Block对象在内存中的副本 */
	Inode* m_inodep;   /* 指向挂载子文件系统的内存INode */
};



//负责管理文件存储设备中的各类存储资源以及磁盘块、DiskINode的分配、释放
class FileSystem {
public:
	/* 定义 SuperBlock 位于磁盘上的扇区号，占据 0，1 两个扇区。 */
	static const int SUPER_BLOCK_SECTOR_NUMBER = 0;

	/* 文件系统根目录外存 Inode 编号 */
	static const int ROOTINO = 0;

	//! 大小与disk.img对应
	static const int BLOCK_NUM = 1024 * 1024 * 32 / 512;

	/* 外存 INode 对象长度为64字节，每个磁盘块可以存 512/64=8 个外存Inode */
	static const int INODE_NUMBER_PER_SECTOR = 8;

	/* 外存 Inode 区位于磁盘上的起始扇区号 */
	static const int INODE_ZONE_START_SECTOR = 2;

	/* 磁盘上外存 Inode 区占据的扇区数 */
	static const int INODE_ZONE_SIZE = 1024 - 2;

	/* 数据区的起始扇区号 */
	static const int DATA_ZONE_START_SECTOR = 1024;

	/* 数据区的结束扇区号 */
	static const int DATA_ZONE_END_SECTOR = 18000 - 1;

	/*数据区占据的扇区数量 */
	static const int DATA_ZONE_SIZE = 18000 - DATA_ZONE_START_SECTOR;

	static const int NMOUNT = 5; /* 系统中用于挂载子文件系统的装配块数量 */

private:
	/* FileSystem 类需要缓存管理模块(BufferManager)提供的接口 */
	BufManager* m_BufManager;

	//Update()函数的锁，该函数用于同步内存各个SuperBlock副本以及被修改过的内存Inode
	//任一时刻只允许一个进程调用该函数
	int updlock;

	/* 文件系统装配块表，Mount[0]用于根文件系统 */
	Mount m_Mount[NMOUNT];

	//仿照Kernel做的文件系统单体类实例
	static FileSystem instance;

public:
	FileSystem();
	~FileSystem();

	static FileSystem* GetInstance();

	//初始化成员变量
	void Initialize();

	//系统初始化时读入 SuperBlock
	void LoadSuperBlock();

	//获取该文件系统的 SuperBlock
	SuperBlock* GetSuperBlock();

	//将 SuperBlock 对象的内存副本更新到存储设备的 SuperBlock 中去
	void Update();

	//在存储设备 dev 上分配一个空闲外存 INode，一般用于创建新的文件
	Inode* IAlloc(int dev);

	//释放存储设备 dev 上编号为 number的外存 INode，一般用于删除文件
	void IFree(int InodeNo);

	//在存储设备 dev 上分配空闲磁盘块
	Buf* Alloc(int dev);

	//释放存储设备 dev 上编号为 blkno 的磁盘块
	void Free(int dev, int blkno);

	//格式化整个磁盘系统
	void FormatDisk(int dev);

private:
	//检查设备dev上编号blkno的磁盘块是否属于数据盘块区
	bool BadBlock(SuperBlock* spb, int blkno);
};
