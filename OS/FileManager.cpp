#include "FileManager.h"
using namespace std;

FileManager& FileManager::GetInstance() {
	return FileManager::instance;
}