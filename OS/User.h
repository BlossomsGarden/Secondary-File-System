#pragma once
#include "User.h"
#include "IOParam.h"
#include "Inode.h"
#include "DIrectoryEntry.h"
#include "OpenFiles.h"

class User {
public:
	enum ErrorCode {
		NOERROR = 0, /* No error */
		EPERM = 1, /* Operation not permitted */
		ENOENT = 2, /* No such file or directory */
		ESRCH = 3, /* No such process */
		EINTR = 4, /* Interrupted system call */
		EIO = 5, /* I/O error */
		ENXIO = 6, /* No such device or address */
		E2BIG = 7, /* Arg list too long */
		ENOEXEC = 8, /* Exec format error */
		EBADF = 9, /* Bad file number */
		ECHILD = 10, /* No child processes */
		EAGAIN = 11, /* Try again */
		ENOMEM = 12, /* Out of memory */
		EACCES = 13, /* Permission denied */
		EFAULT = 14, /* Bad address */
		ENOTBLK = 15, /* Block device required */
		EBUSY = 16, /* Device or resource busy */
		EEXIST = 17, /* File exists */
		EXDEV = 18, /* Cross-device link */
		ENODEV = 19, /* No such device */
		ENOTDIR = 20, /* Not a directory */
		EISDIR = 21, /* Is a directory */
		EINVAL = 22, /* Invalid argument */
		ENFILE = 23, /* File table overflow */
		EMFILE = 24, /* Too many open files */
		ENOTTY = 25, /* Not a typewriter(terminal) */
		ETXTBSY = 26, /* Text file busy */
		EFBIG = 27, /* File too large */
		ENOSPC = 28, /* No space left on device */
		ESPIPE = 29, /* Illegal seek */
		EROFS = 30, /* Read-only file system */
		EMLINK = 31, /* Too many links */
		EPIPE = 32, /* Broken pipe */
		ENOSYS = 100,
		//EFAULT = 106
	};

	/* ϵͳ������س�Ա */
	int u_ar0; //�Ѽ�,���ϵͳ���÷���ֵ
	int u_arg[5]; /* ��ŵ�ǰϵͳ���ò��� */
	char* u_dirp; /* ϵͳ���ò���(һ������ Pathname)��ָ�� */
	/* �ļ�ϵͳ��س�Ա */
	Inode* u_cdir; /* ָ��ǰĿ¼�� Inode ָ�� */
	Inode* u_pdir; /* ָ��Ŀ¼�� Inode ָ�� */
	DirectoryEntry u_dent; /* ��ǰĿ¼��Ŀ¼�� */
	char u_dbuf[DirectoryEntry::DIRSIZ]; /* ��ǰ·������ */
	char u_curdir[128]; /* ��ǰ����Ŀ¼����·�� */
	ErrorCode u_error; /* ��Ŵ����� */
	/* �ļ�ϵͳ��س�Ա */
	OpenFiles u_ofiles; /* ���̴��ļ������������ */
	/* �ļ� I/O ���� */
	IOParam u_IOParam; /* ��¼��ǰ����д�ļ���ƫ�������û�Ŀ�������ʣ��
	�ֽ������� */
};