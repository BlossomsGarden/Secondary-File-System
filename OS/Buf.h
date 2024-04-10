#pragma once
#include "Buf.h"

class Buf {
public:
	/////////////////////////////////
	//为什么这里是乘以2叠加的？？？//
	/////////////////////////////////
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
};
