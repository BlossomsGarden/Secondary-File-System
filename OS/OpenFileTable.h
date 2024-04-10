#pragma once
#include "OpenFileTable.h"
#include "File.h"

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

OpenFileTable ::OpenFileTable ()
{
}

OpenFileTable ::~OpenFileTable ()
{
}