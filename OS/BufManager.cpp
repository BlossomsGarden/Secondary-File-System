#include "BufManager.h"
#include "Windows.h"
#include <thread>
using namespace std;


BufManager::BufManager(){}
BufManager ::~BufManager(){}

void BufManager::Initialize() {
    memset((void*)Buffer, 0, NBUF * BUFFER_SIZE);
    // 初始化Buf实际地址
    for (int i = 0; i < this->NBUF; i++){
        m_Buf[i].b_addr = Buffer[i];
        m_Buf[i].b_back = ((i + 1) < NBUF ? (&m_Buf[i + 1]) : nullptr);
        m_Buf[i].b_forw = (i - 1) >= 0 ? &m_Buf[i - 1] : &bFreeList;
        m_Buf[i].b_no = i;
    }

    bFreeList.b_back = &m_Buf[0];
    bFreeList.b_forw = nullptr;
}


//模仿Kernel类获取缓存管理器的类实例
BufManager& BufManager::GetInstance() {
	return BufManager::instance;
}
Buf& BufManager::GetBFreeList(){
    return BufManager::bFreeList;
}


// 将缓存队列中的块取出，并设置BUSY标致
void BufManager::NotAvail(Buf* bp) {
    // 如果bp是队尾bp->b_back为nullptr
    if (bp->b_back) {
        bp->b_back->b_forw = bp->b_forw;
    }
    bp->b_forw->b_back = bp->b_back;
    bp->b_back = nullptr;
    bp->b_forw = nullptr;
    /* 设置B_BUSY标志 */
    bp->b_flags |= Buf::B_BUSY;
    return;
}


//将磁盘数据块<dev，blkno>同步读入缓存池，返回其首地址
//GetBlk锁住分配给它的缓存控制块，没有的话去申请一块
//Bread在GetBlk结束后解锁
Buf* BufManager::Bread(int blkno) {
    // 查设备表
    Buf* bp = GetBlk(dev, blkno);

    // 若已在缓存队列中，即B_DONE已设置，无需读磁盘了 */
    if (bp->b_flags & Buf::B_DONE) {
        return bp;
    }

    //todo:构造IO请求，读512个字节
    //DeviceManager::getInst()->GetBlockDevice(dev)->Read(bp);

    bp->b_flags |= Buf::BufFlag::B_DONE;
    return bp;
}

Buf* BufManager::GetBlk(int dev, int blkno) {
    Buf* buf_reuse = nullptr;
    // 搜索全部缓存，看有没有之前已经分配给这个块可以重用的
    for (int i = 0; i < NBUF; i++) {
        if (m_Buf[i].b_dev == dev && m_Buf[i].b_blkno == blkno) {
            buf_reuse = &m_Buf[i];
            break;
        }
    }
    // 找到了可重用的缓存
    if (buf_reuse != nullptr && (buf_reuse->b_flags & (Buf::BufFlag::B_BUSY))){
        // B_BUSY时Getblk？应该是块正在进行异步IO，等开锁
        buf_mutex[buf_reuse->b_no].lock();
        buf_mutex[buf_reuse->b_no].unlock();
        NotAvail(buf_reuse);
        return buf_reuse;
    }
    else if (buf_reuse != nullptr && (buf_reuse->b_flags & (Buf::BufFlag::B_DONE))) {
        //若是自由块，直接加B_BUSY标志，移出队列
        NotAvail(buf_reuse);
        return buf_reuse;
    }


    Buf* buf_first = bFreeList.b_back;
    buf_first = bFreeList.b_back;
    // 找不到可重用块，取出自由队列队首吧
    if (buf_reuse == nullptr && buf_first) {
        NotAvail(buf_first);
        //这个缓存块有延迟写标记
        if (buf_first->b_flags & Buf::BufFlag::B_DELWRI) {
            // 直接送Bwrite
            Bwrite(buf_first);
            return GetBlk(dev, blkno);
        }
        buf_first->b_blkno = blkno;
        buf_first->b_dev = dev;
        buf_first->b_flags = Buf::BufFlag::B_BUSY;
        return buf_first;
    }
    // 实在是找不到任何空闲块.....
    else if (buf_reuse == nullptr && !bFreeList.b_back) {
        // 睡会儿算了
        Sleep(200);
        return GetBlk(dev, blkno);
    }

    //其他情况也不知道还有什么情况
    return nullptr;
}


//异步写，完成后释放
//在Bwrite中注册了线程来调用。对缓存块加锁，注意加锁的位置尽量覆盖对缓存的更改
void _Bawrite(BufManager* bufmgr, Buf* bp) {
    bufmgr->Bawrite(bp);
}
void BufManager::Bawrite(Buf* bp)
{
    /* 标记为异步写 */
    buf_mutex[bp->b_no].lock();
    bp->b_flags |= (Buf::B_ASYNC | Buf::B_BUSY);

    //todo:构造IO请求，写入512个字节
    //DeviceManager::getInst()->GetBlockDevice(bp->b_dev)->Write(bp);

    bp->b_flags &= ~Buf::B_ASYNC;
    /*加锁的位置尽量覆盖对缓存的更改 所以先释放再解锁*/
    Brelse(bp);
    buf_mutex[bp->b_no].unlock();
    return;
}
//拿到一个块并已经对缓存完成处理后，完成同步或异步写，设置B_DONE标志并释放
void BufManager::Bwrite(Buf* bp){
    unsigned int flags;

    flags = bp->b_flags;
    bp->b_flags &= ~(Buf::B_READ | Buf::B_DONE | Buf::B_DELWRI);
    bp->b_wcount = BufManager::BUFFER_SIZE; /* 512字节 */

    // 同步写
    if ((flags & Buf::B_ASYNC) == 0){
        //todo:构造IO请求，写入512个字节
        //DeviceManager::getInst()->GetBlockDevice(bp->b_dev)->Write(bp);
        bp->b_flags |= Buf::BufFlag::B_DONE;
        Brelse(bp);
    }
    // 异步写
    else{
        // 创建新线程来模拟异步写
        // thread只能调用普通函数不能调用类函数，于是新定义了_Bawrite
        thread writeThread(_Bawrite, BufManager::GetInstance, bp);
        writeThread.detach();
    }
    return;
}


//延迟写，调整记号并释放
void BufManager::Bdwrite(Buf* bp){
    /* 置上B_DONE允许其它进程使用该磁盘块内容 */
    bp->b_flags |= (Buf::B_DELWRI | Buf::B_DONE);
    BufManager::Brelse(bp);
    return;
}


//释放缓存控制块 buf，指把它放在自由缓存队列队尾去
void BufManager::Brelse(Buf* bp){
    if (bp != nullptr && (bp->b_flags & Buf::B_BUSY)){
        // 放入队尾
        Buf* freeP = &bFreeList;
        while (freeP->b_back != nullptr)
            freeP = freeP->b_back;
        freeP->b_back = bp;
        bp->b_forw = freeP;
        bp->b_back = nullptr;
        /*一定要最后再改标志*/
        bp->b_flags &= ~(Buf::B_BUSY | Buf::B_ASYNC);
    }
}


//清空缓冲区内容
void BufManager::ClrBuf(Buf* bp){
    void* pvoid = bp->b_addr;

    /* 将缓冲区中数据清零 */
    memset((void*)pvoid, 0, BUFFER_SIZE);

    return;
}


//将 dev 指定设备队列中延迟写的缓存全部输出到磁盘
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
