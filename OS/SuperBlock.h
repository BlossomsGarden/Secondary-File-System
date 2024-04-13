#pragma once
#include "SuperBlock.h"

class SuperBlock {
public:
	int s_isize; /* ��� Inode ��ռ�õ��̿��� */
	int s_fsize; /* �̿����� */
	int s_nfree; /* ֱ�ӹ���Ŀ����̿����� */
	int s_free[100]; /* ֱ�ӹ���Ŀ����̿������� */
	int s_ninode; /* ֱ�ӹ���Ŀ������ Inode ���� */
	int s_inode[100]; /* ֱ�ӹ���Ŀ������ Inode ������ */
	int s_flock; /* ���������̿��������־ */
	int s_ilock; /* �������� Inode ���־ */
	int s_fmod; /* �ڴ��� super block �������޸ı�־����ζ����Ҫ��������
   Ӧ�� Super Block */
	int s_ronly; /* ���ļ�ϵͳֻ�ܶ��� */
	int s_time; /* ���һ�θ���ʱ�� */
	int padding[47]; /* ���ʹ SuperBlock ���С���� 1024 �ֽڣ�ռ�� 2 ������ */

public:
	SuperBlock(){};
	~SuperBlock(){};
};
