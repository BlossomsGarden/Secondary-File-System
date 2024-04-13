#include "Inode.h"
#include "algorithm"
#include "Utility.h"
#include "User.h"
#include "BufManager.h"
#include "FileSystem.h"
using namespace std;


/*================================ Inode ===================================*/
Inode::Inode(){}
Inode::~Inode(){}


//逻辑块号转换成对应的物理盘块号
//使用混合索引目录的方式查找
int Inode::Bmap(int lbn) {
	Buf* pFirstBuf;
	Buf* pSecondBuf;
	int phyBlkno; /* 转换后的物理盘块号 */
	int* iTable;  /* 用于访问索引盘块中一次间接、两次间接索引表 */
	int index;
	User& u = User::GetInstance();

	BufManager& bufManager = BufManager::GetInstance();
	FileSystem& fileSystem = FileSystem::GetInstance();

	/*
	 * Unix V6++的文件索引结构：(小型、大型和巨型文件)
	 * (1) i_addr[0] - i_addr[5]为直接索引表，文件长度范围是0 - 6个盘块；
	 *
	 * (2) i_addr[6] - i_addr[7]存放一次间接索引表所在磁盘块号，每磁盘块
	 * 上存放128个文件数据盘块号，此类文件长度范围是7 - (128 * 2 + 6)个盘块；
	 *
	 * (3) i_addr[8] - i_addr[9]存放二次间接索引表所在磁盘块号，每个二次间接
	 * 索引表记录128个一次间接索引表所在磁盘块号，此类文件长度范围是
	 * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	 */


	// 应该不至于出现lbn大于Inode::HUGE_FILE_BLOCK的奇葩情况吧。。。

	//小型文件，一级索引i_addr[0 - 5]中获得物理盘块号
	if (lbn < 6) {
		phyBlkno = this->i_addr[lbn];

		/*
		 * 如果该逻辑块号还没有相应的物理盘块号与之对应，则分配一个物理块。
		 * 这通常发生在对文件的写入，当写入位置超出文件大小，即对当前
		 * 文件进行扩充写入，就需要分配额外的磁盘块，并为之建立逻辑块号
		 * 与物理盘块号之间的映射。
		 */
		if (phyBlkno == 0 && (pFirstBuf = fileSystem.Alloc(this->i_dev)) != NULL) {
			//fileSys.Alloc分配一个干净块，dev和blkno指向一个物理块
			bufManager.Bdwrite(pFirstBuf);
			phyBlkno = pFirstBuf->b_blkno;

			//将逻辑块号lbn映射到物理盘块号phyBlkno
			this->i_addr[lbn] = phyBlkno;
			this->i_flag |= Inode::IUPD;
		}
		/* 找到预读块对应的物理盘块号 */
		Inode::rablock = 0;
		if (lbn <= 4) {
			/*
			 * i_addr[0] - i_addr[5]为直接索引表。如果预读块对应物理块号可以从
			 * 直接索引表中获得，则记录在Inode::rablock中。
			 */
			Inode::rablock = this->i_addr[lbn + 1];
		}
		return phyBlkno;
	}
	//大型文件，从二级索引获取
	//长度介于7 ~ (128 * 2 + 6)个盘块之间
	else if (lbn < Inode::LARGE_FILE_BLOCK) {
		index = (lbn - Inode::SMALL_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK + 6;

		phyBlkno = this->i_addr[index];
		if (0 == phyBlkno) {
			//混合索引目录不在内存中
			this->i_flag |= Inode::IUPD;
			//分配一空闲盘块存放间接索引表
			pFirstBuf = fileSystem.Alloc(this->i_dev);
			if (pFirstBuf == NULL) {
				//分配失败
				return 0;
			}
			/* i_addr[index]中记录间接索引表的物理盘块号 */
			this->i_addr[index] = pFirstBuf->b_blkno;
		}
		else {
			//混合索引目录已在内存中
			//直接读存储间接索引表的字符块
			pFirstBuf = bufManager.Bread(phyBlkno);
		}

		pFirstBuf = pSecondBuf;
		/* 令iTable指向一次间接索引表 */
		iTable = (int*)pSecondBuf->b_addr;

		/* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */
		index = (lbn - Inode::SMALL_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

		if ((phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSystem.Alloc(this->i_dev)) != NULL) {
			/* 将分配到的文件数据盘块号登记在一次间接索引表中 */
			phyBlkno = pSecondBuf->b_blkno;
			iTable[index] = phyBlkno;
			/* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
			bufManager.Bdwrite(pSecondBuf);
			bufManager.Bdwrite(pFirstBuf);
		}
		else {
			/* 释放一次间接索引表占用缓存 */
			bufManager.Brelse(pFirstBuf);
		}
		/* 找到预读块对应的物理盘块号，如果获取预读块号需要额外的一次for间接索引块的IO，不合算，放弃 */
		Inode::rablock = 0;
		if (index + 1 < Inode::ADDRESS_PER_INDEX_BLOCK) {
			Inode::rablock = iTable[index + 1];
		}
		return phyBlkno;
	}
	//巨型文件，从三级索引获取
	//长度介于263 ~ (128 * 128 * 2 + 128 * 2 + 6)个盘块之间
	else {
		index = (lbn - Inode::LARGE_FILE_BLOCK) / (Inode::ADDRESS_PER_INDEX_BLOCK * Inode::ADDRESS_PER_INDEX_BLOCK) + 8;

		phyBlkno = this->i_addr[index];
		if (0 == phyBlkno){
			//混合索引目录不在内存中
			this->i_flag |= Inode::IUPD;
			//分配一空闲盘块存放间接索引表
			pFirstBuf = fileSystem.Alloc(this->i_dev);
			if (pFirstBuf == NULL) {
				//分配失败
				return 0;
			}
			/* i_addr[index]中记录间接索引表的物理盘块号 */
			this->i_addr[index] = pFirstBuf->b_blkno;
		}
		else {
			//混合索引目录已在内存中
			//直接读存储间接索引表的字符块
			pFirstBuf = bufManager.Bread(phyBlkno);
		}

		/* 获取缓冲区首址 */
		iTable = (int*)pFirstBuf->b_addr;
		/*
		* 对于巨型文件的情况，pFirstBuf中是二次间接索引表，
		* 还需根据逻辑块号，经由二次间接索引表找到一次间接索引表
		*/
		index = ((lbn - Inode::LARGE_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
		/* iTable指向缓存中的二次间接索引表。该项为零，不存在一次间接索引表 */
		phyBlkno = iTable[index];
		if (0 == phyBlkno) {
			if ((pSecondBuf = fileSystem.Alloc(this->i_dev)) == NULL) {
				/* 分配一次间接索引表磁盘块失败，释放缓存中的二次间接索引表，然后返回 */
				bufManager.Brelse(pFirstBuf);
				return 0;
			}
			/* 将新分配的一次间接索引表磁盘块号，记入二次间接索引表相应项 */
			iTable[index] = pSecondBuf->b_blkno;
			/* 将更改后的二次间接索引表延迟写方式输出到磁盘 */
			bufManager.Bdwrite(pFirstBuf);
		}
		else
		{
			/* 释放二次间接索引表占用的缓存，并读入一次间接索引表 */
			bufManager.Brelse(pFirstBuf);
			pSecondBuf = bufManager.Bread(phyBlkno);
		}

		pFirstBuf = pSecondBuf;
		/* 令iTable指向一次间接索引表 */
		iTable = (int*)pSecondBuf->b_addr;

		/* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */
		index = (lbn - Inode::LARGE_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

		if ((phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSystem.Alloc(this->i_dev)) != NULL){
			/* 将分配到的文件数据盘块号登记在一次间接索引表中 */
			phyBlkno = pSecondBuf->b_blkno;
			iTable[index] = phyBlkno;
			/* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
			bufManager.Bdwrite(pSecondBuf);
			bufManager.Bdwrite(pFirstBuf);
		}
		else{
			/* 释放一次间接索引表占用缓存 */
			bufManager.Brelse(pFirstBuf);
		}
		/* 找到预读块对应的物理盘块号，如果获取预读块号需要额外的一次for间接索引块的IO，不合算，放弃 */
		Inode::rablock = 0;
		if (index + 1 < Inode::ADDRESS_PER_INDEX_BLOCK){
			Inode::rablock = iTable[index + 1];
		}
		return phyBlkno;
	}
}


//根据 Inode 对象中的物理磁盘块索引表，读取相应的文件数据
void Inode::ReadI() {
	int lbn;	/* 文件逻辑块号 */
	int bn;		/* lbn对应的物理盘块号 */
	int offset; /* 当前字符块内起始传送位置 */
	int nbytes; /* 传送至用户目标区字节数量 */
	int dev;
	Buf* pBuf;
	User& u = User::GetInstance();
	BufManager& bufManager = BufManager::GetInstance();

	//若需要读字节数为零，则返回
	if (u.u_IOParam.m_Count==0){
		return;
	}

	//正在访问文件，上锁
	this->i_flag |= Inode::IACC;

	//一次一个字符块地读入所需全部数据，直至遇到文件尾
	while (u.u_IOParam.m_Count != 0) {
		lbn = bn = u.u_IOParam.m_Offset / Inode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % Inode::BLOCK_SIZE;
		/* 传送到用户区的字节数量，取读请求的剩余字节数与当前字符块内有效字节数较小值 */
		nbytes = std::min(Inode::BLOCK_SIZE - offset /* 块内有效字节数 */, u.u_IOParam.m_Count);

		int remain = this->i_size - u.u_IOParam.m_Offset;
		/* 如果已读到超过文件结尾 */
		if (remain <= 0) {
			return;
		}

		/* 传送的字节数量还取决于剩余文件的长度 */
		nbytes = min(nbytes, remain);
		//将逻辑块号lbn转换成物理盘块号bn ，Bmap有设置Inode::rablock
		//当UNIX认为获取预读块的开销太大时，
		//会放弃预读，此时 Inode::rablock 值为 0

		//不是特殊块设备文件
		if ((this->i_mode & Inode::IFMT) != Inode::IFBLK) {
			int remain = this->i_size - u.u_IOParam.m_Offset;
			/* 如果已读到超过文件结尾 */
			if (remain <= 0) {
				return;
			}
			/* 传送的字节数量还取决于剩余文件的长度 */
			nbytes = std::min(nbytes, remain);

			/* 将逻辑块号lbn转换成物理盘块号bn ，Bmap有设置Inode::rablock。当UNIX认为获取预读块的开销太大时，
			 * 会放弃预读，此时 Inode::rablock 值为 0。
			 * */
			if ((bn = this->Bmap(lbn)) == 0){
				return;
			}
			dev = this->i_dev;
		}
		//特殊块设备文件
		else {
			dev = this->i_addr[0]; /* 特殊块设备文件i_addr[0]中存放的是设备号 */
			Inode::rablock = bn + 1;
		}

		pBuf = bufManager.Bread(bn);

		/* 记录最近读取字符块的逻辑块号 */
		this->i_lastr = lbn;

		/* 缓存中数据起始读位置 */
		unsigned char* start = pBuf->b_addr + offset;

		/* 读操作: 从缓冲区拷贝到用户目标区
		 * i386芯片用同一张页表映射用户空间和内核空间，这一点硬件上的差异 使得i386上实现 iomove操作
		 * 比PDP-11要容易许多*/
		IOMove(start, u.u_IOParam.m_Base, nbytes);

		/* 用传送字节数nbytes更新读写位置 */
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		bufManager.Brelse(pBuf); /* 使用完缓存，释放该资源 */
	}
}


//根据 Inode 对象中的物理磁盘块索引表
//将IOParam中的文件目标位置拷贝到磁盘中
void Inode::WriteI() {
	int lbn;	/* 文件逻辑块号 */
	int bn;		/* lbn对应的物理盘块号 */
	int offset; /* 当前字符块内起始传送位置 */
	int nbytes; /* 传送字节数量 */
	short dev;
	Buf* pBuf;

	User& u = User::GetInstance();
	BufManager& bufManager = BufManager::GetInstance();


	/* 设置Inode被访问标志位 */
	this->i_flag |= (Inode::IACC | Inode::IUPD);

	//需要读字节数为零，直接返回
	if (0 == u.u_IOParam.m_Count){
		return;
	}

	while (User::ErrorCode::NOERROR == u.u_error && u.u_IOParam.m_Count != 0){
		lbn = u.u_IOParam.m_Offset / Inode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % Inode::BLOCK_SIZE;
		nbytes = std::min(Inode::BLOCK_SIZE - offset, u.u_IOParam.m_Count);

		//普通文件
		if ((this->i_mode & Inode::IFMT) != Inode::IFBLK){
			/* 将逻辑块号lbn转换成物理盘块号bn */
			if ((bn = this->Bmap(lbn)) == 0){
				return;
			}
			dev = this->i_dev;
		}
		//块设备文件，也就是硬盘
		else{
			dev = this->i_addr[0];
		}

		if (Inode::BLOCK_SIZE == nbytes){
			/* 如果写入数据正好满一个字符块，则为其分配缓存 */
			pBuf = bufManager.GetBlk(dev, bn);
		}
		else{
			/* 写入数据不满一个字符块，先读后写（读出该字符块以保护不需要重写的数据） */
			pBuf = bufManager.Bread(bn);
		}

		/* 缓存中数据的起始写位置 */
		unsigned char* start = pBuf->b_addr + offset;

		/* 写操作: 从用户目标区拷贝数据到缓冲区 */
		IOMove(u.u_IOParam.m_Base, start, nbytes);

		/* 用传送字节数nbytes更新读写位置 */
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		//写过程出错
		if (u.u_error != User::ErrorCode::NOERROR){
			bufManager.Brelse(pBuf);
		}
		else if ((u.u_IOParam.m_Offset % Inode::BLOCK_SIZE) == 0){
			//如果写满一个字符块
			/* 以异步方式将字符块写入磁盘，进程不需等待I/O操作结束，可以继续往下执行 */
			bufManager.Bawrite(pBuf);
		}
		else{
			//如果缓冲区未写满
			/* 将缓存标记为延迟写，不急于进行I/O操作将字符块输出到磁盘上 */
			bufManager.Bdwrite(pBuf);
		}

		/* 普通文件长度增加 */
		if ((this->i_size < u.u_IOParam.m_Offset) && (this->i_mode & (Inode::IFBLK & Inode::IFCHR)) == 0){
			this->i_size = u.u_IOParam.m_Offset;
		}

		/*
		 * 之前过程中读盘可能导致进程切换，在进程睡眠期间当前内存Inode可能
		 * 被同步到外存Inode，在此需要重新设置更新标志位。
		 * 好像没有必要呀！即使write系统调用没有上锁，iput看到i_count减到0之后才会将内存i节点同步回磁盘。而这在
		 * 文件没有close之前是不会发生的。
		 * 我们的系统对write系统调用上锁就更不可能出现这种情况了。
		 * 真的想把它去掉。
		 */
		this->i_flag |= Inode::IUPD;
	}
}


/*更新磁盘中的Inode节点信息*/
void Inode::IUpdate(int time){
	Buf* pBuf;
	DiskInode dInode;
	FileSystem& filesys = FileSystem::GetInstance();
	User& u = User::GetInstance();
	BufManager& bufManager = BufManager::GetInstance();
	//DeviceManager& devMgr = *DeviceManager::getInst();

	/* 当IUPD和IACC标志之一被设置，才需要更新相应DiskInode
	 * 目录搜索，不会设置所途径的目录文件的IACC和IUPD标志 */
	if ((this->i_flag & (Inode::IUPD | Inode::IACC)) != 0){
		if (filesys.GetSuperBlock()->s_ronly != 0){
			/* 如果该文件系统只读 */
			return;
		}

		/* DR：在缓存池中找到包含本i节点（this->i_number）的缓存块
		 * 这是一个上锁的缓存块，本段代码中的Bwrite()在将缓存块写回磁盘后会释放该缓存块。
		 * 将该存放该DiskInode的字符块读入缓冲区 */
		pBuf = bufManager.Bread( FileSystem::INODE_ZONE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);

		/* 将内存Inode副本中的信息复制到dInode中，然后将dInode覆盖缓存中旧的外存Inode */
		dInode.d_mode = this->i_mode;
		dInode.d_nlink = this->i_nlink;

		//你俩都是不需要的
		//dInode.d_uid = this->i_uid;
		//dInode.d_gid = this->i_gid;
		
		dInode.d_size = this->i_size;
		for (int i = 0; i < 10; i++){
			dInode.d_addr[i] = this->i_addr[i];
		}
		if (this->i_flag & Inode::IACC){
			/* 更新最后访问时间 */
			dInode.d_atime = time;
		}
		if (this->i_flag & Inode::IUPD){
			/* 更新最后访问时间 */
			dInode.d_mtime = time;
		}

		/* 将p指向缓存区中旧外存Inode的偏移位置 */
		unsigned char* p = pBuf->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
		DiskInode* pNode = &dInode;

		/* 用dInode中的新数据覆盖缓存中的旧外存Inode */
		DWordCopy((int*)pNode, (int*)p, sizeof(DiskInode) / sizeof(int));

		/* 将缓存写回至磁盘，达到更新旧外存Inode的目的 */
		bufManager.Bwrite(pBuf);
	}
}


/*删此Inode对应的文件内容*/
void Inode::ITrunc(){
	/* 经由磁盘高速缓存读取存放一次间接、两次间接索引表的磁盘块 */
	BufManager& bufManager = BufManager::GetInstance();
	/* 获取g_FileSystem对象的引用，执行释放磁盘块的操作 */
	FileSystem& filesys = FileSystem::GetInstance();


	//采用FILO方式释放，以尽量使得SuperBlock中记录的空闲盘块号连续
	//一级索引目录i_addr[5]到i_addr[0]
	for (int i = 5; i >= 0; i--) {
		//不存在索引直接跳过
		if (this->i_addr[i] == 0) {
			continue;
		}

		/* 释放索引表本身占用的磁盘块 */
		filesys.Free(this->i_dev, this->i_addr[i]);
		/* 0表示该项不包含索引 */
		this->i_addr[i] = 0;
	}

	//二级索引目录i_addr[7]到i_addr[6]
	for (int i = 7; i >= 6; i--) {
		//不存在索引直接跳过
		if (this->i_addr[i] == 0) {
			continue;
		}

		/* 将间接索引表读入缓存 */
		Buf* pFirstBuf = bufManager.Bread(this->i_addr[i]);
		/* 获取缓冲区首址 */
		int* pFirst = (int*)pFirstBuf->b_addr;

		/* 每张间接索引表记录 512/sizeof(int) = 128个磁盘块号，遍历这全部128个磁盘块 */
		for (int j = 128 - 1; j >= 0; j--) {
			//如果该项存在索引
			if (pFirst[j] != 0) {
				filesys.Free(this->i_dev, pFirst[j]);
			}
		}
		bufManager.Brelse(pFirstBuf);

		/* 释放索引表本身占用的磁盘块 */
		filesys.Free(this->i_dev, this->i_addr[i]);
		/* 0表示该项不包含索引 */
		this->i_addr[i] = 0;
	}

	//三级索引目录i_addr[9]到i_addr[8]
	for (int i = 9; i >= 8; i--) {
		//不存在索引直接跳过
		if (this->i_addr[i] == 0) {
			continue;
		}

		/* 将间接索引表读入缓存 */
		Buf* pFirstBuf = bufManager.Bread(this->i_addr[i]);
		/* 获取缓冲区首址 */
		int* pFirst = (int*)pFirstBuf->b_addr;

		/* 每张间接索引表记录 512/sizeof(int) = 128个磁盘块号，遍历这全部128个磁盘块 */
		for (int j = 128 - 1; j >= 0; j--){
			//如果该项存在索引
			if (pFirst[j] != 0){
				/*
				* 如果是两次间接索引表，i_addr[8]或i_addr[9]项，
				* 那么该字符块记录的是128个一次间接索引表存放的磁盘块号
				*/
				Buf* pSecondBuf = bufManager.Bread(pFirst[j]);
				int* pSecond = (int*)pSecondBuf->b_addr;

				for (int k = 128 - 1; k >= 0; k--){
					if (pSecond[k] != 0){
						/* 释放指定的磁盘块 */
						filesys.Free(this->i_dev, pSecond[k]);
					}
				}
				/* 缓存使用完毕，释放以便被其它进程使用 */
				bufManager.Brelse(pSecondBuf);
						
				filesys.Free(this->i_dev, pFirst[j]);
			}
		}
		bufManager.Brelse(pFirstBuf);
			
		/* 释放索引表本身占用的磁盘块 */
		filesys.Free(this->i_dev, this->i_addr[i]);
		/* 0表示该项不包含索引 */
		this->i_addr[i] = 0;
	}


	/* 盘块释放完毕，文件大小清零 */
	this->i_size = 0;
	/* 增设IUPD标志位，表示此内存Inode需要同步到相应外存Inode */
	this->i_flag |= Inode::IUPD;
	/* 清大文件标志 和原来的RWXRWXRWX比特*/
	this->i_mode &= ~(Inode::ILARG & Inode::IRWXU & Inode::IRWXG & Inode::IRWXO);
	this->i_nlink = 1;
}


//特定用于IAlloc()中清空新分配的DiskInode内部的原有数据
//本函数中不应当清除i_dev, i_number, i_flag, i_count,因为：
//这是内存Inode而非DiskInode包含的旧文件信息，
//Inode类构造函数才需要将其初始化为无效值
void Inode::Clean(){
	 // this->i_flag = 0;
	this->i_mode = 0;
	// this->i_count = 0;
	this->i_nlink = 0;
	// this->i_dev = -1;
	// this->i_number = -1;
	this->i_uid = -1;
	this->i_gid = -1;
	this->i_size = 0;
	this->i_lastr = -1;
	for (int i = 0; i < 10; i++){
		this->i_addr[i] = 0;
	}
}


/*将已有缓存中的第i个Inode拷贝到内存*/
void Inode::ICopy(Buf* bp, int inumber){
	DiskInode dInode;
	DiskInode* pNode = &dInode;

	/* 将p指向缓存区中编号为inumber外存Inode的偏移位置 */
	unsigned char* p = bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
	/* 将缓存中外存Inode数据拷贝到临时变量dInode中，按4字节拷贝 */
	DWordCopy((int*)p, (int*)pNode, sizeof(DiskInode) / sizeof(int));

	/* 将外存Inode变量dInode中信息复制到内存Inode中 */
	this->i_mode = dInode.d_mode;
	this->i_nlink = dInode.d_nlink;
	//this->i_uid = dInode.d_uid;
	//this->i_gid = dInode.d_gid;
	this->i_size = dInode.d_size;
	for (int i = 0; i < 10; i++){
		this->i_addr[i] = dInode.d_addr[i];
	}
}


/*================================ DiskInode ===================================*/
DiskInode::DiskInode(){}
DiskInode::~DiskInode(){}
