#include "BufManager.h"
#include "Windows.h"
#include <thread>
using namespace std;


BufManager::BufManager(){}
BufManager ::~BufManager(){}

void BufManager::Initialize() {
    memset((void*)Buffer, 0, NBUF * BUFFER_SIZE);
    // ��ʼ��Bufʵ�ʵ�ַ
    for (int i = 0; i < this->NBUF; i++){
        m_Buf[i].b_addr = Buffer[i];
        m_Buf[i].b_back = ((i + 1) < NBUF ? (&m_Buf[i + 1]) : nullptr);
        m_Buf[i].b_forw = (i - 1) >= 0 ? &m_Buf[i - 1] : &bFreeList;
        m_Buf[i].b_no = i;
    }

    bFreeList.b_back = &m_Buf[0];
    bFreeList.b_forw = nullptr;
}


//ģ��Kernel���ȡ�������������ʵ��
BufManager& BufManager::GetInstance() {
	return BufManager::instance;
}
Buf& BufManager::GetBFreeList(){
    return BufManager::bFreeList;
}


// ����������еĿ�ȡ����������BUSY����
void BufManager::NotAvail(Buf* bp) {
    // ���bp�Ƕ�βbp->b_backΪnullptr
    if (bp->b_back) {
        bp->b_back->b_forw = bp->b_forw;
    }
    bp->b_forw->b_back = bp->b_back;
    bp->b_back = nullptr;
    bp->b_forw = nullptr;
    /* ����B_BUSY��־ */
    bp->b_flags |= Buf::B_BUSY;
    return;
}


//���������ݿ�<dev��blkno>ͬ�����뻺��أ��������׵�ַ
//GetBlk��ס��������Ļ�����ƿ飬û�еĻ�ȥ����һ��
//Bread��GetBlk���������
Buf* BufManager::Bread(int blkno) {
    // ���豸��
    Buf* bp = GetBlk(dev, blkno);

    // �����ڻ�������У���B_DONE�����ã������������ */
    if (bp->b_flags & Buf::B_DONE) {
        return bp;
    }

    //todo:����IO���󣬶�512���ֽ�
    //DeviceManager::getInst()->GetBlockDevice(dev)->Read(bp);

    bp->b_flags |= Buf::BufFlag::B_DONE;
    return bp;
}

Buf* BufManager::GetBlk(int dev, int blkno) {
    Buf* buf_reuse = nullptr;
    // ����ȫ�����棬����û��֮ǰ�Ѿ�����������������õ�
    for (int i = 0; i < NBUF; i++) {
        if (m_Buf[i].b_dev == dev && m_Buf[i].b_blkno == blkno) {
            buf_reuse = &m_Buf[i];
            break;
        }
    }
    // �ҵ��˿����õĻ���
    if (buf_reuse != nullptr && (buf_reuse->b_flags & (Buf::BufFlag::B_BUSY))){
        // B_BUSYʱGetblk��Ӧ���ǿ����ڽ����첽IO���ȿ���
        buf_mutex[buf_reuse->b_no].lock();
        buf_mutex[buf_reuse->b_no].unlock();
        NotAvail(buf_reuse);
        return buf_reuse;
    }
    else if (buf_reuse != nullptr && (buf_reuse->b_flags & (Buf::BufFlag::B_DONE))) {
        //�������ɿ飬ֱ�Ӽ�B_BUSY��־���Ƴ�����
        NotAvail(buf_reuse);
        return buf_reuse;
    }


    Buf* buf_first = bFreeList.b_back;
    buf_first = bFreeList.b_back;
    // �Ҳ��������ÿ飬ȡ�����ɶ��ж��װ�
    if (buf_reuse == nullptr && buf_first) {
        NotAvail(buf_first);
        //�����������ӳ�д���
        if (buf_first->b_flags & Buf::BufFlag::B_DELWRI) {
            // ֱ����Bwrite
            Bwrite(buf_first);
            return GetBlk(dev, blkno);
        }
        buf_first->b_blkno = blkno;
        buf_first->b_dev = dev;
        buf_first->b_flags = Buf::BufFlag::B_BUSY;
        return buf_first;
    }
    // ʵ�����Ҳ����κο��п�.....
    else if (buf_reuse == nullptr && !bFreeList.b_back) {
        // ˯�������
        Sleep(200);
        return GetBlk(dev, blkno);
    }

    //�������Ҳ��֪������ʲô���
    return nullptr;
}


