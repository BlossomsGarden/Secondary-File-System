#pragma once
#include "File.h"
#include "Inode.h"

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

File::File()
{
}

File::~File()
{
}
