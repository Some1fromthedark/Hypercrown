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
#define DEFAULT_MODE			0
#define HELP_MODE				1
#define MESH_MODE				2
#define IMPORT_TEXTURE_MODE		3
#define EXTRACT_TEXTURE_MODE	4
#define IMPORT_EFFECT_MODE		5
#define EXTRACT_EFFECT_MODE		6
#define ERROR_MODE				UINT_MAX
#endif

using namespace std;

struct Arguments
{
	bool normalize;
	unsigned int mode;
	unsigned int rootDepth;
	string inputFilename;
	string outputFilename;
	string animationsFilename;
	string vertexTemplateModelName;
	string jointTemplateModelName;
	vector<string> textureNames;
};

/* Entry point for the program */
int main(int argc, char **argv)
{
	// Declare variables
	bool eof;
	char byte;
	unsigned int mode, rd, maxJoints = UINT_MAX, numTextures = 0, strInd;
	string fn, on, afn, vtn, jtn, inPre, inExt, outPre, outExt, animPre, animExt, dat = "", mtlDat = "";
	vector<string> outputFilenames, outputData, textureNames, effects;
	vector<Texture> textures, effectTextures;
	ifstream in;
	ofstream out;
	AddInfo additionalArgs;
	Arguments args;

	cout << prog << " v " << version << endl << "by " << tool_author << endl << endl;

	// Parse the arguments
	args = parseArgs(argc, argv);
	// Extract the mode from the parsed args
	mode = args.mode;

	// Make sure the arguments were valid
	if (mode != ERROR_MODE)
	{
		// Extract the remaining arguments
		fn = args.inputFilename;
		on = args.outputFilename;
		rd = args.rootDepth;
		afn = args.animationsFilename;
		jtn = args.jointTemplateModelName;
		vtn = args.vertexTemplateModelName;
		textureNames = args.textureNames;
		additionalArgs.additionalBools.push_back(args.normalize);

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
			cout << "Reading from " << fn << endl;
			// Open the input file
			in.open(fn, ifstream::binary);
			// Make sure it opened successfully
			if (in.is_open())
			{
				// Read each byte from the file
				do
				{
					in.read(&byte, 1);
					eof = !in.eof();
					if (eof)
					{
						// Store the read bytes in the string
						dat += byte;
					}
				} while (eof);
				// Close the input file
				in.close();
				if (fn != "")
				{
					cout << "Determining input file type" << endl;
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
				}
				// Check if a additional args are required by the output format
				if (on != "")
				{
					cout << "Determining output file type" << endl;
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
							string animDat = "";
							// Read from the animations file
							cout << "Reading from " << afn << endl;
							// Open the input file
							in.open(afn, ifstream::binary);
							// Make sure it opened successfully
							if (in.is_open())
							{
								// Read each byte from the file
								do
								{
									in.read(&byte, 1);
									eof = !in.eof();
									if (eof)
									{
										// Store the read bytes in the string
										animDat += byte;
									}
								} while (eof);
								// Close the input file
								in.close();
							}
							// Add the animation data so it can be used by the convertMesh function to add animations to the DAE file
							additionalArgs.additionalStrs.push_back(animDat);
						}
					}
					else if (outExt == ".mdls")
					{
						// Check if any template files were provided
						if (jtn != "")
						{
							cout << "Reading from " << jtn << endl;
							in.open(jtn, ifstream::binary);
							if (in.is_open())
							{
								additionalArgs.additionalStrs.push_back("");
								do
								{
									in.read(&byte, 1);
									eof = !in.eof();
									if (eof)
									{
										additionalArgs.additionalStrs.back().append(1, byte);
									}
								} while (eof);
								in.close();
							}
							else
							{
								cout << "Error: Failed to open " << jtn << " for reading" << endl;
							}
						}
						if (vtn != "")
						{
							if (jtn == "")
							{
								additionalArgs.additionalStrs.push_back("");
							}
							cout << "Reading from " << vtn << endl;
							in.open(vtn, ifstream::binary);
							if (in.is_open())
							{
								additionalArgs.additionalStrs.push_back("");
								do
								{
									in.read(&byte, 1);
									eof = !in.eof();
									if (eof)
									{
										additionalArgs.additionalStrs.back().append(1, byte);
									}
								} while (eof);
								in.close();
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
							cout << "Reading from " << jtn << endl;
							in.open(jtn, ifstream::binary);
							if (in.is_open())
							{
								additionalArgs.additionalStrs.push_back("");
								do
								{
									in.read(&byte, 1);
									eof = !in.eof();
									if (eof)
									{
										additionalArgs.additionalStrs.back().append(1, byte);
									}
								} while (eof);
								in.close();
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
					outputData[0] = convertMesh(dat, inExt, outExt, additionalArgs);
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
								cout << "Reading from " << tfn << endl;
								Texture t;
								if (readTexture(tfn, t))
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
							cout << "Error: You should not be able to get here" << endl;
						}
					}
					else
					{
						cout << "Error: " << inExt << " is not a supported file type for importing textures" << endl;
					}
				}
				else if (mode == EXTRACT_TEXTURE_MODE)
				{
					// Make sure the input file is an mdls file (Currently cannot extract from other file types, shouldn't be necessary)
					if (inExt == ".mdls" || inExt == ".wpn")
					{
						string textureFilename, iAsStr;
						stringstream ss;
						if (inExt == ".mdls")
						{
							// Get the textures
							textures = getMdlsTextures(dat);
							effectTextures = getMdlsSpecialEffectTextures(dat);
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
							if (!writeTexture(textures[i], textureFilename))
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
							if (!writeTexture(effectTextures[i], textureFilename))
							{
								cout << "Error: Failed to write " << textureFilename << " to the filesystem" << endl;
							}
						}
					}
					else
					{
						cout << "Error: " << inExt << " is not a supported file type for extracting textures" << endl;
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
								cout << "Reading from " << efn << endl;
								in.open(efn, ifstream::binary);
								if (in.is_open())
								{
									char chr;
									while (!in.eof())
									{
										in.read(&chr, 1);
										if (!in.eof())
										{
											effectDat.append(1, chr);
										}
									}
									effects.push_back(effectDat);
									in.close();
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
								cout << "       " << efnExt << " is not a supported file type for textures!" << endl;
							}
						}
						// Send the model with updated textures to be written to the disk
						cout << "Importing effects" << endl;
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
							cout << "Error: You should not be able to get here" << endl;
						}
					}
					else
					{
						cout << "Error: " << inExt << " is not a supported file type for importing textures" << endl;
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
							cout << "Error: You should not be able to get here" << endl;
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
						cout << "Error: " << inExt << " is not a supported file type for extracting textures" << endl;
					}
				}
				else
				{
					cout << "Warning: How did you get here?" << endl;
				}
				// Make sure the conversion was successful
				for (unsigned int i = 0; i < outputData.size() && i < outputFilenames.size(); i++)
				{
					if (outputData[i].size() > 0)
					{
						// Open the output file
						cout << "Writing to " << outputFilenames[i] << endl;
						out.open(outputFilenames[i], ofstream::binary);
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
		cout << "    --mesh input output             (-m): Run in Model Converter mode" << endl;
		cout << "    --texture input output texture+ (-t): Run in Import Texture mode" << endl;
		cout << "    --extractTexture input          (-x): Run in Extract Texture mode" << endl;
		cout << "    --effect input output effect+   (-e): Run in Import Effect mode" << endl;
		cout << "    --extractEffect input           (-w): Run in Extract Effect mode" << endl;
		cout << endl;
		cout << "Optional Arguments:" << endl;
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
		cout << "Positional Arguments:" << endl;
		cout << "    input                               : The model file to convert from" << endl;
		cout << "    output                              : The name of the output file, the" << endl;
		cout << "                                          format depends on the extension" << endl;
		cout << "    texture                             : The name of the texture image to" << endl;
		cout << "                                          use when importing a texture" << endl;
		cout << "    effect                              : The name of the effect file to" << endl;
		cout << "                                          use when importing an effect" << endl;
		cout << "Supported Filetypes:" << endl;
		cout << "    COLLADA    (.dae)" << endl;
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

Arguments parseArgs(int argc, char **argv)
{
	Arguments args;
	string arg;
	unsigned int positionalArgCounter = 0, stringToUINT, minPositionalArgs = 0, maxPositionalArgs = 0;
	stringstream ss;
	// Initialize the argument values
	args.normalize = false;
	args.mode = DEFAULT_MODE;
	args.rootDepth = 0;
	args.inputFilename = "";
	args.outputFilename = "";
	args.animationsFilename = "";
	args.vertexTemplateModelName = "";
	args.jointTemplateModelName = "";
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
			default:
				switch (args.mode)
				{
				case IMPORT_EFFECT_MODE:
				case IMPORT_TEXTURE_MODE:
					args.textureNames.push_back(arg);
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
