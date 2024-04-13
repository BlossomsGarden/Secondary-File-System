#pragma once
#include "Buf.h"

class Buf {
public:
	//////////////////////////////////////
	//todo:为什么这里是乘以2叠加的？？？//
	////////////////////////////////////// 
	enum BufFlag {
		B_WRITE=0x1,
		B_READ=0x2,
		B_DONE=0x4,
		B_ERROR=0x8,
		B_BUSY=0x10,
		B_WANTED=0x20,
		B_ASYNC=0x40,
		B_DELWRI=0x80
	};

	//缓存控制块的标志位
	unsigned int b_flags;
	//4字节填充，作者说是使b_forw和b_back在Buf类中与Dev_tab类中的字段顺序能够一致
	//避免强转出错
	int padding;
	//缓存队列勾连指针
	Buf* b_forw;
	Buf* b_back;
	//传送的字节数
	int b_wcount;
	//该缓存控制块所管理的缓冲区首地址
	unsigned char* b_addr;
	//磁盘逻辑块号
	int b_blkno;

	//主、次设备号，其中高8位是主设备号，低8位是次设备号
	//似乎本作业中并没有什么卵用
	int b_dev;

	//Buffer在BufferManager的15个缓存中的序数
	int b_no;
};
