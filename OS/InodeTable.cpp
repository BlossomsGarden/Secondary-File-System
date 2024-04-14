#include "InodeTable.h"
#include "User.h"
#include "BufManager.h"
#include <iostream>
using namespace std;



//定义内存Inode表的实例
//头文件里的 extern 只是声明这里有一个，你这里还要定义的
InodeTable g_InodeTable;


InodeTable::InodeTable(){}
InodeTable ::~InodeTable(){}


void InodeTable::Initialize(){
    this->m_FileSystem = FileSystem::GetInstance();
}


/*通过设备号inumber和获取一个Inode的内存拷贝*/
Inode* InodeTable::IGet(int inumber){
    Inode* pInode;
    User& u = *User::GetInstance();

    /* 检查指定设备dev中编号为inumber的外存Inode是否有内存拷贝 */
    int index = InodeTable::InodeIsLoaded(inumber);

    //找到内存拷贝
    if (index >= 0){
        pInode = &(this->m_Inode[index]);
        /*
         * 程序执行到这里表示：内存Inode高速缓存中找到相应内存Inode，
         * 增加其引用计数，增设ILOCK标志并返回之
         */
        pInode->i_count++;
        return pInode;
    }
    //没有Inode的内存拷贝，则分配一个空闲内存Inode
    else{
        pInode = this->GetFreeInode();
        //若内存Inode表已满，分配空闲Inode失败
        if (NULL == pInode){
            cout << "Inode Table Overflow !" << endl;
            u.u_error = User::ENFILE_;
            return NULL;
        }
        //分配空闲Inode成功，将外存Inode读入新分配的内存Inode
        else{
            //外存Inode编号，增加引用计数，对索引节点上锁
            pInode->i_number = inumber;
            pInode->i_count = 1; // DEBUG 修改了从内存读取来的Inode引用数默认为1
            pInode->i_lastr = -1;

            BufManager& bufmanager = *BufManager::GetInstance();
            /* 将该外存Inode读入缓冲区 */
            Buf* pBuf = bufmanager.Bread(FileSystem::INODE_ZONE_START_SECTOR + inumber / FileSystem::INODE_NUMBER_PER_SECTOR);
            /* 将缓冲区中的外存Inode信息拷贝到新分配的内存Inode中 */
            pInode->ICopy(pBuf, inumber);
            /* 释放缓存 */
            bufmanager.Brelse(pBuf);
            return pInode;
        }
    }

    return NULL; /* GCC likes it! */
}



//关闭文件时调用Iput主要操作如下：
// 
//内存i节点计数 i_count--；若为0，释放内存 i节点；若有改动写回磁盘
//搜索文件途径的所有目录文件，搜索经过后都会Iput其内存i节点
//路径名的倒数第2个路径分量一定是个目录文件
//若在其中创建新文件、或删除已有文件、或在其中创建删除子目录，
//则必须将这个目录文件所对应的内存 i节点写回磁盘
//这个目录文件无论是否经历过更改，我们必须将它的i节点写回磁盘
void InodeTable::IPut(Inode* pNode){
    /* 当前进程为引用该内存Inode的唯一进程，且准备释放该内存Inode */
    if (pNode->i_number == 0)
        return; // 永不释放根目录
    if (pNode->i_count == 1){
        /*
         * 上锁，因为在整个释放过程中可能因为磁盘操作而使得该进程睡眠，
         * 此时有可能另一个进程会对该内存Inode进行操作，这将有可能导致错误。
         */
        if (pNode->i_number < 0)
            return;
        /* 该文件已经没有目录路径指向它 */
        if (pNode->i_nlink <= 0){
            /* 释放该文件占据的数据盘块 */
            pNode->ITrunc();
            pNode->i_mode = 0;
            /* 释放对应的外存Inode */
            this->m_FileSystem->IFree(pNode->i_number);
        }

        /* 更新外存Inode信息 */
        pNode->IUpdate(std::time(nullptr));

        /* 清除内存Inode的所有标志位 */
        pNode->i_flag = 0;
        /* 这是内存inode空闲的标志之一，另一个是i_count == 0 */
        pNode->i_number = -1;
    }

    /* 减少内存Inode的引用计数，唤醒等待进程 */
    pNode->i_count--;
}


void InodeTable::UpdateInodeTable(){
    for (int i = 0; i < InodeTable::NINODE; i++){
        if (this->m_Inode[i].i_count != 0){
            this->m_Inode[i].IUpdate(time(nullptr));
        }
    }
}

int InodeTable::InodeIsLoaded(int inumber){
    /* 寻找指定外存Inode的内存拷贝 */
    for (int i = 0; i < InodeTable::NINODE; i++){
        if (this->m_Inode[i].i_number == inumber && this->m_Inode[i].i_count != 0)
        {
            return i;
        }
    }
    return -1;
}


Inode* InodeTable::GetFreeInode(){
    for (int i = 0; i < InodeTable::NINODE; i++){
        /* 如果该内存Inode引用计数为零，则该Inode表示空闲 */
        if (this->m_Inode[i].i_count == 0){
            return &(this->m_Inode[i]);
        }
    }
    return NULL; /* 寻找失败 */
}