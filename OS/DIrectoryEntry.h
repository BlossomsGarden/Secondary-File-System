#pragma once
#include "DIrectoryEntry.h"


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
	DirectoryEntry ();
	~DirectoryEntry ();
};

DirectoryEntry ::DirectoryEntry ()
{
}

DirectoryEntry ::~DirectoryEntry ()
{
}
