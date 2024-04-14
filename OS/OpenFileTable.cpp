#include "OpenFileTable.h"
#include "User.h"
#include "InodeTable.h"
#include <iostream>
using namespace std;



//����ȫ�ִ��ļ����ʵ��
//ͷ�ļ���� extern ֻ������������һ���������ﻹҪ�����
OpenFileTable g_OpenFileTable;

OpenFileTable::OpenFileTable(){}
OpenFileTable ::~OpenFileTable(){}


void OpenFileTable::CloseF(File* pFile){
    Inode* pNode =new Inode();
    if (pFile->f_count <= 1){
        /*
         * �����ǰ���������һ�����ø��ļ��Ľ��̣�
         * ��������豸���ַ��豸�ļ�������Ӧ�Ĺرպ���
         */
        //pFile->f_inode->CloseI(pFile->f_flag & File::FWRITE);
        g_InodeTable.IPut(pFile->f_inode);
    }

    /* ���õ�ǰFile�Ľ�������1 */
    pFile->f_count--;
}

/*���ã����̴��ļ������������ҵĿ�����  ֮ �±�  д�� u_ar0[EAX]*/
File* OpenFileTable::FAlloc(){
    int fd;
    User& u = *User::GetInstance();

    /* �ڽ��̴��ļ����������л�ȡһ�������� */
    fd = u.u_ofiles.AllocFreeSlot();

    //Ѱ�ҿ�����ʧ��
    if (fd < 0){
        return NULL;
    }

    for (int i = 0; i < OpenFileTable::NFILE; i++){
        /* f_count==0��ʾ������� */
        if (this->m_File[i].f_count == 0){
            /* ������������File�ṹ�Ĺ�����ϵ */
            u.u_ofiles.SetFd2File(fd, &this->m_File[i]);
            /* ���Ӷ�file�ṹ�����ü��� */
            this->m_File[i].f_count++;
            /* ����ļ�����дλ�� */
            this->m_File[i].f_offset = 0;
            return (&this->m_File[i]);
        }
    }

    cout << "No Free File Struct\n" << endl;
    u.u_error = User::ENFILE_;
    return NULL;
}

