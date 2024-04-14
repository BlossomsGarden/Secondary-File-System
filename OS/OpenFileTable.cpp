#include "OpenFileTable.h"
#include "User.h"
#include "InodeTable.h"
#include <iostream>
using namespace std;



//定义全局打开文件表的实例
//头文件里的 extern 只是声明这里有一个，你这里还要定义的
OpenFileTable g_OpenFileTable;

OpenFileTable::OpenFileTable(){}
OpenFileTable ::~OpenFileTable(){}


void OpenFileTable::CloseF(File* pFile){
    Inode* pNode =new Inode();
    if (pFile->f_count <= 1){
        /*
         * 如果当前进程是最后一个引用该文件的进程，
         * 对特殊块设备、字符设备文件调用相应的关闭函数
         */
        //pFile->f_inode->CloseI(pFile->f_flag & File::FWRITE);
        g_InodeTable.IPut(pFile->f_inode);
    }

    /* 引用当前File的进程数减1 */
    pFile->f_count--;
}

/*作用：进程打开文件描述符表中找的空闲项  之 下标  写入 u_ar0[EAX]*/
File* OpenFileTable::FAlloc(){
    int fd;
    User& u = *User::GetInstance();

    /* 在进程打开文件描述符表中获取一个空闲项 */
    fd = u.u_ofiles.AllocFreeSlot();

    //寻找空闲项失败
    if (fd < 0){
        return NULL;
    }

    for (int i = 0; i < OpenFileTable::NFILE; i++){
        /* f_count==0表示该项空闲 */
        if (this->m_File[i].f_count == 0){
            /* 建立描述符和File结构的勾连关系 */
            u.u_ofiles.SetFd2File(fd, &this->m_File[i]);
            /* 增加对file结构的引用计数 */
            this->m_File[i].f_count++;
            /* 清空文件读、写位置 */
            this->m_File[i].f_offset = 0;
            return (&this->m_File[i]);
        }
    }

    cout << "No Free File Struct\n" << endl;
    u.u_error = User::ENFILE_;
    return NULL;
}

