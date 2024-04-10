#pragma once
#include "BufManager.h"
#include "Buf.h"
#include "DiskDriver.h"

//单开一个类用以管理Buf类塑造出的缓存块与自由缓存队列
//包括分配、读写、回收等操作
class BufManager {
public:
	//缓存控制块、缓冲区的数量
	static const int NBUF = 15;
	//缓冲区大小。 以字节为单位
	static const int BUFFER_SIZE = 512;

	//自由缓存队列控制块
	Buf bFreeList;
	//缓存控制块数组
	Buf m_Buf[NBUF];
	//缓冲区数组
	unsigned char Buffer[NBUF][BUFFER_SIZE];

	//指向磁盘驱动模块全局对象
	//通过调用其接口在缓存块和虚拟磁盘文件之间进行读写
	DiskDriver* m_DiskDriver;

public:
	BufManager ();
	~BufManager ();

	//缓存控制块队列的初始化。将缓存控制块中 b_addr 指向相应缓冲区首地址
	void Initialize();

	//申请一块缓存，用于读写设备 dev 上的字符块blkno
	Buf* GetBlk(int blkno);

	//释放缓存控制块 buf
	void Brelse(Buf* bp);

	//读一个磁盘块。dev 为主、次设备号，blkno 为目标磁盘块逻辑块号
	Buf* Bread(int blkno);

	//写一个磁盘块
	void Bwrite(Buf* bp);

	//延迟写磁盘块
	void Bdwrite(Buf* bp);

	//清空缓冲区内容
	void ClrBuf(Buf* bp);

	//将 dev 指定设备队列中延迟写的缓存全部输出到磁盘
	void Bflush();

	//获取自由缓存队列控制块 Buf 对象引用
	Buf& GetBFreeList();
};

BufManager ::BufManager ()
{
}

BufManager ::~BufManager ()
{
}

