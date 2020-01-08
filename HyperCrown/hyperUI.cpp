#include"hyperUI.h"

using namespace std;

// CLI function definitions

CLI::CLI() : UserInterface()
{
	return;
}

void CLI::setup()
{
	// Create Console for this application
	AllocConsole();
	HWND hConsole = GetConsoleWindow();
	SetConsoleTitle((prog + " v " + version).c_str());
	// Get stdin
	HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
	int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
	FILE *CInputHandle = _fdopen(SystemInput, "r");
	// Get stdout
	HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
	FILE *COutputHandle = _fdopen(SystemOutput, "w");
	// Get stderr
	HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
	int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
	FILE *CErrorHandle = _fdopen(SystemError, "w");
	// Sync with cout, etc
	ios::sync_with_stdio(true);
	freopen_s(&CInputHandle, "CONIN$", "r", stdin);
	freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
	freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);
	// Clear error states
	cout.clear();
	wcout.clear();
	cerr.clear();
	wcerr.clear();
	cin.clear();
	wcin.clear();
	return;
}

int CLI::run(Arguments args)
{
	// Declare variables
	unsigned int mode, rd, maxJoints = UINT_MAX, strInd;
	string pth, fn, on, afn, vtn, jtn, rfn, inPre, inExt, outPre, outExt, animPre, animExt, refPre, refExt, dat = "", mtlDat = "";
	vector<string> outputFilenames, outputData, textureNames, effects, hdFileNames, hdFileDats;
	vector<unsigned int> refInds, groupInds;
	vector<Texture> textures, effectTextures, texaTextures, hudTextures, skyboxTextures;
	//ifstream in;
	ofstream out;
	AddInfo additionalArgs;

	cout << prog << " v " << version << endl << "by " << tool_author << endl << endl;

	/*
	testPs2Textures();
	return 0;
	*/

	// Extract the mode from the parsed args
	mode = args.mode;

	// Make sure the arguments were valid
	if (mode != ERROR_MODE)
	{
		// Extract the remaining arguments
		pth = args.path;
		fn = args.inputFilename;
		on = args.outputFilename;
		rd = args.rootDepth;
		afn = args.animationsFilename;
		jtn = args.jointTemplateModelName;
		vtn = args.vertexTemplateModelName;
		rfn = args.referenceFilename;
		textureNames = args.textureNames;
		groupInds = args.groupInds;
		refInds = args.referenceInds;
		additionalArgs.additionalBools.push_back(args.normalize);
		additionalArgs.additionalBools.push_back(args.flipUVCoords);
		additionalArgs.additionalBools.push_back(args.combineMeshes);
		additionalArgs.additionalBools.push_back(args.swapChirality);

		// Process the path
		if (pth.size() > 0)
		{
			// Convert any forward slashes to backslashes
			for (unsigned int i = 0; i < pth.size(); i++)
			{
				if (pth[i] == '/')
				{
					pth[i] = '\\';
				}
			}
			// Make sure the path ends with a backslash
			if (pth[pth.size() - 1] != '\\')
			{
				pth.append(1, '\\');
			}
		}

		// Get the prefix and extension of the input filenames
		strInd = fn.find_last_of('.');
		if (strInd <= fn.size())
		{
			inPre = fn.substr(0, strInd);
			inExt = fn.substr(strInd, fn.size() - strInd);
			// Convert the extension to lowercase
			for (unsigned int i = 1; i < inExt.size(); i++)
			{
				if (inExt[i] >= 'A' && inExt[i] <= 'Z')
				{
					inExt[i] += 32;
				}
			}
		}
		else
		{
			inPre = "";
			inExt = "";
		}
		// Get the prefix and extension of the output filenames
		strInd = on.find_last_of('.');
		if (strInd <= on.size())
		{
			outPre = on.substr(0, strInd);
			outExt = on.substr(strInd, on.size() - strInd);
			// Convert the extension to lowercase
			for (unsigned int i = 0; i < outExt.size(); i++)
			{
				if (outExt[i] >= 'A' && outExt[i] <= 'Z')
				{
					outExt[i] += 32;
				}
			}
		}
		else
		{
			outPre = "";
			outExt = "";
		}

		if (afn != "")
		{
			strInd = afn.find_last_of('.');
			if (strInd <= afn.size())
			{
				animPre = afn.substr(0, strInd);
				animExt = afn.substr(strInd, afn.size() - strInd);
				// Convert the ext to lowercase
				for (unsigned int i = 0; i < animExt.size(); i++)
				{
					if (animExt[i] >= 'A' && animExt[i] <= 'Z')
					{
						animExt[i] += 32;
					}
				}
			}
			else
			{
				animPre = "";
				animExt = "";
			}
		}

		if (rfn != "")
		{
			strInd = rfn.find_last_of('.');
			if (strInd <= rfn.size())
			{
				refPre = rfn.substr(0, strInd);
				refExt = rfn.substr(strInd, rfn.size() - strInd);
				// Convert the ext to lowercase
				for (unsigned int i = 0; i < refExt.size(); i++)
				{
					if (refExt[i] >= 'A' && refExt[i] <= 'Z')
					{
						refExt[i] += 32;
					}
				}
			}
			else
			{
				animPre = "";
				animExt = "";
			}
		}

		if (on != "")
		{
			// Add the output filename to the list of output filenames
			outputFilenames.push_back(on);
			// Temporarily add an empty string to the list of output data to synchronize the list of names and list of data
			outputData.push_back("");
		}

		// Check if running in help mode
		if (mode != HELP_MODE)
		{
			// Make sure we got an input filename
			if (fn != "")
			{
				cout << "Reading from " << fn << endl;
				// Read from the input file
				if (binaryFileRead(pth + fn, dat))
				{
					cout << "Determining input file type" << endl;
					// Figure out what the input format is based on the extension
					if (inExt == ".dae")
					{
						// Add the root node depth as an additional argument
						additionalArgs.additionalInts.push_back(rd);
					}
					else if (inExt == ".wpn")
					{
						string referenceDat = "";
						if (rfn != "")
						{
							cout << "Reading from " << rfn << endl;
							if (!binaryFileRead(pth + rfn, referenceDat))
							{
								cout << "Error: Failed to read from " << rfn << endl;
							}
						}
						additionalArgs.additionalStrs.push_back(referenceDat);
					}
					// Check if a additional args are required by the output format
					if (on != "")
					{
						cout << "Determining output file type" << endl;
						if (outExt == ".moa")
						{
							// Check if any template files were provided
							if (jtn != "")
							{
								string jointTemplateDat;
								cout << "Reading from " << jtn << endl;
								if (binaryFileRead(pth + jtn, jointTemplateDat))
								{
									additionalArgs.additionalStrs.push_back(jointTemplateDat);
								}
								else
								{
									cout << "Error: Failed to open " << jtn << " for reading" << endl;
								}
							}
							if (vtn != "")
							{
								string vertexTemplateDat;
								if (jtn == "")
								{
									additionalArgs.additionalStrs.push_back("");
								}
								cout << "Reading from " << vtn << endl;
								if (binaryFileRead(pth + vtn, vertexTemplateDat))
								{
									additionalArgs.additionalStrs.push_back(vertexTemplateDat);
								}
								else
								{
									cout << "Error: Failed to open " << vtn << " for reading" << endl;
								}
							}
						}
						else if (outExt == ".obj")
						{
							// Generate the mtl filename (To-Do: Add an option to provide if you don't want to generate it?)
							string mtlFN = outPre + ".mtl";
							// Add the filename to the list of additional arguments for the conversion process
							additionalArgs.additionalStrs.push_back(mtlFN);
							additionalArgs.additionalStrs.push_back(inPre);
							// Add the filename to the list of output files
							outputFilenames.push_back(mtlFN);
						}
						else if (outExt == ".dae")
						{
							// Pass the input filename to convert mesh for texture filenames
							additionalArgs.additionalStrs.push_back(inPre);
							// Check the animations file is being used
							if (afn != "")
							{
								string animDat;
								// Read from the animations file
								cout << "Reading from " << afn << endl;
								if (binaryFileRead(pth + afn, animDat))
								{
									// Add the animation data so it can be used by the convertMesh function to add animations to the DAE file
									additionalArgs.additionalStrs.push_back(animDat);
								}
								else
								{
									cout << "Error: Failed to open " << afn << " for reading" << endl;
								}
							}
						}
						else if (outExt == ".mdls")
						{
							// Check if any template files were provided
							if (jtn != "")
							{
								string jointTemplateDat;
								cout << "Reading from " << jtn << endl;
								if (binaryFileRead(pth + jtn, jointTemplateDat))
								{
									additionalArgs.additionalStrs.push_back(jointTemplateDat);
								}
								else
								{
									cout << "Error: Failed to open " << jtn << " for reading" << endl;
								}
							}
							if (vtn != "")
							{
								string vertexTemplateDat;
								if (jtn == "")
								{
									additionalArgs.additionalStrs.push_back("");
								}
								cout << "Reading from " << vtn << endl;
								if (binaryFileRead(pth + vtn, vertexTemplateDat))
								{
									additionalArgs.additionalStrs.push_back(vertexTemplateDat);
								}
								else
								{
									cout << "Error: Failed to open " << vtn << " for reading" << endl;
								}
							}
						}
						else if (outExt == ".wpn")
						{
							// Check if any template files were provided
							if (jtn != "")
							{
								string jointTemplateDat;
								cout << "Reading from " << jtn << endl;
								if (binaryFileRead(pth + jtn, jointTemplateDat))
								{
									additionalArgs.additionalStrs.push_back(jointTemplateDat);
								}
								else
								{
									cout << "Error: Failed to open " << jtn << " for reading" << endl;
								}
							}
						}
					}
					// Convert the input to the viewing friendly format
					if (mode == MESH_MODE)
					{
						// Convert the model data into the requested format
						cout << "Converting model from " << inExt << " to " << outExt << endl;
						outputData = convertMesh(dat, inExt, outExt, additionalArgs);
					}
					else if (mode == SHADOW_MODE)
					{
						if (inExt == ".mdls")
						{
							// Convert the model data into the requested format
							cout << "Converting model from " << inExt << " to " << outExt << endl;
							additionalArgs.additionalBools.push_back(true);
							outputData = convertMesh(dat, inExt, outExt, additionalArgs);
						}
						else
						{
							cout << "Error: Shadow mode only supports MDLS models" << endl;
						}
					}
					else if (mode == IMPORT_TEXTURE_MODE)
					{
						if (inExt == ".moa" || inExt == ".mdls" || inExt == ".wpn" || inExt == ".tzb" || inExt == ".ps2" || inExt == ".spe")
						{
							string textureDat = "";
							// Open each texture file
							for (unsigned int i = 0; i < textureNames.size(); i++)
							{
								// Get the texture's extension
								string tfn = textureNames[i], tfnExt = "";
								strInd = tfn.find_last_of('.');
								if (strInd < tfn.size())
								{
									tfnExt = tfn.substr(strInd, tfn.size() - strInd);
									// Convert the extension to lowercase
									for (unsigned int j = 1; j < tfnExt.size(); j++)
									{
										if (tfnExt[j] >= 'A' && tfnExt[j] <= 'Z')
										{
											tfnExt[j] += 32;
										}
									}
								}
								// Make sure the texture is a png
								if (tfnExt == ".png")
								{
									cout << "Reading from " << tfn << endl;
									Texture t;
									if (readTexture(pth + tfn, t))
									{
										textures.push_back(t);
									}
									else
									{
										cout << "Error: Failed to read the texture from " << tfn << endl;
									}
								}
								else
								{
									cout << "Error: Failed to import " << tfn << endl;
									cout << "       " << tfnExt << " is not a supported file type for textures!" << endl;
								}
							}
							// Send the model with updated textures to be written to the disk
							cout << "Importing textures" << endl;
							if (outputData.size() > 0)
							{
								if (inExt == ".moa")
								{
									outputData[0] = setMoaTextures(dat, textures);
								}
								else if (inExt == ".mdls")
								{
									outputData[0] = setMdlsTextures(dat, textures);
								}
								else if (inExt == ".wpn")
								{
									outputData[0] = setWpnTextures(dat, textures);
								}
								else if (inExt == ".tzb")
								{
									outputData[0] = setTezbTextures(dat, textures);
								}
								else if (inExt == ".ps2")
								{
									outputData[0] = setPs2Textures(dat, textures);
								}
								else if (inExt == ".spe")
								{
									outputData[0] = setSpeTextures(dat, textures);
								}
								else
								{
									cout << "Error: You should not be able to get here" << endl;
								}
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for importing textures" << endl;
						}
					}
					else if (mode == IMPORT_TEXA_TEXTURE_MODE)
					{
						if (inExt == ".mset" || inExt == ".wpn")
						{
							if (refExt == ".mdls" || refExt == ".wpn")
							{
								string textureDat = "", referenceDat = "";
								// Get the raw reference data
								cout << "Reading from " << rfn << endl;
								if (binaryFileRead(pth + rfn, referenceDat))
								{
									// Get the reference textures
									vector<Texture> refTextures;
									if (refExt == ".mdls")
									{
										refTextures = getMdlsTextures(referenceDat);
									}
									else if (refExt == ".wpn")
									{
										refTextures = getWpnTextures(referenceDat);
									}
									// Open each texture file
									for (unsigned int i = 0; i < textureNames.size(); i++)
									{
										// Get the texture's extension
										string tfn = textureNames[i], tfnExt = "";
										strInd = tfn.find_last_of('.');
										if (strInd < tfn.size())
										{
											tfnExt = tfn.substr(strInd, tfn.size() - strInd);
											// Convert the extension to lowercase
											for (unsigned int j = 1; j < tfnExt.size(); j++)
											{
												if (tfnExt[j] >= 'A' && tfnExt[j] <= 'Z')
												{
													tfnExt[j] += 32;
												}
											}
										}
										// Make sure the texture is a png
										if (tfnExt == ".png")
										{
											cout << "Reading from " << tfn << endl;
											Texture t;
											if (readTexture(pth + tfn, t))
											{
												textures.push_back(t);
											}
											else
											{
												cout << "Error: Failed to read the texture from " << tfn << endl;
											}
										}
										else
										{
											cout << "Error: Failed to import " << tfn << endl;
											cout << "       " << tfnExt << " is not a supported file type for textures!" << endl;
										}
									}
									// Send the model with updated textures to be written to the disk
									cout << "Importing textures" << endl;
									if (outputData.size() > 0)
									{
										if (inExt == ".mset")
										{
											outputData[0] = setMsetTextures(dat, textures, refTextures, refInds);
										}
										else if (inExt == ".wpn")
										{
											cout << "Error: Importing to a WPN's TEXA section is not currently supported" << endl;
											//outputData[0] = setWpnTextures(dat, textures);
											outputData[0] = "";
										}
										else
										{
											cout << "Error: You should not be able to get here" << endl;
										}
									}
								}
								else
								{
									cout << "Error: Failed to read from " << rfn << endl;
								}
							}
							else
							{
								cout << "Error: " << refExt << " is not a supported file type for a TEXA reference" << endl;
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for importing TEXA textures" << endl;
						}
					}
					else if (mode == EXTRACT_TEXTURE_MODE)
					{
						// Make sure the input file is an mdls file (Currently cannot extract from other file types, shouldn't be necessary)
						if (inExt == ".bin" || inExt == ".dae"|| inExt == ".img" || inExt == ".mag" || inExt == ".mdls" || inExt == ".mfa" || inExt == ".moa" || inExt == ".obj" || inExt == ".ps2" || inExt == ".rtb" || inExt == ".spe" || inExt == ".tzb" || inExt == ".wpn")
						{
							string textureFilename, iAsStr;
							stringstream ss;
							if (inExt == ".bin")
							{
								skyboxTextures = getBinTextures(dat);
								effectTextures = getBinSpecialEffectTextures(dat);
							}
							else if (inExt == ".dae" || inExt == ".obj")
							{
								// Declare variables for using assimp importers
								Assimp::Importer importer;
								const aiScene *scene;
								unsigned int assimpImporterFlags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_RemoveComponent | aiProcess_GenNormals | aiProcess_SortByPType | aiProcess_GenUVCoords;
								// Specify components to remove when importing
								importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, PP_RVC_FLAGS);
								// Import the model using the assimp importer
								scene = importer.ReadFileFromMemory(dat.c_str(), dat.size(), assimpImporterFlags);
								if (scene)
								{
									textures = getSceneTextures(scene, pth);
								}
								else
								{
									cout << "Error: Failed to import " << inExt << " file" << endl;
								}
							}
							else if (inExt == ".img")
							{
								textures = getImgTextures(dat);
							}
							else if (inExt == ".mag")
							{
								effectTextures = getMagSpecialEffectTextures(dat);
							}
							else if (inExt == ".mdls")
							{
								// Get the textures
								textures = getMdlsTextures(dat);
								hudTextures = getMdlsHUDTextures(dat);
								effectTextures = getMdlsSpecialEffectTextures(dat);
								getMdlsHDFiles(dat, hdFileNames, hdFileDats);
								if (afn != "")
								{
									string animDat;
									if (binaryFileRead(pth + afn, animDat))
									{
										texaTextures = getMsetTextures(animDat, textures);
									}
								}
							}
							else if (inExt == ".mfa")
							{
								textures = getMfaTextures(dat);
							}
							else if(inExt == ".moa")
							{
								textures = getMoaTextures(dat);
							}
							else if (inExt == ".ps2")
							{
								textures = getPs2Textures(dat);
							}
							else if (inExt == ".rtb")
							{
								textures = getRtbTextures(dat);
							}
							else if (inExt == ".spe")
							{
								textures = getSpeTextures(dat);
							}
							else if (inExt == ".tzb")
							{
								textures = getTezbTextures(dat);
							}
							else if (inExt == ".wpn")
							{
								textures = getWpnTextures(dat);
								effectTextures = getWpnSpecialEffectTextures(dat);
							}
							else
							{
								cout << "Error: You should not be able to get here" << endl;
							}
							for (unsigned int i = 0; i < textures.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								textureFilename = inPre + "-" + iAsStr + ".png";
								cout << "Writing to " << textureFilename << endl;
								if (textures[i].palette.colors.size() > 1)
								{
									cout << "Warning: " << textureFilename << " has multiple palettes, use the GUI mode to ensure the correct palette is applied" << endl;
								}
								if (!writeTexture(textures[i], pth + textureFilename))
								{
									cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
								}
							}
							for (unsigned int i = 0; i < texaTextures.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								textureFilename = inPre + "-m" + iAsStr + ".png";
								cout << "Writing to " << textureFilename << endl;
								if (!writeTexture(texaTextures[i], pth + textureFilename))
								{
									cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
								}
							}
							for (unsigned int i = 0; i < effectTextures.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								textureFilename = inPre + "-sp" + iAsStr + ".png";
								cout << "Writing to " << textureFilename << endl;
								if (!writeTexture(effectTextures[i], pth + textureFilename))
								{
									cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
								}
							}
							for (unsigned int i = 0; i < hudTextures.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								textureFilename = inPre + "-h" + iAsStr + ".png";
								cout << "Writing to " << textureFilename << endl;
								if (!writeTexture(hudTextures[i], pth + textureFilename))
								{
									cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
								}
							}
							for (unsigned int i = 0; i < skyboxTextures.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								textureFilename = inPre + "-k" + iAsStr + ".png";
								cout << "Writing to " << textureFilename << endl;
								if (!writeTexture(skyboxTextures[i], pth + textureFilename))
								{
									cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
								}
							}
							for (unsigned int i = 0; i < hdFileNames.size() && i < hdFileDats.size(); i++)
							{
								cout << "Writing to " << hdFileNames[i] << endl;
								out.open(pth + hdFileNames[i], ofstream::binary);
								if (out.is_open())
								{
									out.write(hdFileDats[i].c_str(), hdFileDats[i].size());
									out.close();
								}
								else
								{
									cout << "Failed to open " << hdFileNames[i] << " for writing" << endl;
								}
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for extracting textures" << endl;
						}
					}
					else if (mode == IMPORT_EFFECT_TEXTURE_MODE)
					{
						if (inExt == ".bin" || inExt == ".mag" || inExt == ".mdls" || inExt == ".wpn")
						{
							// Initialize variables
							string textureDat = "";
							vector<vector<Texture>> groupedTextures;
							unsigned int numGroups = 0;
							// Determine the number of groups from the group inds
							for (unsigned int i = 0; i < groupInds.size(); i++)
							{
								if (groupInds[i] >= numGroups)
								{
									numGroups = groupInds[i] + 1;
								}
							}
							// Reserve the number of groups
							groupedTextures.reserve(numGroups);
							// Create the texture group vectors
							for (unsigned int i = 0; i < numGroups; i++)
							{
								groupedTextures.push_back(vector<Texture>());
							}
							// Open each texture file
							for (unsigned int i = 0; i < textureNames.size(); i++)
							{
								// Get the texture's extension
								string tfn = textureNames[i], tfnExt = "";
								strInd = tfn.find_last_of('.');
								if (strInd < tfn.size())
								{
									tfnExt = tfn.substr(strInd, tfn.size() - strInd);
									// Convert the extension to lowercase
									for (unsigned int j = 1; j < tfnExt.size(); j++)
									{
										if (tfnExt[j] >= 'A' && tfnExt[j] <= 'Z')
										{
											tfnExt[j] += 32;
										}
									}
								}
								// Make sure the texture is a png
								if (tfnExt == ".png")
								{
									cout << "Reading from " << tfn << endl;
									Texture t;
									if (readTexture(pth + tfn, t))
									{
										textures.push_back(t);
									}
									else
									{
										cout << "Error: Failed to read the texture from " << tfn << endl;
									}
								}
								else
								{
									cout << "Error: Failed to import " << tfn << endl;
									cout << "       " << tfnExt << " is not a supported file type for textures!" << endl;
								}
							}
							// Fill the texture groups
							for (unsigned int i = 0; i < groupInds.size(); i++)
							{
								groupedTextures[groupInds[i]].push_back(textures[i]);
							}
							// Send the model with updated textures to be written to the disk
							cout << "Importing textures" << endl;
							if (outputData.size() > 0)
							{
								if (inExt == ".bin")
								{
									cout << "Error: Importing special effect textures into .bin files is not currently supported" << endl;
								}
								else if (inExt == ".mag")
								{
									outputData[0] = setMagSpecialEffectTextures(dat, groupedTextures);
								}
								else if (inExt == ".mdls")
								{
									outputData[0] = setMdlsSpecialEffectTextures(dat, groupedTextures);
								}
								else if (inExt == ".wpn")
								{
									outputData[0] = setWpnSpecialEffectTextures(dat, groupedTextures);
								}
								else
								{
									cout << "Error: You should not be able to get here" << endl;
								}
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for importing textures" << endl;
						}
					}
					else if (mode == IMPORT_EFFECT_MODE)
					{
						// Make sure the input file is an MDLS or WPN
						if (inExt == ".ard" || inExt == ".mdls" || inExt == ".wpn" || inExt == ".rtb" || inExt == ".tarc")
						{
							string effectDat;
							unsigned int importMode = 0;
							// Open each effect file
							for (unsigned int i = 0; i < textureNames.size(); i++)
							{
								// Reset the effect
								effectDat = "";
								// Get the texture's extension
								string efn = textureNames[i], efnExt = "";
								strInd = efn.find_last_of('.');
								if (strInd < efn.size())
								{
									efnExt = efn.substr(strInd, efn.size() - strInd);
									// Convert the extension to lowercase
									for (unsigned int j = 1; j < efnExt.size(); j++)
									{
										if (efnExt[j] >= 'A' && efnExt[j] <= 'Z')
										{
											efnExt[j] += 32;
										}
									}
								}
								if (inExt == ".dpx")
								{
									// Make sure the texture is an spe (special effect)
									if (efnExt == ".spe")
									{
										cout << "Reading from " << efn << endl;
										if (binaryFileRead(pth + efn, effectDat))
										{
											effects.push_back(effectDat);
										}
										else
										{
											cout << "Error: Failed to import " << efn << endl;
											cout << "       Unable to open file!" << endl;
										}
									}
									else
									{
										cout << "Error: Failed to import " << efn << endl;
										cout << "       " << efnExt << " is not a supported file type for " << inExt << " effects!" << endl;
									}
								}
								else if (inExt == ".mdls" || inExt == ".wpn")
								{
									// Make sure the texture is an spe (special effect)
									if (efnExt == ".dpx")
									{
										cout << "Reading from " << efn << endl;
										if (binaryFileRead(pth + efn, effectDat))
										{
											effects.push_back(effectDat);
										}
										else
										{
											cout << "Error: Failed to import " << efn << endl;
											cout << "       Unable to open file!" << endl;
										}
									}
									else
									{
										cout << "Error: Failed to import " << efn << endl;
										cout << "       " << efnExt << " is not a supported file type for " << inExt << " effects!" << endl;
									}
								}
								else if (inExt == ".rtb" || inExt == ".tarc")
								{
									cout << "Reading from " << efn << endl;
									if (binaryFileRead(pth + efn, effectDat))
									{
										effects.push_back(effectDat);
									}
									else
									{
										cout << "Error: Failed to import " << efn << endl;
										cout << "       Unable to open file!" << endl;
									}
								}
								else if (inExt == ".ard")
								{
									if (efnExt == ".moa" || efnExt == ".mfa")
									{
										if (importMode == 0)
										{
											if (efnExt == ".moa")
											{
												importMode = 1;
											}
											else
											{
												importMode = 2;
											}
										}
										if (efnExt == ".moa" && importMode == 1 || efnExt == ".mfa" && importMode == 2)
										{
											cout << "Reading from " << efn << endl;
											if (binaryFileRead(pth + efn, effectDat))
											{
												effects.push_back(effectDat);
											}
											else
											{
												cout << "Error: Failed to import " << efn << endl;
												cout << "       Unable to open file!" << endl;
											}
										}
										else
										{
											cout << "Error: Cannot import to multiple sections simultaneously" << endl;
											cout << "       Ignoring " << efn << endl;
										}
									}
									else
									{
										cout << "Error: Failed to import " << efn << endl;
										cout << "       " << efnExt << " is not a supported file type for effects!" << endl;
									}
								}
							}
							// Send the model with updated textures to be written to the disk
							cout << "Importing effects" << endl;
							if (outputData.size() > 0)
							{
								if (inExt == ".ard")
								{
									if (importMode == 1)
									{
										outputData[0] = setArdMoas(dat, effects);
									}
									else if (importMode == 2)
									{
										outputData[0] = setArdMfas(dat, effects);
									}
								}
								if (inExt == ".dpx")
								{
									outputData[0] = setDpxSpecialEffects(effects);
								}
								else if (inExt == ".mdls")
								{
									string dpx = "";
									if (effects.size() > 0)
									{
										dpx = effects.back();
									}
									outputData[0] = setMdlsDpx(dat, dpx);
								}
								else if (inExt == ".wpn")
								{
									string dpx = "";
									if (effects.size() > 0)
									{
										dpx = effects.back();
									}
									outputData[0] = setWpnDpx(dat, dpx);
								}
								else if (inExt == ".rtb" || inExt == ".tarc")
								{
									outputData[0] = setTarcFiles(dat, effects);
								}
								else
								{
									cout << "Error: You should not be able to get here" << endl;
								}
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for importing files" << endl;
						}
					}
					else if (mode == EXTRACT_EFFECT_MODE)
					{
						// Make sure the input file is an mdls or wpn file (Currently cannot extract from other file types, shouldn't be necessary)
						if (inExt == ".ard" || inExt == ".bin" || inExt == ".dpx" || inExt == ".mag" || inExt == ".mdls" || inExt == ".moa" || inExt == ".wpn" || inExt == ".rtb" || inExt == ".tarc")
						{
							string effectFilename, iAsStr, tag = "";
							stringstream ss;
							vector<string> outExts, filenames;
							// Get the effects based on the file type
							if (inExt == ".ard")
							{
								vector<string> tempEffects, tempFilenames = getArdFilenames(dat);
								effects = getArdMoas(dat);
								outExts.reserve(effects.size());
								filenames.reserve(effects.size());
								for (unsigned int i = 0; i < effects.size(); i++)
								{
									outExts.push_back(".moa");
								}
								for (unsigned int i = 0; i < tempFilenames.size(); i++)
								{
									unsigned int dotInd = tempFilenames[i].find_first_of('.');
									if (dotInd < tempFilenames[i].size())
									{
										string fn = tempFilenames[i], ext = fn.substr(dotInd);
										if (ext == ".moa")
										{
											filenames.push_back(fn);
										}
									}
								}
								tempEffects = getArdMfas(dat);
								effects.reserve(effects.size() + tempEffects.size() + 1);
								outExts.reserve(outExts.size() + tempEffects.size() + 1);
								for (unsigned int i = 0; i < tempEffects.size(); i++)
								{
									effects.push_back(tempEffects[i]);
									outExts.push_back(".mfa");
								}
								filenames.reserve(effects.size() + 1);
								for (unsigned int i = 0; i < tempFilenames.size(); i++)
								{
									unsigned int dotInd = tempFilenames[i].find_first_of('.');
									if (dotInd < tempFilenames[i].size())
									{
										string fn = tempFilenames[i], ext = fn.substr(dotInd);
										if (ext == ".mfa")
										{
											filenames.push_back(fn);
										}
									}
								}
								string filenameList = "";
								for (unsigned int i = 0; i < tempFilenames.size(); i++)
								{
									filenameList += tempFilenames[i] + "\r\n";
								}
								effects.push_back(filenameList);
								filenames.push_back(inPre + ".log");
								outExts.push_back(".log");
							}
							else if (inExt == ".bin")
							{
								effects.reserve(1);
								outExts.reserve(1);
								effects.push_back(getBinDpx(dat));
								outExts.push_back(".dpx");
								tag = "s";
							}
							else if (inExt == ".dpx")
							{
								effects = getDpxSpecialEffects(dat);
								outExts.reserve(effects.size());
								for (unsigned int i = 0; i < effects.size(); i++)
								{
									outExts.push_back(".spe");
								}
							}
							else if (inExt == ".mag")
							{
								effects.reserve(1);
								outExts.reserve(1);
								effects.push_back(getMagDpx(dat));
								outExts.push_back(".dpx");
								tag = "s";
							}
							else if (inExt == ".moa")
							{
								effects.reserve(1);
								outExts.reserve(1);
								effects.push_back(getMoaDpx(dat));
								outExts.push_back(".dpx");
								tag = "s";
							}
							else if (inExt == ".mdls")
							{
								effects.reserve(1);
								outExts.reserve(1);
								effects.push_back(getMdlsDpx(dat));
								outExts.push_back(".dpx");
								tag = "s";
							}
							else if (inExt == ".wpn")
							{
								effects.reserve(1);
								outExts.reserve(1);
								effects.push_back(getWpnDpx(dat));
								outExts.reserve(effects.size());
								outExts.push_back(".dpx");
								tag = "s";
							}
							else if (inExt == ".rtb" || inExt == ".tarc")
							{
								effects = getTarcFiles(dat);
								outExts.reserve(effects.size());
								for (unsigned int i = 0; i < effects.size(); i++)
								{
									if (effects[i].size() >= 4)
									{
										string magic = effects[i].substr(0, 4);
										if (magic == "TARC")
										{
											outExts.push_back(".tarc");
										}
										else if (magic == "TEZB")
										{
											outExts.push_back(".tzb");
										}
										else if (magic == "MOPB")
										{
											outExts.push_back(".mlb");
										}
										else
										{
											cout << "Warning: Unknown File Type found - " << magic << endl << "Using .bin extension" << endl;
											outExts.push_back(".bin");
										}
									}
									else
									{
										cout << "Warning: File is too short to have magic value" << endl << "Using .bin extension" << endl;
										outExts.push_back(".bin");
									}
								}
							}
							else
							{
								cout << "Error: You should not be able to get here" << endl;
							}
							for (unsigned int i = 0; i < effects.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								if (i < filenames.size())
								{
									effectFilename = filenames[i];
								}
								else if (i < outExts.size())
								{
									effectFilename = inPre + "-" + tag + iAsStr + outExts[i];
								}
								else
								{
									effectFilename = inPre + "-" + tag + iAsStr;
								}
								outputFilenames.push_back(effectFilename);
								outputData.push_back(effects[i]);
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for extracting effects" << endl;
						}
					}
					else if (mode == IMPORT_HUD_TEXTURE_MODE)
					{
						if (inExt == ".mdls")
						{
							Texture t;
							string textureDat = "";
							// Open each texture file
							if (textureNames.size() > 0)
							{
								// Get the texture's extension
								string tfn = textureNames[0], tfnExt = "";
								strInd = tfn.find_last_of('.');
								if (strInd < tfn.size())
								{
									tfnExt = tfn.substr(strInd, tfn.size() - strInd);
									// Convert the extension to lowercase
									for (unsigned int j = 1; j < tfnExt.size(); j++)
									{
										if (tfnExt[j] >= 'A' && tfnExt[j] <= 'Z')
										{
											tfnExt[j] += 32;
										}
									}
								}
								// Make sure the texture is a png
								if (tfnExt == ".png")
								{
									cout << "Reading from " << tfn << endl;
									if (!readTexture(pth + tfn, t))
									{
										cout << "Error: Failed to read the texture from " << tfn << endl;
									}
								}
								else
								{
									cout << "Error: Failed to import " << tfn << endl;
									cout << "       " << tfnExt << " is not a supported file type for textures!" << endl;
								}
							}
							// Send the model with updated textures to be written to the disk
							cout << "Importing texture" << endl;
							if (outputData.size() > 0)
							{
								if (inExt == ".mdls")
								{
									outputData[0] = setMdlsHUDTexture(dat, t);
								}
								else
								{
									cout << "Error: You should not be able to get here" << endl;
								}
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for importing textures" << endl;
						}
					}
					else if(mode == EXTRACT_SOUND_MODE)
					{
						// Make sure the input file is an mdls or wpn file (Currently cannot extract from other file types, shouldn't be necessary)
						if (inExt == ".mdls" || inExt == ".se" || inExt == ".vsb" || inExt == ".vset" || inExt == ".wd")
						{
							string iAsStr;
							stringstream ss;
							vector<string> effectFilenames, outExts, tmp;
							// Get the effects based on the file type
							if (inExt == ".mdls")
							{
								string se = getMdlsSe(dat);
								tmp = getMdlsVags(dat);
								effects.reserve(tmp.size() + 1);
								effects.push_back(se);
								for (unsigned int i = 0; i < tmp.size(); i++)
								{
									effects.push_back(tmp[i]);
								}
							}
							else if (inExt == ".se")
							{
								effects = getSeWds(dat);
							}
							else if (inExt == ".vsb")
							{
								effects = getVsbVags(dat);
							}
							else if (inExt == ".vset")
							{
								effects = getVsetVags(dat);
							}
							else if (inExt == ".wd")
							{
								effects = getWdVags(dat, inPre);
							}
							else
							{
								cout << "Some1fromthedark you forgot to add support for the new filetype in main() you big dummy" << endl;
							}
							effectFilenames.reserve(effects.size());
							outExts.reserve(effects.size());
							for (unsigned int i = 0; i < effects.size(); i++)
							{
								string ei = effects[i];
								if (ei.size() >= 4)
								{
									string magic = ei.substr(0, 4);
									if (magic == "VAGp")
									{
										if (ei.size() >= 48)
										{
											unsigned int dotInd;
											effectFilenames.push_back(string(ei.substr(32, 16).c_str()));
											dotInd = (unsigned int)effectFilenames[i].find_last_of('.');
											if (dotInd >= effectFilenames[i].size() || effectFilenames[i].substr(dotInd, 4) != ".vag")
											{
												outExts.push_back(".vag");
											}
											else
											{
												outExts.push_back("");
											}
										}
										else
										{
											ss.clear();
											ss.str("");
											ss << i;
											ss >> iAsStr;
											effectFilenames.push_back(inPre + "-s" + iAsStr);
											outExts.push_back(".vag");
										}
									}
									else if (magic.substr(0, 2) == "WD")
									{
										ss.clear();
										ss.str("");
										ss << i;
										ss >> iAsStr;
										effectFilenames.push_back(inPre + "-w" + iAsStr);
										outExts.push_back(".wd");
									}
									else if (ei.size() >= 16 && *(unsigned int *)&(ei[0]) == *(unsigned int *)&(ei[4]) && *(unsigned int *)&(ei[12]) == 0
												&& ei.size() >= (*(unsigned int *)&(ei[0]) + *(unsigned int *)&(ei[8]) + 16))
									{
										ss.clear();
										ss.str("");
										ss << i;
										ss >> iAsStr;
										effectFilenames.push_back(inPre + "-s" + iAsStr);
										outExts.push_back(".se");
									}
									else
									{
										cout << "Warning: Unknown file type for sound " << i << endl;
										ss.clear();
										ss.str("");
										ss << i;
										ss >> iAsStr;
										effectFilenames.push_back(inPre + "-b" + iAsStr);
										outExts.push_back(".bin");
									}
								}
								else
								{
									cout << "Warning: Unknown file type for sound " << i << endl;
									ss.clear();
									ss.str("");
									ss << i;
									ss >> iAsStr;
									effectFilenames.push_back(inPre + "-s" + iAsStr);
									outExts.push_back(".bin");
								}
							}
							for (unsigned int i = 0; i < effects.size(); i++)
							{
								outputFilenames.push_back(effectFilenames[i] + outExts[i]);
								outputData.push_back(effects[i]);
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for extracting sounds" << endl;
						}
					}
					else if (mode == IMPORT_SOUND_MODE)
					{
						// Make sure the input file is an MDLS, VSB, or VSET
						if (inExt == ".mdls" || inExt == ".se" || inExt == ".vsb" || inExt == ".vset" || inExt == ".wd")
						{
							string effectDat, expectedExts = "";
							// Open each effect file
							for (unsigned int i = 0; i < textureNames.size(); i++)
							{
								// Reset the effect
								effectDat = "";
								// Get the texture's extension
								string efn = textureNames[i], efnExt = "";
								strInd = efn.find_last_of('.');
								if (strInd < efn.size())
								{
									efnExt = efn.substr(strInd, efn.size() - strInd);
									// Convert the extension to lowercase
									for (unsigned int j = 1; j < efnExt.size(); j++)
									{
										if (efnExt[j] >= 'A' && efnExt[j] <= 'Z')
										{
											efnExt[j] += 32;
										}
									}
								}
								if (expectedExts == "")
								{
									expectedExts = efnExt;
								}
								if (efnExt == expectedExts)
								{
									// Make sure the texture is an spe (special effect)
									if (efnExt == ".vag" || efnExt == ".wd" || efnExt == ".se")
									{
										cout << "Reading from " << efn << endl;
										if (binaryFileRead(pth + efn, effectDat))
										{
											effects.push_back(effectDat);
										}
										else
										{
											cout << "Error: Failed to import " << efn << endl;
											cout << "       Unable to open file!" << endl;
										}
									}
									else
									{
										cout << "Error: Failed to import " << efn << endl;
										cout << "       " << efnExt << " is not a supported file type for effects!" << endl;
									}
								}
								else
								{
									cout << "Error: All the input sound files must be of the same type" << endl;
								}
							}
							// Send the model with updated textures to be written to the disk
							cout << "Importing sounds" << endl;
							if (outputData.size() > 0)
							{
								bool mismatch = false;
								if (inExt == ".mdls")
								{
									if (expectedExts == ".vag")
									{
										outputData[0] = setMdlsVags(dat, effects);
									}
									else if (expectedExts == ".se")
									{
										outputData[0] = setMdlsSe(dat, effects[0]);
									}
									else
									{
										mismatch = true;
									}
								}
								else if (inExt == ".se")
								{
									if (expectedExts == ".wd")
									{
										outputData[0] = setSeWds(dat, effects);
									}
									else
									{
										mismatch = true;
									}
								}
								else if (inExt == ".vsb")
								{
									if (expectedExts == ".vag")
									{
										outputData[0] = setVsbVags(dat, effects);
									}
									else
									{
										mismatch = true;
									}
								}
								else if (inExt == ".vset")
								{
									if (expectedExts == ".vag")
									{
										outputData[0] = setVsetVags(effects);
									}
									else
									{
										mismatch = true;
									}
								}
								else if (inExt == ".wd")
								{
									if (expectedExts == ".vag")
									{
										outputData[0] = setWdVags(dat, effects);
									}
									else
									{
										mismatch = true;
									}
								}
								else
								{
									cout << "Error: You should not be able to get here" << endl;
								}
								if (mismatch)
								{
									cout << "Error: " << inExt << " cannot contain " << expectedExts << " files" << endl;
								}
							}
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for importing files" << endl;
						}
					}
					else if (mode == EXTRACT_COLLISION_MODE)
					{
						vector<vector<VertexGlobal>> collisionVerts;
						vector<vector<Face>> collisionFaces;
						vector<JointRelative> joints;
						getBasicJoints(joints);
						if (inExt == ".mfa")
						{
							getMfaCollisions(dat, collisionVerts, collisionFaces);
						}
						else if (inExt == ".bin")
						{
							getBinCollisions(dat, collisionVerts, collisionFaces);
						}
						else
						{
							cout << "Error: " << inExt << " is not a supported file type for extracting collision" << endl;
						}
						if (outputData.size() > 0)
						{
							outputData[0] = convertMeshToDae(joints, collisionVerts, collisionFaces, vector<Animation>(), inPre);
						}
					}
					else
					{
						cout << "Warning: How did you get here?" << endl;
						cout << "Some1fromthedark you forgot to call the new mode from main() you big dummy" << endl;
					}
					// Make sure the conversion was successful
					for (unsigned int i = 0; i < outputData.size() && i < outputFilenames.size(); i++)
					{
						if (outputData[i].size() > 0)
						{
							// Open the output file
							cout << "Writing to " << outputFilenames[i] << endl;
							out.open(pth + outputFilenames[i], ofstream::binary);
							// Make sure it opened successfully
							if (out.is_open())
							{
								// Write the converted file
								out.write(outputData[i].c_str(), outputData[i].size());
								// Close the output file
								out.close();
							}
							else
							{
								cout << "Error: Failed to open " << outputFilenames[i] << " for writing" << endl;
							}
						}
					}
				}
				else
				{
					cout << "Error: Failed to read from " << fn << endl;
				}
			}
		}
	}
	else
	{
		// Failed to parse arguments, switch to help mode to help the user
		if (mode == ERROR_MODE)
		{
			cout << "Error: Invalid Call" << endl << endl;
		}
		mode = HELP_MODE;
	}
	// Check if the program was invoked in help mode (or improperly)
	if (mode == HELP_MODE)
	{
		// Display help message
		cout << "Usage: HyperCrown [mode] [options] [positional]" << endl;
		cout << endl;
		cout << "Modes:" << endl;
		cout << "    --help                          (-h): Displays this message" << endl;
		cout << "    --mesh input output [reference] (-m): Run in Model Converter mode" << endl;
		cout << "    --shadow input output           (-d): Run in Shadow mode" << endl;
		cout << "    --texture input output texture+ (-t): Run in Import Texture mode" << endl;
		cout << "    --texa input output reference   (-i): Run in TEXA Texture mode" << endl;
		cout << "        (texture origin)+" << endl;
		cout << "    --hud input output texture      (-u): Run in Import HUD Texture mode" << endl;
		cout << "    --extractTexture input [-a mset](-x): Run in Extract Texture mode" << endl;
		cout << "    --effectTexture input output    (-k): Run in Import Effect Texture mode" << endl;
		cout << "       (texture group)+ [sectionInd]" << endl;
		cout << "    --effect input output effect+   (-e): Run in Import Effect mode" << endl;
		cout << "    --extractEffect input           (-w): Run in Extract Effect mode" << endl;
		cout << "    --vag input output sound+       (-V): Run in Import Sound mode" << endl;
		cout << "    --extractSound input            (-b): Run in Extract Sounds mode" << endl;
		cout << "    --extractCollision input        (-C): Run in Extract Collision mode" << endl;
		cout << endl;
		cout << "Optional Arguments:" << endl;
		cout << "    --path file_path                (-p): Provide the path to append all" << endl;
		cout << "                                          filenames to when reading or writing" << endl;
		cout << "    --animations filename           (-a): Provide MSET file to add" << endl;
		cout << "                                          animations to a DAE conversion" << endl;
		cout << "    --normalize                     (-n): Normalize the input model to" << endl;
		cout << "                                          the range [-1, 1]" << endl;
		cout << "    --vTemplate filename            (-v): Provide a template file for" << endl;
		cout << "                                          vertex data when converting to mdls" << endl;
		cout << "    --jTemplate filename            (-j): Provide a template file for" << endl;
		cout << "                                          joint data when converting to mdls" << endl;
		cout << "    --rootDepth depth               (-r): Provide the depth of the root node" << endl;
		cout << "                                          when converting from dae (Default: 0)" << endl;
		cout << "    --flipUVs                       (-f): Set flag to flip the input model's" << endl;
		cout << "                                          UV coordinates in mesh mode" << endl;
		cout << "    --combineMeshes                 (-c): Combine the input meshes into one" << endl;
		cout << "                                          mesh when converting models" << endl;
		cout << "    --swapChirality                 (-s): Change a model from Z-Axis Up to " << endl;
		cout << "                                          Y-Axis Up when converting models" << endl;
		cout << "    --debug                         (-g): Display the console with the GUI" << endl;
		cout << endl;
		cout << "Positional Arguments:" << endl;
		cout << "    input                               : The model file to convert from" << endl;
		cout << "    output                              : The name of the output file, the" << endl;
		cout << "                                          format depends on the extension" << endl;
		cout << "    texture                             : The name of the texture image to" << endl;
		cout << "                                          use when importing a texture" << endl;
		cout << "    reference                           : The file containing the base textures" << endl;
		cout << "    origin                              : The index into reference's textures" << endl;
		cout << "                                          to use the color palette of" << endl;
		cout << "    effect                              : The name of the effect file to" << endl;
		cout << "                                          use when importing an effect" << endl;
		cout << "    mset                                : The moveset file to extract TEXA" << endl;
		cout << "                                          textures from in Extract Texture mode" << endl;
		cout << "    sound                               : The name of the sound file to" << endl;
		cout << "                                          use when importing sounds" << endl;
		cout << "Supported Filetypes:" << endl;
		cout << "    BIN        (.bin)" << endl;
		cout << "    COLLADA    (.dae)" << endl;
		cout << "    MAG        (.mag)" << endl;
		cout << "    MDLS       (.mdls)" << endl;
		cout << "    MSET       (.mset)" << endl;
		cout << "    OBJ        (.obj)" << endl;
		cout << "    PNG        (.png)" << endl;
		cout << "    SPE        (.spe)" << endl;
		cout << "    WPN        (.wpn)" << endl;
		cout << endl;
	}
	system("PAUSE");
	return 0;
}

void CLI::displayString(string message)
{
	cout << message << endl;
	return;
}

CLI::~CLI()
{
	return;
}

// GUI function definitions

GUI::GUI() : UserInterface()
{
	return;
}

void GUI::setup()
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	return;
}

int GUI::run(Arguments args)
{
	if (args.debug)
	{
		// Create Console for this application
		AllocConsole();
		// Get stdin
		HANDLE ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
		int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
		FILE *CInputHandle = _fdopen(SystemInput, "r");
		// Get stdout
		HANDLE ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
		FILE *COutputHandle = _fdopen(SystemOutput, "w");
		// Get stderr
		HANDLE ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
		int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
		FILE *CErrorHandle = _fdopen(SystemError, "w");
		// Sync with cout, etc
		ios::sync_with_stdio(true);
		freopen_s(&CInputHandle, "CONIN$", "r", stdin);
		freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
		freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);
		// Clear error states
		cout.clear();
		wcout.clear();
		cerr.clear();
		wcerr.clear();
		cin.clear();
		wcin.clear();
	}
	HyperCrown::BaseForm form;
	Application::Run(%form);
	return 0;
}

void GUI::displayString(string message)
{
	cout << message << endl;
	return;
}

GUI::~GUI()
{
	return;
}

UserInterface::UserInterface()
{
	return;
}

void UserInterface::setup()
{
	return;
}

int UserInterface::run(Arguments args)
{
	return 0;
}

void UserInterface::displayString(string message)
{
	return;
}

UserInterface::~UserInterface()
{
	return;
}

Arguments parseArgs(vector<string> argv)
{
	Arguments args;
	string arg;
	unsigned int argc = argv.size(), positionalArgCounter = 0, stringToUINT, minPositionalArgs = 0, maxPositionalArgs = 0;
	stringstream ss;
	// Initialize the argument values
	args.normalize = false;
	args.flipUVCoords = false;
	args.combineMeshes = false;
	args.swapChirality = false;
	args.mode = DEFAULT_MODE;
	args.rootDepth = 0;
	args.sectionInd = -1;
	args.path = "";
	args.inputFilename = "";
	args.outputFilename = "";
	args.animationsFilename = "";
	args.vertexTemplateModelName = "";
	args.jointTemplateModelName = "";
	args.referenceFilename = "";
	// Parse each of the arguments (ignoring the name of the program)
	for (unsigned int i = 0; i < argc; i++)
	{
		arg = argv[i];
		// Check if this value is an optional argument flag
		if (arg == "-h" || arg == "--help")
		{
			// Check if the mode is still default mode
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to help
				args.mode = HELP_MODE;
				minPositionalArgs = 0;
				maxPositionalArgs = 0;
			}
		}
		else if (arg == "-m" || arg == "--mesh")
		{
			// Check if the mode is still default mode
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to mesh
				args.mode = MESH_MODE;
				minPositionalArgs = 2;
				maxPositionalArgs = 3;
			}
			else
			{
				// Cannot set multiple modes modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-d" || arg == "--shadow")
		{
			// Check if the mode is still default mode
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to shadow
				args.mode = SHADOW_MODE;
				minPositionalArgs = 2;
				maxPositionalArgs = 2;
			}
		}
		else if (arg == "-t" || arg == "--texture")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to texture
				args.mode = IMPORT_TEXTURE_MODE;
				minPositionalArgs = 2;
				maxPositionalArgs = UINT_MAX;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-i" || arg == "--texa")
		{
			if (args.mode == DEFAULT_MODE)
			{
				args.mode = IMPORT_TEXA_TEXTURE_MODE;
				minPositionalArgs = 3;
				maxPositionalArgs = UINT_MAX;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-u" || arg == "--hud")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to import hud texture
				args.mode = IMPORT_HUD_TEXTURE_MODE;
				minPositionalArgs = 3;
				maxPositionalArgs = 3;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-x" || arg == "--extractTexture")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to extract texture
				args.mode = EXTRACT_TEXTURE_MODE;
				minPositionalArgs = 1;
				maxPositionalArgs = 1;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-k" || arg == "--effectTexture")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to import special effects
				args.mode = IMPORT_EFFECT_TEXTURE_MODE;
				minPositionalArgs = 2;
				maxPositionalArgs = UINT_MAX;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-e" || arg == "--effect")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to import special effects
				args.mode = IMPORT_EFFECT_MODE;
				minPositionalArgs = 2;
				maxPositionalArgs = UINT_MAX;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-w" || arg == "--extractEffect")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to extract special effects
				args.mode = EXTRACT_EFFECT_MODE;
				minPositionalArgs = 1;
				maxPositionalArgs = 1;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-b" || arg == "--extractSound")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to extract sounds
				args.mode = EXTRACT_SOUND_MODE;
				minPositionalArgs = 1;
				maxPositionalArgs = 1;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-V" || arg == "--vag")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to import sounds
				args.mode = IMPORT_SOUND_MODE;
				minPositionalArgs = 2;
				maxPositionalArgs = UINT_MAX;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-C" || arg == "--extractCollision")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to extract collision
				args.mode = EXTRACT_COLLISION_MODE;
				minPositionalArgs = 2;
				maxPositionalArgs = 2;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-h" || arg == "--help")
		{
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to help
				args.mode = HELP_MODE;
				minPositionalArgs = 0;
				maxPositionalArgs = 0;
			}
			else
			{
				// Cannot set multiple modes
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-a" || arg == "--animations")
		{
			// Make sure the animations filename was also provided
			if ((i + 1) < argc)
			{
				args.animationsFilename = argv[++i];
			}
			else
			{
				// Required arguement not provided
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-p" || arg == "--path")
		{
			// Make sure the path argument was also provided
			if ((i + 1) < argc)
			{
				args.path = argv[++i];
			}
			else
			{
				// Required argument not provided
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-v" || arg == "--vTemplate")
		{
			// Make sure the template filename argument was also provided
			if ((i + 1) < argc)
			{
				args.vertexTemplateModelName = argv[++i];
			}
			else
			{
				// Required argument not provided
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-j" || arg == "--jTemplate")
		{
			// Make sure the template filename argument was also provided
			if ((i + 1) < argc)
			{
				args.jointTemplateModelName = argv[++i];
			}
			else
			{
				// Required argument not provided
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-r" || arg == "--rootDepth")
		{
			// Make sure the root depth argument was also provided
			if ((i + 1) < argc)
			{
				// Convert the string to an unsigned int
				ss.clear();
				ss.str("");
				ss << argv[++i];
				ss >> stringToUINT;
				// Store the root depth
				args.rootDepth = stringToUINT;
			}
			else
			{
				// Required argument not provided
				args.mode = ERROR_MODE;
				break;
			}
		}
		else if (arg == "-n" || arg == "--normalize")
		{
			// Set the normalize flag
			args.normalize = true;
		}
		else if (arg == "-f" || arg == "--flipUVs")
		{
			// Set flip UV Coords flag
			args.flipUVCoords = true;
		}
		else if (arg == "-c" || arg == "--combineMeshes")
		{
			// Set the combine meshes flag
			args.combineMeshes = true;
		}
		else if (arg == "-s" || arg == "--swapChirality")
		{
			// Set the swap chirality flag
			args.swapChirality = true;
		}
		else if (arg == "-g" || arg == "--debug")
		{
			// Set the debug flag
			args.debug = true;
		}
		else
		{
			// This is a positional argument
			switch (positionalArgCounter)
			{
			case 0:
				args.inputFilename = arg;
				break;
			case 1:
				args.outputFilename = arg;
				break;
			case 2:
				switch (args.mode)
				{
				case IMPORT_EFFECT_MODE:
				case IMPORT_TEXTURE_MODE:
				case IMPORT_HUD_TEXTURE_MODE:
				case IMPORT_SOUND_MODE:
					args.textureNames.push_back(arg);
					break;
				case MESH_MODE:
				case IMPORT_TEXA_TEXTURE_MODE:
					args.referenceFilename = arg;
					break;
				case IMPORT_EFFECT_TEXTURE_MODE:
					if ((i + 1) < argc)
					{
						unsigned int groupInd;
						args.textureNames.push_back(arg);
						arg = argv[++i];
						ss.clear();
						ss.str("");
						ss << arg;
						ss >> groupInd;
						args.groupInds.push_back(groupInd);
					}
					else
					{
						unsigned int sectionInd;
						ss.clear();
						ss.str("");
						ss << arg;
						ss >> sectionInd;
						args.sectionInd = args.sectionInd;
					}
					break;
				default:
					break;
				}
				break;
			default:
				switch (args.mode)
				{
				case IMPORT_EFFECT_MODE:
				case IMPORT_TEXTURE_MODE:
				case IMPORT_SOUND_MODE:
					args.textureNames.push_back(arg);
					break;
				case IMPORT_TEXA_TEXTURE_MODE:
					if ((i + 1) < argc)
					{
						unsigned int referenceInd;
						args.textureNames.push_back(arg);
						arg = argv[++i];
						ss.clear();
						ss.str("");
						ss << arg;
						ss >> referenceInd;
						args.referenceInds.push_back(referenceInd);
					}
					else
					{
						args.mode = ERROR_MODE;
						i = argc;
					}
					break;
				case IMPORT_EFFECT_TEXTURE_MODE:
					if ((i + 1) < argc)
					{
						unsigned int groupInd;
						args.textureNames.push_back(arg);
						arg = argv[++i];
						ss.clear();
						ss.str("");
						ss << arg;
						ss >> groupInd;
						args.groupInds.push_back(groupInd);
					}
					else
					{
						unsigned int sectionInd;
						ss.clear();
						ss.str("");
						ss << arg;
						ss >> sectionInd;
						args.sectionInd = args.sectionInd;
					}
					break;
				default:
					break;
				}
				break;
			}
			positionalArgCounter++;
		}
	}
	// Make sure all of the positional arguments were provided
	if (positionalArgCounter < minPositionalArgs || positionalArgCounter > maxPositionalArgs)
	{
		args.mode = ERROR_MODE;
	}
	// If a mode wasn't provided, set the mode to help mode
	else if (args.mode == DEFAULT_MODE)
	{
		//args.mode = HELP_MODE;
		args.mode = GUI_MODE;
	}
	return args;
}
