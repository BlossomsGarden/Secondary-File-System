#include "FileSystem.h"
#include "FileManager.h"
#include "Utility.h"
#include "User.h"
#include "Windows.h"
#include "DiskInode.h"
#include "Buf.h"
#include <iostream>
using namespace std;


/*==============================class SuperBlock===================================*/
/* ϵͳȫ�ֳ�����SuperBlock���� */
SuperBlock g_spb;


/*=================================class Mount=====================================*/
Mount::Mount()
{
	this->m_dev = -1;
	this->m_spb = NULL;
	this->m_inodep = NULL;
}

Mount::~Mount()
{
	this->m_dev = -1;
	this->m_inodep = NULL;
	// �ͷ��ڴ�SuperBlock����
	if (this->m_spb != NULL)
	{
		// delete this->m_spb;
		this->m_spb = NULL;
	}
}


/*==================================FileSystem=====================================*/
FileSystem::FileSystem(){}
FileSystem::~FileSystem(){}

FileSystem& FileSystem::GetInstance() {
	return FileSystem::instance;
}


void FileSystem::Initialize(){
	for (int i = 0; i < FileSystem::NMOUNT; i++){
		this->m_Mount[i].m_spb = NULL;
		this->m_Mount[i].m_dev = -1;
		this->m_Mount[i].m_inodep = NULL;
	}
	this->m_BufManager = &BufManager::GetInstance();
	this->updlock = 0;
}

void FileSystem::FormatDisk(int dev){
	//DeviceManager& dv = *DeviceManager::getInst();

	SuperBlock superBlock;
	//this->m_Mount[0].m_dev = DeviceManager::ROOTDEV;
	this->m_Mount[0].m_spb = &g_spb;

	superBlock.s_isize = 200;		/* ���Inode��ռ�õ��̿��� */
	superBlock.s_fsize = BLOCK_NUM; /* �̿����� */
	superBlock.s_nfree = 0;			/* ֱ�ӹ���Ŀ����̿����� */
	for (int i = 0; i < 100; i++){
		superBlock.s_free[i] = 0;
	}
	superBlock.s_ninode = 0; /* ֱ�ӹ���Ŀ������Inode���� */
	/*�����̿����s_free*/

	for (int i = 0; i < 100; i++){
		superBlock.s_inode[i] = 0;
	}
	superBlock.s_flock = 0;			   /* ���������̿��������־ */
	superBlock.s_ilock = 0;			   /* ��������Inode���־ */
	superBlock.s_fmod = 0;			   /* �ڴ���super block�������޸ı�־����ζ����Ҫ��������Ӧ��Super Block */
	superBlock.s_ronly = 0;			   /* ���ļ�ϵͳֻ�ܶ��� */
	superBlock.s_time = time(nullptr); /* ���һ�θ���ʱ�� */
	for (int i = 0; i < sizeof(superBlock.padding) / sizeof(int); i++)
	{
		superBlock.padding[i] = 0;
	}
	for (int j = 0; j < 2; j++)
	{
		/* ��һ��pָ��SuperBlock�ĵ�0�ֽڣ��ڶ���pָ���512�ֽ� */
		int* p = (int*)&superBlock + j * 128;

		/* ��Ҫд�뵽�豸dev�ϵ�SUPER_BLOCK_SECTOR_NUMBER + j������ȥ */
		Buf* pBuf = this->m_BufManager->GetBlk(dev, FileSystem::SUPER_BLOCK_SECTOR_NUMBER + j);

		/* ��SuperBlock�е�0 - 511�ֽ�д�뻺���� */
		DWordCopy(p, (int*)pBuf->b_addr, 128);

		/* ���������е�����д�������� */
		m_BufManager->Bwrite(pBuf);
	}
	m_BufManager->Bflush();
	LoadSuperBlock();

	for (int i = superBlock.s_fsize - 1; i >= DATA_ZONE_START_SECTOR; --i){
		this->Free(dev, i);
	}
	Inode rootdir;
	rootdir.i_dev = 0;
	rootdir.i_size = 0;
	rootdir.i_nlink = 1;
	rootdir.i_count = 1;
	rootdir.i_flag = Inode::IUPD;
	rootdir.i_number = ROOTINO;
	rootdir.i_mode |= Inode::IFDIR | Inode::IALLOC | Inode::IRWXU;

	rootdir.IUpdate(time(nullptr));
	Update();

	FileManager::GetInstance().Initialize();
}

