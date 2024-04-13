#pragma once
#include "SuperBlock.h"

class SuperBlock {
public:
	int s_isize; /* 外存 Inode 区占用的盘块数 */
	int s_fsize; /* 盘块总数 */
	int s_nfree; /* 直接管理的空闲盘块数量 */
	int s_free[100]; /* 直接管理的空闲盘块索引表 */
	int s_ninode; /* 直接管理的空闲外存 Inode 数量 */
	int s_inode[100]; /* 直接管理的空闲外存 Inode 索引表 */
	int s_flock; /* 封锁空闲盘块索引表标志 */
	int s_ilock; /* 封锁空闲 Inode 表标志 */
	int s_fmod; /* 内存中 super block 副本被修改标志，意味着需要更新外存对
   应的 Super Block */
	int s_ronly; /* 本文件系统只能读出 */
	int s_time; /* 最近一次更新时间 */
	int padding[47]; /* 填充使 SuperBlock 块大小等于 1024 字节，占据 2 个扇区 */

public:
	SuperBlock(){};
	~SuperBlock(){};
};
