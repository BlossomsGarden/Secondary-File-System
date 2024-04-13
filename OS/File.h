#pragma once
#include "File.h"
#include "Inode.h"

//��¼�˶��ļ�����дʱ���õ��Ķ�дƫ�������ֽ����Լ�Ŀ�������׵�ַ����
class IOParam {
public:
	//��ǰ����д�û�Ŀ��������׵�ַ
	unsigned char* m_Base;
	//��ǰ����д�ļ����ֽ�ƫ����
	int m_Offset;
	//��ǰ��ʣ��Ķ���д�ֽ�����
	int m_Count;

public:
	IOParam();
	~IOParam();
};




//��¼���̴��ļ��Ķ���д�������͡��ļ���дλ�õ�
class File{
public:
	enum FileFlags {
		//������
		FREAD = 0x1,
		//д����
		FWRITE = 0x2,
		//�ܵ�
		FPIPE=0x4
	};

	//���ļ��Ķ�д��������
	unsigned int f_flag;
	//���ø��ļ����ƿ�Ľ�������
	int f_count;
	//ָ����ļ����ڴ�Inodeָ��
	Inode* f_inode;
	//�ļ���дλ��ָ��
	int f_offset;


public:
	File();
	~File();

};




//OpenFiles ���ǽ��̵� u �ṹ�а�����һ������
// ά���˵�ǰ���̵����д��ļ�
class OpenFiles {
public:
	//��������򿪵�����ļ���
	static const int MAXOPENFILES = 15;

private:
	//File�����ָ�����飬ָ��ϵͳ���ļ����е�File����
	File* ProcessOpenFileTable[MAXOPENFILES];

public:
	OpenFiles();
	~OpenFiles();

	//���̴��ļ�ʱ��������ļ����з���һ�����б�����
	int AllocFreeSlot();

	//�����û�ϵͳ�����ṩ���ļ����������� fd���ҵ���Ӧ�Ĵ��ļ����ƿ� File �ṹ
	File* GetFileByFd(int fd);

	//Ϊ�ѷ��䵽�Ŀ��������� fd ���ѷ���Ĵ��ļ����п��� File ������������ϵ
	void SetFd2File(int fd, File* pFile);
};



