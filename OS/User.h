#pragma once
#include "Inode.h"
#include "FileManager.h"

class User {
public:
	enum ErrorCode {
		NOERROR = 0, /* No error */
		//EPERM = 1, /* Operation not permitted */
		ENOENT_ = 2, /* No such file or directory */
		//ESRCH = 3, /* No such process */
		//EINTR = 4, /* Interrupted system call */
		//EIO = 5, /* I/O error */
		//ENXIO = 6, /* No such device or address */
		//E2BIG = 7, /* Arg list too long */
		//ENOEXEC = 8, /* Exec format error */
		EBADF_ = 9, /* Bad file number */
		//ECHILD = 10, /* No child processes */
		//EAGAIN = 11, /* Try again */
		//ENOMEM = 12, /* Out of memory */
		EACCES_ = 13, /* Permission denied */
		//EFAULT = 14, /* Bad address */
		//ENOTBLK = 15, /* Block device required */
		//EBUSY = 16, /* Device or resource busy */
		//EEXIST = 17, /* File exists */
		//EXDEV = 18, /* Cross-device link */
		//ENODEV = 19, /* No such device */
		ENOTDIR_ = 20, /* Not a directory */
		EISDIR_ = 21, /* Is a directory */
		//EINVAL = 22, /* Invalid argument */
		ENFILE_ = 23, /* File table overflow */
		//EMFILE = 24, /* Too many open files */
		//ENOTTY = 25, /* Not a typewriter(terminal) */
		//ETXTBSY = 26, /* Text file busy */
		//EFBIG = 27, /* File too large */
		ENOSPC_ = 28, /* No space left on device */
		ESPIPE_ = 29, /* Illegal seek */
		EROFS_ = 30, /* Read-only file system */
		//EMLINK = 31, /* Too many links */
		//EPIPE = 32, /* Broken pipe */
		//ENOSYS = 100,
		//EFAULT = 106
	};


	/* 系统调用相关成员 */
	//由 u.u_ar0[EAX] 简化而来（因为static int EAX=0），存放系统调用返回值
	int u_ar0; 
	int u_arg[5]; /* 存放当前系统调用参数 */
	const char* u_dirp; /* 系统调用参数(一般用于 Pathname)的指针 */
	/* 文件系统相关成员 */
	Inode* u_cdir; /* 指向当前目录的 Inode 指针 */
	Inode* u_pdir; /* 指向父目录的 Inode 指针 */
	DirectoryEntry u_dent; /* 当前目录的目录项 */
	char u_dbuf[DirectoryEntry::DIRSIZ]; /* 当前路径分量 */
	char u_curdir[128]; /* 当前工作目录完整路径 */
	ErrorCode u_error; /* 存放错误码 */
	/* 文件系统相关成员 */
	OpenFiles u_ofiles; /* 进程打开文件描述符表对象 */
	/* 文件 I/O 操作 */
	IOParam u_IOParam; /* 记录当前读、写文件的偏移量，用户目标区域和剩余字节数参数 */

	int debug;

private:

	//仿照Kernel做的用户单体类实例
	static User instance;

public:
	static User* GetInstance();

	const char* Pwd();

	void clear();
};