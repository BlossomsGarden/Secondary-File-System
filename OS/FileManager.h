#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "InodeTable.h"
#include "OpenFileTable.h"


//封装了对文件的系统调用的核心态操作，供顶层 API 模块进行调用
class FileManager {
public:
	/* 目录搜索模式，用于 NameI()函数 */
	enum DirectorySearchMode {
		OPEN = 0, /* 以打开文件方式搜索目录 */
		CREATE = 1, /* 以新建文件方式搜索目录 */
		DELETE = 2 /* 以删除文件方式搜索目录 */
	};
	/* 根目录内存 Inode */
	Inode* rootDirInode;

	/* 对全局对象 g_FileSystem 的引用，该对象负责管理文件系统存储资源 */
	FileSystem* m_FileSystem;

	/* 对全局对象 g_InodeTable 的引用，该对象负责内存 Inode 表的管理 */
	InodeTable* m_InodeTable;

	/* 对全局对象 g_OpenFileTable 的引用，该对象负责打开文件表项的管理 */
	OpenFileTable* m_OpenFileTable;


	static FileManager instance;

public:
	FileManager ();
	~FileManager ();

	static FileManager* GetInstance();

	
	//初始化对全局对象的引用
	void Initialize();
	
	//Open()系统调用处理过程
	void Open(const char* path);
	
	//Creat()系统调用处理过程
	int Creat(const char* path);
	
	//Open()、Creat()系统调用的公共部分
	void Open1(Inode* pInode, int mode, int trf);

	/*
   * Close()系统调用处理过程
   */
	void Close();

	/*
   * Seek()系统调用处理过程
   */
	void Seek();

	/*
   * FStat()获取文件信息
   */
	void FStat();

	/*
   * FStat()获取文件信息
   */
	void Stat();

	/* FStat()和 Stat()系统调用的共享例程 */
	void Stat1(Inode* pInode, unsigned long statBuf);

	/*
   * Read()系统调用处理过程
   */
	int Read(int fd, unsigned char* buffer, int count);

	int Write(int fd, unsigned char* buffer, int count);

	/*
   * 读写系统调用公共部分代码
   */
	int Rdwr(enum File::FileFlags mode, int fd, unsigned char* buffer, int count);

	/*
   * 目录搜索，将路径转化为相应的 Inode，
   * 返回上锁后的 Inode
   */
	Inode* NameI(char (*func)(), enum DirectorySearchMode mode);

	/*
   * 获取路径中的下一个字符
   */
	static char NextChar();

	/*
   * 被 Creat()系统调用使用，用于为创建新文件分配内核资源
   */
	Inode* MakNode(unsigned int mode);

	/*
   * 向父目录的目录文件写入一个目录项
   */
	void WriteDir(Inode* pInode);

	/*
   * 设置当前工作路径
   */
	void SetCurDir(const char* pathname);

	/* 改变当前工作目录 */
	void ChDir(const char* path);

	/* 取消文件 */
	void UnLink(const char* path);

	/* 用于建立特殊设备文件的系统调用 */
	int MkNod(const char* path, int mode);
};


