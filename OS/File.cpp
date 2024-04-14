#include "File.h"
#include "User.h"


//---------------------------------IOParam类--------------------------------//
//记录了对文件读、写时需用到的读写偏移量、字节数以及目标区域首地址参数
IOParam::IOParam() {
	IOParam::m_Base = 0;
	IOParam::m_Count = 0;
	IOParam::m_Offset = 0;
};
IOParam::~IOParam() {};



//----------------------------------File类----------------------------------//
//记录进程打开文件的读、写请求类型、文件读写位置等属性
File::File(){
	File::f_count = 0;
	File::f_flag = 0;
	File::f_offset = 0;
	File::f_inode = nullptr;
}
File::~File(){}



//-------------------------------OpenFiles类--------------------------------//
//维护进程中的所有打开文件
OpenFiles::OpenFiles(){
	for (int i = 0; i < OpenFiles::MAXOPENFILES; i++){
		ProcessOpenFileTable[i] = nullptr;
	}
}
OpenFiles::~OpenFiles(){}


//进程打开文件时，在其OpenFileTable中分配一个空闲表项插槽
int OpenFiles::AllocFreeSlot(){
	int i;
	User& u = *User::GetInstance();

	for (i = 0; i < OpenFiles::MAXOPENFILES; i++){
		if (this->ProcessOpenFileTable[i] == nullptr){
			/*一定要加！不然外层函数无法返回fd*/
			//u.u_ar0[EAX]窜访访问现场保护区中 EAX 寄存器的偏移量
			//这里直接用u.u_ar0替代了
			u.u_ar0 = i;
			return i;
		}
	}
	return -1;
}


//根据用户系统调用提供的文件描述符参数fd，找到对应的打开文件控制块File结构
File* OpenFiles::GetFileByFd(int fd){
	File* pFile;
	User& u = *User::GetInstance();

	//如果打开文件描述符的值超出了范围
	if (fd < 0 || fd >= OpenFiles::MAXOPENFILES){
		u.u_error = User::EBADF_;
		return nullptr;
	}

	pFile = this->ProcessOpenFileTable[fd];
	if (pFile == nullptr){
		u.u_error = User::EBADF_;
	}

	//即使pFile == NULL也返回它，由调用GetF的函数来判断返回值
	return pFile;
}


/*将OpenFileTable中第fd条表项指向pFile*/
void OpenFiles::SetFd2File(int fd, File* pFile){
	if (fd < 0 || fd >= OpenFiles::MAXOPENFILES){
		return;
	}
	/* 打开文件描述符 = 相应的File结构*/
	this->ProcessOpenFileTable[fd] = pFile;
	return;
}

