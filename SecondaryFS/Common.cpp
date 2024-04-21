#include "Common.h"
#include <iostream>
#include <sstream>

vector<string> getCmd(string str){
	vector<string> result;
	result.clear();

	stringstream cmd(str); // 把str作为流输入
	string tempResult;
	while (cmd >> tempResult) {
		result.push_back(tempResult);
	}

	if (DEBUG) {
		cout << "读到的命令行内容为" << endl;
		for (int cnt = 0; cnt < result.size(); cnt++) {
			cout << result[cnt] << ' ';
		}
		cout << endl;
	}

	return result;
}

void Usage() {
	cout << endl;
	cout << endl;
	cout << "ls                                 list directorys and files in current path\n";
	cout << "mkdir <path>                       create a directory\n";
	cout << "cd <path>                          change current path\n";
	cout << "fcreat <path>                      create a file or directory\n";
	cout << "                                   mode: -rw     read and write\n";
	cout << "                                         -r      only read\n";
	cout << "                                         -w      only write\n";
	cout << "rm <path>                          delete a file or directory\n";
	cout << "fopen <path> [mode]                open a file\n";
	cout << "                                   mode: -rw     read and write\n";
	cout << "                                         -r      only read\n";
	cout << "                                         -w      only write\n";
	cout << "fclose <fd>                        close a file\n";
	cout << "flseek <fd> <pos> [mode]           set the read/write pointer of file(fd={fd}) at {pos}\n";
	cout << "                                   mode:  0      [defalt] pos is the offset from beginning\n";
	cout << "                                          1      pos is the offset from current position\n";
	cout << "                                          2      pos is the offset from end\n";
	cout << "fread <fd> <count>                 read file(fd={fd}) for {count} byte\n";
	cout << "fwrite <fd> <content> <count>      write file(fd={fd}) for {count} byte from {content}\n";
	cout << "fin <ex_path> <in_path>			import file from {ex_path} for {count} byte to {in_path}\n";
	cout << "fout <in_path> <ex_path>			export file(fd={fd}) for {count} byte as {path}\n";
	cout << "help                               more detailed command info\n";
	cout << "shutdown                           save your changes and shutdown the shell\n";
	cout << "exit                               format the disk and exit the system\n";
	cout << endl;
	cout << endl;
}

void exeCmd(vector<string> cmdList){
	if (cmdList.size() == 0) {
		return;
	}

	string cmd = cmdList[0];

	if (cmd == "fcreat") {
		// globalUser.Mkdir(cmdList[1]);
		// cout << cmdList[1] << " " << cmdList[2] << endl;
		globalUser.Create(cmdList[1], cmdList[2]);
	}
	else if (cmd == "fopen") {
		globalUser.Open(cmdList[1], cmdList[2]);
	}
	else if (cmd == "fclose") {
		globalUser.Close(cmdList[1]);
	}
	else if (cmd == "flseek") {
		globalUser.Seek(cmdList[1], cmdList[2], cmdList[3]);
	}

	else if (cmd == "fread") {
		globalUser.Read(cmdList[1], cmdList[2]);
	}
	else if (cmd == "fwrite") {
		globalUser.Write(cmdList[1], cmdList[2], cmdList[3]);
	}

	else if (cmd == "fin") {
		globalUser.FileIn(cmdList[1], cmdList[2]);
	}
	else if (cmd == "fout") {
		globalUser.FileOut(cmdList[1], cmdList[2]);
	}
	else if (cmd == "mkdir") {
		globalUser.Mkdir(cmdList[1]);
	}
	else if (cmd == "rm") {
		globalUser.Delete(cmdList[1]);
	}
	else if (cmd == "ls") {
		globalUser.Ls();
	}
	else if (cmd == "cd") {
		globalUser.Cd(cmdList[1]);
	}
	else if (cmd == "shutdown") {
		//内存中所有有改变的Inode和SuperBlock的改变全部写回磁盘
		//globalUser.Update();
		cout << "All Changes Saved, Exiting" << endl;
		exit(0);
	}
	else if (cmd == "exit") {
		globalOpenFileTable.Format();
		globalINodeTable.Format();
		globalBufferManager.Bformat();
		globalFileSystem.FormatFileSystem();

		cout << "Exiting... This will Lose All Your Data." << endl;
		exit(0);
	}
	else if (cmd == "help") {
		Usage();
	}
	else if (cmd == "") {
		return;
	}
	else {
		cout << "未识别到的符号: " << cmd << endl;
	}
}