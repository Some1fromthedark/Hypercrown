#include"string_utils.h"

using namespace System;
using namespace std;

// Read the contents of a binary file and store it as a string
bool binaryFileRead(string filename, string &dat)
{
	bool valid = true;
	char *tmp;
	ifstream in;
	// Clear the output
	dat = "";
	// Open the input file
	in.open(filename, ifstream::binary);
	// Make sure it opened successfully
	if (in.is_open())
	{
		in.seekg(0, ifstream::end);
		unsigned int fileLen = (unsigned int)in.tellg();
		in.seekg(0, ifstream::beg);
		tmp = new char[fileLen];
		in.read(tmp, fileLen);
		// Close the input file
		in.close();
		dat = string(tmp, fileLen);
		delete[]tmp;
	}
	else
	{
		valid = false;
	}
	return valid;
}

bool binaryFileWrite(std::string filename, std::string dat)
{
	bool valid = true;
	ofstream out(filename, ofstream::binary);
	if (out.is_open())
	{
		out.write(dat.c_str(), dat.size());
		out.close();
	}
	else
	{
		valid = false;
	}
	return valid;
}

System::String ^std2system(std::string s)
{
	String ^o = gcnew System::String(s.c_str());
	return o;
}

std::string system2std(System::String ^s)
{
	using namespace Runtime::InteropServices;
	const char *chars = (const char *)Marshal::StringToHGlobalAnsi(s).ToPointer();
	std::string o = chars;
	Marshal::FreeHGlobal(IntPtr((void *)chars));
	return o;
}

std::string formatInputStr(std::string s)
{
	std::string o = s;
	unsigned int extInd = o.find_last_of('.');
	for (unsigned int i = 0; i < o.size(); i++)
	{
		// Replace all backslashes with forward slashes
		if (o[i] == '\\')
		{
			o[i] = '/';
		}
		// Replace all uppercase letters in the extension with lowercase
		else if (i > extInd && o[i] >= 'A' && o[i] <= 'Z')
		{
			o[i] += 32;
		}
	}
	return o;
}
