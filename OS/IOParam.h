#pragma once
#include "IOParam.h"


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
	IOParam ();
	~IOParam ();
};

IOParam ::IOParam ()
{
}

IOParam ::~IOParam ()
{
}