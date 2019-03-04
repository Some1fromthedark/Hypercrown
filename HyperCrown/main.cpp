#include<fstream>
#include<iostream>
#include<string>
#include<vector>

// Prototype structs
struct Arguments;

// Prototype functions
int main(int argc, char **argv);
Arguments parseArgs(int argc, char **argv);


#include"matmul.h"
#include"mdls.h"

// Define the run modes for the program
#ifndef RUN_MODES
#define RUN_MODES
#define DEFAULT_MODE				0
#define HELP_MODE					1
#define MESH_MODE					2
#define IMPORT_TEXTURE_MODE			3
#define EXTRACT_TEXTURE_MODE		4
#define IMPORT_EFFECT_MODE			5
#define EXTRACT_EFFECT_MODE			6
#define SHADOW_MODE					7
#define IMPORT_TEXA_TEXTURE_MODE	8
#define ERROR_MODE					UINT_MAX
#endif

using namespace std;

struct Arguments
{
	bool normalize;
	bool flipUVCoords;
	bool combineMeshes;
	bool swapChirality;
	unsigned int mode;
	unsigned int rootDepth;
	string path;
	string inputFilename;
	string outputFilename;
	string animationsFilename;
	string vertexTemplateModelName;
	string jointTemplateModelName;
	string referenceFilename;
	vector<string> textureNames;
	vector<unsigned int> referenceInds;
};

