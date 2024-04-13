#pragma once
#include "BufManager.h"
#include "Buf.h"
#include "DiskDriver.h"
#include <mutex>

//����һ�������Թ���Buf��������Ļ���������ɻ������
//�������䡢��д�����յȲ���
class BufManager {
public:
	// BufManager ������ʵ��
	static BufManager instance;

	//������ƿ顢������������
	static const int NBUF = 15;
	//��������С�� ���ֽ�Ϊ��λ
	static const int BUFFER_SIZE = 512;

	//���ɻ�����п��ƿ�
	Buf bFreeList;
	//������ƿ�����
	Buf m_Buf[NBUF];
	//����������
	unsigned char Buffer[NBUF][BUFFER_SIZE];


	//���Խ���첽д��getblk���̳�ͻ
	mutex buf_mutex[NBUF];

	//ָ���������ģ��ȫ�ֶ���
	//ͨ��������ӿ��ڻ�������������ļ�֮����ж�д
	DiskDriver* m_DiskDriver;

public:
	BufManager ();
	~BufManager ();

	static BufManager& GetInstance();

	//������ƿ���еĳ�ʼ������������ƿ��� b_addr ָ����Ӧ�������׵�ַ
	void Initialize();

	//����һ�黺�棬���ڶ�д�豸 dev �ϵ��ַ���blkno
	Buf* GetBlk(int dev, int blkno);

	//�ͷŻ�����ƿ� buf��ָ�����������ɻ�����ж�βȥ
	void Brelse(Buf* bp);

	//��һ�����̿顣dev Ϊ�������豸�ţ�blkno ΪĿ����̿��߼����
	Buf* Bread(int blkno);

	//дһ�����̿�
	void Bwrite(Buf* bp);

	//�첽д����ɺ��ͷš���Bwrite��ע�����߳�������
	//�Ի���������ע�������λ�þ������ǶԻ���ĸ���
	void Bawrite(Buf* bp);

	//�ӳ�д���̿�
	void Bdwrite(Buf* bp);

	//��ջ���������
	void ClrBuf(Buf* bp);

	//�� dev ָ���豸�������ӳ�д�Ļ���ȫ�����������
	void Bflush();

	//��ȡ���ɻ�����п��ƿ� Buf ��������
	Buf& GetBFreeList();

	// ����������еĿ�ȡ����������BUSY����
	void NotAvail(Buf *bp);
};


