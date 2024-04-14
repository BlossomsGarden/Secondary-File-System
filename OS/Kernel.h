#pragma once
#include "BufManager.h"
#include "FileManager.h"
#include "User.h"


//��װ�����ں���ص�ȫ����ʵ������
//Kernel �����ڴ���Ϊ����ģʽ����֤�ں��з�װ���ں�ģ��Ķ���ֻ��һ������
class Kernel {
private:
	static Kernel instance; /* Kernel ������ʵ�� */

	BufManager* m_BufManager;
	FileSystem* m_FileSystem;
	FileManager* m_FileManager;
	User* m_User;


	//���̾����ļ���
	static const char* DISK_FILE_NAME;
	//���ݿ��СΪ512�ֽ�
	static const int BLOCK_SIZE = 512;
	//���̾����ļ�ָ��
	File* fp;

public:
	Kernel();
	~Kernel();

	static Kernel& GetInstance();

	/* �ú�����ɳ�ʼ���ں˴󲿷����ݽṹ�ĳ�ʼ�� */
	void Initialize(); 

	BufManager& GetBufManager();
	FileManager& GetFileManager();
	FileSystem& GetFileSystem();
	User& GetUser(); /* ��ȡ��ǰ���̵� User �ṹ */

private:
	void InitDiskDriver();
	void InitBuffer();
	void InitFileSystem();
};
