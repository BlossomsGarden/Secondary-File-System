#pragma once
#include "BufferManager.h"
#include "DiskManager.h"
#include "Inode.h"
#include "SuperBlock.h"

/*
 * 文件系统类(FileSystem)管理文件存储设备中
 * 的各类存储资源，磁盘块、外存INode的分配、
 * 释放。
 */
class FileSystem{
public:
	/* static consts */
	static const int SUPER_BLOCK_SECTOR_NUMBER = 0;	/* 定义SuperBlock位于磁盘上的扇区号，占据200，201两个扇区。 */
	static const int DISK_SIZE = 16384;
	static const int ROOTINO = 0;			/* 文件系统根目录外存Inode编号 */

	static const int INODE_NUMBER_PER_SECTOR = 8;		/* 外存INode对象长度为64字节，每个磁盘块可以存放512/64 = 8个外存Inode */
	static const int INODE_ZONE_START_SECTOR = 2;		/* 外存Inode区位于磁盘上的起始扇区号 */
	static const int INODE_ZONE_SIZE = 1024 - 2;		/* 磁盘上外存Inode区占据的扇区数 */

	static const int DATA_ZONE_START_SECTOR = 1024;		/* 数据区的起始扇区号 */
	static const int DATA_ZONE_END_SECTOR = 16383;	/* 数据区的结束扇区号 */
	static const int DATA_ZONE_SIZE = DISK_SIZE - DATA_ZONE_START_SECTOR;	/* 数据区占据的扇区数量 */

private:
	BufferManager* m_BufferManager;		/* FileSystem类需要缓存管理模块(BufferManager)提供的接口 */
	SuperBlock* m_SuperBlock;
	DiskManager* m_DeviceManager;

public:
	FileSystem();
	~FileSystem();

	/*
	* @comment 系统初始化时读入SuperBlock
	*/
	void LoadSuperBlock();
	/*
	 * @comment 将SuperBlock对象的内存副本更新到
	 * 存储设备的SuperBlock中去
	 */
	void Update();
	/*
	 * @comment  在存储设备dev上分配一个空闲
	 * 外存INode，一般用于创建新的文件。
	 */
	Inode* IAlloc();
	/*
	 * @comment  释放存储设备dev上编号为number
	 * 的外存INode，一般用于删除文件。
	 */
	void IFree(int number);
	/*
	 * @comment 在存储设备dev上分配空闲磁盘块
	 */
	Buf* Alloc();
	/*
	 * @comment 释放存储设备dev上编号为blkno的磁盘块
	 */
	void Free(int blkno);
	/*
	 * @comment 重新构建SuperBlock
	 */
	void FormatSuperBlock();
	/*
	 * @comment 重新构建整个文件系统
	 */
	void FormatFileSystem();
};