//��ȡ���SuperBlock���������������ļ���ͷ�����ȫ��SuperBlock����
void FileSystem::LoadSuperBlock(){
	BufManager& bufManager = BufManager::GetInstance();
	Buf* pBuf;
	for (int i = 0; i < 2; i++){
		int* p = (int*)&g_spb + i * 128;
		pBuf = bufManager.Bread(FileSystem::SUPER_BLOCK_SECTOR_NUMBER + i);
		DWordCopy((int*)pBuf->b_addr, p, 128);
		bufManager.Brelse(pBuf);
	}

	//this->m_Mount[0].m_dev = DeviceManager::ROOTDEV;
	this->m_Mount[0].m_spb = &g_spb;

	g_spb.s_flock = 0;
	g_spb.s_ilock = 0;
	g_spb.s_ronly = 0;
	g_spb.s_time = time(nullptr);
}


//��ȡ���ļ�ϵͳ��Superblock
SuperBlock* FileSystem::GetSuperBlock(){
	SuperBlock* superBlock;
	/* ����ϵͳװ����Ѱ���豸��Ϊdev���豸���ļ�ϵͳ��SuperBlock */
	for (int i = 0; i < FileSystem::NMOUNT; i++){
		if (this->m_Mount[i].m_spb != NULL){
			/* ��ȡSuperBlock�ĵ�ַ */
			superBlock = this->m_Mount[i].m_spb;
			if (superBlock->s_nfree > 100 || superBlock->s_ninode > 100){
				superBlock->s_nfree = 0;
				superBlock->s_ninode = 0;
			}
			return superBlock;
		}
	}
	cout << "No SuperBlock in this File System!" << endl;
	return NULL;
}


// ���ڴ���SuperBlock��Inode�е����ݸ��µ���������ȥ
void FileSystem::Update(){
	int i;
	SuperBlock* superBlock;
	Buf* pBuf;

	 /* ��һ�������ڽ���ͬ�������һ������ */
	 if (this->updlock){
		 Sleep(200);
		 return FileSystem::Update();
	 }
	 /* ����Update()�����Ļ���������ֹ������������ */
	 this->updlock++;

	/* ͬ��SuperBlock������ */
	for (i = 0; i < FileSystem::NMOUNT; i++){
		//��Mountװ����Ӧĳ���ļ�ϵͳ
		if (this->m_Mount[i].m_spb != NULL){
			superBlock = this->m_Mount[i].m_spb;

			//����SuperBlock�ڴ渱��û�б��޸�
			//��ֱ�ӹ���inode�Ϳ����̿鱻����
			//�����ļ�ϵͳ��ֻ���ļ�ϵͳ
			if (
				superBlock->s_fmod == 0 || 
				superBlock->s_ilock != 0 || 
				superBlock->s_flock != 0 || 
				superBlock->s_ronly != 0
			){
				continue;
			}

			/* ��SuperBlock�޸ı�־ */
			superBlock->s_fmod = 0;
			/* д��SuperBlock�����ʱ�� */
			superBlock->s_time = std::time(nullptr);
			/*
			 * Ϊ��Ҫд�ص�������ȥ��SuperBlock����һ�黺�棬���ڻ�����СΪ512�ֽڣ�
			 * SuperBlock��СΪ1024�ֽڣ�ռ��2��������������������Ҫ2��д�������
			 */
			for (int j = 0; j < 2; j++){
				/* ��һ��pָ��SuperBlock�ĵ�0�ֽڣ��ڶ���pָ���512�ֽ� */
				int* p = (int*)superBlock + j * 128;

				/* ��Ҫд�뵽�豸dev�ϵ�SUPER_BLOCK_SECTOR_NUMBER + j������ȥ */
				pBuf = this->m_BufManager->GetBlk(this->m_Mount[i].m_dev, FileSystem::SUPER_BLOCK_SECTOR_NUMBER + j);

				/* ��SuperBlock�е�0 - 511�ֽ�д�뻺���� */
				DWordCopy(p, (int*)pBuf->b_addr, 128);

				/* ���������е�����д�������� */
				this->m_BufManager->Bwrite(pBuf);
			}
		}
	}

	/* ͬ���޸Ĺ����ڴ�Inode����Ӧ���Inode */
	g_InodeTable.UpdateInodeTable();

	/* ���Update()������ */
	 this->updlock = 0;

	 /* ���ӳ�д�Ļ����д�������� */
	this->m_BufManager->Bflush();
}


