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
	cout << "Usage:" << endl;
	cout << endl;
	cout << "ls                                 list directorys and files in current path\n";
	cout << "mkdir <path>                       create a directory\n";
	cout << "cd <path>                          change current path\n";
	cout << "fcreat <path> [mode]               create a file or directory\n";
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
	cout << "                                   mode:  0      pos is the offset from beginning\n";
	cout << "                                          1      pos is the offset from current position\n";
	cout << "                                          2      pos is the offset from end\n";
	cout << "fread <fd> <count>                 read file (fd={fd}) for {count} byte\n";
	cout << "fwrite <fd> <content> <count>      write file (fd={fd}) for {count} byte from {content}\n";
	cout << "fin <ex_path> <in_path>            import external file from {ex_path} into secondary filesystem {in_path}\n";
	cout << "fout <in_path> <ex_path>           export secondary filesystem file (fd={fd}) to external file {path}\n";
	cout << "help                               detailed command info\n";
	cout << "shutdown                           save your changes and shutdown the cmd shell\n";
	cout << "exit                               format the disk and exit\n";
	cout << endl;
	cout << endl;
}

void exeCmd(vector<string> cmdList){
	if (cmdList.size() == 0) {
		return;
	}
	string cmd = cmdList[0];

	if (cmd == "fcreat") {
		if (cmdList.size() < 3) {
			cout << endl << "Command - fcreat requires 2 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Create(cmdList[1], cmdList[2]);
	}
	else if (cmd == "fopen") {
		if (cmdList.size() < 3) {
			cout << endl << "Command - fopen requires 2 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Open(cmdList[1], cmdList[2]);
	}
	else if (cmd == "fclose") {
		if (cmdList.size() < 2) {
			cout << endl << "Command - fclose requires 1 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Close(cmdList[1]);
	}
	else if (cmd == "flseek") {
		if (cmdList.size() < 4) {
			cout << endl << "Command - flseek needs 3 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Seek(cmdList[1], cmdList[2], cmdList[3]);
	}

	else if (cmd == "fread") {
		if (cmdList.size() < 3) {
			cout << endl << "Command - fread needs 2 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Read(cmdList[1], cmdList[2]);
	}
	else if (cmd == "fwrite") {
		if (cmdList.size() < 4) {
			cout << endl << "Command - fwrite needs 3 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Write(cmdList[1], cmdList[2], cmdList[3]);
	}
	else if (cmd == "fin") {
		if (cmdList.size() < 3) {
			cout << endl << "Command - fin needs 2 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.FileIn(cmdList[1], cmdList[2]);
	}
	else if (cmd == "fout") {
		if (cmdList.size() < 3) {
			cout << endl << "Command - fout needs 2 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.FileOut(cmdList[1], cmdList[2]);
	}
	else if (cmd == "mkdir") {
		if (cmdList.size() < 2) {
			cout << endl << "Command - mkdir needs 1 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Mkdir(cmdList[1]);
	}
	else if (cmd == "rm") {
		if (cmdList.size() < 2) {
			cout << endl << "Command - rm needs 1 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Delete(cmdList[1]);
	}
	else if (cmd == "ls") {
		globalUser.Ls();
	}
	else if (cmd == "cd") {
		if (cmdList.size() < 2) {
			cout << endl << "Command - cd needs 1 args!" << endl << endl;;
			Usage();
			return;
		}
		globalUser.Cd(cmdList[1]);
	}
	else if (cmd == "shutdown") {
		//内存中所有有改变的Inode和SuperBlock的改变全部写回磁盘
		//globalUser.Update();
		//直接写在析构里了
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
		cout << "未识别到的指令: " << cmd << endl;
	}
}
