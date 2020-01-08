#pragma once

#include<fstream>
#include<string>

bool binaryFileRead(std::string filename, std::string &dat);
bool binaryFileWrite(std::string filename, std::string dat);
System::String ^std2system(std::string s);
std::string system2std(System::String ^s);
std::string formatInputStr(std::string s);
