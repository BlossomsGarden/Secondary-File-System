## Welcome！ 👋

西兰花 2024 年操作系统课程设计的代码仓库！


## 💿 内容介绍 

本项目的设计目标是基于宿主机的操作系统设计“二级文件系统”，该二级系统的磁盘是宿主机上的一个大小 32MB、后缀名 .img 的文件。

本次课设只要求实现单用户，因此删除了Kernel类、进程管理、用户组权限管理等模块，将请求系统调用的钩子函数统一写在User类中，代码源文件均在 /SecondaryFS 文件夹下，同时提供课程设计要求以及课程设计报告供参考。
为保证 User、DiskManager 等类的全局唯一性，本项目 extern 声明全局变量并在 main.cpp 中统一实例化。
项目中有 format、shutdown 写回等“生命周期”命令，直接写在了对应类的构造和析构函数中


本项目根据 [UNIX V6++](https://git.tongji.edu.cn/vesper-system/unix-v6pp-tongji) 裁剪改编而来。


## 🔧 运行说明

本项目在 Windows + Visual Studio 2022 集成环境中开发，使用 x64 Debug 编译模式，所有文件编码统一为 UTF-16 LE。

在适应的环境下将仓库克隆至本地后双击 .sln 文件即可直接编译运行，系统默认在 /SecondaryFS 路径下创建大小为 32MB 的磁盘映像文件 FS.img。
/SecondaryFS 文件夹下有测试文件 1.jpg 1.pdf 1.txt 供导入导出测试，如连续输入命令：

fin 1.pdf pdf

fout pdf out.pdf

会发现 /SecondaryFS 路径下多了 out.pdf，双击可以像 1.pdf 一样打开，查看属性看到二者字节数相同。



## 👨‍💻 编码问题

如前所述，本项目从Linux版本源码移植过来时出现了中文乱码，于是手动修改编码格式为GB2312。但 Github 默认编码格式为UTF-8，手动使用记事本修改为UTF-8后Visual Studio报大量 C4819 Warning 与头文件读入失败的错误。
最后使用VS的“高级文件保存选项”一一修改解决，行尾选用 CRLF，彻底更改为一个 Windows 系统项目，如下图。（因此也推荐 Win 用户而不是 Mac 用户使用该仓库）

![image](https://github.com/BlossomsGarden/Secondary-File-System/assets/110208412/b16643e2-9f5a-49c0-b645-019743b35c3b)


关于 Visual Studio 的编码设置问题参考博客：[CSDN - VS2022 设置编码方式为 UTF-8 的三种方式](https://blog.csdn.net/hfy1237/article/details/129858976)



[注] 当前版本连续导入多次 pdf 文件之后会卡死退出，暂时不知道什么原因，短期内决定课设答辩时避开这个坑展示

