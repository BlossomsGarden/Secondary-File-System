#pragma once
#include "IOParam.h"
#include "DirectoryEntry.h"
#include "FileManager.h"
#include <string>
using namespace std;


/*
 * @comment 该类与Unixv6中 struct user结构对应，因此只改变
 * 类名，不修改成员结构名字，关于数据类型的对应关系如下:
 */
class User{
public:
	static const int EAX = 0;	/* u.u_ar0[EAX]；访问现场保护区中EAX寄存器的偏移量 */

	static const int MAX_FILE_LENGTH = 5 * 1024 * 1024;		/* 最大文件长度为5M */

	/* u_error's Error Code */
	enum ErrorCode{
		myNOERROR = 0,	/* No error */
		myEPERM = 1,	/* Operation not permitted */
		myENOENT = 2,	/* No such file or directory */
		myESRCH = 3,	/* No such process */
		myEINTR = 4,	/* Interrupted system call */
		myEIO = 5,	/* I/O error */
		myENXIO = 6,	/* No such device or address */
		myE2BIG = 7,	/* Arg list too long */
		myENOEXEC = 8,	/* Exec format error */
		myEBADF = 9,	/* Bad file number */
		myECHILD = 10,	/* No child processes */
		myEAGAIN = 11,	/* Try again */
		myENOMEM = 12,	/* Out of memory */
		myEACCES = 13,	/* Permission denied */
		myEFAULT = 14,	/* Bad address */
		myENOTBLK = 15,	/* Block device required */
		myEBUSY = 16,	/* Device or resource busy */
		myEEXIST = 17,	/* File exists */
		myEXDEV = 18,	/* Cross-device link */
		myENODEV = 19,	/* No such device */
		myENOTDIR = 20,	/* Not a directory */
		myEISDIR = 21,	/* Is a directory */
		myEINVAL = 22,	/* Invalid argument */
		myENFILE = 23,	/* File table overflow */
		myEMFILE = 24,	/* Too many open files */
		myENOTTY = 25,	/* Not a typewriter(terminal) */
		myETXTBSY = 26,	/* Text file busy */
		myEFBIG = 27,	/* File too large */
		myENOSPC = 28,	/* No space left on device */
		myESPIPE = 29,	/* Illegal seek */
		myEROFS = 30,	/* Read-only file system */
		myEMLINK = 31,	/* Too many links */
		myEPIPE = 32,	/* Broken pipe */
		myENOSYS = 100,
		//myEFAULT	= 106
	};

	unsigned long u_rsav[2];	/* 用于保存esp与ebp指针 */
	unsigned long u_ssav[2];	/* 用于对esp和ebp指针的二次保护 */

	/* 系统调用相关成员 */
	unsigned int u_ar0[1024];		/* 指向核心栈现场保护区中EAX寄存器
								存放的栈单元，本字段存放该栈单元的地址。
								在V6中r0存放系统调用的返回值给用户程序，
								x86平台上使用EAX存放返回值，替代u.u_ar0[R0] */

	int u_arg[5];				/* 存放当前系统调用参数 */
	string u_dirp;				/* 系统调用参数(一般用于Pathname)的指针 */


	/* 文件系统相关成员 */
	Inode* u_cdir;		/* 指向当前目录的Inode指针 */
	Inode* u_pdir;		/* 指向父目录的Inode指针 */

	DirectoryEntry u_dent;					/* 当前目录的目录项 */
	char u_dbuf[DirectoryEntry::DIRSIZ];	/* 当前路径分量，长度应当为 DirectoryEntry::DIRSIZ */
	string u_curdir;						/* 当前工作目录完整路径 */

	ErrorCode u_error;			/* 存放错误码 */


	/* 文件系统相关成员 */
	OpenFiles u_ofiles;		/* 进程打开文件描述符表对象 */

	/* 文件I/O操作 */
	IOParameter u_IOParam;	/* 记录当前读、写文件的偏移量，用户目标区域和剩余字节数参数 */

	FileManager* u_FileManager;
	string ls;


public:
	User();
	~User();

	void Cd(string dirName);
	void Mkdir(string dirName);
	void Create(string fileName, string mode);
	void Delete(string fileName);
	void Open(string fileName, string mode);
	void Close(string fd);
	void Seek(string fd, string offset, string origin);
	void Ls();
	void Update();

	void Read(string sfd, string size);
	void Write(string sfd, string inputStream, string size);
	void FileIn(string exName, string inName);
	void FileOut(string inName, string exName);

	bool IsError();
	void EchoError(enum ErrorCode err);
	int INodeMode(string mode);
	int FileMode(string mode);
	bool checkPathName(string path);
};

