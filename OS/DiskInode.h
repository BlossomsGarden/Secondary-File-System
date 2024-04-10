#pragma once
#include "DiskInode.h"

//������һ��Inode�������Ա�ʾ�ļ��洢�豸�����Inode��
//���Inode����ÿ���ļ�Ψһ��Ӧ����� Inode����¼���ļ���Ӧ�Ŀ�����Ϣ
class DiskInode {
public:
	//״̬�ı�־λ
	//����� enum INodeFlag
	unsigned int d_mode;
	//�ļ���������������ļ���Ŀ¼���в�ͬ·����������
	int d_nlink;
	//�ļ���С���ֽ�Ϊ��λ
	int d_size; 
	//�����ļ��߼���ź�������ת���Ļ���������
	int d_addr[10]; 

	int d_atime; /* ������ʱ�� */
	int d_mtime; /* ����޸�ʱ�� */

public:
	DiskInode();
	~DiskInode();
};

DiskInode::DiskInode()
{
}

DiskInode::~DiskInode()
{
}
