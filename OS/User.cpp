#include "User.h"
using namespace std;


User* User::GetInstance() {
	return &User::instance;
}


const char* User::Pwd() {
	return u_curdir; 
};


void User::clear() {
	u_error = NOERROR; 
}