/*���豸�Ϸ���һ������Inode*/
Inode* FileSystem::IAlloc(int dev){
	SuperBlock* superBlock;
	Buf* pBuf;
	Inode* pNode;
	User& u = User::GetInstance();
	int ino; /* ���䵽�Ŀ������Inode��� */

	/* ��ȡ��Ӧ�豸��SuperBlock�ڴ渱�� */
	superBlock = this->GetSuperBlock();

	// ���SuperBlock����Inode����������˯�ߵȴ�������
	//while (superBlock->s_ilock){
	//	u.u_procp->Sleep((unsigned long)&superBlock->s_ilock, ProcessManager::PINOD);
	//}
	/* ��һ�������ڽ���ͬ�������һ������ */
	if (superBlock->s_ilock) {
		Sleep(2000);
		return FileSystem::IAlloc(dev);
	}


	if (superBlock->s_ninode <= 0){
		/* ���Inode��Ŵ�0��ʼ���ⲻͬ��Unix V6�����Inode��1��ʼ��� */
		ino = -1;
		/* ���ζ������Inode���еĴ��̿飬�������п������Inode���������Inode������ */
		for (int i = 0; i < superBlock->s_isize; i++){
			pBuf = this->m_BufManager->Bread(FileSystem::INODE_ZONE_START_SECTOR + i);
			/* ��ȡ��������ַ */
			int* p = (int*)pBuf->b_addr;
			/* ���û�������ÿ�����Inode��i_mode != 0����ʾ�Ѿ���ռ�� */
			for (int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++){
				ino++;
				int mode = *(p + j * sizeof(DiskInode) / sizeof(int));
				/* �����Inode�ѱ�ռ�ã����ܼ������Inode������ */
				if (mode != 0){
					continue;
				}
				/*
				 * ������inode��i_mode==0����ʱ������ȷ��
				 * ��inode�ǿ��еģ���Ϊ�п������ڴ�inodeû��д��
				 * ������,����Ҫ���������ڴ�inode���Ƿ�����Ӧ����
				 */
				if (g_InodeTable.IsLoaded(dev, ino) == -1){
					/* �����Inodeû�ж�Ӧ���ڴ濽��������������Inode������ */
					superBlock->s_inode[superBlock->s_ninode++] = ino;

					/* ��������������Ѿ�װ�����򲻼������� */
					if (superBlock->s_ninode >= 100)
					{
						break;
					}
				}
			}

			/* �����Ѷ��굱ǰ���̿飬�ͷ���Ӧ�Ļ��� */
			this->m_BufManager->Brelse(pBuf);

			/* ��������������Ѿ�װ�����򲻼������� */
			if (superBlock->s_ninode >= 100){
				break;
			}
		}

		/* ����ڴ�����û���������κο������Inode������NULL */
		if (superBlock->s_ninode <= 0){
			u.u_error = User::ErrorCode::ENOSPC_;
			return NULL;
		}
	}

	/*
	 * ���沿���Ѿ���֤������ϵͳ��û�п������Inode��
	 * �������Inode�������бض����¼�������Inode�ı�š�
	 */
	while (true){
		/* ��������ջ������ȡ�������Inode��� */
		ino = superBlock->s_inode[--superBlock->s_ninode];

		/* ������Inode�����ڴ� */
		pNode = g_InodeTable.IGet(dev, ino);
		/* δ�ܷ��䵽�ڴ�inode */
		if (NULL == pNode){
			return NULL;
		}

		/* �����Inode����,���Inode�е����� */
		if (0 == pNode->i_mode){
			pNode->Clean();
			/* ����SuperBlock���޸ı�־ */
			superBlock->s_fmod = 1;
			return pNode;
		}
		//�����Inode�ѱ�ռ��
		else{
			g_InodeTable.IPut(pNode);
			continue; /* whileѭ�� */
		}
	}
	return NULL; /* GCC likes it! */
}


