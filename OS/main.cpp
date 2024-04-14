#include "BufManager.h"
#include "User.h"
#include "FileSystem.h"
#include "Utility.h"
#include "Shell.h"
#include <cstring>
#include <fstream>

#define RUN_TEST_CASE false
#define TEST_BUFFER 0
#define TEST_FLUSH 0
#define FORMAT_DISK 0
#define TEST_CREATE 1
#define TEST_WRITE 1
#define TEST_READ 0
#define TEST_CRW 0 // ��д�ۺϲ���



int main(){
    BufManager& bufManager = *(BufManager::GetInstance());
    User& user = *User::GetInstance();
    FileSystem& fileSystem = *(FileSystem::GetInstance());
    FileManager& fileManager = *(FileManager::GetInstance());
    fileManager.Initialize();
    Inode* pRootInode = FORMAT_DISK ? NULL : fileManager.rootDirInode = g_InodeTable.IGet(FileSystem::ROOTINO);
    if (!FORMAT_DISK)  fileSystem.LoadSuperBlock();/*����loadһ�£���Ȼ����������*/
    User& us = *User::GetInstance();
    us.u_cdir = pRootInode;
    us.u_pdir = pRootInode;
    ConstStringCopy("/", us.u_curdir);

#if RUN_TEST_CASE
    if (TEST_BUFFER)
    {
        bufferManager.showFreeList();
        printf("=====test async begin======\n");
        Buf* b1 = bufferManager.GetBlk(0, 1);
        strcpy((char*)b1->b_addr, "abcdefg");
        b1->b_flags |= Buf::BufFlag::B_ASYNC;
        printf("Async write (0,1).\n");
        bufferManager.Bwrite(b1); /*�첽дһ�����棬��done�꣬�����ͷ�*/
        printf("Ask to read (0,1) again.\n");
        Buf* b2 = bufferManager.Bread(0, 1); /*��һ���飬��������顢ͬ��������done�꣬�������ͷ�*/
        bufferManager.Brelse(b2);
        bufferManager.showFreeList();
        printf("=====test async end======\n");
    }
    if (TEST_FLUSH)
    {
        bufferManager.showFreeList();
        Buf* b0 = bufferManager.GetBlk(0, 202);
        Buf* b1 = bufferManager.GetBlk(0, 203);
        Buf* b2 = bufferManager.GetBlk(0, 204);

        strcpy((char*)b0->b_addr, "abcdefg");
        strcpy((char*)b1->b_addr, "hijklmn");
        strcpy((char*)b2->b_addr, "hello");
        bufferManager.Bdwrite(b0);
        bufferManager.Bdwrite(b1);
        bufferManager.Bdwrite(b2);
        bufferManager.showFreeList();
        bufferManager.Bflush(0);
        bufferManager.showFreeList();
    }
    if (FORMAT_DISK)
    {

        fileSystem.formatDisk(DEFALT_DEV);

        /*��ȡ���豸��sb*/
        // fileSystem.LoadSuperBlock();
        // Buf *b0 = fileSystem.Alloc(DEFALT_DEV);
        // Inode *pI = fileSystem.IAlloc(DEFALT_DEV);

        SuperBlock* sb1 = fileSystem.m_Mount[0].m_spb; // �ϵ���Բ鿴����
        // fileSystem.Update();
        bufferManager.Bflush(DEFALT_DEV);

        return 0;
    }
    if (TEST_CREATE)
    {
        // fileSystem.LoadSuperBlock();

        // debug:֮ǰ������Ŀ¼�������Inode�е�ģʽû��д�ԣ����¶�ȡĿ¼�ļ�ʧ�ܡ�������Ȼ�޸���ģʽ������ΪĿ¼���Ѿ����ڣ�����������Inodeģʽ����.ֻ�ܸ�ʽ��������
        // ��ʱ�����ֺ����ص�����
        // debug��ʹ��create���������ļ����������޸��ڴ�Inode������Inode��û��д��.

        int inode_u = fileManager.MkNod("/user", Inode::IFDIR | Inode::IRWXU);
        int inode_h = fileManager.MkNod("/home", Inode::IFDIR | Inode::IRWXU);
        Inode* pI = fileManager.m_InodeTable->IGet(0, inode_h);
        int inode_d = fileManager.MkNod("/dev", Inode::IFDIR | Inode::IRWXU);
        int inode_t = fileManager.MkNod("/home/texts", Inode::IFDIR | Inode::IRWXU);
        fileManager.MkNod("/home/reports", Inode::IFDIR | Inode::IRWXU);
        fileManager.MkNod("/home/photos", Inode::IFDIR | Inode::IRWXU);
        fileManager.ChDir("/home/texts");
        int fd1 = fileManager.Creat("Jerry.txt", Inode::IRWXU);
        fileManager.Close(fd1);
        bufferManager.Bflush(DEFALT_DEV);

        // ��ȡ��Ŀ¼����
        unsigned char buffer[201] = { 0 };

        int fd2 = fileManager.Open("/", File::FREAD); /*����fdд��u.u_ar0 Ƕ�ײ���̫�಻��ֱ�ӷ���*/
        DirectoryEntry etr[3];
        fileManager.Read(fd2, buffer, 32 * 3);
        Utility::DWordCopy((int*)buffer, (int*)etr, 32 * 3);
        printf("entry0 ino=%d,name=%s\n", etr[0].m_ino, etr[0].m_name);
        fileSystem.Update(); // debug:TEST_CREAT��TEST_WRITE����������������Ϊû��ͬ�������̾�����loadSuperBlock��
        bufferManager.Bflush(DEFALT_DEV);
    }

    if (TEST_WRITE)
    {
        static char str[17 * 1024] = { 0 };
        // fileSystem.LoadSuperBlock();
        int fd = fileManager.Open("/home/texts/Jerry.txt", File::FWRITE | File::FREAD);
        if (fd >= 0)
        {
            char str[200] = "0123456789012345678901234567890123456789";
            int count = fileManager.Write(fd, (unsigned char*)str, 30);
            File* pFile = user.u_ofiles.GetF(fd);
            Inode* pInode = pFile->f_inode;
            printf("write:count=%d,inode.number=%d,isize=%d,tellp=%d\n", count, pInode->i_number, pInode->i_size, fileManager.Tellp(fd));
            fileManager.Close(fd);
        }
        printf("��һ��ͼƬ����home/photos�ļ�����\n");
        fd = fileManager.Creat("/home/photos/testimage.jpg", Inode::IRWXU);
        Inode* pInode = user.u_ofiles.GetF(fd)->f_inode;

        if (fd >= 0)
        {


            std::ifstream ufile("img/testimage.jpg", std::ios::in | std::ios::binary);
            ufile.read(str, 17 * 1024);
            int imgsize = ufile.gcount();
            int count = fileManager.Write(fd, (unsigned char*)str, imgsize);
            File* pFile = user.u_ofiles.GetF(fd);
            Inode* pInode = pFile->f_inode;
            printf("read:count=%d,inode.number=%d,isize=%d,tellp=%d\n", count, pInode->i_number, pInode->i_size, fileManager.Tellp(fd));
            fileManager.Close(fd);
            ufile.close();
        }
        printf("���������home/reports�ļ�����\n");

        fd = fileManager.Creat("/home/reports/reports.pdf", Inode::IRWXU);
        if (fd >= 0)
        {

            memset(str, 0, 17 * 1024);
            std::ifstream ufile("report.pdf", std::ios::in | std::ios::binary);
            File* pFile = user.u_ofiles.GetF(fd);
            Inode* pInode = pFile->f_inode;

            while (!ufile.eof() && ufile.is_open())
            {
                ufile.read(str, 16 * 1024);
                int imgsize = ufile.gcount();
                int count = fileManager.Write(fd, (unsigned char*)str, imgsize);
                printf("write:count=%d,inode.number=%d,isize=%d,tellp=%d\n", count, pInode->i_number, pInode->i_size, fileManager.Tellp(fd));
            }
            ufile.close();
            fileManager.Close(fd);
        }
    }
    if (TEST_READ)
    {
        // fileSystem.LoadSuperBlock();
        int fd = fileManager.Open("/home/texts/LL.txt", File::FWRITE | File::FREAD);
        if (fd >= 0)
        {
            char str[200] = { 0 };
            int count = fileManager.Read(fd, (unsigned char*)str, 50);
            File* pFile = user.u_ofiles.GetF(fd);
            Inode* pInode = pFile->f_inode;
            printf("read:count=%d,inode.number=%d,isize=%d,tellp=%d\n", count, pInode->i_number, pInode->i_size, fileManager.Tellp(fd));
            fileManager.Close(fd);
        }
    }
    if (TEST_CRW)
    {
        // fileSystem.LoadSuperBlock();

        printf("�½��ļ�/test/Jerry���򿪸��ļ�������д��800���ֽ�\n");
        int fd = fileManager.Creat("/home/texts/Jerry.txt", Inode::IRWXU);
        if (fd >= 0)
        {
            char str[801] = { 0 };
            std::ifstream ufile("img/file_with_800B.txt", std::ios::in | std::ios::binary);
            ufile.read(str, 800);
            int fsize = ufile.gcount();
            int count = fileManager.Write(fd, (unsigned char*)str, 800);
            File* pFile = user.u_ofiles.GetF(fd);
            Inode* pInode = pFile->f_inode;
            printf("read:count=%d,inode.number=%d,isize=%d,tellp=%d\n", count, pInode->i_number, pInode->i_size, fileManager.Tellp(fd));
            printf("���ļ���дָ�붨λ����500�ֽڣ�����500���ֽڵ��ַ���abc��\n");
            char abc[501] = { 0 };
            fileManager.Seek(fd, 500, 0);
            count = fileManager.Read(fd, (unsigned char*)abc, 500);
            printf("read:count=%d,inode.number=%d,isize=%d,tellp=%d\n", count, pInode->i_number, pInode->i_size, fileManager.Tellp(fd));
            printf("��abcд���ļ���\n");
            count = fileManager.Write(fd, (unsigned char*)abc, 500);
            printf("read:count=%d,inode.number=%d,isize=%d,tellp=%d\n", count, pInode->i_number, pInode->i_size, fileManager.Tellp(fd));
            fileManager.Close(fd);
        }
    }
#else
    // fileSystem.LoadSuperBlock();
    Shell shell;
    shell.Start(Shell::DebugMode::OFF);

#endif

    return 0;
}