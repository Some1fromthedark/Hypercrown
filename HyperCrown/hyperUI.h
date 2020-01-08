#pragma once

#include<string>
#include<iostream>
#include<fstream>
#include<sstream>
#include<vector>
#include<Windows.h>
#include<io.h>
#include<fcntl.h>

#include"BaseForm.h"
#include"progDetails.h"
#include"mdls.h"

using namespace System;
using namespace System::Windows::Forms;

// Prototype structs
struct Arguments;

// Prototype classes
class CLI;
class GUI;
class UserInterface;

// Prototype functions
Arguments parseArgs(std::vector<std::string> argv);

struct Arguments
{
	bool normalize;
	bool flipUVCoords;
	bool combineMeshes;
	bool swapChirality;
	bool debug;
	unsigned int mode;
	unsigned int rootDepth;
	unsigned int sectionInd;
	std::string path;
	std::string inputFilename;
	std::string outputFilename;
	std::string animationsFilename;
	std::string vertexTemplateModelName;
	std::string jointTemplateModelName;
	std::string referenceFilename;
	std::vector<std::string> textureNames;
	std::vector<unsigned int> referenceInds;
	std::vector<unsigned int> groupInds;
};

class UserInterface
{
private:

public:
	UserInterface();
	virtual void setup();
	virtual int run(Arguments args);
	virtual void displayString(std::string message);
	~UserInterface();
};

class CLI : public UserInterface
{
private:

public:
	CLI();
	void setup();
	int run(Arguments args);
	void displayString(std::string message);	
	~CLI();
};

class GUI : public UserInterface
{
private:
	
public:
	GUI();
	void setup();
	int run(Arguments args);
	void displayString(std::string message);
	~GUI();
};
