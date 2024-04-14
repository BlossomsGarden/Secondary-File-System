#define _CRT_SECURE_NO_WARNINGS
#include "Kernel.h"
#include <iostream>
using namespace std;


//��ǧ������˶�����Щstatic��������Ȼ���ϱ���
//LNK2001 �޷��������ⲿ���� "private: static class User User::instance" (?instance@User@@0V1@A)
Kernel Kernel::instance;
// ��̬����ʵ����
//  �漰������ʱ��Դ�����ͷţ����ʵ������˳���ܸ�
BufManager BufManager::instance;
User User::instance;
FileSystem FileSystem::instance;
FileManager FileManager::instance;


//��������������������ദ��
Kernel::Kernel() {};
Kernel::~Kernel() {};


//���¼�������ʵ�ּ�����ȡprivate��Ա�Ľӿ�
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
	//��ȡ��ǰ���̵� User �ṹ
	return *(this->m_User);
}




void Kernel::Initialize() {
	cout << "Initialize System..." << endl;

	//��ʼ�������
	InitBuffer();
	InitFileSystem();

	//����ļ�����ʵ��
	FileManager* fileManager = &Kernel::GetInstance().GetFileManager();

	//��ø��ڵ�� Inode
	fileManager->rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
	//�������־λ
	fileManager->rootDirInode->i_flag &= (~Inode::ILOCK);

	//���س�����
	Kernel::GetInstance().GetFileSystem().LoadSuperBlock();

	//�����û�
	User* u = &Kernel::GetInstance().GetUser();
	u->clear();
	u->u_cdir = g_InodeTable.IGet(FileSystem::ROOTINO);
	u->u_pdir = NULL;
	//�������־λ
	u->u_cdir->i_flag &= (~Inode::ILOCK);
	//����·��
	strcpy(u->u_curdir, "/");
	u->u_dirp = "/";
	memset(u->u_arg, 0, sizeof(u->u_arg));


	cout << "[Info] System initializing successfully!" << endl;
}



//���¼�������ʵ�ּ�����ȡprivate��Ա�Ľӿ�
void Kernel::InitBuffer() {
	this->m_BufManager = BufManager::GetInstance();

	cout << "Initialize Buffer..." << endl;
	this->m_BufManager->Initialize();
}
void Kernel::InitFileSystem() {

}
