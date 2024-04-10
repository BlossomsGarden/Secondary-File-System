#pragma once
#include "OpenFiles.h"
#include "File.h"

class OpenFiles {
public:
	//��������򿪵�����ļ���
	static const int NOFILES = 15;

private:
	//File�����ָ�����飬ָ��ϵͳ���ļ����е�File����
	File* ProcessOpenFileTable[NOFILES];

public:
	OpenFiles ();
	~OpenFiles ();

	//����������ļ�ʱ���ڴ��ļ����������з���һ�����б���
	int AllocFreeSlot();

	//�����û�ϵͳ�����ṩ���ļ����������� fd���ҵ���Ӧ�Ĵ��ļ����ƿ� File �ṹ
	File* GetF(int fd);

	//Ϊ�ѷ��䵽�Ŀ��������� fd ���ѷ���Ĵ��ļ����п��� File ������������ϵ
	void SetF(int fd, File* pFile);
};

OpenFiles ::OpenFiles ()
{
}

OpenFiles ::~OpenFiles ()
{
}
