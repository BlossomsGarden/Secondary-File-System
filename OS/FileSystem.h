#pragma once
#include "Inode.h"
#include "SuperBlock.h"
#include "BufManager.h"



/*
 * �ļ�ϵͳװ���(Mount)�Ķ��塣
 * װ�������ʵ�����ļ�ϵͳ��
 * ���ļ�ϵͳ�����ӡ�
 */
class Mount
{
	/* Functions */
public:
	/* Constructors */
	Mount();
	/* Destructors */
	~Mount();

	/* Members */
public:
	short m_dev;       /* �ļ�ϵͳ�豸�� */
	SuperBlock* m_spb; /* ָ���ļ�ϵͳ��Super Block�������ڴ��еĸ��� */
	Inode* m_inodep;   /* ָ��������ļ�ϵͳ���ڴ�INode */
};



//��������ļ��洢�豸�еĸ���洢��Դ�Լ����̿顢DiskINode�ķ��䡢�ͷ�
class FileSystem {
public:
	/* ���� SuperBlock λ�ڴ����ϵ������ţ�ռ�� 0��1 ���������� */
	static const int SUPER_BLOCK_SECTOR_NUMBER = 0;

	/* �ļ�ϵͳ��Ŀ¼��� Inode ��� */
	static const int ROOTINO = 0;

	//! ��С��disk.img��Ӧ
	static const int BLOCK_NUM = 1024 * 1024 * 32 / 512;

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

	static const int NMOUNT = 5; /* ϵͳ�����ڹ������ļ�ϵͳ��װ������� */

private:
	/* FileSystem ����Ҫ�������ģ��(BufferManager)�ṩ�Ľӿ� */
	BufManager* m_BufManager;

	//Update()�����������ú�������ͬ���ڴ����SuperBlock�����Լ����޸Ĺ����ڴ�Inode
	//��һʱ��ֻ����һ�����̵��øú���
	int updlock;

	/* �ļ�ϵͳװ����Mount[0]���ڸ��ļ�ϵͳ */
	Mount m_Mount[NMOUNT];

	//����Kernel�����ļ�ϵͳ������ʵ��
	static FileSystem instance;

public:
	FileSystem();
	~FileSystem();

	static FileSystem* GetInstance();

	//��ʼ����Ա����
	void Initialize();

	//ϵͳ��ʼ��ʱ���� SuperBlock
	void LoadSuperBlock();

	//��ȡ���ļ�ϵͳ�� SuperBlock
	SuperBlock* GetSuperBlock();

	//�� SuperBlock ������ڴ渱�����µ��洢�豸�� SuperBlock ��ȥ
	void Update();

	//�ڴ洢�豸 dev �Ϸ���һ��������� INode��һ�����ڴ����µ��ļ�
	Inode* IAlloc(int dev);

	//�ͷŴ洢�豸 dev �ϱ��Ϊ number����� INode��һ������ɾ���ļ�
	void IFree(int InodeNo);

	//�ڴ洢�豸 dev �Ϸ�����д��̿�
	Buf* Alloc(int dev);

	//�ͷŴ洢�豸 dev �ϱ��Ϊ blkno �Ĵ��̿�
	void Free(int dev, int blkno);

	//��ʽ����������ϵͳ
	void FormatDisk(int dev);

private:
	//����豸dev�ϱ��blkno�Ĵ��̿��Ƿ����������̿���
	bool BadBlock(SuperBlock* spb, int blkno);
};
