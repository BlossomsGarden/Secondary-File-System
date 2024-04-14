#pragma once
#include "Inode.h"
#include "FileSystem.h"

//������ϲ�ᱨ��C4430 ȱ������˵���� - �ٶ�Ϊ int
class InodeTable;

//���������й���һ��ȫ�ֵ�InodeTable
extern InodeTable g_InodeTable;


//�ڴ��е�Inode��
//�����ڴ� Inode �ķ�����ͷ�
class InodeTable {
public:
	//�ڴ� Inode ������
	static const int NINODE = 100;

	//�ڴ� Inode ���飬ÿ�����ļ�����ռ��һ���ڴ�Inode
	Inode m_Inode[NINODE];

	//��ȫ�ֶ��� g_FileSystem ������
	FileSystem* m_FileSystem;

public:
	InodeTable ();
	~InodeTable ();

	//��ʼ���� g_FileSystem ���������
	void Initialize();

	//����ָ����� Inode ��Ż�ȡ��Ӧ Inode:
	//����� Inode �Ѿ����ڴ��У��������������ظ��ڴ� Inode
	//��������ڴ��У���������ڴ�����������ظ��ڴ� Inode
	Inode* IGet(int inumber);


	//���ٸ��ڴ� Inode �����ü���
	//���Ѿ�û��Ŀ¼��ָ�������޽������ã����ͷŴ�Inodeָ����ļ���ռ�õĴ��̿�
	void IPut(Inode* pNode);


	//�����б��޸Ĺ����ڴ� Inode ���µ���Ӧ��� Inode ��
	void UpdateInodeTable();
	
	//����豸 dev �ϱ��Ϊ inumber ����� inode �Ƿ����ڴ濽��
	//������򷵻ظ��ڴ� Inode ���ڴ� Inode ���е�����
	int InodeIsLoaded(int inumber);

	//���ڴ� Inode ����Ѱ��һ�����е��ڴ� Inode
	Inode* GetFreeInode();

};

