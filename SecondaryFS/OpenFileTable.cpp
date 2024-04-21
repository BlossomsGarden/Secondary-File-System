#include "OpenFileTable.h"
#include "User.h"
#include "Common.h"
#include <iostream>

OpenFileTable::OpenFileTable(){}

OpenFileTable::~OpenFileTable(){}


/*作用：进程打开文件描述符表中找的空闲项  之 下标  写入 u_ar0[EAX]*/
File* OpenFileTable::FAlloc(){
	int fd;
	User& u = globalUser;

	/* 在进程打开文件描述符表中获取一个空闲项 */
	fd = u.u_ofiles.AllocFreeSlot();

	if (fd < 0)	/* 如果寻找空闲项失败 */
	{
		return NULL;
	}

	for (int i = 0; i < OpenFileTable::NFILE; i++)
	{
		/* f_count==0表示该项空闲 */
		if (this->m_File[i].f_count == 0)
		{
			/* 建立描述符和File结构的勾连关系 */
			u.u_ofiles.SetF(fd, &this->m_File[i]);
			/* 增加对file结构的引用计数 */
			this->m_File[i].f_count++;
			/* 清空文件读、写位置 */
			this->m_File[i].f_offset = 0;
			return (&this->m_File[i]);
		}
	}

	u.u_error = User::myENFILE;
	return NULL;
}

void OpenFileTable::CloseF(File* pFile)
{
	/* 引用当前File的进程数减1 */
	pFile->f_count--;
	if (pFile->f_count <= 0) {
		globalINodeTable.IPut(pFile->f_inode);
	}
}

void OpenFileTable::Format(){
	File emptyFile;
	for (int i = 0; i < OpenFileTable::NFILE; ++i) {
		memcpy(m_File + i, &emptyFile, sizeof(File));
	}
}