//�첽д����ɺ��ͷ�
//��Bwrite��ע�����߳������á��Ի���������ע�������λ�þ������ǶԻ���ĸ���
void _Bawrite(BufManager* bufmgr, Buf* bp) {
    bufmgr->Bawrite(bp);
}
void BufManager::Bawrite(Buf* bp)
{
    /* ���Ϊ�첽д */
    buf_mutex[bp->b_no].lock();
    bp->b_flags |= (Buf::B_ASYNC | Buf::B_BUSY);

    //todo:����IO����д��512���ֽ�
    //DeviceManager::getInst()->GetBlockDevice(bp->b_dev)->Write(bp);

    bp->b_flags &= ~Buf::B_ASYNC;
    /*������λ�þ������ǶԻ���ĸ��� �������ͷ��ٽ���*/
    Brelse(bp);
    buf_mutex[bp->b_no].unlock();
    return;
}
//�õ�һ���鲢�Ѿ��Ի�����ɴ�������ͬ�����첽д������B_DONE��־���ͷ�
void BufManager::Bwrite(Buf* bp){
    unsigned int flags;

    flags = bp->b_flags;
    bp->b_flags &= ~(Buf::B_READ | Buf::B_DONE | Buf::B_DELWRI);
    bp->b_wcount = BufManager::BUFFER_SIZE; /* 512�ֽ� */

    // ͬ��д
    if ((flags & Buf::B_ASYNC) == 0){
        //todo:����IO����д��512���ֽ�
        //DeviceManager::getInst()->GetBlockDevice(bp->b_dev)->Write(bp);
        bp->b_flags |= Buf::BufFlag::B_DONE;
        Brelse(bp);
    }
    // �첽д
    else{
        // �������߳���ģ���첽д
        // threadֻ�ܵ�����ͨ�������ܵ����ຯ���������¶�����_Bawrite
        thread writeThread(_Bawrite, BufManager::GetInstance, bp);
        writeThread.detach();
    }
    return;
}


//�ӳ�д�������ǺŲ��ͷ�
void BufManager::Bdwrite(Buf* bp){
    /* ����B_DONE������������ʹ�øô��̿����� */
    bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);
    BufManager::Brelse(bp);
    return;
}


//�ͷŻ�����ƿ� buf��ָ�����������ɻ�����ж�βȥ
void BufManager::Brelse(Buf* bp){
    if (bp != nullptr && (bp->b_flags & Buf::B_BUSY)){
        // �����β
        Buf* freeP = &bFreeList;
        while (freeP->b_back != nullptr)
            freeP = freeP->b_back;
        freeP->b_back = bp;
        bp->b_forw = freeP;
        bp->b_back = nullptr;
        /*һ��Ҫ����ٸı�־*/
        bp->b_flags &= ~(Buf::B_BUSY | Buf::B_ASYNC);
    }
}


//��ջ���������
void BufManager::ClrBuf(Buf* bp){
    void* pvoid = bp->b_addr;

    /* ������������������ */
    memset((void*)pvoid, 0, BUFFER_SIZE);

    return;
}


//�� dev ָ���豸�������ӳ�д�Ļ���ȫ�����������
void BufManager::Bflush(){
    Buf* freeP = bFreeList.b_back;
    while (freeP != nullptr){
        if (freeP->b_flags & Buf::B_DELWRI){
            this->Bwrite(freeP);
        }
        freeP = freeP->b_back;
    }
    return;
}
