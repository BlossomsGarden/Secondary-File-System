#pragma once
#include "BufManager.h"
#include "Buf.h"
#include "DiskDriver.h"
#include <mutex>

//单开一个类用以管理Buf类塑造出的缓存块与自由缓存队列
//包括分配、读写、回收等操作
class BufManager {
public:
	// BufManager 单体类实例
	static BufManager instance;

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


	//用以解决异步写和getblk进程冲突
	mutex buf_mutex[NBUF];

	//指向磁盘驱动模块全局对象
	//通过调用其接口在缓存块和虚拟磁盘文件之间进行读写
	DiskDriver* m_DiskDriver;

public:
	BufManager ();
	~BufManager ();

	static BufManager& GetInstance();

	//缓存控制块队列的初始化。将缓存控制块中 b_addr 指向相应缓冲区首地址
	void Initialize();

	//申请一块缓存，用于读写设备 dev 上的字符块blkno
	Buf* GetBlk(int dev, int blkno);

	//释放缓存控制块 buf，指把它放在自由缓存队列队尾去
	void Brelse(Buf* bp);

	//读一个磁盘块。dev 为主、次设备号，blkno 为目标磁盘块逻辑块号
	Buf* Bread(int blkno);

	//写一个磁盘块
	void Bwrite(Buf* bp);

	//异步写，完成后释放。在Bwrite中注册了线程来调用
	//对缓存块加锁，注意加锁的位置尽量覆盖对缓存的更改
	void Bawrite(Buf* bp);

	//延迟写磁盘块
	void Bdwrite(Buf* bp);

	//清空缓冲区内容
	void ClrBuf(Buf* bp);

	//将 dev 指定设备队列中延迟写的缓存全部输出到磁盘
	void Bflush();

	//获取自由缓存队列控制块 Buf 对象引用
	Buf& GetBFreeList();

	// 将缓存队列中的块取出，并设置BUSY标致
	void NotAvail(Buf *bp);
};


