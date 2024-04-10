#pragma once
#include "IOParam.h"


//记录了对文件读、写时需用到的读写偏移量、字节数以及目标区域首地址参数
class IOParam {
public:
	//当前读、写用户目标区域的首地址
	unsigned char* m_Base;
	//当前读、写文件的字节偏移量
	int m_Offset;
	//当前还剩余的读、写字节数量
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