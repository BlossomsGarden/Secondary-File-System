#pragma once

class DirectoryEntry {
public:
	static const int DIRSIZ = 28;	/* 目录项中路径部分的最大字符串长度 */

	int m_ino;		    /* 目录项中INode编号部分 */
	char name[DIRSIZ];	/* 目录项中路径名部分 */
};
