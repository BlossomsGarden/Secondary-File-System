#pragma once
#include <string>
#include <iostream>


void IOMove(unsigned char* from, unsigned char* to, int count);

void DWordCopy(int* src, int* dst, int count);

bool IsNumberString(std::string str);

void ConstStringCopy(char const* src, char* dst);