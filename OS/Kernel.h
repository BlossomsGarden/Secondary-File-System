#pragma once
#include "BufManager.h"
#include "FileManager.h"
#include "User.h"


//封装所有内核相关的全局类实例对象
//Kernel 类在内存中为单体模式，保证内核中封装各内核模块的对象都只有一个副本
class Kernel {
private:
	static Kernel instance; /* Kernel 单体类实例 */

	BufManager* m_BufManager;
	FileSystem* m_FileSystem;
	FileManager* m_FileManager;
	User* m_User;


	//磁盘镜像文件名
	static const char* DISK_FILE_NAME;
	//数据块大小为512字节
	static const int BLOCK_SIZE = 512;
	//磁盘镜像文件指针
	File* fp;

public:
	Kernel();
	~Kernel();

	static Kernel& GetInstance();

	/* 该函数完成初始化内核大部分数据结构的初始化 */
	void Initialize(); 

	BufManager& GetBufManager();
	FileManager& GetFileManager();
	FileSystem& GetFileSystem();
	User& GetUser(); /* 获取当前进程的 User 结构 */

private:
	void InitDiskDriver();
	void InitBuffer();
	void InitFileSystem();
};
