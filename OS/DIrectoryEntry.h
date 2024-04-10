#pragma once
#include "DIrectoryEntry.h"


//定义了目录项的结构(Inode编号+路径名组成)
//目录文件保存在磁盘上，一个盘块可保存 16 个目录项
//根据目录项的 Inode 编号找到目录中的文件
class DirectoryEntry {
public:
	//目录项中路径部分的最大字符串长度
	static const int DIRSIZ = 28;

	//目录项中 Inode 编号部分
	int m_ino;
	//目录项中路径名部分
	char m_name[DIRSIZ];

public:
	DirectoryEntry ();
	~DirectoryEntry ();
};

DirectoryEntry ::DirectoryEntry ()
{
}

DirectoryEntry ::~DirectoryEntry ()
{
}
