#include "File.h"
#include "User.h"


//---------------------------------IOParam��--------------------------------//
//��¼�˶��ļ�����дʱ���õ��Ķ�дƫ�������ֽ����Լ�Ŀ�������׵�ַ����
IOParam::IOParam() {
	IOParam::m_Base = 0;
	IOParam::m_Count = 0;
	IOParam::m_Offset = 0;
};
IOParam::~IOParam() {};



//----------------------------------File��----------------------------------//
//��¼���̴��ļ��Ķ���д�������͡��ļ���дλ�õ�����
File::File(){
	File::f_count = 0;
	File::f_flag = 0;
	File::f_offset = 0;
	File::f_inode = nullptr;
}
File::~File(){}



//-------------------------------OpenFiles��--------------------------------//
//ά�������е����д��ļ�
OpenFiles::OpenFiles(){
	for (int i = 0; i < OpenFiles::MAXOPENFILES; i++){
		ProcessOpenFileTable[i] = nullptr;
	}
}
OpenFiles::~OpenFiles(){}


//���̴��ļ�ʱ������OpenFileTable�з���һ�����б�����
int OpenFiles::AllocFreeSlot(){
	int i;
	User& u = *User::GetInstance();

	for (i = 0; i < OpenFiles::MAXOPENFILES; i++){
		if (this->ProcessOpenFileTable[i] == nullptr){
			/*һ��Ҫ�ӣ���Ȼ��㺯���޷�����fd*/
			//u.u_ar0[EAX]�ܷ÷����ֳ��������� EAX �Ĵ�����ƫ����
			//����ֱ����u.u_ar0�����
			u.u_ar0 = i;
			return i;
		}
	}
	return -1;
}


//�����û�ϵͳ�����ṩ���ļ�����������fd���ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ
File* OpenFiles::GetFileByFd(int fd){
	File* pFile;
	User& u = *User::GetInstance();

	//������ļ���������ֵ�����˷�Χ
	if (fd < 0 || fd >= OpenFiles::MAXOPENFILES){
		u.u_error = User::EBADF_;
		return nullptr;
	}

	pFile = this->ProcessOpenFileTable[fd];
	if (pFile == nullptr){
		u.u_error = User::EBADF_;
	}

	//��ʹpFile == NULLҲ���������ɵ���GetF�ĺ������жϷ���ֵ
	return pFile;
}


/*��OpenFileTable�е�fd������ָ��pFile*/
void OpenFiles::SetFd2File(int fd, File* pFile){
	if (fd < 0 || fd >= OpenFiles::MAXOPENFILES){
		return;
	}
	/* ���ļ������� = ��Ӧ��File�ṹ*/
	this->ProcessOpenFileTable[fd] = pFile;
	return;
}

