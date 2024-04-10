#pragma once
#include "Inode.h"
#include "Buf.h"


class Inode {
public:
	unsigned int i_flag; /* ״̬�ı�־λ������� enum INodeFlag */
	unsigned int i_mode; /* �ļ�������ʽ��Ϣ */
	int i_count; /* ���ü��� */
	int i_nlink; /* �ļ���������������ļ���Ŀ¼���в�ͬ·���������� */
	short i_dev; /* ��� inode ���ڴ洢�豸���豸�� */
	int i_number; /* ��� inode ���еı�� */
	short i_uid; /* �ļ������ߵ��û���ʶ�� */
	short i_gid; /* �ļ������ߵ����ʶ�� */
	int i_size; /* �ļ���С���ֽ�Ϊ��λ */
	int i_addr[10]; /* �����ļ��߼���ź�������ת���Ļ��������� */
	int i_lastr; /* ������һ�ζ�ȡ�ļ����߼���ţ������ж��Ƿ���ҪԤ�� */

public:
	Inode();
	~Inode();
	
	//���� Inode �����е�������̿���������ȡ��Ӧ���ļ�����
	void ReadI();

	//���� Inode �����е�������̿�������������д���ļ�
	void WriteI();

	//���ļ����߼����ת���ɶ�Ӧ�������̿��
	int Bmap(int lbn);
	
	//������� Inode �����ķ���ʱ�䡢�޸�ʱ��
	void IUpdate(int time);
	
	//�ͷ� Inode ��Ӧ�ļ�ռ�õĴ��̿�
	void ITrunc();

	//��� Inode �����е�����
	void Clean();

	//��������� Inode �ַ�������Ϣ�������ڴ� Inode ��
	void ICopy(Buf* bp, int inumber);
};

Inode ::Inode ()
{
}

Inode ::~Inode ()
{
}
