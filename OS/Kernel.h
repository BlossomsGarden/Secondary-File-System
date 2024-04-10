#pragma once
#include "Kernel.h"
#include "BufManager.h"
#include "FileManager.h"
#include "User.h"


//��װ�����ں���ص�ȫ����ʵ������
//Kernel �����ڴ���Ϊ����ģʽ����֤�ں��з�װ���ں�ģ��Ķ���ֻ��һ������
class Kernel {
public:
	Kernel();
	~Kernel();

	static Kernel& Instance();
	void Initialize(); /* �ú�����ɳ�ʼ���ں˴󲿷����ݽṹ�ĳ�ʼ�� */
	BufManager& GetBufferManager();
	FileSystem& GetFileSystem();
	FileManager& GetFileManager();
	DiskDriver& GetDiskDriver();
	User& GetUser(); /* ��ȡ��ǰ���̵� User �ṹ */

private:
	void InitDiskDriver();
	void InitBuffer();
	void InitFileSystem();

private:
	static Kernel instance; /* Kernel ������ʵ�� */
	BufManager* m_BufferManager;
	FileSystem* m_FileSystem;
	FileManager* m_FileManager;
	DiskDriver* m_DiskDriver;
	User* m_User;
};