/* Entry point for the program */
int main(int argc, char **argv)
{
	// Declare variables
	unsigned int mode, rd, maxJoints = UINT_MAX, numTextures = 0, strInd;
	string pth, fn, on, afn, vtn, jtn, rfn, inPre, inExt, outPre, outExt, animPre, animExt, refPre, refExt, dat = "", mtlDat = "";
	vector<string> outputFilenames, outputData, textureNames, effects;
	vector<unsigned int> refInds;
	vector<Texture> textures, effectTextures, texaTextures;
	//ifstream in;
	ofstream out;
	AddInfo additionalArgs;
	Arguments args;

	std::cout << prog << " v " << version << endl << "by " << tool_author << endl << endl;

	// Parse the arguments
	args = parseArgs(argc, argv);
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
				std::cout << "Reading from " << fn << endl;
				// Read from the input file
				if (binaryFileRead(pth + fn, dat))
				{
					std::cout << "Determining input file type" << endl;
					// Figure out what the input format is based on the extension
					if (inExt == ".dae")
					{
						// Add the root node depth as an additional argument
						additionalArgs.additionalInts.push_back(rd);
					}
					else if (inExt == ".obj")
					{
						// No additional arguments required
					}
					else if (inExt == ".mdls")
					{
						// Get the number of textures
						numTextures = getNumMdlsTextures(dat);
					}
					else if (inExt == ".wpn")
					{
						// Get the number of textures
						numTextures = getNumWpnTextures(dat);
					}
					// Check if a additional args are required by the output format
					if (on != "")
					{
						std::cout << "Determining output file type" << endl;
						if (outExt == ".obj")
						{
							// Generate the mtl filename (To-Do: Add an option to provide if you don't want to generate it?)
							string mtlFN = outPre + ".mtl";
							// Add the filename to the list of additional arguments for the conversion process
							additionalArgs.additionalStrs.push_back(mtlFN);
							// Add the filename to the list of output files
							outputFilenames.push_back(mtlFN);
							// Create an mtl file and add it to the list of data to write to the output file
							outputData.push_back(createMTL(numTextures, inPre));
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
								std::cout << "Reading from " << afn << endl;
								if (binaryFileRead(pth + afn, animDat))
								{
									// Add the animation data so it can be used by the convertMesh function to add animations to the DAE file
									additionalArgs.additionalStrs.push_back(animDat);
								}
								else
								{
									std::cout << "Error: Failed to open " << afn << " for reading" << endl;
								}
							}
						}
						else if (outExt == ".mdls")
						{
							// Check if any template files were provided
							if (jtn != "")
							{
								string jointTemplateDat;
								std::cout << "Reading from " << jtn << endl;
								if (binaryFileRead(pth + jtn, jointTemplateDat))
								{
									additionalArgs.additionalStrs.push_back(jointTemplateDat);
								}
								else
								{
									std::cout << "Error: Failed to open " << jtn << " for reading" << endl;
								}
							}
							if (vtn != "")
							{
								string vertexTemplateDat;
								if (jtn == "")
								{
									additionalArgs.additionalStrs.push_back("");
								}
								std::cout << "Reading from " << vtn << endl;
								if (binaryFileRead(pth + vtn, vertexTemplateDat))
								{
									additionalArgs.additionalStrs.push_back(vertexTemplateDat);
								}
								else
								{
									std::cout << "Error: Failed to open " << vtn << " for reading" << endl;
								}
							}
						}
						else if (outExt == ".wpn")
						{
							// Check if any template files were provided
							if (jtn != "")
							{
								string jointTemplateDat;
								std::cout << "Reading from " << jtn << endl;
								if (binaryFileRead(pth + jtn, jointTemplateDat))
								{
									additionalArgs.additionalStrs.push_back(jointTemplateDat);
								}
								else
								{
									std::cout << "Error: Failed to open " << jtn << " for reading" << endl;
								}
							}
						}
					}
					// Convert the input to the viewing friendly format
					if (mode == MESH_MODE)
					{
						// Convert the model data into the requested format
						std::cout << "Converting model from " << inExt << " to " << outExt << endl;
						outputData[0] = convertMesh(dat, inExt, outExt, additionalArgs);
					}
					else if (mode == SHADOW_MODE)
					{
						if (inExt == ".mdls")
						{
							// Convert the model data into the requested format
							std::cout << "Converting model from " << inExt << " to " << outExt << endl;
							additionalArgs.additionalBools.push_back(true);
							outputData[0] = convertMesh(dat, inExt, outExt, additionalArgs);
						}
						else
						{
							std::cout << "Error: Shadow mode only supports MDLS models" << endl;
						}
					}
					else if (mode == IMPORT_TEXTURE_MODE)
					{
						if (inExt == ".mdls" || inExt == ".wpn")
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
									std::cout << "Reading from " << tfn << endl;
									Texture t;
									if (readTexture(pth + tfn, t))
									{
										textures.push_back(t);
									}
									else
									{
										std::cout << "Error: Failed to read the texture from " << tfn << endl;
									}
								}
								else
								{
									std::cout << "Error: Failed to import " << tfn << endl;
									std::cout << "       " << tfnExt << " is not a supported file type for textures!" << endl;
								}
							}
							// Send the model with updated textures to be written to the disk
							std::cout << "Importing textures" << endl;
							if (inExt == ".mdls")
							{
								outputData[0] = setMdlsTextures(dat, textures);
							}
							else if (inExt == ".wpn")
							{
								outputData[0] = setWpnTextures(dat, textures);
							}
							else
							{
								std::cout << "Error: You should not be able to get here" << endl;
							}
						}
						else
						{
							std::cout << "Error: " << inExt << " is not a supported file type for importing textures" << endl;
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
								std::cout << "Reading from " << rfn << endl;
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
											std::cout << "Reading from " << tfn << endl;
											Texture t;
											if (readTexture(pth + tfn, t))
											{
												textures.push_back(t);
											}
											else
											{
												std::cout << "Error: Failed to read the texture from " << tfn << endl;
											}
										}
										else
										{
											std::cout << "Error: Failed to import " << tfn << endl;
											std::cout << "       " << tfnExt << " is not a supported file type for textures!" << endl;
										}
									}
									// Send the model with updated textures to be written to the disk
									std::cout << "Importing textures" << endl;
									if (inExt == ".mset")
									{
										outputData[0] = setMsetTextures(dat, textures, refTextures, refInds);
									}
									else if (inExt == ".wpn")
									{
										std::cout << "Error: Importing to a WPN's TEXA section is not currently supported" << endl;
										//outputData[0] = setWpnTextures(dat, textures);
										outputData[0] = "";
									}
									else
									{
										std::cout << "Error: You should not be able to get here" << endl;
									}
								}
								else
								{
									std::cout << "Error: Failed to read from " << rfn << endl;
								}
							}
							else
							{
								std::cout << "Error: " << refExt << " is not a supported file type for a TEXA reference" << endl;
							}
						}
						else
						{
							std::cout << "Error: " << inExt << " is not a supported file type for importing TEXA textures" << endl;
						}
					}
					else if (mode == EXTRACT_TEXTURE_MODE)
					{
						// Make sure the input file is an mdls file (Currently cannot extract from other file types, shouldn't be necessary)
						if (inExt == ".mdls" || inExt == ".wpn" || inExt == ".mag")
						{
							string textureFilename, iAsStr;
							stringstream ss;
							if (inExt == ".mdls")
							{
								// Get the textures
								textures = getMdlsTextures(dat);
								effectTextures = getMdlsSpecialEffectTextures(dat);
								if (afn != "")
								{
									string animDat;
									if (binaryFileRead(pth + afn, animDat))
									{
										texaTextures = getMsetTextures(animDat, textures);
									}
								}
							}
							else if (inExt == ".wpn")
							{
								textures = getWpnTextures(dat);
								effectTextures = getWpnSpecialEffectTextures(dat);
							}
							else if (inExt == ".mag")
							{
								effectTextures = getMagSpecialEffectTextures(dat);
							}
							else
							{
								std::cout << "Error: You should not be able to get here" << endl;
							}
							for (unsigned int i = 0; i < textures.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								textureFilename = inPre + "-" + iAsStr + ".png";
								std::cout << "Writing to " << textureFilename << endl;
								if (!writeTexture(textures[i], pth + textureFilename))
								{
									std::cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
								}
							}
							for (unsigned int i = 0; i < texaTextures.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								textureFilename = inPre + "-m" + iAsStr + ".png";
								std::cout << "Writing to " << textureFilename << endl;
								if (!writeTexture(texaTextures[i], pth + textureFilename))
								{
									std::cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
								}
							}
							for (unsigned int i = 0; i < effectTextures.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								textureFilename = inPre + "-sp" + iAsStr + ".png";
								std::cout << "Writing to " << textureFilename << endl;
								if (!writeTexture(effectTextures[i], pth + textureFilename))
								{
									std::cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
								}
							}
						}
						else
						{
							std::cout << "Error: " << inExt << " is not a supported file type for extracting textures" << endl;
						}
					}
					else if (mode == IMPORT_EFFECT_MODE)
					{
						// Make sure the input file is an MDLS or WPN
						if (inExt == ".mdls" || inExt == ".wpn")
						{
							string effectDat;
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
								// Make sure the texture is an spe (special effect)
								if (efnExt == ".spe")
								{
									std::cout << "Reading from " << efn << endl;
									if (binaryFileRead(pth + efn, effectDat))
									{
										effects.push_back(effectDat);
									}
									else
									{
										std::cout << "Error: Failed to import " << efn << endl;
										std::cout << "       Unable to open file!" << endl;
									}
								}
								else
								{
									std::cout << "Error: Failed to import " << efn << endl;
									std::cout << "       " << efnExt << " is not a supported file type for effects!" << endl;
								}
							}
							// Send the model with updated textures to be written to the disk
							std::cout << "Importing effects" << endl;
							if (inExt == ".mdls")
							{
								outputData[0] = setMdlsSpecialEffects(dat, effects);
							}
							else if (inExt == ".wpn")
							{
								outputData[0] = setWpnSpecialEffects(dat, effects);
							}
							else
							{
								std::cout << "Error: You should not be able to get here" << endl;
							}
						}
						else
						{
							std::cout << "Error: " << inExt << " is not a supported file type for importing textures" << endl;
						}
					}
					else if (mode == EXTRACT_EFFECT_MODE)
					{
						// Make sure the input file is an mdls or wpn file (Currently cannot extract from other file types, shouldn't be necessary)
						if (inExt == ".mdls" || inExt == ".wpn")
						{
							string effectFilename, iAsStr;
							stringstream ss;
							if (inExt == ".mdls")
							{
								// Get the textures
								effects = getMdlsSpecialEffects(dat);
							}
							else if (inExt == ".wpn")
							{
								effects = getWpnSpecialEffects(dat);
							}
							else
							{
								std::cout << "Error: You should not be able to get here" << endl;
							}
							for (unsigned int i = 0; i < effects.size(); i++)
							{
								ss.clear();
								ss.str("");
								ss << i;
								ss >> iAsStr;
								effectFilename = inPre + "-" + iAsStr + ".spe";
								outputFilenames.push_back(effectFilename);
								outputData.push_back(effects[i]);
							}
						}
						else
						{
							std::cout << "Error: " << inExt << " is not a supported file type for extracting textures" << endl;
						}
					}
					else
					{
						std::cout << "Warning: How did you get here?" << endl;
					}
					// Make sure the conversion was successful
					for (unsigned int i = 0; i < outputData.size() && i < outputFilenames.size(); i++)
					{
						if (outputData[i].size() > 0)
						{
							// Open the output file
							std::cout << "Writing to " << outputFilenames[i] << endl;
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
								std::cout << "Error: Failed to open " << outputFilenames[i] << " for writing" << endl;
							}
						}
					}
				}
				else
				{
					std::cout << "Error: Failed to read from " << fn << endl;
				}
			}
		}
	}
	else
	{
		// Failed to parse arguments, switch to help mode to help the user
		if (mode == ERROR_MODE)
		{
			std::cout << "Error: Invalid Call" << endl << endl;
		}
		mode = HELP_MODE;
	}
	// Check if the program was invoked in help mode (or improperly)
	if (mode == HELP_MODE)
	{
		// Display help message
		std::cout << "Usage: HyperCrown [mode] [options] [positional]" << endl;
		std::cout << endl;
		std::cout << "Modes:" << endl;
		std::cout << "    --help                          (-h): Displays this message" << endl;
		std::cout << "    --mesh input output             (-m): Run in Model Converter mode" << endl;
		std::cout << "    --shadow input output           (-d): Run in Shadow mode" << endl;
		std::cout << "    --texture input output texture+ (-t): Run in Import Texture mode" << endl;
		std::cout << "    --texa input output reference...(-i): Run in TEXA Texture mode" << endl;
		std::cout << "       ... (texture origin)+" << endl;
		std::cout << "    --extractTexture input [-a mset](-x): Run in Extract Texture mode" << endl;
		std::cout << "    --effect input output effect+   (-e): Run in Import Effect mode" << endl;
		std::cout << "    --extractEffect input           (-w): Run in Extract Effect mode" << endl;
		std::cout << endl;
		std::cout << "Optional Arguments:" << endl;
		std::cout << "    --path file_path                (-p): Provide the path to append all" << endl;
		std::cout << "                                          filenames to when reading or writing" << endl;
		std::cout << "    --animations filename           (-a): Provide MSET file to add" << endl;
		std::cout << "                                          animations to a DAE conversion" << endl;
		std::cout << "    --normalize                     (-n): Normalize the input model to" << endl;
		std::cout << "                                          the range [-1, 1]" << endl;
		std::cout << "    --vTemplate filename            (-v): Provide a template file for" << endl;
		std::cout << "                                          vertex data when converting to mdls" << endl;
		std::cout << "    --jTemplate filename            (-j): Provide a template file for" << endl;
		std::cout << "                                          joint data when converting to mdls" << endl;
		std::cout << "    --rootDepth depth               (-r): Provide the depth of the root node" << endl;
		std::cout << "                                          when converting from dae (Default: 0)" << endl;
		std::cout << "    --flipUVs                       (-f): Set flag to flip the input model's" << endl;
		std::cout << "                                          UV coordinates when converting a model" << endl;
		std::cout << "    --combineMeshes                 (-c): Combine the input meshes into a single" << endl;
		std::cout << "                                          mesh when converting models" << endl;
		std::cout << "    --swapChirality                 (-s): Change a model from Z-Axis Up to " << endl;
		std::cout << "                                          Y-Axis Up when converting models" << endl;
		std::cout << "Positional Arguments:" << endl;
		std::cout << "    input                               : The model file to convert from" << endl;
		std::cout << "    output                              : The name of the output file, the" << endl;
		std::cout << "                                          format depends on the extension" << endl;
		std::cout << "    texture                             : The name of the texture image to" << endl;
		std::cout << "                                          use when importing a texture" << endl;
		std::cout << "    reference                           : The " << endl;
		std::cout << "    origin                              : The " << endl;
		std::cout << "    effect                              : The name of the effect file to" << endl;
		std::cout << "                                          use when importing an effect" << endl;
		std::cout << "Supported Filetypes:" << endl;
		std::cout << "    COLLADA    (.dae)" << endl;
		std::cout << "    MAG        (.mag)" << endl;
		std::cout << "    MDLS       (.mdls)" << endl;
		std::cout << "    MSET       (.mset)" << endl;
		std::cout << "    OBJ        (.obj)" << endl;
		std::cout << "    PNG        (.png)" << endl;
		std::cout << "    SPE        (.spe)" << endl;
		std::cout << "    WPN        (.wpn)" << endl;
		std::cout << endl;
	}
	system("PAUSE");
	return 0;
}

