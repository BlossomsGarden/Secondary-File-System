#include "InodeTable.h"
#include "User.h"
#include "BufManager.h"
#include <iostream>
using namespace std;



//�����ڴ�Inode���ʵ��
//ͷ�ļ���� extern ֻ������������һ���������ﻹҪ�����
InodeTable g_InodeTable;


InodeTable::InodeTable(){}
InodeTable ::~InodeTable(){}


void InodeTable::Initialize(){
    this->m_FileSystem = FileSystem::GetInstance();
}


/*ͨ���豸��inumber�ͻ�ȡһ��Inode���ڴ濽��*/
Inode* InodeTable::IGet(int inumber){
    Inode* pInode;
    User& u = *User::GetInstance();

    /* ���ָ���豸dev�б��Ϊinumber�����Inode�Ƿ����ڴ濽�� */
    int index = InodeTable::InodeIsLoaded(inumber);

    //�ҵ��ڴ濽��
    if (index >= 0){
        pInode = &(this->m_Inode[index]);
        /*
         * ����ִ�е������ʾ���ڴ�Inode���ٻ������ҵ���Ӧ�ڴ�Inode��
         * ���������ü���������ILOCK��־������֮
         */
        pInode->i_count++;
        return pInode;
    }
    //û��Inode���ڴ濽���������һ�������ڴ�Inode
    else{
        pInode = this->GetFreeInode();
        //���ڴ�Inode���������������Inodeʧ��
        if (NULL == pInode){
            cout << "Inode Table Overflow !" << endl;
            u.u_error = User::ENFILE_;
            return NULL;
        }
        //�������Inode�ɹ��������Inode�����·�����ڴ�Inode
        else{
            //���Inode��ţ��������ü������������ڵ�����
            pInode->i_number = inumber;
            pInode->i_count = 1; // DEBUG �޸��˴��ڴ��ȡ����Inode������Ĭ��Ϊ1
            pInode->i_lastr = -1;

            BufManager& bufmanager = *BufManager::GetInstance();
            /* �������Inode���뻺���� */
            Buf* pBuf = bufmanager.Bread(FileSystem::INODE_ZONE_START_SECTOR + inumber / FileSystem::INODE_NUMBER_PER_SECTOR);
            /* ���������е����Inode��Ϣ�������·�����ڴ�Inode�� */
            pInode->ICopy(pBuf, inumber);
            /* �ͷŻ��� */
            bufmanager.Brelse(pBuf);
            return pInode;
        }
    }

    return NULL; /* GCC likes it! */
}



//�ر��ļ�ʱ����Iput��Ҫ�������£�
// 
//�ڴ�i�ڵ���� i_count--����Ϊ0���ͷ��ڴ� i�ڵ㣻���иĶ�д�ش���
//�����ļ�;��������Ŀ¼�ļ������������󶼻�Iput���ڴ�i�ڵ�
//·�����ĵ�����2��·������һ���Ǹ�Ŀ¼�ļ�
//�������д������ļ�����ɾ�������ļ����������д���ɾ����Ŀ¼��
//����뽫���Ŀ¼�ļ�����Ӧ���ڴ� i�ڵ�д�ش���
//���Ŀ¼�ļ������Ƿ��������ģ����Ǳ��뽫����i�ڵ�д�ش���
void InodeTable::IPut(Inode* pNode){
    /* ��ǰ����Ϊ���ø��ڴ�Inode��Ψһ���̣���׼���ͷŸ��ڴ�Inode */
    if (pNode->i_number == 0)
        return; // �����ͷŸ�Ŀ¼
    if (pNode->i_count == 1){
        /*
         * ��������Ϊ�������ͷŹ����п�����Ϊ���̲�����ʹ�øý���˯�ߣ�
         * ��ʱ�п�����һ�����̻�Ը��ڴ�Inode���в������⽫�п��ܵ��´���
         */
        if (pNode->i_number < 0)
            return;
        /* ���ļ��Ѿ�û��Ŀ¼·��ָ���� */
        if (pNode->i_nlink <= 0){
            /* �ͷŸ��ļ�ռ�ݵ������̿� */
            pNode->ITrunc();
            pNode->i_mode = 0;
            /* �ͷŶ�Ӧ�����Inode */
            this->m_FileSystem->IFree(pNode->i_number);
        }

        /* �������Inode��Ϣ */
        pNode->IUpdate(std::time(nullptr));

        /* ����ڴ�Inode�����б�־λ */
        pNode->i_flag = 0;
        /* �����ڴ�inode���еı�־֮һ����һ����i_count == 0 */
        pNode->i_number = -1;
    }

    /* �����ڴ�Inode�����ü��������ѵȴ����� */
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
    /* Ѱ��ָ�����Inode���ڴ濽�� */
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
        /* ������ڴ�Inode���ü���Ϊ�㣬���Inode��ʾ���� */
        if (this->m_Inode[i].i_count == 0){
            return &(this->m_Inode[i]);
        }
    }
    return NULL; /* Ѱ��ʧ�� */
}