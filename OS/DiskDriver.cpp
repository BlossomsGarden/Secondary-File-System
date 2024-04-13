#include "DiskDriver.h"
#include "Kernel.h"
#include <fcntl.h>
using namespace std;


DiskDriver::DiskDriver(){}

DiskDriver::~DiskDriver(){}


void DiskDriver::initialize() {
    //this->m_BufManager = &Kernel::Instance().GetBufManager();

    //��д��ʽ�򿪴���
    int fd = open(this->DISK_FILE_NAME, O_RDWR);
    if (fd == -1) {
        fd = open(devpath, O_RDWR | O_CREAT, 0666);
        if (fd == -1) {
            perror("Error opening file");
            exit(-1);
        }

        //��������ʱ����Ҫ��ʼ��
        this->ini(fd);
    }
    this->init_mmap(fd);
    this->img_fd = fd;
}