Arguments parseArgs(int argc, char **argv)
{
	Arguments args;
	string arg;
	unsigned int positionalArgCounter = 0, stringToUINT, minPositionalArgs = 0, maxPositionalArgs = 0;
	stringstream ss;
	// Initialize the argument values
	args.normalize = false;
	args.flipUVCoords = false;
	args.combineMeshes = false;
	args.swapChirality = false;
	args.mode = DEFAULT_MODE;
	args.rootDepth = 0;
	args.path = "";
	args.inputFilename = "";
	args.outputFilename = "";
	args.animationsFilename = "";
	args.vertexTemplateModelName = "";
	args.jointTemplateModelName = "";
	args.referenceFilename = "";
	// Parse each of the arguments (ignoring the name of the program)
	for (int i = 1; i < argc; i++)
	{
		arg = argv[i];
		// Check if this value is an optional argument flag
		if (arg == "-m" || arg == "--mesh")
		{
			// Check if the mode is still default mode
			if (args.mode == DEFAULT_MODE)
			{
				// Set the mode to mesh
				args.mode = MESH_MODE;
				minPositionalArgs = 2;
				maxPositionalArgs = 2;
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
		else if (arg == "-w" || arg == "extractEffect")
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
		else if (arg == "-s" || arg == "swapChirality")
		{
			// Set the swap chirality flag
			args.swapChirality = true;
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
					args.textureNames.push_back(arg);
					break;
				case IMPORT_TEXA_TEXTURE_MODE:
					args.referenceFilename = arg;
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
		args.mode = HELP_MODE;
	}
	return args;
}
