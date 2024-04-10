#pragma once
#include "FileSystem.h"
#include "SuperBlock.h"
#include "BufManager.h"

//负责管理文件存储设备中的各类存储资源，以及磁盘块、DiskINode的分配、释放
class FileSystem {
public:
	/* 定义 SuperBlock 位于磁盘上的扇区号，占据 0，1 两个扇区。 */
	static const int SUPER_BLOCK_SECTOR_NUMBER = 0; 

	/* 文件系统根目录外存 Inode 编号 */
	static const int ROOTINO = 0; 

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

private:
	/* FileSystem 类需要缓存管理模块(BufferManager)提供的接口 */
	BufManager* m_BufferManager; 

public:
	FileSystem ();
	~FileSystem ();

	
	//初始化成员变量
	void Initialize();
	
	//系统初始化时读入 SuperBlock
	void LoadSuperBlock();
	
	//根据文件存储设备的设备号 dev 获取该文件系统的 SuperBlock
	SuperBlock* GetFS();
	
	//将 SuperBlock 对象的内存副本更新到存储设备的 SuperBlock 中去
	void Update();
	
	//在存储设备 dev 上分配一个空闲外存 INode，一般用于创建新的文件
	Inode* IAlloc();
	
	//释放存储设备 dev 上编号为 number的外存 INode，一般用于删除文件
	void IFree(int number);
	
	//在存储设备 dev 上分配空闲磁盘块
	Buf* Alloc();
	
	//释放存储设备 dev 上编号为 blkno 的磁盘块
	void Free(int blkno);
};

FileSystem ::FileSystem ()
{
}

FileSystem ::~FileSystem ()
{
}