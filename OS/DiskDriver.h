#pragma once
#include "DiskDriver.h"
#include "File.h"
#include "Buf.h"


class DiskDriver {
private:
	//磁盘镜像文件名
	static const char* DISK_FILE_NAME;
	//数据块大小为512字节
	static const int BLOCK_SIZE = 512;
	//磁盘镜像文件指针
	File* fp;

public:
	DiskDriver();
	~DiskDriver();
	
	//初始化磁盘镜像
	void initialize();

	//根据IO请求块进行读写
	void IO(Buf* bp);
};

DiskDriver::DiskDriver()
{
}

DiskDriver::~DiskDriver()
{
}
