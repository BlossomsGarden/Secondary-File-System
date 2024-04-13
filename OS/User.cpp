#include "User.h"
using namespace std;


User& User::GetInstance() {
	return User::instance;
}