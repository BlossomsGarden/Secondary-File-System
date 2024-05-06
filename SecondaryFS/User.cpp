#include "common.h"
#include "User.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <sstream>
using namespace std;


User::User() {
    u_error = myNOERROR;
    u_FileManager = &globalFileManager;
    u_dirp = "/";
    u_curdir = "/";
    u_cdir = u_FileManager->rootDirInode;
    u_pdir = NULL;
    memset(u_arg, 0, sizeof(u_arg));
}

User::~User() {}


void User::Ls() {
    ls.clear();
    u_FileManager->Ls();
    if (IsError()) {
        return;
    }
    cout << ls << endl;
}

void User::Mkdir(string dirName) {
    if (!checkPathName(dirName)) {
        return;
    }
    u_arg[1] = Inode::IFDIR;
    u_FileManager->Creat();
    IsError();
}

void User::Cd(string dirName) {
    if (!checkPathName(dirName)) {
        return;
    }
    u_FileManager->ChDir();
    IsError();
}

void User::Update() {
    u_FileManager->m_FileSystem->Update();
    IsError();
}

void User::Create(string fileName, string mode) {
    if (!checkPathName(fileName)) {
        return;
    }
    int md = INodeMode(mode);
    if (md == 0) {
        cout << "this mode is undefined !  \n";
        return;
    }

    u_arg[1] = md;
    u_FileManager->Creat();
    IsError();
}

void User::Delete(string fileName) {
    if (!checkPathName(fileName)) {
        return;
    }
    u_FileManager->UnLink();
    IsError();
}

void User::Open(string fileName, string mode) {
    if (!checkPathName(fileName)) {
        return;
    }
    int md = FileMode(mode);
    if (md == 0) {
        cout << "Open参数错误！\n";
        return;
    }

    u_arg[1] = md;
    u_FileManager->Open();
    if (IsError()) {
        cout << "打开文件" << fileName << "失败" << endl;
    }
    else {
        cout << "打开文件" << fileName << "成功！句柄fd=" << u_ar0[EAX] << endl;
    }
}

void User::Close(string sfd) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        cout << "Close参数错误！\n";
        return;
    }
    u_arg[0] = stoi(sfd);;
    u_FileManager->Close();
    if (IsError()) {
        cout << "关闭文件fd= " << sfd << "失败" << endl;
    }
    else {
        cout << "成功关闭文件fd=" << sfd << endl;
    }
}

void User::Seek(string sfd, string offset, string origin) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        cout << "Seek参数错误！\n";
        return;
    }
    if (offset.empty()) {
        cout << "Seek参数错误！\n";
        return;
    }
    if (origin.empty() || !isdigit(origin.front())) {
        cout << "Seek参数错误！\n";
        return;
    }
    u_arg[0] = stoi(sfd);
    u_arg[1] = stoi(offset);
    u_arg[2] = stoi(origin);
    u_FileManager->Seek();
    IsError();
}



/*
 * exName：要导入的外部文件
 * inName：写入FS.img的文件名
 * size：  写入字节数
 */
void User::FileIn(string exName, string inName) {
    if (exName.empty() || inName.empty()) {
        cout << "write参数错误！\n";
        return;
    }
    //从exName文件中读入文件信息
    fstream fin(exName, ios::in | ios::binary);
    std::ostringstream  tmp;
    tmp << fin.rdbuf();
    std::string  content = tmp.str();
    fin.close();

    int usize = content.length();
    //创建inName文件
    const string mode = "-rw";
    this->Create(inName, mode);
    //打开inName文件
    this->Open(inName, mode);
    //这里要记录u_ar0参数，因为下面Write会将u_ar0[0]改为写入字节数
    int fd = this->u_ar0[EAX];
    
    //写入文件内容
    this->Write(to_string(fd), content, to_string(usize));
    //关闭inName文件
    this->Close(to_string(fd));
}

/*
 * inName：要从FS.img中导出的文件名
 * exName：要导出到外部系统的路径
 * size：  读出字节数
 */
void User::FileOut(string inName, string exName) {
    if (inName.empty() || exName.empty()) {
        cout << "write参数错误！\n";
        return;
    }

    const string mode = "-rw";
    //打开inName文件
    this->Open(inName, mode);
    //fd->OpenFileTable->f_inode->i_size获取文件长度
    int fd = this->u_ar0[EAX];
    int usize = this->u_ofiles.GetF(fd)->f_inode->i_size;

    //读取inName文件
    char* buffer = new char[usize];
    u_arg[0] = fd;
    //u_arg[1] = (long)buffer;
    u_arg[2] = usize;
    u_FileManager->Read((unsigned char*)buffer);
    if (IsError()) {
        cout << "读取" << usize << "字节数据失败！" << endl;
    }

    //写入exName文件
    fstream fout(exName, ios::out | ios::binary);
    if (!fout) {
        cout << "打开输出文件 " << exName << " 失败！ \n";
        return;
    }
    fout.write(buffer, u_ar0[User::EAX]);
    fout.close();
    cout << "输出到文件 " << exName << " 成功！ \n";

    //关闭inName文件
    this->Close(to_string(fd));

    delete[] buffer;
}

