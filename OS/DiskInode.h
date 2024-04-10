#pragma once
#include "DiskInode.h"

//单独拿一个Inode出来用以表示文件存储设备的外存Inode区
//外存Inode区中每个文件唯一对应的外存 Inode，记录该文件对应的控制信息
class DiskInode {
public:
	//状态的标志位
	//定义见 enum INodeFlag
	unsigned int d_mode;
	//文件联结计数，即该文件在目录树中不同路径名的数量
	int d_nlink;
	//文件大小，字节为单位
	int d_size; 
	//用于文件逻辑块号和物理块号转换的基本索引表
	int d_addr[10]; 

	int d_atime; /* 最后访问时间 */
	int d_mtime; /* 最后修改时间 */

public:
	DiskInode();
	~DiskInode();
};

DiskInode::DiskInode()
{
}

DiskInode::~DiskInode()
{
}
