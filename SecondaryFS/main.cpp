#include "common.h"
#include <iostream>

bool DEBUG;


DiskManager globalDiskManager;
BufferManager globalBufferManager;
OpenFileTable globalOpenFileTable;
SuperBlock globalSuperBlock;
FileSystem globalFileSystem;
InodeTable globalINodeTable;
FileManager globalFileManager;
User globalUser;

string line;

int main() {
	DEBUG = false;

	while (1) {
		cout << "[BlossomsGarden " + globalUser.u_curdir + "]$ ";
		getline(cin, line);

		vector<string> cmdList = getCmd(line); // 此时第一项总为操作数

		exeCmd(cmdList);
	}

	return 0;
}