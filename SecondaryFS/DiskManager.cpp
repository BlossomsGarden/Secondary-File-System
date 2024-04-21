#define _CRT_SECURE_NO_WARNINGS
#include "Buf.h"
#include "BufferManager.h"
#include "DiskManager.h"
#include <iostream>
#include "File.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "INode.h"
#include "OpenFileTable.h"
#include "User.h"
using namespace std;


const char* DiskManager::DISK_FILE_NAME = "FS.img";

DiskManager::DiskManager() {
    fp = fopen(DISK_FILE_NAME, "rb+");
}

DiskManager::~DiskManager() {
    if (fp) {
        fclose(fp);
    }
}

/* 检查镜像文件是否存在 */
bool DiskManager::Exists() {
    return fp != NULL;
}

/* 打开镜像文件 */
void DiskManager::OpenDisk() {
    fp = fopen(DISK_FILE_NAME, "wb+");
    if (fp == NULL) {
        printf("打开或新建文件%s失败！", DISK_FILE_NAME);
        exit(-1);
    }
}

/* 实际写磁盘函数 */
void DiskManager::write(const void* buffer, unsigned int size, int offset, unsigned int origin) {
    if (offset >= 0) {
        fseek(fp, offset, origin);
    }
    fwrite(buffer, size, 1, fp);
}

/* 实际读磁盘函数 */
void DiskManager::read(void* buffer, unsigned int size, int offset, unsigned int origin) {
    if (offset >= 0) {
        fseek(fp, offset, origin);
    }
    fread(buffer, size, 1, fp);
}