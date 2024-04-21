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
		cout << "[u2152189 " + globalUser.u_curdir + "]$ ";
		getline(cin, line);

		vector<string> cmdList = getCmd(line); // 此时第一项总为操作数

		try {
			exeCmd(cmdList);
		}
		catch (...) {
			cout << "Something Wrong! Please Try Again or See Help." << endl;
		}
	}

	return 0;
}