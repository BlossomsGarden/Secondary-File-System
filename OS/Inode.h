#pragma once
#include "Inode.h"
#include "Buf.h"


/*========================================== Inode =========================================*/
class Inode {
public:
	/* i_flag 中标志位 */
	enum INodeFlag
	{
		ILOCK = 0x1, /* 索引节点上锁 */
		IUPD = 0x2, /* 内存 inode 被修改过，需要更新相应外存 inode */
		IACC = 0x4, /* 内存 inode 被访问过，需要修改最近一次访问时间 */
		IMOUNT = 0x8, /* 内存 inode 用于挂载子文件系统 */
		IWANT = 0x10, /* 有进程正在等待该内存 inode 被解锁，清 ILOCK 标志时，要唤
	   醒这种进程 */
	   ITEXT = 0x20 /* 内存 inode 对应进程图像的正文段 */
	};

	/* static const member */
	static const unsigned int IALLOC = 0x8000; /* 文件被使用 */
	static const unsigned int IFMT = 0x6000; /* 文件类型掩码 */
	static const unsigned int IFDIR = 0x4000; /* 文件类型：目录文件 */
	static const unsigned int IFCHR = 0x2000; /* 字符设备特殊类型文件 */
	static const unsigned int IFBLK = 0x6000; /* 块设备特殊类型文件，为 0 表示常规数据文件 */
	static const unsigned int ILARG = 0x1000; /* 文件长度类型：大型或巨型文件 */
	static const unsigned int ISUID = 0x800; /* 执行时文件时将用户的有效用户 ID 修改为文件所有者的 User ID */
	static const unsigned int ISGID = 0x400; /* 执行时文件时将用户的有效组 ID 修改为文件所有者的 Group ID */
	static const unsigned int ISVTX = 0x200; /* 使用后仍然位于交换区上的正文段 */
	static const unsigned int IREAD = 0x100; /* 对文件的读权限 */
	static const unsigned int IWRITE = 0x80; /* 对文件的写权限 */
	static const unsigned int IEXEC = 0x40; /* 对文件的执行权限 */
	static const unsigned int IRWXU = (IREAD | IWRITE | IEXEC); /* 文件主对文件的读、写、执行权限 */
	static const unsigned int IRWXG = ((IRWXU) >> 3); /* 文件主同组用户对文件的读、写、执行权限 */
	static const unsigned int IRWXO = ((IRWXU) >> 6); /* 其他用户对文件的读、写、执行权限 */

	static const int BLOCK_SIZE = 512; /*文件逻辑块大小: 512 字节 */
	static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);
	/* 每个间接索引表(或索引块)包含的物理盘块号 */
	static const int SMALL_FILE_BLOCK = 6; /* 小型文件：直接索引表最多可寻址的逻辑块号 */
	static const int LARGE_FILE_BLOCK = 128 * 2 + 6; /* 大型文件：经一次间接索引表最多可寻址的逻辑块号 */
	static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6; /* 巨型文件：经二次间接索引最大可寻址文件逻辑块号 */
	static const int PIPSIZ = SMALL_FILE_BLOCK * BLOCK_SIZE;


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

	//顺序读时，预读读入文件的下一字符块，rablock记录下一逻辑块号
	//经过bmap转换得到的物理盘块号
	//将rablock作为静态变量的原因：调用一次bmap的开销
	//对当前块和预读块的逻辑块号进行转换，bmap返回当前块的物理盘块号
	//并且将预读块的物理盘块号保存在rablock中。
	static int rablock;

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



/*======================================== DiskInode ======================================*/
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
