#pragma once
#include "Inode.h"
#include "Buf.h"


class Inode {
public:
	unsigned int i_flag; /* 状态的标志位，定义见 enum INodeFlag */
	unsigned int i_mode; /* 文件工作方式信息 */
	int i_count; /* 引用计数 */
	int i_nlink; /* 文件联结计数，即该文件在目录树中不同路径名的数量 */
	short i_dev; /* 外存 inode 所在存储设备的设备号 */
	int i_number; /* 外存 inode 区中的编号 */
	short i_uid; /* 文件所有者的用户标识数 */
	short i_gid; /* 文件所有者的组标识数 */
	int i_size; /* 文件大小，字节为单位 */
	int i_addr[10]; /* 用于文件逻辑块号和物理块号转换的基本索引表 */
	int i_lastr; /* 存放最近一次读取文件的逻辑块号，用于判断是否需要预读 */

public:
	Inode();
	~Inode();
	
	//根据 Inode 对象中的物理磁盘块索引表，读取相应的文件数据
	void ReadI();

	//根据 Inode 对象中的物理磁盘块索引表，将数据写入文件
	void WriteI();

	//将文件的逻辑块号转换成对应的物理盘块号
	int Bmap(int lbn);
	
	//更新外存 Inode 的最后的访问时间、修改时间
	void IUpdate(int time);
	
	//释放 Inode 对应文件占用的磁盘块
	void ITrunc();

	//清空 Inode 对象中的数据
	void Clean();

	//将包含外存 Inode 字符块中信息拷贝到内存 Inode 中
	void ICopy(Buf* bp, int inumber);
};

Inode ::Inode ()
{
}

Inode ::~Inode ()
{
}
