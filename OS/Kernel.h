#pragma once
#include "Kernel.h"
#include "BufManager.h"
#include "FileManager.h"
#include "User.h"


//封装所有内核相关的全局类实例对象
//Kernel 类在内存中为单体模式，保证内核中封装各内核模块的对象都只有一个副本
class Kernel {
private:
	static Kernel instance; /* Kernel 单体类实例 */

	BufManager* m_BufferManager;
	FileSystem* m_FileSystem;
	FileManager* m_FileManager;
	DiskDriver* m_DiskDriver;
	User* m_User;


public:
	Kernel();
	~Kernel();

	static Kernel& Instance();

	/* 该函数完成初始化内核大部分数据结构的初始化 */
	void Initialize(); 

	BufManager& GetBufManager();
	FileManager& GetFileManager();
	FileSystem& GetFileSystem();
	DiskDriver& GetDiskDriver();
	User& GetUser(); /* 获取当前进程的 User 结构 */

private:
	void InitDiskDriver();
	void InitBuffer();
	void InitFileSystem();
};
