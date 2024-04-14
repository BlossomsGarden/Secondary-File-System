#define _CRT_SECURE_NO_WARNINGS
#include "Kernel.h"
#include <iostream>
using namespace std;


//你千万别忘了定义那些static变量，不然马上报错
//LNK2001 无法解析的外部符号 "private: static class User User::instance" (?instance@User@@0V1@A)
Kernel Kernel::instance;
// 静态对象实例化
//  涉及到析构时资源有序释放，因此实例化的顺序不能改
BufManager BufManager::instance;
User User::instance;
FileSystem FileSystem::instance;
FileManager FileManager::instance;


//构造和析构函数不做过多处理
Kernel::Kernel() {};
Kernel::~Kernel() {};


//以下几个函数实现几个获取private成员的接口
Kernel& Kernel::GetInstance() {
	return Kernel::instance;
}
BufManager& Kernel::GetBufManager() {
	return *(this->m_BufManager);
}
FileManager& Kernel::GetFileManager() {
	return *(this->m_FileManager);
}
FileSystem& Kernel::GetFileSystem() {
	return *(this->m_FileSystem);
}
User& Kernel::GetUser() {
	//获取当前进程的 User 结构
	return *(this->m_User);
}




void Kernel::Initialize() {
	cout << "Initialize System..." << endl;

	//初始化几大件
	InitBuffer();
	InitFileSystem();

	//获得文件管理实例
	FileManager* fileManager = &Kernel::GetInstance().GetFileManager();

	//获得根节点的 Inode
	fileManager->rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
	//清除锁标志位
	fileManager->rootDirInode->i_flag &= (~Inode::ILOCK);

	//加载超级块
	Kernel::GetInstance().GetFileSystem().LoadSuperBlock();

	//加载用户
	User* u = &Kernel::GetInstance().GetUser();
	u->clear();
	u->u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);
	u->u_pdir = NULL;
	//清除锁标志位
	u->u_cdir->i_flag &= (~Inode::ILOCK);
	//设置路径
	strcpy(u->u_curdir, "/");
	u->u_dirp = "/";
	memset(u->u_arg, 0, sizeof(u->u_arg));


	cout << "[Info] System initializing successfully!" << endl;
}



//以下几个函数实现几个获取private成员的接口
void Kernel::InitBuffer() {
	this->m_BufManager = BufManager::GetInstance();

	cout << "Initialize Buffer..." << endl;
	this->m_BufManager->Initialize();
}
void Kernel::InitFileSystem() {

}
