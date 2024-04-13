#include "Kernel.h"
using namespace std;


//��������������������ദ��
Kernel::Kernel() {};
Kernel::~Kernel() {};


//���¼�������ʵ�ּ�����ȡprivate��Ա�Ľӿ�
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
	//��ȡ��ǰ���̵� User �ṹ
	return *(this->m_User);
}


//���¼�������ʵ�ּ�����ȡprivate��Ա�Ľӿ�

void Kernel::InitBuffer() {

}

void Kernel::InitFileSystem() {

}

void Kernel::Initialize() {
	//��ʼ���Ĵ��
	InitBuffer();
	InitFileSystem();
	InitUser();

	//����ļ�����ʵ��
	FileManager& fileMgr = Kernel::Instance().GetFileManager();
	//��ø��ڵ�� Inode
	fileMgr.rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
	//�������־λ
	fileMgr.rootDirInode->i_flag &= (~Inode::ILOCK);
	//����
	pthread_mutex_unlock(&fileMgr.rootDirInode->mutex);
	//���س�����
	Kernel::Instance().GetFileSystem().LoadSuperBlock();
	User& usr = Kernel::Instance().GetUser();
	usr.u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);

	//�������־λ
	usr.u_cdir->i_flag &= (~Inode::ILOCK);

	//����
	pthread_mutex_unlock(&usr.u_cdir->mutex);

	//����·��
	strcpy(usr.u_curdir, "/");

	//��ʼ����Ŀ¼�ṹ
	Sys_ChDir(usr.u_curdir);
	Sys_Mkdir("bin");
	Sys_Mkdir("etc");
	Sys_Mkdir("dev");

	cout << "[Info] System initializing successfully!" << endl;
	return;
}