/*�ͷ��豸�ϵ�number��Inode*/
void FileSystem::IFree(int InodeNo){
	SuperBlock* superBlock;
	//��õ�ǰSuperBlock�ڴ渱��
	superBlock = this->GetSuperBlock();

	/*
	 * ���������ֱ�ӹ���Ŀ������Inode����100����
	 * ͬ�����ͷŵ����Inodeɢ���ڴ���Inode���С�
	 */
	if (superBlock->s_ninode >= 100){
		return;
	}

	superBlock->s_inode[superBlock->s_ninode++] = InodeNo;

	/* ����SuperBlock���޸ı�־ */
	superBlock->s_fmod = 1;
}


/*���豸�з���һ�����д��̿飬������Buf��*/
//�����ļ����������ļ�����ʱ����Ҫ����ռ䣬������Ҫ�����̵����뻺�棬���д�ر���
Buf* FileSystem::Alloc(int dev){
	int blkno; /* ���䵽�Ŀ��д��̿��� */
	SuperBlock* superBlock;
	Buf* pBuf;
	User& u = User::GetInstance();

	/* ��ȡSuperBlock������ڴ渱�� */
	superBlock = this->GetSuperBlock();

	/*
	 * ������д��̿����������ڱ���������������������
	 * ���ڲ������д��̿����������������������ͨ��
	 * ������������̵���Free()��Alloc()��ɵġ�
	 */
	 // while (superBlock->s_flock)
	 // {
	 // 	/* ����˯��ֱ����ø����ż��� */
	 // 	u.u_procp->Sleep((unsigned long)&superBlock->s_flock, ProcessManager::PINOD);
	 // }
	/* ��һ�������ڽ���ͬ�������һ������ */
	if (superBlock->s_ilock) {
		Sleep(2000);
		return FileSystem::Alloc(dev);
	}

	 /* ��������ջ������ȡ���д��̿��� */
	blkno = superBlock->s_free[--superBlock->s_nfree];

	/*
	 * ����ȡ���̿���Ϊ�㣬���ʾ�ѷ��価���еĿ��д��̿顣
	 * ���߷��䵽�Ŀ��д��̿��Ų����������̿�������(��BadBlock()���)��
	 * ����ζ�ŷ�����д��̿����ʧ�ܡ�
	 */
	if (0 == blkno)
	{
		superBlock->s_nfree = 0;
		printf("No Space On %d !\n", dev);
		u.u_error = User::ENOSPC_;
		return NULL;
	}
	if (this->BadBlock(superBlock, blkno))
	{
		return NULL;
	}

	/*
	 * ջ�ѿգ��·��䵽���д��̿��м�¼����һ����д��̿�ı��,
	 * ����һ����д��̿�ı�Ŷ���SuperBlock�Ŀ��д��̿�������s_free[100]�С�
	 */
	if (superBlock->s_nfree <= 0){
		/*
		 * �˴���������Ϊ����Ҫ���ж��̲������п��ܷ��������л���
		 * ����̨�Ľ��̿��ܶ�SuperBlock�Ŀ����̿���������ʣ��ᵼ�²�һ���ԡ�
		 */
		superBlock->s_flock++;

		/* ����ÿ��д��̿� */
		pBuf = this->m_BufManager->Bread(blkno);

		/* �Ӹô��̿��0�ֽڿ�ʼ��¼����ռ��4(s_nfree)+400(s_free[100])���ֽ� */
		int* p = (int*)pBuf->b_addr;

		/* ���ȶ��������̿���s_nfree */
		superBlock->s_nfree = *p++;

		/* ��ȡ�����к���λ�õ����ݣ�д�뵽SuperBlock�����̿�������s_free[100]�� */
		DWordCopy(p, superBlock->s_free, 100);

		/* ����ʹ����ϣ��ͷ��Ա㱻��������ʹ�� */
		this->m_BufManager->Brelse(pBuf);

		// /* ����Կ��д��̿������������������Ϊ�ȴ�����˯�ߵĽ��� */
		 superBlock->s_flock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&superBlock->s_flock);
	}

	/* ��ͨ����³ɹ����䵽һ���д��̿� */
	pBuf = this->m_BufManager->GetBlk(dev, blkno); /* Ϊ�ô��̿����뻺�� */
	this->m_BufManager->ClrBuf(pBuf);			  /* ��ջ����е����� */
	superBlock->s_fmod = 1;									  /* ����SuperBlock���޸ı�־ */

	return pBuf;
}