/*
 * sfd：FS.img打开的文件的fd
 * content：从文件读出的内容
 * size：要读取的字节数
 */
void User::Read(string sfd, string size) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        cout << "read参数错误！ \n";
        return;
    }
    int fd = stoi(sfd);

    int usize = -1;
    if (size.empty() || !isdigit(size.front()) || (usize = stoi(size)) < 0) {
        cout << "read参数错误！ \n";
        return;
    }

    char* buffer = new char[usize];
    u_arg[0] = fd;
    //u_arg[1] = (long)buffer;
    u_arg[2] = usize;
    u_FileManager->Read((unsigned char*)buffer);
    if (IsError()) {
        cout << "读取" << usize << "字节数据失败！" << endl;
    }
    else {
        cout << "从文件fd= " << fd << "成功读取" << usize << "字节：" << endl;
        for (int i = 0; i < usize; i++) {
            cout << buffer[i];
        }
        cout << endl;
    }

    delete[] buffer;
}

/*
 * sfd：FS.img打开的文件的fd
 * content：要写入的文件流
 * size：要读取的字节数
 */
void User::Write(string sfd, string content, string size) {
    if (sfd.empty() || !isdigit(sfd.front())) {
        cout << "write参数错误！\n";
        return;
    }
    int fd = stoi(sfd);

    int usize;
    if (size.empty() || (usize = stoi(size)) < 0) {
        cout << "write参数错误！\n";
        return;
    }

    char* buffer = new char[usize];
    memcpy(buffer, content.c_str(), usize);

    u_arg[0] = fd;
    //u_arg[1] = (long)buffer;
    u_arg[2] = usize;
    u_FileManager->Write((unsigned char*)buffer);

    if (IsError())
        return;
    cout << "向文件fd= " << fd << "写入" << usize << "字节成功！" << endl;

    delete[] buffer;
}

int User::INodeMode(string mode) {
    int md = 0;
    if (mode.find("-r") != string::npos) {
        md |= Inode::IREAD;
    }
    if (mode.find("-w") != string::npos) {
        md |= Inode::IWRITE;
    }
    if (mode.find("-rw") != string::npos) {
        md |= (Inode::IREAD | Inode::IWRITE);
    }
    return md;
}

int User::FileMode(string mode) {
    int md = 0;
    if (mode.find("-r") != string::npos) {
        md |= File::FREAD;
    }
    if (mode.find("-w") != string::npos) {
        md |= File::FWRITE;
    }
    if (mode.find("-rw") != string::npos) {
        md |= (File::FREAD | File::FWRITE);
    }
    return md;
}

bool User::checkPathName(string path) {
    // FileManager 中函数不判断参数的合法性，最好在User中过滤，
    // 暂不考虑过多的参数不合法情况
    if (path.empty()) {
        cout << "parameter path can't be empty ! \n";
        return false;
    }

    if (path.substr(0, 2) != "..") {
        u_dirp = path;
    }
    else {
        string pre = u_curdir;
        unsigned int p = 0;
        //可以多重相对路径 .. ../ ../.. ../../
        for (; pre.length() > 3 && p < path.length() && path.substr(p, 2) == ".."; ) {
            pre.pop_back();
            pre.erase(pre.find_last_of('/') + 1);
            p += 2;
            p += p < path.length() && path[p] == '/';
        }
        u_dirp = pre + path.substr(p);
    }

    if (u_dirp.length() > 1 && u_dirp.back() == '/') {
        u_dirp.pop_back();
    }

    for (unsigned int p = 0, q = 0; p < u_dirp.length(); p = q + 1) {
        q = u_dirp.find('/', p);
        q = min(q, (unsigned int)u_dirp.length());
        if (q - p > DirectoryEntry::DIRSIZ) {
            cout << "文件路径长度过大！ \n";
            return false;
        }
    }
    return true;
}

bool User::IsError() {
    if (u_error != myNOERROR) {
        //cout << "errno = " << u_error;
        EchoError(u_error);
        u_error = myNOERROR;
        return true;
    }
    return false;
}

void User::EchoError(enum ErrorCode err) {
    string estr;
    switch (err) {
    case User::myNOERROR:
        estr = " No u_error ";
        break;
    case User::myENOENT:
        estr = " No such file or directory ";
        break;
    case User::myEBADF:
        estr = " Bad file number ";
        break;
    case User::myEACCES:
        estr = " Permission denied ";
        break;
    case User::myENOTDIR:
        estr = " Not a directory ";
        break;
    case User::myENFILE:
        estr = " File table overflow ";
        break;
    case User::myEMFILE:
        estr = " Too many open files ";
        break;
    case User::myEFBIG:
        estr = " File too large ";
        break;
    case User::myENOSPC:
        estr = " No space left on device ";
        break;
    default:
        break;
    }
    cout << estr << endl;
}
