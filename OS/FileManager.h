#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "InodeTable.h"
#include "OpenFileTable.h"


//��װ�˶��ļ���ϵͳ���õĺ���̬������������ API ģ����е���
class FileManager {
public:
	/* Ŀ¼����ģʽ������ NameI()���� */
	enum DirectorySearchMode {
		OPEN = 0, /* �Դ��ļ���ʽ����Ŀ¼ */
		CREATE = 1, /* ���½��ļ���ʽ����Ŀ¼ */
		DELETE = 2 /* ��ɾ���ļ���ʽ����Ŀ¼ */
	};
	/* ��Ŀ¼�ڴ� Inode */
	Inode* rootDirInode;

	/* ��ȫ�ֶ��� g_FileSystem �����ã��ö���������ļ�ϵͳ�洢��Դ */
	FileSystem* m_FileSystem;

	/* ��ȫ�ֶ��� g_InodeTable �����ã��ö������ڴ� Inode ��Ĺ��� */
	InodeTable* m_InodeTable;

	/* ��ȫ�ֶ��� g_OpenFileTable �����ã��ö�������ļ�����Ĺ��� */
	OpenFileTable* m_OpenFileTable;


	static FileManager instance;

public:
	FileManager ();
	~FileManager ();

	static FileManager* GetInstance();

	
	//��ʼ����ȫ�ֶ��������
	void Initialize();
	
	//Open()ϵͳ���ô������
	void Open(const char* path);
	
	//Creat()ϵͳ���ô������
	int Creat(const char* path);
	
	//Open()��Creat()ϵͳ���õĹ�������
	void Open1(Inode* pInode, int mode, int trf);

	/*
   * Close()ϵͳ���ô������
   */
	void Close();

	/*
   * Seek()ϵͳ���ô������
   */
	void Seek();

	/*
   * FStat()��ȡ�ļ���Ϣ
   */
	void FStat();

	/*
   * FStat()��ȡ�ļ���Ϣ
   */
	void Stat();

	/* FStat()�� Stat()ϵͳ���õĹ������� */
	void Stat1(Inode* pInode, unsigned long statBuf);

	/*
   * Read()ϵͳ���ô������
   */
	int Read(int fd, unsigned char* buffer, int count);

	int Write(int fd, unsigned char* buffer, int count);

	/*
   * ��дϵͳ���ù������ִ���
   */
	int Rdwr(enum File::FileFlags mode, int fd, unsigned char* buffer, int count);

	/*
   * Ŀ¼��������·��ת��Ϊ��Ӧ�� Inode��
   * ����������� Inode
   */
	Inode* NameI(char (*func)(), enum DirectorySearchMode mode);

	/*
   * ��ȡ·���е���һ���ַ�
   */
	static char NextChar();

	/*
   * �� Creat()ϵͳ����ʹ�ã�����Ϊ�������ļ������ں���Դ
   */
	Inode* MakNode(unsigned int mode);

	/*
   * ��Ŀ¼��Ŀ¼�ļ�д��һ��Ŀ¼��
   */
	void WriteDir(Inode* pInode);

	/*
   * ���õ�ǰ����·��
   */
	void SetCurDir(const char* pathname);

	/* �ı䵱ǰ����Ŀ¼ */
	void ChDir(const char* path);

	/* ȡ���ļ� */
	void UnLink(const char* path);

	/* ���ڽ��������豸�ļ���ϵͳ���� */
	int MkNod(const char* path, int mode);
};


