#pragma once
#include "DiskDriver.h"
#include "File.h"
#include "Buf.h"


class DiskDriver {
private:
	//���̾����ļ���
	static const char* DISK_FILE_NAME;
	//���ݿ��СΪ512�ֽ�
	static const int BLOCK_SIZE = 512;
	//���̾����ļ�ָ��
	File* fp;

public:
	DiskDriver();
	~DiskDriver();
	
	//��ʼ�����̾���
	void initialize();

	//����IO�������ж�д
	void IO(Buf* bp);
};

DiskDriver::DiskDriver()
{
}

DiskDriver::~DiskDriver()
{
}
