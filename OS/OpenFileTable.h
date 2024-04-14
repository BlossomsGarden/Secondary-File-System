#pragma once
#include "File.h"


class OpenFileTable;

//��һ��ȫ�ִ��ļ���
extern OpenFileTable g_OpenFileTable;


//���̵Ĵ��ļ��ṹ
class OpenFileTable {
public:
	static const int NFILE = 100; /* ���ļ����ƿ� File �ṹ������ */

public:
	OpenFileTable ();
	~OpenFileTable ();

	//��ϵͳ���ļ����з���һ�����е� File �ṹ
	File* FAlloc();
	
	//�Դ��ļ����ƿ� File �ṹ�����ü��� f_count �� 1,�����ü��� f_count Ϊ 0�����ͷ� File �ṹ��
	void CloseF(File* pFile);

	//ϵͳ���ļ���Ϊ���н��̹������̴��ļ����������а���ָ����ļ����ж�Ӧ File �ṹ��ָ��
	File m_File[NFILE];
};


//������Ŀ¼��Ľṹ(Inode���+·�������)
//Ŀ¼�ļ������ڴ����ϣ�һ���̿�ɱ��� 16 ��Ŀ¼��
//����Ŀ¼��� Inode ����ҵ�Ŀ¼�е��ļ�
class DirectoryEntry {
public:
	//Ŀ¼����·�����ֵ�����ַ�������
	static const int DIRSIZ = 28;

	//Ŀ¼���� Inode ��Ų���
	int m_ino;
	//Ŀ¼����·��������
	char m_name[DIRSIZ];

public:
	DirectoryEntry() {};
	~DirectoryEntry() {};
};