/*�ͷ�һ���̿�*/
void FileSystem::Free(int dev, int blkno){
	SuperBlock* superBlock;
	Buf* pBuf;

	superBlock = this->GetSuperBlock();
	/*
	 * ��������SuperBlock���޸ı�־���Է�ֹ���ͷ�
	 * ���̿�Free()ִ�й����У���SuperBlock�ڴ渱��
	 * ���޸Ľ�������һ�룬�͸��µ�����SuperBlockȥ
	 */
	superBlock->s_fmod = 1;

	/* ����ͷŴ��̿�ĺϷ��� */
	if (this->BadBlock(superBlock, blkno)){
		return;
	}

	/*
	 * �����ǰϵͳ���Ѿ�û�п����̿飬
	 * �����ͷŵ���ϵͳ�е�1������̿�
	 */
	if (superBlock->s_nfree <= 0){
		superBlock->s_nfree = 1;
		superBlock->s_free[0] = 0; /* ʹ��0��ǿ����̿���������־ */
	}

	/* SuperBlock��ֱ�ӹ�����д��̿�ŵ�ջ���� */
	if (superBlock->s_nfree >= 100){
		 superBlock->s_flock++;

		/*
		 * ʹ�õ�ǰFree()������Ҫ�ͷŵĴ��̿飬���ǰһ��100������
		 * ���̿��������
		 */
		pBuf = this->m_BufManager->GetBlk(dev, blkno); /* Ϊ��ǰ��Ҫ�ͷŵĴ��̿���仺�� */

		/* �Ӹô��̿��0�ֽڿ�ʼ��¼����ռ��4(s_nfree)+400(s_free[100])���ֽ� */
		int* p = (int*)pBuf->b_addr;

		/* ����д������̿��������˵�һ��Ϊ99�飬����ÿ�鶼��100�� */
		*p++ = superBlock->s_nfree;
		/* ��SuperBlock�Ŀ����̿�������s_free[100]д�뻺���к���λ�� */
		DWordCopy(superBlock->s_free, p, 100);

		superBlock->s_nfree = 0;
		/* ����ſ����̿�������ġ���ǰ�ͷ��̿顱д����̣���ʵ���˿����̿��¼�����̿�ŵ�Ŀ�� */
		this->m_BufManager->Bwrite(pBuf);

		superBlock->s_flock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&superBlock->s_flock);
	}
	// s_free�׸�λ�ü�¼��ǰ�ͷ��̿��
	superBlock->s_free[superBlock->s_nfree++] = blkno; /* SuperBlock�м�¼�µ�ǰ�ͷ��̿�� */
	superBlock->s_fmod = 1;
}


bool FileSystem::BadBlock(SuperBlock* spb, int blkno){
	return blkno < DATA_ZONE_START_SECTOR;
}
