#pragma once
#include "User.h"
#include "Buf.h"
#include "BufferManager.h"
#include "FileSystem.h"
#include "InodeTable.h"
#include "FileManager.h"
#include "OpenFileTable.h"
#include "DiskManager.h"
#include <vector>
#include <string>
using namespace std;


extern bool DEBUG;

//E0147 声明与 "User globalUser" (已声明 所在行数 : 16，所属文件 : "D:\C++专用\OS\SecondaryFS\SecondaryFS\Common.h") 不兼容
//属于是A.h包含了B.h，B.h又包含了A.h
//额，是因为你的User.h忘记加#pragma once了
extern User globalUser;
extern DiskManager globalDiskManager;
extern BufferManager globalBufferManager;
extern OpenFileTable globalOpenFileTable;
extern SuperBlock globalSuperBlock;
extern FileSystem globalFileSystem;
extern InodeTable globalINodeTable;
extern FileManager globalFileManager;


// 处理输入的命令行
vector<string> getCmd(string str);

// 进行对应的运算
void exeCmd(vector<string> cmdList);

//展示用法帮助
void Usage();
