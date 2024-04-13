#include "Kernel.h"
using namespace std;


//构造和析构函数不做过多处理
Kernel::Kernel() {};
Kernel::~Kernel() {};


//以下几个函数实现几个获取private成员的接口
Kernel& Kernel::Instance() {
	return Kernel::instance;
}
BufManager& Kernel::GetBufManager() {
	return *(this->m_BufferManager);
}
FileManager& Kernel::GetFileManager() {
	return *(this->m_FileManager);
}
FileSystem& Kernel::GetFileSystem() {
	return *(this->m_FileSystem);
}
DiskDriver& Kernel::GetDiskDriver() {
	return *(this->m_DiskDriver);
}
User& Kernel::GetUser() {
	//获取当前进程的 User 结构
	return *(this->m_User);
}


//以下几个函数实现几个获取private成员的接口

void Kernel::InitBuffer() {

}

void Kernel::InitFileSystem() {

}

void Kernel::Initialize() {
	//初始化四大件
	InitBuffer();
	InitFileSystem();
	InitUser();

	//获得文件管理实例
	FileManager& fileMgr = Kernel::Instance().GetFileManager();
	//获得根节点的 Inode
	fileMgr.rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
	//清除锁标志位
	fileMgr.rootDirInode->i_flag &= (~Inode::ILOCK);
	//解锁
	pthread_mutex_unlock(&fileMgr.rootDirInode->mutex);
	//加载超级块
	Kernel::Instance().GetFileSystem().LoadSuperBlock();
	User& usr = Kernel::Instance().GetUser();
	usr.u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);

	//清除锁标志位
	usr.u_cdir->i_flag &= (~Inode::ILOCK);

	//解锁
	pthread_mutex_unlock(&usr.u_cdir->mutex);

	//设置路径
	strcpy(usr.u_curdir, "/");

	//初始化根目录结构
	Sys_ChDir(usr.u_curdir);
	Sys_Mkdir("bin");
	Sys_Mkdir("etc");
	Sys_Mkdir("dev");

	cout << "[Info] System initializing successfully!" << endl;
	return;
}