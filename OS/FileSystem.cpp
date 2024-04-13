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
/* 系统全局超级块SuperBlock对象 */
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
	// 释放内存SuperBlock副本
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

	superBlock.s_isize = 200;		/* 外存Inode区占用的盘块数 */
	superBlock.s_fsize = BLOCK_NUM; /* 盘块总数 */
	superBlock.s_nfree = 0;			/* 直接管理的空闲盘块数量 */
	for (int i = 0; i < 100; i++){
		superBlock.s_free[i] = 0;
	}
	superBlock.s_ninode = 0; /* 直接管理的空闲外存Inode数量 */
	/*所有盘块加入s_free*/

	for (int i = 0; i < 100; i++){
		superBlock.s_inode[i] = 0;
	}
	superBlock.s_flock = 0;			   /* 封锁空闲盘块索引表标志 */
	superBlock.s_ilock = 0;			   /* 封锁空闲Inode表标志 */
	superBlock.s_fmod = 0;			   /* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
	superBlock.s_ronly = 0;			   /* 本文件系统只能读出 */
	superBlock.s_time = time(nullptr); /* 最近一次更新时间 */
	for (int i = 0; i < sizeof(superBlock.padding) / sizeof(int); i++)
	{
		superBlock.padding[i] = 0;
	}
	for (int j = 0; j < 2; j++)
	{
		/* 第一次p指向SuperBlock的第0字节，第二次p指向第512字节 */
		int* p = (int*)&superBlock + j * 128;

		/* 将要写入到设备dev上的SUPER_BLOCK_SECTOR_NUMBER + j扇区中去 */
		Buf* pBuf = this->m_BufManager->GetBlk(dev, FileSystem::SUPER_BLOCK_SECTOR_NUMBER + j);

		/* 将SuperBlock中第0 - 511字节写入缓存区 */
		DWordCopy(p, (int*)pBuf->b_addr, 128);

		/* 将缓冲区中的数据写到磁盘上 */
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

//读取存放SuperBlock的两个扇区到本文件开头定义的全局SuperBlock变量
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


//获取该文件系统的Superblock
SuperBlock* FileSystem::GetSuperBlock(){
	SuperBlock* superBlock;
	/* 遍历系统装配块表，寻找设备号为dev的设备中文件系统的SuperBlock */
	for (int i = 0; i < FileSystem::NMOUNT; i++){
		if (this->m_Mount[i].m_spb != NULL){
			/* 获取SuperBlock的地址 */
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


// 将内存中SuperBlock和Inode中的内容更新到磁盘里面去
void FileSystem::Update(){
	int i;
	SuperBlock* superBlock;
	Buf* pBuf;

	 /* 另一进程正在进行同步，则等一等再来 */
	 if (this->updlock){
		 Sleep(200);
		 return FileSystem::Update();
	 }
	 /* 设置Update()函数的互斥锁，防止其它进程重入 */
	 this->updlock++;

	/* 同步SuperBlock到磁盘 */
	for (i = 0; i < FileSystem::NMOUNT; i++){
		//该Mount装配块对应某个文件系统
		if (this->m_Mount[i].m_spb != NULL){
			superBlock = this->m_Mount[i].m_spb;

			//若该SuperBlock内存副本没有被修改
			//若直接管理inode和空闲盘块被上锁
			//若该文件系统是只读文件系统
			if (
				superBlock->s_fmod == 0 || 
				superBlock->s_ilock != 0 || 
				superBlock->s_flock != 0 || 
				superBlock->s_ronly != 0
			){
				continue;
			}

			/* 清SuperBlock修改标志 */
			superBlock->s_fmod = 0;
			/* 写入SuperBlock最后存访时间 */
			superBlock->s_time = std::time(nullptr);
			/*
			 * 为将要写回到磁盘上去的SuperBlock申请一块缓存，由于缓存块大小为512字节，
			 * SuperBlock大小为1024字节，占据2个连续的扇区，所以需要2次写入操作。
			 */
			for (int j = 0; j < 2; j++){
				/* 第一次p指向SuperBlock的第0字节，第二次p指向第512字节 */
				int* p = (int*)superBlock + j * 128;

				/* 将要写入到设备dev上的SUPER_BLOCK_SECTOR_NUMBER + j扇区中去 */
				pBuf = this->m_BufManager->GetBlk(this->m_Mount[i].m_dev, FileSystem::SUPER_BLOCK_SECTOR_NUMBER + j);

				/* 将SuperBlock中第0 - 511字节写入缓存区 */
				DWordCopy(p, (int*)pBuf->b_addr, 128);

				/* 将缓冲区中的数据写到磁盘上 */
				this->m_BufManager->Bwrite(pBuf);
			}
		}
	}

	/* 同步修改过的内存Inode到对应外存Inode */
	g_InodeTable.UpdateInodeTable();

	/* 清除Update()函数锁 */
	 this->updlock = 0;

	 /* 将延迟写的缓存块写到磁盘上 */
	this->m_BufManager->Bflush();
}


/*从设备上分配一个空闲Inode*/
Inode* FileSystem::IAlloc(int dev){
	SuperBlock* superBlock;
	Buf* pBuf;
	Inode* pNode;
	User& u = User::GetInstance();
	int ino; /* 分配到的空闲外存Inode编号 */

	/* 获取相应设备的SuperBlock内存副本 */
	superBlock = this->GetSuperBlock();

	// 如果SuperBlock空闲Inode表被上锁，则睡眠等待至解锁
	//while (superBlock->s_ilock){
	//	u.u_procp->Sleep((unsigned long)&superBlock->s_ilock, ProcessManager::PINOD);
	//}
	/* 另一进程正在进行同步，则等一等再来 */
	if (superBlock->s_ilock) {
		Sleep(2000);
		return FileSystem::IAlloc(dev);
	}


	if (superBlock->s_ninode <= 0){
		/* 外存Inode编号从0开始，这不同于Unix V6中外存Inode从1开始编号 */
		ino = -1;
		/* 依次读入磁盘Inode区中的磁盘块，搜索其中空闲外存Inode，记入空闲Inode索引表 */
		for (int i = 0; i < superBlock->s_isize; i++){
			pBuf = this->m_BufManager->Bread(FileSystem::INODE_ZONE_START_SECTOR + i);
			/* 获取缓冲区首址 */
			int* p = (int*)pBuf->b_addr;
			/* 检查该缓冲区中每个外存Inode的i_mode != 0，表示已经被占用 */
			for (int j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++){
				ino++;
				int mode = *(p + j * sizeof(DiskInode) / sizeof(int));
				/* 该外存Inode已被占用，不能记入空闲Inode索引表 */
				if (mode != 0){
					continue;
				}
				/*
				 * 如果外存inode的i_mode==0，此时并不能确定
				 * 该inode是空闲的，因为有可能是内存inode没有写到
				 * 磁盘上,所以要继续搜索内存inode中是否有相应的项
				 */
				if (g_InodeTable.IsLoaded(dev, ino) == -1){
					/* 该外存Inode没有对应的内存拷贝，将其记入空闲Inode索引表 */
					superBlock->s_inode[superBlock->s_ninode++] = ino;

					/* 如果空闲索引表已经装满，则不继续搜索 */
					if (superBlock->s_ninode >= 100)
					{
						break;
					}
				}
			}

			/* 至此已读完当前磁盘块，释放相应的缓存 */
			this->m_BufManager->Brelse(pBuf);

			/* 如果空闲索引表已经装满，则不继续搜索 */
			if (superBlock->s_ninode >= 100){
				break;
			}
		}

		/* 如果在磁盘上没有搜索到任何可用外存Inode，返回NULL */
		if (superBlock->s_ninode <= 0){
			u.u_error = User::ErrorCode::ENOSPC_;
			return NULL;
		}
	}

	/*
	 * 上面部分已经保证，除非系统中没有可用外存Inode，
	 * 否则空闲Inode索引表中必定会记录可用外存Inode的编号。
	 */
	while (true){
		/* 从索引表“栈顶”获取空闲外存Inode编号 */
		ino = superBlock->s_inode[--superBlock->s_ninode];

		/* 将空闲Inode读入内存 */
		pNode = g_InodeTable.IGet(dev, ino);
		/* 未能分配到内存inode */
		if (NULL == pNode){
			return NULL;
		}

		/* 如果该Inode空闲,清空Inode中的数据 */
		if (0 == pNode->i_mode){
			pNode->Clean();
			/* 设置SuperBlock被修改标志 */
			superBlock->s_fmod = 1;
			return pNode;
		}
		//如果该Inode已被占用
		else{
			g_InodeTable.IPut(pNode);
			continue; /* while循环 */
		}
	}
	return NULL; /* GCC likes it! */
}


/*释放设备上的number号Inode*/
void FileSystem::IFree(int InodeNo){
	SuperBlock* superBlock;
	//获得当前SuperBlock内存副本
	superBlock = this->GetSuperBlock();

	/*
	 * 如果超级块直接管理的空闲外存Inode超过100个，
	 * 同样让释放的外存Inode散落在磁盘Inode区中。
	 */
	if (superBlock->s_ninode >= 100){
		return;
	}

	superBlock->s_inode[superBlock->s_ninode++] = InodeNo;

	/* 设置SuperBlock被修改标志 */
	superBlock->s_fmod = 1;
}


/*从设备中分配一个空闲磁盘块，并读到Buf中*/
//创建文件或者增加文件内容时常常要更多空间，于是需要将磁盘的移入缓存，最后写回保存
Buf* FileSystem::Alloc(int dev){
	int blkno; /* 分配到的空闲磁盘块编号 */
	SuperBlock* superBlock;
	Buf* pBuf;
	User& u = User::GetInstance();

	/* 获取SuperBlock对象的内存副本 */
	superBlock = this->GetSuperBlock();

	/*
	 * 如果空闲磁盘块索引表正在被上锁，表明有其它进程
	 * 正在操作空闲磁盘块索引表，因而对其上锁。这通常
	 * 是由于其余进程调用Free()或Alloc()造成的。
	 */
	 // while (superBlock->s_flock)
	 // {
	 // 	/* 进入睡眠直到获得该锁才继续 */
	 // 	u.u_procp->Sleep((unsigned long)&superBlock->s_flock, ProcessManager::PINOD);
	 // }
	/* 另一进程正在进行同步，则等一等再来 */
	if (superBlock->s_ilock) {
		Sleep(2000);
		return FileSystem::Alloc(dev);
	}

	 /* 从索引表“栈顶”获取空闲磁盘块编号 */
	blkno = superBlock->s_free[--superBlock->s_nfree];

	/*
	 * 若获取磁盘块编号为零，则表示已分配尽所有的空闲磁盘块。
	 * 或者分配到的空闲磁盘块编号不属于数据盘块区域中(由BadBlock()检查)，
	 * 都意味着分配空闲磁盘块操作失败。
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
	 * 栈已空，新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号,
	 * 将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free[100]中。
	 */
	if (superBlock->s_nfree <= 0){
		/*
		 * 此处加锁，因为以下要进行读盘操作，有可能发生进程切换，
		 * 新上台的进程可能对SuperBlock的空闲盘块索引表访问，会导致不一致性。
		 */
		superBlock->s_flock++;

		/* 读入该空闲磁盘块 */
		pBuf = this->m_BufManager->Bread(blkno);

		/* 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 */
		int* p = (int*)pBuf->b_addr;

		/* 首先读出空闲盘块数s_nfree */
		superBlock->s_nfree = *p++;

		/* 读取缓存中后续位置的数据，写入到SuperBlock空闲盘块索引表s_free[100]中 */
		DWordCopy(p, superBlock->s_free, 100);

		/* 缓存使用完毕，释放以便被其它进程使用 */
		this->m_BufManager->Brelse(pBuf);

		// /* 解除对空闲磁盘块索引表的锁，唤醒因为等待锁而睡眠的进程 */
		 superBlock->s_flock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&superBlock->s_flock);
	}

	/* 普通情况下成功分配到一空闲磁盘块 */
	pBuf = this->m_BufManager->GetBlk(dev, blkno); /* 为该磁盘块申请缓存 */
	this->m_BufManager->ClrBuf(pBuf);			  /* 清空缓存中的数据 */
	superBlock->s_fmod = 1;									  /* 设置SuperBlock被修改标志 */

	return pBuf;
}


/*释放一个盘块*/
void FileSystem::Free(int dev, int blkno){
	SuperBlock* superBlock;
	Buf* pBuf;

	superBlock = this->GetSuperBlock();
	/*
	 * 尽早设置SuperBlock被修改标志，以防止在释放
	 * 磁盘块Free()执行过程中，对SuperBlock内存副本
	 * 的修改仅进行了一半，就更新到磁盘SuperBlock去
	 */
	superBlock->s_fmod = 1;

	/* 检查释放磁盘块的合法性 */
	if (this->BadBlock(superBlock, blkno)){
		return;
	}

	/*
	 * 如果先前系统中已经没有空闲盘块，
	 * 现在释放的是系统中第1块空闲盘块
	 */
	if (superBlock->s_nfree <= 0){
		superBlock->s_nfree = 1;
		superBlock->s_free[0] = 0; /* 使用0标记空闲盘块链结束标志 */
	}

	/* SuperBlock中直接管理空闲磁盘块号的栈已满 */
	if (superBlock->s_nfree >= 100){
		 superBlock->s_flock++;

		/*
		 * 使用当前Free()函数正要释放的磁盘块，存放前一组100个空闲
		 * 磁盘块的索引表
		 */
		pBuf = this->m_BufManager->GetBlk(dev, blkno); /* 为当前正要释放的磁盘块分配缓存 */

		/* 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 */
		int* p = (int*)pBuf->b_addr;

		/* 首先写入空闲盘块数，除了第一组为99块，后续每组都是100块 */
		*p++ = superBlock->s_nfree;
		/* 将SuperBlock的空闲盘块索引表s_free[100]写入缓存中后续位置 */
		DWordCopy(superBlock->s_free, p, 100);

		superBlock->s_nfree = 0;
		/* 将存放空闲盘块索引表的“当前释放盘块”写入磁盘，即实现了空闲盘块记录空闲盘块号的目标 */
		this->m_BufManager->Bwrite(pBuf);

		superBlock->s_flock = 0;
		// Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&superBlock->s_flock);
	}
	// s_free首个位置记录当前释放盘块号
	superBlock->s_free[superBlock->s_nfree++] = blkno; /* SuperBlock中记录下当前释放盘块号 */
	superBlock->s_fmod = 1;
}


bool FileSystem::BadBlock(SuperBlock* spb, int blkno){
	return blkno < DATA_ZONE_START_SECTOR;
}
