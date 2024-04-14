#pragma once
#include "User.h"
#include "FileSystem.h"
#include "FileManager.h"
#include "BufManager.h"

class Shell{
public:
    enum DebugMode{
        OFF,
        ON
    };

protected:
    User* m_user;
    FileManager* m_fileManager;
    FileSystem* m_fileSystem;
    BufManager* m_bufferManager;
    char buffer[1000];

    void Interface();

public:
    Shell();
    ~Shell();

    void Start(int mode = DebugMode::OFF);

    void Usage();
};