#pragma once
#include "Buf.h"

class Buf {
public:
	/////////////////////////////////
	//Ϊʲô�����ǳ���2���ӵģ�����//
	/////////////////////////////////
	enum BufFlag {
		B_WRITE=0x1,
		B_READ=0x2,
		B_DONE=0x4,
		B_ERROR=0x8,
		B_BUSY=0x10,
		B_WANTED=0x20,
		B_ASYNC=0x40,
		B_DELWRI=0x80
	};

	//������ƿ�ı�־λ
	unsigned int b_flags;
	//4�ֽ���䣬����˵��ʹb_forw��b_back��Buf������Dev_tab���е��ֶ�˳���ܹ�һ��
	//����ǿת����
	int padding;
	//������й���ָ��
	Buf* b_forw;
	Buf* b_back;
	//���͵��ֽ���
	int b_wcount;
	//�û�����ƿ�������Ļ������׵�ַ
	unsigned char* b_addr;
	//�����߼����
	int b_blkno;
};
