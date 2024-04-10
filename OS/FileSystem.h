#pragma once
#include "FileSystem.h"
#include "SuperBlock.h"
#include "BufManager.h"

//��������ļ��洢�豸�еĸ���洢��Դ���Լ����̿顢DiskINode�ķ��䡢�ͷ�
class FileSystem {
public:
	/* ���� SuperBlock λ�ڴ����ϵ������ţ�ռ�� 0��1 ���������� */
	static const int SUPER_BLOCK_SECTOR_NUMBER = 0; 

	/* �ļ�ϵͳ��Ŀ¼��� Inode ��� */
	static const int ROOTINO = 0; 

	/* ��� INode ���󳤶�Ϊ64�ֽڣ�ÿ�����̿���Դ� 512/64=8 �����Inode */
	static const int INODE_NUMBER_PER_SECTOR = 8; 
	
	/* ��� Inode ��λ�ڴ����ϵ���ʼ������ */
	static const int INODE_ZONE_START_SECTOR = 2; 

	/* ��������� Inode ��ռ�ݵ������� */
	static const int INODE_ZONE_SIZE = 1024 - 2; 
	
	/* ����������ʼ������ */
	static const int DATA_ZONE_START_SECTOR = 1024; 

	/* �������Ľ��������� */
	static const int DATA_ZONE_END_SECTOR = 18000 - 1; 

	/*������ռ�ݵ��������� */
	static const int DATA_ZONE_SIZE = 18000 - DATA_ZONE_START_SECTOR; 

private:
	/* FileSystem ����Ҫ�������ģ��(BufferManager)�ṩ�Ľӿ� */
	BufManager* m_BufferManager; 

public:
	FileSystem ();
	~FileSystem ();

	
	//��ʼ����Ա����
	void Initialize();
	
	//ϵͳ��ʼ��ʱ���� SuperBlock
	void LoadSuperBlock();
	
	//�����ļ��洢�豸���豸�� dev ��ȡ���ļ�ϵͳ�� SuperBlock
	SuperBlock* GetFS();
	
	//�� SuperBlock ������ڴ渱�����µ��洢�豸�� SuperBlock ��ȥ
	void Update();
	
	//�ڴ洢�豸 dev �Ϸ���һ��������� INode��һ�����ڴ����µ��ļ�
	Inode* IAlloc();
	
	//�ͷŴ洢�豸 dev �ϱ��Ϊ number����� INode��һ������ɾ���ļ�
	void IFree(int number);
	
	//�ڴ洢�豸 dev �Ϸ�����д��̿�
	Buf* Alloc();
	
	//�ͷŴ洢�豸 dev �ϱ��Ϊ blkno �Ĵ��̿�
	void Free(int blkno);
};

FileSystem ::FileSystem ()
{
}

FileSystem ::~FileSystem ()
{
}