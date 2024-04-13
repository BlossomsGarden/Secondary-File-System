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


//�߼����ת���ɶ�Ӧ�������̿��
//ʹ�û������Ŀ¼�ķ�ʽ����
int Inode::Bmap(int lbn) {
	Buf* pFirstBuf;
	Buf* pSecondBuf;
	int phyBlkno; /* ת����������̿�� */
	int* iTable;  /* ���ڷ��������̿���һ�μ�ӡ����μ�������� */
	int index;
	User& u = User::GetInstance();

	BufManager& bufManager = BufManager::GetInstance();
	FileSystem& fileSystem = FileSystem::GetInstance();

	/*
	 * Unix V6++���ļ������ṹ��(С�͡����ͺ;����ļ�)
	 * (1) i_addr[0] - i_addr[5]Ϊֱ���������ļ����ȷ�Χ��0 - 6���̿飻
	 *
	 * (2) i_addr[6] - i_addr[7]���һ�μ�����������ڴ��̿�ţ�ÿ���̿�
	 * �ϴ��128���ļ������̿�ţ������ļ����ȷ�Χ��7 - (128 * 2 + 6)���̿飻
	 *
	 * (3) i_addr[8] - i_addr[9]��Ŷ��μ�����������ڴ��̿�ţ�ÿ�����μ��
	 * �������¼128��һ�μ�����������ڴ��̿�ţ������ļ����ȷ�Χ��
	 * (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	 */


	// Ӧ�ò����ڳ���lbn����Inode::HUGE_FILE_BLOCK����������ɡ�����

	//С���ļ���һ������i_addr[0 - 5]�л�������̿��
	if (lbn < 6) {
		phyBlkno = this->i_addr[lbn];

		/*
		 * ������߼���Ż�û����Ӧ�������̿����֮��Ӧ�������һ������顣
		 * ��ͨ�������ڶ��ļ���д�룬��д��λ�ó����ļ���С�����Ե�ǰ
		 * �ļ���������д�룬����Ҫ�������Ĵ��̿飬��Ϊ֮�����߼����
		 * �������̿��֮���ӳ�䡣
		 */
		if (phyBlkno == 0 && (pFirstBuf = fileSystem.Alloc(this->i_dev)) != NULL) {
			//fileSys.Alloc����һ���ɾ��飬dev��blknoָ��һ�������
			bufManager.Bdwrite(pFirstBuf);
			phyBlkno = pFirstBuf->b_blkno;

			//���߼����lbnӳ�䵽�����̿��phyBlkno
			this->i_addr[lbn] = phyBlkno;
			this->i_flag |= Inode::IUPD;
		}
		/* �ҵ�Ԥ�����Ӧ�������̿�� */
		Inode::rablock = 0;
		if (lbn <= 4) {
			/*
			 * i_addr[0] - i_addr[5]Ϊֱ�����������Ԥ�����Ӧ�����ſ��Դ�
			 * ֱ���������л�ã����¼��Inode::rablock�С�
			 */
			Inode::rablock = this->i_addr[lbn + 1];
		}
		return phyBlkno;
	}
	//�����ļ����Ӷ���������ȡ
	//���Ƚ���7 ~ (128 * 2 + 6)���̿�֮��
	else if (lbn < Inode::LARGE_FILE_BLOCK) {
		index = (lbn - Inode::SMALL_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK + 6;

		phyBlkno = this->i_addr[index];
		if (0 == phyBlkno) {
			//�������Ŀ¼�����ڴ���
			this->i_flag |= Inode::IUPD;
			//����һ�����̿��ż��������
			pFirstBuf = fileSystem.Alloc(this->i_dev);
			if (pFirstBuf == NULL) {
				//����ʧ��
				return 0;
			}
			/* i_addr[index]�м�¼���������������̿�� */
			this->i_addr[index] = pFirstBuf->b_blkno;
		}
		else {
			//�������Ŀ¼�����ڴ���
			//ֱ�Ӷ��洢�����������ַ���
			pFirstBuf = bufManager.Bread(phyBlkno);
		}

		pFirstBuf = pSecondBuf;
		/* ��iTableָ��һ�μ�������� */
		iTable = (int*)pSecondBuf->b_addr;

		/* �����߼����lbn����λ��һ�μ���������еı������index */
		index = (lbn - Inode::SMALL_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

		if ((phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSystem.Alloc(this->i_dev)) != NULL) {
			/* �����䵽���ļ������̿�ŵǼ���һ�μ���������� */
			phyBlkno = pSecondBuf->b_blkno;
			iTable[index] = phyBlkno;
			/* �������̿顢���ĺ��һ�μ�����������ӳ�д��ʽ��������� */
			bufManager.Bdwrite(pSecondBuf);
			bufManager.Bdwrite(pFirstBuf);
		}
		else {
			/* �ͷ�һ�μ��������ռ�û��� */
			bufManager.Brelse(pFirstBuf);
		}
		/* �ҵ�Ԥ�����Ӧ�������̿�ţ������ȡԤ�������Ҫ�����һ��for����������IO�������㣬���� */
		Inode::rablock = 0;
		if (index + 1 < Inode::ADDRESS_PER_INDEX_BLOCK) {
			Inode::rablock = iTable[index + 1];
		}
		return phyBlkno;
	}
	//�����ļ���������������ȡ
	//���Ƚ���263 ~ (128 * 128 * 2 + 128 * 2 + 6)���̿�֮��
	else {
		index = (lbn - Inode::LARGE_FILE_BLOCK) / (Inode::ADDRESS_PER_INDEX_BLOCK * Inode::ADDRESS_PER_INDEX_BLOCK) + 8;

		phyBlkno = this->i_addr[index];
		if (0 == phyBlkno){
			//�������Ŀ¼�����ڴ���
			this->i_flag |= Inode::IUPD;
			//����һ�����̿��ż��������
			pFirstBuf = fileSystem.Alloc(this->i_dev);
			if (pFirstBuf == NULL) {
				//����ʧ��
				return 0;
			}
			/* i_addr[index]�м�¼���������������̿�� */
			this->i_addr[index] = pFirstBuf->b_blkno;
		}
		else {
			//�������Ŀ¼�����ڴ���
			//ֱ�Ӷ��洢�����������ַ���
			pFirstBuf = bufManager.Bread(phyBlkno);
		}

		/* ��ȡ��������ַ */
		iTable = (int*)pFirstBuf->b_addr;
		/*
		* ���ھ����ļ��������pFirstBuf���Ƕ��μ��������
		* ��������߼���ţ����ɶ��μ���������ҵ�һ�μ��������
		*/
		index = ((lbn - Inode::LARGE_FILE_BLOCK) / Inode::ADDRESS_PER_INDEX_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;
		/* iTableָ�򻺴��еĶ��μ������������Ϊ�㣬������һ�μ�������� */
		phyBlkno = iTable[index];
		if (0 == phyBlkno) {
			if ((pSecondBuf = fileSystem.Alloc(this->i_dev)) == NULL) {
				/* ����һ�μ����������̿�ʧ�ܣ��ͷŻ����еĶ��μ��������Ȼ�󷵻� */
				bufManager.Brelse(pFirstBuf);
				return 0;
			}
			/* ���·����һ�μ����������̿�ţ�������μ����������Ӧ�� */
			iTable[index] = pSecondBuf->b_blkno;
			/* �����ĺ�Ķ��μ���������ӳ�д��ʽ��������� */
			bufManager.Bdwrite(pFirstBuf);
		}
		else
		{
			/* �ͷŶ��μ��������ռ�õĻ��棬������һ�μ�������� */
			bufManager.Brelse(pFirstBuf);
			pSecondBuf = bufManager.Bread(phyBlkno);
		}

		pFirstBuf = pSecondBuf;
		/* ��iTableָ��һ�μ�������� */
		iTable = (int*)pSecondBuf->b_addr;

		/* �����߼����lbn����λ��һ�μ���������еı������index */
		index = (lbn - Inode::LARGE_FILE_BLOCK) % Inode::ADDRESS_PER_INDEX_BLOCK;

		if ((phyBlkno = iTable[index]) == 0 && (pSecondBuf = fileSystem.Alloc(this->i_dev)) != NULL){
			/* �����䵽���ļ������̿�ŵǼ���һ�μ���������� */
			phyBlkno = pSecondBuf->b_blkno;
			iTable[index] = phyBlkno;
			/* �������̿顢���ĺ��һ�μ�����������ӳ�д��ʽ��������� */
			bufManager.Bdwrite(pSecondBuf);
			bufManager.Bdwrite(pFirstBuf);
		}
		else{
			/* �ͷ�һ�μ��������ռ�û��� */
			bufManager.Brelse(pFirstBuf);
		}
		/* �ҵ�Ԥ�����Ӧ�������̿�ţ������ȡԤ�������Ҫ�����һ��for����������IO�������㣬���� */
		Inode::rablock = 0;
		if (index + 1 < Inode::ADDRESS_PER_INDEX_BLOCK){
			Inode::rablock = iTable[index + 1];
		}
		return phyBlkno;
	}
}


//���� Inode �����е�������̿���������ȡ��Ӧ���ļ�����
void Inode::ReadI() {
	int lbn;	/* �ļ��߼���� */
	int bn;		/* lbn��Ӧ�������̿�� */
	int offset; /* ��ǰ�ַ�������ʼ����λ�� */
	int nbytes; /* �������û�Ŀ�����ֽ����� */
	int dev;
	Buf* pBuf;
	User& u = User::GetInstance();
	BufManager& bufManager = BufManager::GetInstance();

	//����Ҫ���ֽ���Ϊ�㣬�򷵻�
	if (u.u_IOParam.m_Count==0){
		return;
	}

	//���ڷ����ļ�������
	this->i_flag |= Inode::IACC;

	//һ��һ���ַ���ض�������ȫ�����ݣ�ֱ�������ļ�β
	while (u.u_IOParam.m_Count != 0) {
		lbn = bn = u.u_IOParam.m_Offset / Inode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % Inode::BLOCK_SIZE;
		/* ���͵��û������ֽ�������ȡ�������ʣ���ֽ����뵱ǰ�ַ�������Ч�ֽ�����Сֵ */
		nbytes = std::min(Inode::BLOCK_SIZE - offset /* ������Ч�ֽ��� */, u.u_IOParam.m_Count);

		int remain = this->i_size - u.u_IOParam.m_Offset;
		/* ����Ѷ��������ļ���β */
		if (remain <= 0) {
			return;
		}

		/* ���͵��ֽ�������ȡ����ʣ���ļ��ĳ��� */
		nbytes = min(nbytes, remain);
		//���߼����lbnת���������̿��bn ��Bmap������Inode::rablock
		//��UNIX��Ϊ��ȡԤ����Ŀ���̫��ʱ��
		//�����Ԥ������ʱ Inode::rablock ֵΪ 0

		//����������豸�ļ�
		if ((this->i_mode & Inode::IFMT) != Inode::IFBLK) {
			int remain = this->i_size - u.u_IOParam.m_Offset;
			/* ����Ѷ��������ļ���β */
			if (remain <= 0) {
				return;
			}
			/* ���͵��ֽ�������ȡ����ʣ���ļ��ĳ��� */
			nbytes = std::min(nbytes, remain);

			/* ���߼����lbnת���������̿��bn ��Bmap������Inode::rablock����UNIX��Ϊ��ȡԤ����Ŀ���̫��ʱ��
			 * �����Ԥ������ʱ Inode::rablock ֵΪ 0��
			 * */
			if ((bn = this->Bmap(lbn)) == 0){
				return;
			}
			dev = this->i_dev;
		}
		//������豸�ļ�
		else {
			dev = this->i_addr[0]; /* ������豸�ļ�i_addr[0]�д�ŵ����豸�� */
			Inode::rablock = bn + 1;
		}

		pBuf = bufManager.Bread(bn);

		/* ��¼�����ȡ�ַ�����߼���� */
		this->i_lastr = lbn;

		/* ������������ʼ��λ�� */
		unsigned char* start = pBuf->b_addr + offset;

		/* ������: �ӻ������������û�Ŀ����
		 * i386оƬ��ͬһ��ҳ��ӳ���û��ռ���ں˿ռ䣬��һ��Ӳ���ϵĲ��� ʹ��i386��ʵ�� iomove����
		 * ��PDP-11Ҫ�������*/
		IOMove(start, u.u_IOParam.m_Base, nbytes);

		/* �ô����ֽ���nbytes���¶�дλ�� */
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		bufManager.Brelse(pBuf); /* ʹ���껺�棬�ͷŸ���Դ */
	}
}


//���� Inode �����е�������̿�������
//��IOParam�е��ļ�Ŀ��λ�ÿ�����������
void Inode::WriteI() {
	int lbn;	/* �ļ��߼���� */
	int bn;		/* lbn��Ӧ�������̿�� */
	int offset; /* ��ǰ�ַ�������ʼ����λ�� */
	int nbytes; /* �����ֽ����� */
	short dev;
	Buf* pBuf;

	User& u = User::GetInstance();
	BufManager& bufManager = BufManager::GetInstance();


	/* ����Inode�����ʱ�־λ */
	this->i_flag |= (Inode::IACC | Inode::IUPD);

	//��Ҫ���ֽ���Ϊ�㣬ֱ�ӷ���
	if (0 == u.u_IOParam.m_Count){
		return;
	}

	while (User::ErrorCode::NOERROR == u.u_error && u.u_IOParam.m_Count != 0){
		lbn = u.u_IOParam.m_Offset / Inode::BLOCK_SIZE;
		offset = u.u_IOParam.m_Offset % Inode::BLOCK_SIZE;
		nbytes = std::min(Inode::BLOCK_SIZE - offset, u.u_IOParam.m_Count);

		//��ͨ�ļ�
		if ((this->i_mode & Inode::IFMT) != Inode::IFBLK){
			/* ���߼����lbnת���������̿��bn */
			if ((bn = this->Bmap(lbn)) == 0){
				return;
			}
			dev = this->i_dev;
		}
		//���豸�ļ���Ҳ����Ӳ��
		else{
			dev = this->i_addr[0];
		}

		if (Inode::BLOCK_SIZE == nbytes){
			/* ���д������������һ���ַ��飬��Ϊ����仺�� */
			pBuf = bufManager.GetBlk(dev, bn);
		}
		else{
			/* д�����ݲ���һ���ַ��飬�ȶ���д���������ַ����Ա�������Ҫ��д�����ݣ� */
			pBuf = bufManager.Bread(bn);
		}

		/* ���������ݵ���ʼдλ�� */
		unsigned char* start = pBuf->b_addr + offset;

		/* д����: ���û�Ŀ�����������ݵ������� */
		IOMove(u.u_IOParam.m_Base, start, nbytes);

		/* �ô����ֽ���nbytes���¶�дλ�� */
		u.u_IOParam.m_Base += nbytes;
		u.u_IOParam.m_Offset += nbytes;
		u.u_IOParam.m_Count -= nbytes;

		//д���̳���
		if (u.u_error != User::ErrorCode::NOERROR){
			bufManager.Brelse(pBuf);
		}
		else if ((u.u_IOParam.m_Offset % Inode::BLOCK_SIZE) == 0){
			//���д��һ���ַ���
			/* ���첽��ʽ���ַ���д����̣����̲���ȴ�I/O�������������Լ�������ִ�� */
			bufManager.Bawrite(pBuf);
		}
		else{
			//���������δд��
			/* ��������Ϊ�ӳ�д�������ڽ���I/O�������ַ�������������� */
			bufManager.Bdwrite(pBuf);
		}

		/* ��ͨ�ļ��������� */
		if ((this->i_size < u.u_IOParam.m_Offset) && (this->i_mode & (Inode::IFBLK & Inode::IFCHR)) == 0){
			this->i_size = u.u_IOParam.m_Offset;
		}

		/*
		 * ֮ǰ�����ж��̿��ܵ��½����л����ڽ���˯���ڼ䵱ǰ�ڴ�Inode����
		 * ��ͬ�������Inode���ڴ���Ҫ�������ø��±�־λ��
		 * ����û�б�Ҫѽ����ʹwriteϵͳ����û��������iput����i_count����0֮��ŻὫ�ڴ�i�ڵ�ͬ���ش��̡�������
		 * �ļ�û��close֮ǰ�ǲ��ᷢ���ġ�
		 * ���ǵ�ϵͳ��writeϵͳ���������͸������ܳ�����������ˡ�
		 * ��������ȥ����
		 */
		this->i_flag |= Inode::IUPD;
	}
}


/*���´����е�Inode�ڵ���Ϣ*/
void Inode::IUpdate(int time){
	Buf* pBuf;
	DiskInode dInode;
	FileSystem& filesys = FileSystem::GetInstance();
	User& u = User::GetInstance();
	BufManager& bufManager = BufManager::GetInstance();
	//DeviceManager& devMgr = *DeviceManager::getInst();

	/* ��IUPD��IACC��־֮һ�����ã�����Ҫ������ӦDiskInode
	 * Ŀ¼����������������;����Ŀ¼�ļ���IACC��IUPD��־ */
	if ((this->i_flag & (Inode::IUPD | Inode::IACC)) != 0){
		if (filesys.GetSuperBlock()->s_ronly != 0){
			/* ������ļ�ϵͳֻ�� */
			return;
		}

		/* DR���ڻ�������ҵ�������i�ڵ㣨this->i_number���Ļ����
		 * ����һ�������Ļ���飬���δ����е�Bwrite()�ڽ������д�ش��̺���ͷŸû���顣
		 * ���ô�Ÿ�DiskInode���ַ�����뻺���� */
		pBuf = bufManager.Bread( FileSystem::INODE_ZONE_START_SECTOR + this->i_number / FileSystem::INODE_NUMBER_PER_SECTOR);

		/* ���ڴ�Inode�����е���Ϣ���Ƶ�dInode�У�Ȼ��dInode���ǻ����оɵ����Inode */
		dInode.d_mode = this->i_mode;
		dInode.d_nlink = this->i_nlink;

		//�������ǲ���Ҫ��
		//dInode.d_uid = this->i_uid;
		//dInode.d_gid = this->i_gid;
		
		dInode.d_size = this->i_size;
		for (int i = 0; i < 10; i++){
			dInode.d_addr[i] = this->i_addr[i];
		}
		if (this->i_flag & Inode::IACC){
			/* ����������ʱ�� */
			dInode.d_atime = time;
		}
		if (this->i_flag & Inode::IUPD){
			/* ����������ʱ�� */
			dInode.d_mtime = time;
		}

		/* ��pָ�򻺴����о����Inode��ƫ��λ�� */
		unsigned char* p = pBuf->b_addr + (this->i_number % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
		DiskInode* pNode = &dInode;

		/* ��dInode�е������ݸ��ǻ����еľ����Inode */
		DWordCopy((int*)pNode, (int*)p, sizeof(DiskInode) / sizeof(int));

		/* ������д�������̣��ﵽ���¾����Inode��Ŀ�� */
		bufManager.Bwrite(pBuf);
	}
}


/*ɾ��Inode��Ӧ���ļ�����*/
void Inode::ITrunc(){
	/* ���ɴ��̸��ٻ����ȡ���һ�μ�ӡ����μ��������Ĵ��̿� */
	BufManager& bufManager = BufManager::GetInstance();
	/* ��ȡg_FileSystem��������ã�ִ���ͷŴ��̿�Ĳ��� */
	FileSystem& filesys = FileSystem::GetInstance();


	//����FILO��ʽ�ͷţ��Ծ���ʹ��SuperBlock�м�¼�Ŀ����̿������
	//һ������Ŀ¼i_addr[5]��i_addr[0]
	for (int i = 5; i >= 0; i--) {
		//����������ֱ������
		if (this->i_addr[i] == 0) {
			continue;
		}

		/* �ͷ���������ռ�õĴ��̿� */
		filesys.Free(this->i_dev, this->i_addr[i]);
		/* 0��ʾ����������� */
		this->i_addr[i] = 0;
	}

	//��������Ŀ¼i_addr[7]��i_addr[6]
	for (int i = 7; i >= 6; i--) {
		//����������ֱ������
		if (this->i_addr[i] == 0) {
			continue;
		}

		/* �������������뻺�� */
		Buf* pFirstBuf = bufManager.Bread(this->i_addr[i]);
		/* ��ȡ��������ַ */
		int* pFirst = (int*)pFirstBuf->b_addr;

		/* ÿ�ż���������¼ 512/sizeof(int) = 128�����̿�ţ�������ȫ��128�����̿� */
		for (int j = 128 - 1; j >= 0; j--) {
			//��������������
			if (pFirst[j] != 0) {
				filesys.Free(this->i_dev, pFirst[j]);
			}
		}
		bufManager.Brelse(pFirstBuf);

		/* �ͷ���������ռ�õĴ��̿� */
		filesys.Free(this->i_dev, this->i_addr[i]);
		/* 0��ʾ����������� */
		this->i_addr[i] = 0;
	}

	//��������Ŀ¼i_addr[9]��i_addr[8]
	for (int i = 9; i >= 8; i--) {
		//����������ֱ������
		if (this->i_addr[i] == 0) {
			continue;
		}

		/* �������������뻺�� */
		Buf* pFirstBuf = bufManager.Bread(this->i_addr[i]);
		/* ��ȡ��������ַ */
		int* pFirst = (int*)pFirstBuf->b_addr;

		/* ÿ�ż���������¼ 512/sizeof(int) = 128�����̿�ţ�������ȫ��128�����̿� */
		for (int j = 128 - 1; j >= 0; j--){
			//��������������
			if (pFirst[j] != 0){
				/*
				* ��������μ��������i_addr[8]��i_addr[9]�
				* ��ô���ַ����¼����128��һ�μ���������ŵĴ��̿��
				*/
				Buf* pSecondBuf = bufManager.Bread(pFirst[j]);
				int* pSecond = (int*)pSecondBuf->b_addr;

				for (int k = 128 - 1; k >= 0; k--){
					if (pSecond[k] != 0){
						/* �ͷ�ָ���Ĵ��̿� */
						filesys.Free(this->i_dev, pSecond[k]);
					}
				}
				/* ����ʹ����ϣ��ͷ��Ա㱻��������ʹ�� */
				bufManager.Brelse(pSecondBuf);
						
				filesys.Free(this->i_dev, pFirst[j]);
			}
		}
		bufManager.Brelse(pFirstBuf);
			
		/* �ͷ���������ռ�õĴ��̿� */
		filesys.Free(this->i_dev, this->i_addr[i]);
		/* 0��ʾ����������� */
		this->i_addr[i] = 0;
	}


	/* �̿��ͷ���ϣ��ļ���С���� */
	this->i_size = 0;
	/* ����IUPD��־λ����ʾ���ڴ�Inode��Ҫͬ������Ӧ���Inode */
	this->i_flag |= Inode::IUPD;
	/* ����ļ���־ ��ԭ����RWXRWXRWX����*/
	this->i_mode &= ~(Inode::ILARG & Inode::IRWXU & Inode::IRWXG & Inode::IRWXO);
	this->i_nlink = 1;
}


//�ض�����IAlloc()������·����DiskInode�ڲ���ԭ������
//�������в�Ӧ�����i_dev, i_number, i_flag, i_count,��Ϊ��
//�����ڴ�Inode����DiskInode�����ľ��ļ���Ϣ��
//Inode�๹�캯������Ҫ�����ʼ��Ϊ��Чֵ
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


/*�����л����еĵ�i��Inode�������ڴ�*/
void Inode::ICopy(Buf* bp, int inumber){
	DiskInode dInode;
	DiskInode* pNode = &dInode;

	/* ��pָ�򻺴����б��Ϊinumber���Inode��ƫ��λ�� */
	unsigned char* p = bp->b_addr + (inumber % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskInode);
	/* �����������Inode���ݿ�������ʱ����dInode�У���4�ֽڿ��� */
	DWordCopy((int*)p, (int*)pNode, sizeof(DiskInode) / sizeof(int));

	/* �����Inode����dInode����Ϣ���Ƶ��ڴ�Inode�� */
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
