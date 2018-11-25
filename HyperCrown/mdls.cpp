#include"mdls.h"

bool assimpTest()
{
	bool valid = true;
	Assimp::Importer importer;
	string filename = "SoraProg.dae", dat = "";
	const aiScene *scene;
	ifstream in;
	unsigned int fileLen;
	in.open(filename, ifstream::binary);
	in.seekg(0, in.end);
	fileLen = (unsigned int)in.tellg();
	in.seekg(0, in.beg);
	char *w = new char[fileLen];
	in.read(w, fileLen);
	dat = string(w, fileLen);
	delete[]w;
	//scene = importer.ReadFile(filename, 0);
	scene = importer.ReadFileFromMemory(dat.c_str(), dat.length(), 0);
	if (!scene)
	{
		cout << importer.GetErrorString() << endl;
		valid = false;
	}
	else
	{
		;
	}
	return valid;
}

/* Calls the appropriate convert functions based on the input and output formats */
string convertMesh(string raw, string inputFormat, string outputFormat, AddInfo optionalVals)
{
	// Declare variables
	bool valid, normalize;
	float adjusts[numDims], max;
	string converted;
	vector<JointRelative> joints;
	vector<VertexRelative> vertsR;
	vector<VertexGlobal> vertsG;
	vector<Face> faces;
	vector<Animation> anims;
	// Declare variables for using assimp importers
	Assimp::Importer importer;
	const aiScene *scene;
	unsigned int assimpImporterFlags = aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_RemoveComponent | aiProcess_GenNormals | aiProcess_SortByPType | aiProcess_GenUVCoords,
		addStrIndex = 0, addIntIndex = 0, addFloatIndex = 0, addBoolIndex = 0;
	// Check if there is a value for the normalize flag provided
	if (optionalVals.additionalBools.size() > addBoolIndex)
	{
		// Store the provided value
		normalize = optionalVals.additionalBools[addBoolIndex++];
	}
	else
	{
		// Default to not normalizing
		normalize = false;
	}
	// Check the input format
	if (inputFormat == ".dae")
	{
		unsigned int jointStartDepth = 0;
		if (optionalVals.additionalInts.size() > addIntIndex)
		{
			jointStartDepth = optionalVals.additionalInts[addIntIndex++];
		}
		// Specify components to remove when importing
		importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, PP_RVC_FLAGS);
		// Import the model using the assimp importer
		scene = importer.ReadFileFromMemory(raw.c_str(), raw.size(), assimpImporterFlags);
		valid = scene;
		if (valid)
		{
			// Convert the scene to the joints, vertices, and faces vectors
			// Get the joints (if the scene contains a skeleton)
			if (getDaeJoints(scene, joints, jointStartDepth))
			{
				// Get the vertices (if the scene contains (a) mesh(es))
				valid = getDaeVerticesAndFaces(scene, vertsG, faces, joints);
			}
			else
			{
				// Failed to get joints, but we will need it to get the relative vertices, so assume a joint at (0, 0, 0)
				joints.clear();
				joints.push_back(JointRelative());
				joints.back().ind = 0;
				joints.back().name = "Joint0";
				joints.back().special = 0;
				joints.back().jointInfo.childIndex = 0;
				joints.back().jointInfo.parentIndex = 1023;
				joints.back().jointInfo.unknownFlag = true;
				joints.back().jointInfo.unknownIndex = 0;
				for (unsigned int i = 0; i < numDims; i++)
				{
					joints.back().coordinates[i] = 0;
					joints.back().rotations[i] = 0;
					joints.back().scaleFactors[i] = 1;
				}
				// Get the vertex data
				valid = getDaeVerticesAndFaces(scene, vertsG, faces);
			}
			// Calculate the relative coordinates of the vertices
			if (valid)
			{
				valid = getVerticesRelative(joints, vertsG, vertsR);
				if (!valid)
				{
					cout << "Error: Failed convert the vertices to the joints' frames of reference" << endl;
				}
			}
			else
			{
				cout << "Error: Failed to extract vertex data" << endl;
			}
		}
		else
		{
			cout << "Error: Failed to import model" << endl << importer.GetErrorString() << endl;
		}
	}
	else if (inputFormat == ".obj")
	{
		// To-Do: Get the joint positions (obj alone doesn't contain joint info)
		joints;
		// Get the vertex positions and face configuration
		vertsG;
		faces;
		// Calculate the 
		vertsR;
		valid = false;
		if (!valid)
		{
			cout << "Error: Converting OBJ files to another format is not currently supported" << endl;
		}
	}
	else if(inputFormat == ".mdls")
	{
		// Get the joint positions
		joints = getMdlsJoints(raw);
		// Get the vertex positions and face configuration
		valid = getMdlsVerticesAndFaces(raw, joints, vertsR, faces);
		if (valid)
		{
			// Get the global vertices
			valid = getVerticesGlobal(joints, vertsR, vertsG);
		}
		// If we failed to get the verts and faces, print an error message
		if (!valid)
		{
			cout << "Error: Failed to parse MDLS file" << endl;
		}
	}
	else if (inputFormat == ".wpn")
	{
		// Get the joint positions
		//joints = getMdlsJoints(raw);
		// Construct joints
		joints.reserve(3);
		for (unsigned int i = 0; i < 3; i++)
		{
			JointRelative ji;
			ji.ind = i;
			ji.name = "";
			ji.jointInfo.unknownFlag = 1;
			if (i > 0)
			{
				ji.jointInfo.parentIndex = i - 1;
				if (joints[i - 1].childrenIndices.size() == 0)
				{
					joints[i - 1].jointInfo.childIndex = i;
				}
				joints[i - 1].childrenIndices.push_back(i);
			}
			else
			{
				ji.jointInfo.parentIndex = absoluteMaxJoints;
			}
			ji.jointInfo.unknownIndex = absoluteMaxJoints;
			ji.jointInfo.childIndex = absoluteMaxJoints;
			for (unsigned int j = 0; j < numDims; j++)
			{
				ji.scaleFactors[j] = 1;
				ji.rotations[j] = 0;
				ji.coordinates[j] = 0;
			}
			ji.special = 0;
			joints.push_back(ji);
		}
		// Get the vertex positions and face configuration
		valid = getWpnVerticesAndFaces(raw, vertsR, faces);
		if (valid)
		{
			// Get the global vertices
			valid = getVerticesGlobal(joints, vertsR, vertsG);
		}
		// If we failed to get the verts and faces, print an error message
		if (!valid)
		{
			cout << "Error: Failed to parse WPN file" << endl;
		}
	}
	// To-Do: Support other input formats
	else
	{
		valid = false;
		cout << "Error: Input is in an unsupported format (" + inputFormat + ")" << endl;
	}
	// Make sure the input is valid
	if (valid)
	{
		// Check if wee need to normalize the model
		if (normalize)
		{
			// Limit the coordinates to the range [-1, 1]
			valid = normalizePosition(vertsG, adjusts, max);
			if (!valid)
			{
				cout << "Error: Failed to normalize model" << endl;
			}
		}
		else
		{
			// Set the adjust and max values to identity
			for (unsigned int i = 0; i < numDims; i++)
			{
				adjusts[i] = 0;
			}
			max = 1;
		}
		// Check which outputFormat we are outputing
		if (outputFormat == ".obj")
		{
			string mtlFilename = "";
			if (optionalVals.additionalStrs.size() > addStrIndex)
			{
				mtlFilename = optionalVals.additionalStrs[addStrIndex++];
			}
			// Convert to obj format
			converted = convertMeshToObj(vertsG, faces, adjusts, max, mtlFilename);
		}
		else if (outputFormat == ".dae")
		{
			string texturePrefix = "", animDat;
			if (optionalVals.additionalStrs.size() > addStrIndex)
			{
				texturePrefix = optionalVals.additionalStrs[addStrIndex++];
			}
			if (optionalVals.additionalStrs.size() > addStrIndex)
			{
				vector<MsetStaticAnimation> mAnims;
				animDat = optionalVals.additionalStrs[addStrIndex++];
				mAnims = getMsetAnimations(animDat);
				anims = convertMsetStaticAnimationsToAnimations(joints, mAnims);
			}
			// Convert to dae (Collada) format
			if (texturePrefix != "")
			{
				converted = convertMeshToDae(joints, vertsG, faces, anims, adjusts, max, texturePrefix);
			}
			else
			{
				converted = convertMeshToDae(joints, vertsG, faces, anims, adjusts, max);
			}
		}
		else if (outputFormat == ".mdls")
		{
			string exampleVertices = "", exampleRig = "";
			if (optionalVals.additionalStrs.size() > addStrIndex)
			{
				exampleRig = optionalVals.additionalStrs[addStrIndex++];
			}
			if (optionalVals.additionalStrs.size() > addStrIndex)
			{
				exampleVertices = optionalVals.additionalStrs[addStrIndex++];
			}
			if (exampleRig != "")
			{
				converted = convertMeshToMdls(joints, vertsR, faces, exampleRig, exampleVertices, adjusts, max);
			}
			else
			{
				converted = convertMeshToMdls(joints, vertsR, faces, adjusts, max);
			}
		}
		else if (outputFormat == ".wpn")
		{
			string exampleVertices;
			if (optionalVals.additionalStrs.size() > addStrIndex)
			{
				exampleVertices = optionalVals.additionalStrs[addStrIndex++];
			}
			if (exampleVertices != "")
			{
				converted = convertMeshToWpn(vertsR, faces, exampleVertices, adjusts, max);
			}
			else
			{
				converted = convertMeshToWpn(vertsR, faces, adjusts, max);
			}
		}
		// To-Do: Support more formats
		else
		{
			cout << "Error: Unable to convert to an unsupported format" << endl;
		}
		
	}
	return converted;
}

/* Converts the skeleton from the mdls outputFormat to a display file for my viewer */
string convertSkeleton(string raw, string format, unsigned int maxJoints)
{
	// Declare variables
	string converted = "", val;
	vector<JointRelative> joints;
	vector<VertexRelative> vertsR;
	vector<VertexGlobal> vertsG;
	unsigned int parentInd;
	float adjusts[numDims], max;
	stringstream ss;
	// Get the joints from the file
	joints = getMdlsJoints(raw, maxJoints);
	// Create vertices at each joint to create a model of the skeleton
	vertsR = convertJointsToVerts(joints);
	// Convert the relative vertices to global vertices
	if (getVerticesGlobal(joints, vertsR, vertsG))
	{
		// Normalize the vertex positions
		if (normalizePosition(vertsG, adjusts, max))
		{
			// Check which outputFormat we are converting to
			if (format == ".obj" || format == ".OBJ") // Convert to .obj (assumes positive X is the direction the model faces and positive Z is up)
			{
				// Add each a vertex
				for (unsigned int i = 0; i < vertsG.size(); i++)
				{
					// VertexRelative Element symbol
					converted += "v";
					// For each dimension
					for (unsigned int j = 0; j < numDims; j++)
					{
						// Convert the coordinate to a string
						ss.clear();
						ss.str("");
						ss << (vertsG[i].coordinates[j] - adjusts[j]) / max;
						ss >> val;
						// Add the coordinate to the numAsStr
						converted += " " + val;
					}
					// Go to the next numAsStr
					converted += "\n";
				}
				// Add a point at each joint
				converted += "\n";
				// Form each numAsStr from the vertices
				for (unsigned int i = 0; i < joints.size(); i++)
				{
					// Get the parent joint
					parentInd = joints[i].jointInfo.parentIndex;
					// Check if we need to add the numAsStr segment
					if (parentInd != absoluteMaxJoints)
					{
						// Convert the vertex index of the numAsStr to a string
						ss.clear();
						ss.str("");
						ss << (i + 1); // obj outputFormat indices start at 1
						ss >> val;
						// Add the Line element symbol and the first vertex index
						converted += "l " + val + " ";
						// Convert the parent vertex index of the numAsStr to a string
						ss.clear();
						ss.str("");
						ss << (parentInd + 1); // obj outputFormat indices start at 1
						ss >> val;
						// Add the second vertex index and then go to the next numAsStr
						converted += val + "\n";
					}
				}
			}
			// To-Do: Support more formats
			else
			{
				cout << "Error: Failed to convert to unsupported format" << endl;
			}
		}
		else
		{
			cout << "Error: Failed to normalize the skeleton" << endl;
		}
	}
	else
	{
		cout << "Error: Failed to get the global vertex positions" << endl;
	}
	// Return the converted file
	return converted;
}

string convertMeshToObj(vector<VertexGlobal> verts, vector<Face> faces, float adjustments[], float max, string mtlName)
{
	string converted = "", numAsStr;
	stringstream ss;
	unsigned int textureInd = UINT_MAX;
	const unsigned int dimOrder[numDims] = { 0, 2, 1 };
	const int dimScalars[numDims] = { 1, -1, 1 };
	// Check if there is an mtl filename to use
	if (mtlName != "")
	{
		// Start with a mtllib command to link textures
		converted += "mtllib " + mtlName + "\n\n";
	}
	// Copy each vertex into the file
	for (unsigned int i = 0; i < verts.size(); i++)
	{
		// VertexRelative Identifier
		converted += "v";
		for (unsigned int j = 0; j < numDims; j++)
		{
			//  Convert each coordinate from float to string
			ss.clear();
			ss.str("");
			// Use dim order to swap y and z since most obj imports use y as the vertical axis
			ss << dimScalars[dimOrder[j]] * (verts[i].coordinates[dimOrder[j]] - adjustments[dimOrder[j]]) / max;
			ss >> numAsStr;
			// Append the coordinate to the numAsStr
			converted += " " + numAsStr;
		}
		// Go to the next numAsStr
		converted += "\n";
		// VertexRelative Texture Identifier
		converted += "vt";
		for (unsigned int j = 0; j < 2; j++)
		{
			// Convert the U and V values from float to string
			ss.clear();
			ss.str("");
			ss << verts[i].textureMap[j];
			ss >> numAsStr;
			// Append the texture map to the numAsStr
			converted += " " + numAsStr;
		}
		// Go to the next numAsStr
		converted += "\n";
	}
	// Add another blank line to separate the vertices and faces for read-ability
	converted += "\n";
	// Add each face into the file
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		// Check if the texture index changed
		if (textureInd != verts[faces[i].vertexIndices[0]].textureIndex)
		{
			// Update the texture index
			textureInd = verts[faces[i].vertexIndices[0]].textureIndex;
			// Change the material being used
			ss.clear();
			ss.str("");
			ss << textureInd;
			ss >> numAsStr;
			converted += "usemtl mat" + numAsStr + "\n";
		}
		// Face Identifier
		converted += "f";
		for (unsigned int j = 0; j < numVertsPerFace; j++)
		{
			// Convert each index from unsigned int to string
			ss.clear();
			ss.str("");
			ss << (faces[i].vertexIndices[j] + 1); // .obj indices begin at 1, not 0
			ss >> numAsStr;
			// Append the index to the numAsStr
			converted += " " + numAsStr + "/" + numAsStr;
		}
		// Go to the next line
		converted += "\n";
	}
	return converted;
}

string convertMeshToMdls(vector<JointRelative> joints, vector<VertexRelative> verts, vector<Face> faces, float adjustments[], float max) 
{
	// Initialize mdls data to empty string
	string converted = "";
	// Add the number of sections (8) in an mdls
	converted.append("\x08\x00\x00\x00", 4);
	// Add the offset to the MOBJ section
	converted.append("\x80\x00\x00\x00", 4);
	// Add the rest of the header and padding (we need to update the other offsets later)
	converted.append(120, 0);
	// Create MOBJ Section
	converted.append("MOBJ");
	// Pad the offsets and length fields for now
	converted.append(60, 0);
	// Fill in the 3D model data section offset
	*(unsigned int *)&(converted[160]) = 0x40;
	// Create 3D model data section
	converted.append(16, 0);
	// Fill the number of bones
	*(unsigned int *)&(converted[192]) = joints.size();
	// To-Do: Create the polygon collection table (Need to determine the number of polygon collections)
	// Padding before foot placement section
	converted.append(32, 0);
	// Foot placement section
	converted.append(16, 0); // Just use 0x00 * 16 for now since we haven't cracked these values
	// To-Do: Create the unknown entries section

	return converted;
}

string convertMeshToMdls(vector<JointRelative> joints, vector<VertexRelative> verts, vector<Face> faces, string templateRigData, string templateVertexData, float adjustments[], float max)
{
	vector<vector<vector<vector<unsigned int>>>> polygonCollections;
	// The copy variables determine if we copy their sections from the templates (true), or if the tool generates those sections (false)
	bool updateRig, copyVertexData = false, copyJointData = false;
	string converted = "", mobj, mobjV, newMobj;
	unsigned int templateRigLen = templateRigData.size(), templateVertexLen = templateVertexData.size(), numSections, mobjOff, mobjVOff, mobjEnd, mobjVEnd, mobjLen, mobjVLen, subsectionOff, subsectionLen, numJoints, numTemplateJoints, numPolygonCollections, numVerts, textureInd, dimOff, jointInfo, origins[numOrigins], reset[numOrigins], remainder;
	// Flags for excluding and/or swapping sections of the mobj and mdls (0 = copy from joint template, 1 = exclude section/copy from vertex template, swapping only occurs if updateRig is true)
	const unsigned char excludeMdls = 0x00, excludeMobj = 0x02, swapMdls = 0x01, swapMobj = 0x1F;
	// Determine if we are updating the rigging
	updateRig = templateVertexLen > 0;
	// Make sure the template is long enough to have the MDLS header
	if (templateRigLen >= 128 && (!updateRig || templateVertexLen > 128))
	{
		// Validate the rig template if we are updating the rig
		if (updateRig)
		{
			// Get the number of section in the joint template
			numSections = *(unsigned int *)&(templateVertexData[0]);
			if (numSections == 8)
			{
				// If we are updating the rig, then we must also copy the vertex data
				copyVertexData = true;
			}
			else
			{
				// This is an invalid joint template, clear the update rig flag so we don't use it
				updateRig = false;
			}
		}
		// Get the number of sections in the vertex template
		numSections = *(unsigned int *)&(templateRigData[0]);
		// An MDLS starts with 8 offsets, so verify it has the correct number
		if (numSections == 8)
		{
			// Create the MDLS Header (we will need to update the offsets later)
			converted.append((char *)&(numSections), sizeof(unsigned int));	// Number of Sections
			converted.append(124, 0);										// Offsets and Padding
			// Update the offset to the first section
			*(unsigned int *)&(converted[4]) = converted.size();
			// Check if we are updating the rig
			if (updateRig)
			{
				// Get the offset and length of the MOBJ subsection
				mobjVOff = *(unsigned int *)&(templateVertexData[4]);
				mobjVEnd = *(unsigned int *)&(templateVertexData[8]);
				// Get the length of the subsection
				mobjVLen = mobjVEnd - mobjVOff;
				// Copy the MOBJ
				mobjV = templateVertexData.substr(mobjVOff, mobjVLen);
			}
			// Get the offset and end of the MOBJ subsection
			mobjOff = *(unsigned int *)&(templateRigData[4]);
			mobjEnd = *(unsigned int *)&(templateRigData[8]);
			// Get the length of the MOBJ subsection
			mobjLen = mobjEnd - mobjOff;
			// Separate the MOBJ section from the rest of the MDLS
			mobj = templateRigData.substr(mobjOff, mobjLen);
			// Make sure the mobj has a header
			if ((mobjLen >= mobjHeaderLen) && (!updateRig || (mobjVLen >= mobjHeaderLen)))
			{
				// Copy the mobj header to the new mobj
				//newMobj = mobj.substr(0, mobjHeaderLen);
				newMobj = "MOBJ";
				newMobj.append(mobjHeaderLen - 4, 0); // Fill in the lengths and offsets later
				// Fill in the subsection 0 offset
				*(unsigned int *)&(newMobj[32]) = mobjHeaderLen;
				// Check if we are copying the vertex data from a template
				if (!copyVertexData)
				{
					// Sort the faces to optimize polygon collections
					faces = sortFaces(faces, verts);
					// Convert the faces into polygon collections
					if (getPolygonCollections(verts, faces, polygonCollections))
					{
						// Save the number of polygon collections
						numPolygonCollections = polygonCollections.size();
					}
					else
					{
						// There are no polygon collections
						numPolygonCollections = 0;
					}
				}
				else
				{
					// Get the number of polygon collections from the appropriate template
					if (updateRig && ((swapMobj & 1) != 0))
					{
						if (mobjVLen >= mobjHeaderLen + 16)
						{
							numPolygonCollections = *(unsigned int *)&(mobjV[mobjHeaderLen + 12]);
						}
						else
						{
							numPolygonCollections = 0;
						}
					}
					else
					{
						if (mobjLen >= mobjHeaderLen + 16)
						{
							numPolygonCollections = *(unsigned int *)&(mobj[mobjHeaderLen + 12]);
						}
						else
						{
							numPolygonCollections = 0;
						}
					}
				}

				if (!copyJointData)
				{
					numJoints = joints.size();
				}
				else
				{
					if (mobjLen >= (mobjHeaderLen + 4))
					{
						numJoints = *(unsigned int *)&(mobj[mobjHeaderLen]);
					}
					else
					{
						numJoints = 0;
					}
				}


				// Create model data section header
				newMobj.append((char *)&numJoints, sizeof(unsigned int));
				newMobj.append(8, 0); // Need to fill in these offset values later
				newMobj.append((char *)&numPolygonCollections, sizeof(unsigned int));

				if (!copyVertexData)
				{
					// Create the polygon collection table
					for (unsigned int i = 0; i < numPolygonCollections; i++)
					{
						if (polygonCollections[i].size() > 0 && polygonCollections[i][0].size() > 0 && polygonCollections[i][0][0].size() > 0)
						{
							textureInd = verts[polygonCollections[i][0][0][0]].textureIndex;
						}
						else
						{
							textureInd = 0;
						}
						newMobj.append(4, 0);
						newMobj.append((char *)&textureInd, sizeof(unsigned int));
						newMobj.append((char *)&textureInd, sizeof(unsigned int));
						newMobj.append(4, 0); // Need to fill in this offset later
					}
					// Append 32 bytes padding
					newMobj.append(32, 0);
				}
				else
				{
					// Copy the polygon table from the template
					subsectionOff = mobjHeaderLen + 16;
					subsectionLen = numPolygonCollections * 16 + 32;
					if (updateRig && ((swapMobj & 1) != 0))
					{
						newMobj.append(mobjV.substr(subsectionOff, subsectionLen));
					}
					else
					{
						newMobj.append(mobj.substr(subsectionOff, subsectionLen));
					}
				}

				// Update the subsubsection 1 offset
				*(unsigned int *)&(newMobj[72]) = newMobj.size() - mobjHeaderLen;
				// Copy the subsubsection 1 from the template
				subsectionLen = 240;
				if (updateRig && ((swapMobj & 1) != 0))
				{
					subsectionOff = *(unsigned int *)&(mobjV[72]) + mobjHeaderLen;
					if (mobjVLen >= (subsectionOff + subsectionLen))
					{
						newMobj += mobjV.substr(subsectionOff, subsectionLen);
					}
					else
					{
						newMobj.append(subsectionLen, 0);
					}
				}
				else
				{
					subsectionOff = *(unsigned int *)&(mobj[72]) + mobjHeaderLen;
					if (mobjLen >= (subsectionOff + subsectionLen))
					{
						newMobj += mobj.substr(subsectionOff, subsectionLen);
					}
					else
					{
						newMobj.append(subsectionLen, 0);
					}
				}
				// Update joint data section offset
				*(unsigned int *)&(newMobj[68]) = (numPolygonCollections + 18) * 16;

				// Create the joint data section from joints vector
				if (!copyJointData)
				{
					if (updateRig && copyVertexData)
					{
						// Get the number of joints in the template file
						numTemplateJoints = *(unsigned int *)&(mobjV[mobjHeaderLen]);
						if (numTemplateJoints != numJoints)
						{
							// Fix the discrepancy in the polygon collection table
							for (unsigned int i = 0; i < numPolygonCollections; i++)
							{
								*(unsigned int *)&(newMobj[mobjHeaderLen + (i * 16) + 28]) += ((int)numJoints - (int)numTemplateJoints) * 48;
							}
						}
					}

					// For each joint...
					for (unsigned int i = 0; i < numJoints; i++)
					{
						// Add joint i's scale factors
						for (unsigned int j = 0; j < numDims; j++)
						{
							dimOff = (j + 1) % numDims;
							newMobj.append((char *)&(joints[i].scaleFactors[dimOff]), 4);
						}
						// Add the joints index
						newMobj.append((char *)&(joints[i].ind), 4);
						// Add the joint rotations
						for (unsigned int j = 0; j < numDims; j++)
						{
							dimOff = (j + 1) % numDims;
							newMobj.append((char *)&(joints[i].rotations[dimOff]), 4);
						}
						// Add the special value
						newMobj.append((char *)&(joints[i].special), 4);
						// Add the joint position
						for (unsigned int j = 0; j < numDims; j++)
						{
							dimOff = (j + 1) % numDims;
							newMobj.append((char *)&(joints[i].coordinates[dimOff]), 4);
						}
						jointInfo = (joints[i].jointInfo.unknownFlag << 30) | (joints[i].jointInfo.childIndex << 20) | (joints[i].jointInfo.unknownIndex << 10) | joints[i].jointInfo.parentIndex;
						// Add the joint info
						newMobj.append((char *)&(jointInfo), 4);
					}
				}
				else
				{
					// Copy the joint data from the template
					if (mobjLen >= mobjHeaderLen + 8)
					{
						subsectionOff = mobjHeaderLen + *(unsigned int *)&(mobj[mobjHeaderLen + 4]);
						subsectionLen = *(unsigned int *)&(mobj[mobjHeaderLen]) * 48;
						if (mobjLen >= (subsectionOff + subsectionLen))
						{
							newMobj.append(mobj.substr(subsectionOff, subsectionLen));
						}
						else
						{
							newMobj.append(subsectionLen, 0);
						}
					}
				}

				// Check if we are excluding the model data from the mobj
				if ((excludeMobj & 0x01) == 0)
				{
					if (!copyVertexData)
					{
						// Handle Polygon Collections and Joint References
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{

							// Update offset in the polygon collection table
							subsectionOff = newMobj.size() - mobjHeaderLen;
							*(unsigned int *)&(newMobj[mobjHeaderLen + i * 16 + 28]) = subsectionOff;

							// Set origin joint indices to uninitialized and clear the reset
							for (unsigned int j = 0; j < numOrigins; j++)
							{
								origins[j] = absoluteMaxJoints;
								reset[j] = absoluteMaxJoints;
							}
							bool initialized = false, reinitialize = false;
							// For each polygon sub-collection in the polygon collection
							for (unsigned int j = 0; j < polygonCollections[i].size(); j++)
							{

								// Store the current size of the mobj for later so we can calculate the length of the polygon collection
								subsectionOff = newMobj.size();
								// Padding
								newMobj.append(8, 0); // Fill in the length field later
								// Unknown Stuff
								newMobj.append("\x01\x01\x00\x01\x00\x80\x00\x6C", 8);	// Fill in ?2 field later (length - 1)

								// For each triangle list in the polygon sub-collection
								for (unsigned int k = 0; k < polygonCollections[i][j].size(); k++)
								{
									// For each vertex in the triangle list
									for (unsigned int l = 0; initialized && l < polygonCollections[i][j][k].size(); l++)
									{
										// Check if we need to reinitialize the origins
										for (unsigned int m = 0; initialized && m < numOrigins; m++)
										{
											// Check if this vertex's origin joint matches any of the stored origin joints
											if (verts[polygonCollections[i][j][k][l]].originJointIndex == origins[m])
											{
												// The origin joint is valid, we can continue without reinitializing
												break;
											}
											// If we didn't find it
											else if (m == (numOrigins - 1))
											{
												// We need to reinitialize
												initialized = false;
											}
										}
									}
									// Check if the origins have been initialized with real indices
									if (!initialized)
									{
										// Store the previous values of the origins array in the reset array and clear the origins array
										for (unsigned int l = 0; l < numOrigins; l++)
										{
											reset[l] = origins[l];
											origins[l] = absoluteMaxJoints;
										}
										// Start from the current triangle list
										for (unsigned int k2 = k; !initialized && k2 < polygonCollections[i][j].size(); k2++)
										{
											// For each vertex in the triangle list
											for (unsigned int l = 0; !initialized && l < polygonCollections[i][j][k2].size(); l++)
											{
												// For each origin we can reference simultaneously
												for (unsigned int m = 0; m < numOrigins; m++)
												{
													// Check if this vertex has a new joint we need to reference
													if (verts[polygonCollections[i][j][k2][l]].originJointIndex != origins[m] && origins[m] == absoluteMaxJoints)
													{
														// Store the new index
														origins[m] = verts[polygonCollections[i][j][k2][l]].originJointIndex;
														// Check if we just filled the origins array
														if (m == (numOrigins - 1))
														{
															// We are done initializing
															initialized = true;
														}
														// Break from the loop so we don't duplicate the entry
														break;
													}
													else if (verts[polygonCollections[i][j][k2][l]].originJointIndex == origins[m])
													{
														// This is a repeat, just break from the loop to try the next vertex
														break;
													}
													// Check if this vertex matches the joint from the previous initialization
													else //if (verts[polygonCollections[i][j][k2][l]].originJointIndex == reset[m] && origins[m] == absoluteMaxJoints)
													{
														// Check if this vertex matches the joint from a previous initialization
														bool repeat = false;
														if (origins[m] == absoluteMaxJoints)
														{
															for (unsigned int n = 0; !repeat && (n < numOrigins); n++)
															{
																if (verts[polygonCollections[i][j][k2][l]].originJointIndex == reset[n])
																{
																	// Store the previous index
																	origins[m] = reset[n];
																	repeat = true;
																}
															}
														}
														// Store the previous index
														//origins[m] = reset[m];
														if (repeat)
														{
															// Check if we just filled the origins array
															if (m == (numOrigins - 1))
															{
																// We are done initializing
																initialized = true;
															}
															// Break from the loop so we don't duplicate the entry
															break;
														}
													}
												}
											}
										}
										initialized = true;
										reinitialize = true;
									}
									// At this point we should have the correct origins
									if (reinitialize)
									{
										// For each origin
										for (unsigned int l = 0; l < numOrigins; l++)
										{
											bool writeOrigin = origins[l] != absoluteMaxJoints;
											for (unsigned int m = 0; writeOrigin && (m < numOrigins); m++)
											{
												writeOrigin = writeOrigin && (origins[l] != reset[m]);
											}
											// Check if we need to update the origin by comparing with reset
											//if (writeOrigin)
											if (origins[l] != absoluteMaxJoints && origins[l] != reset[l])
											{
												// Write the origin update subsection
												newMobj.append(4, 0);						// Origin Update identifier
												newMobj.append((char *)&(origins[l]), 4);	// Origin Joint Index
												newMobj.append((char *)&(l), 4);			// Origin Index
												newMobj.append(116, 0);						// Padding (used for computation)
											}
										}

										// Clear the reinitialize flag
										reinitialize = false;
									}

									// Write triangle list header
									numVerts = polygonCollections[i][j][k].size();
									// Calculate the length of the triangle list section
									subsectionLen = numVerts * 48 + 32;
									// Triangle list identifier
									newMobj.append("\x01\x00\x00\x00", 4);
									// Length of triangle list
									newMobj.append((char *)&(subsectionLen), sizeof(unsigned int));
									// Number of vertices in the triangle list
									newMobj.append((char *)&(numVerts), sizeof(unsigned int));
									// Initial culling direction flag (0 - Anticlockwise, 1 - Clockwise), We assume Anticlockwise 
									newMobj.append(4, 0);
									// Copy of the number of vertices
									newMobj.append((char *)&(numVerts), sizeof(unsigned int));
									// Unknown stuff
									newMobj.append("\x00\x40\x3E\x30\x12\x04\x00\x00", 8);
									// Padding?
									newMobj.append(4, 0);

									// For each vertex in the triangle list
									for (unsigned int l = 0; l < polygonCollections[i][j][k].size(); l++)
									{
										VertexRelative v = verts[polygonCollections[i][j][k][l]];
										// Write the normals
										for (unsigned int m = 0; m < numDims; m++)
										{
											dimOff = (m + 1) % numDims;
											newMobj.append((char *)&(v.normal[dimOff]), sizeof(float));
										}
										// Find and write the origin index
										for (unsigned int m = 0; m < numOrigins; m++)
										{
											// Find the corresponding origin
											if (origins[m] == v.originJointIndex)
											{
												// Write the origin index
												newMobj.append((char *)&(m), sizeof(unsigned int));
												break;
											}
											else if (m == (numOrigins - 1))
											{
												// Fail safe
												newMobj.append(4, 0);
											}
										}
										// For each dimension
										for (unsigned int m = 0; m < numDims; m++)
										{
											dimOff = (m + 1) % numDims;
											// Write the vertex's coordinates
											newMobj.append((char *)&(v.coordinates[dimOff]), sizeof(float));
										}
										// Write the vertex's coordinate scale factor
										newMobj.append((char *)&(v.coordinatesScaleFactor), sizeof(float));
										// For each UV coordinate
										for (unsigned int m = 0; m < 2; m++)
										{
											// Get the UV coordinate
											float texVal = v.textureMap[m];
											// Invert the V coordinate, because mdls format is special
											if (m == 1)
											{
												texVal = 1 - texVal;
											}
											// Write the vertex's UV coordinates
											newMobj.append((char *)&(texVal), sizeof(float));
										}
										// Write the vertex's texture scale factor
										newMobj.append((char *)&(v.textureScaleFactor), sizeof(float));
										// Padding
										newMobj.append(4, 0);
									}
								}

								// Write the polygon (sub) collection footer
								newMobj.append("\x00\x80\x00\x00", 4);
								if (j == (polygonCollections[i].size() - 1))
								{
									// This is the end of the polygon collection
									newMobj.append(1, 0);
								}
								else
								{
									// We still have more to do in this polygon collection
									newMobj.append(1, 48);
								}
								newMobj.append(11, 0);
								newMobj.append("\x00\x00\x00\x17", 4);
								newMobj.append(12, 0);
								// Update the polygon collection's length field
								*(unsigned int *)&(newMobj[subsectionOff]) = ((newMobj.size() - subsectionOff) / 16) - 1;
								newMobj[subsectionOff + 3] = 16;
								// Update ?2 field
								newMobj[subsectionOff + 14] = newMobj[subsectionOff] - 1;

							}

							// To-Do: End of Polygon Collection?

						}
					}
					else
					{
						string templateVertices, vertexData;
						// Copy the vertex data from the template vertices file
						if (updateRig && ((swapMobj & 1) != 0))
						{
							if (mobjVLen >= (mobjHeaderLen + 16))
							{
								subsectionOff = mobjHeaderLen + *(unsigned int *)&(mobjV[mobjHeaderLen + 12]) * 16 + 288 + *(unsigned int *)&(mobjV[mobjHeaderLen]) * 48;
								subsectionLen = *(unsigned int *)&(mobjV[36]) - (*(unsigned int *)&(mobjV[mobjHeaderLen + 12]) * 16 + 288 + *(unsigned int *)&(mobjV[mobjHeaderLen]) * 48);
								if (mobjVLen >= (subsectionOff + subsectionLen))
								{
									templateVertices = mobjV.substr(subsectionOff, subsectionLen);
								}
								else
								{
									templateVertices = "";
									templateVertices.append(subsectionLen, 0);
								}
							}
						}
						else
						{
							if (mobjLen >= (mobjHeaderLen + 16))
							{
								subsectionOff = mobjHeaderLen + *(unsigned int *)&(mobj[mobjHeaderLen + 12]) * 16 + 288 + *(unsigned int *)&(mobj[mobjHeaderLen]) * 48;
								subsectionLen = *(unsigned int *)&(mobj[36]) - (*(unsigned int *)&(mobj[mobjHeaderLen + 12]) * 16 + 288 + *(unsigned int *)&(mobj[mobjHeaderLen]) * 48);
								if (mobjLen >= (subsectionOff + subsectionLen))
								{
									templateVertices = mobjV.substr(subsectionOff, subsectionLen);
								}
								else
								{
									templateVertices = "";
									templateVertices.append(subsectionLen, 0);
								}
							}
						}
						// Check if we are updating the rigging
						if (updateRig)
						{
							// Get the joints from the vertex data
							vector<JointRelative> templateJoints = getMdlsJoints(templateVertexData);
							// Try to update the rig
							if (!updateRigging(joints, templateJoints, verts, faces, templateVertices, vertexData))
							{
								// In case the update fails, just use the template data
								vertexData = templateVertices;
							}
						}
						else
						{
							// We aren't updating the rig, so just use the template data
							vertexData = templateVertices;
						}
						// Add the vertex data to the new mobj section
						newMobj.append(vertexData);
					}
					subsectionLen = newMobj.size() - mobjHeaderLen;
				}
				else
				{
					// Don't do this, this is really dumb
					subsectionLen = 0;
				}

				// Update the model data section length
				*(unsigned int *)&(newMobj[36]) = subsectionLen;
				// To-Do: Subsection 1
				subsectionOff = newMobj.size();
				// Update the Subsection 1 offset
				*(unsigned int *)&(newMobj[40]) = subsectionOff;
				// Check if we are including this section
				if ((excludeMobj & 0x02) == 0)
				{
					// For now just copy Subsection 1 from the template
					// Check which template to copy from
					if ((swapMobj & 0x02) != 0)
					{
						// Get the offset and length of the subsection
						subsectionOff = *(unsigned int *)&(mobjV[40]);
						subsectionLen = *(unsigned int *)&(mobjV[44]);
						// Make sure there is data to copy
						if (subsectionLen > 0 && mobjVLen >= (subsectionOff + subsectionLen))
						{
							newMobj += mobjV.substr(subsectionOff, subsectionLen);
						}
						else
						{
							newMobj.append(subsectionLen, 0);
						}
					}
					else
					{
						// Get the offset and length of the subsection
						subsectionOff = *(unsigned int *)&(mobj[40]);
						subsectionLen = *(unsigned int *)&(mobj[44]);
						// Make sure there is data to copy
						if (subsectionLen > 0 && mobjLen >= (subsectionOff + subsectionLen))
						{
							newMobj += mobj.substr(subsectionOff, subsectionLen);
						}
						else
						{
							newMobj.append(subsectionLen, 0);
						}
					}
				}
				else
				{
					// Were aren't including this section, so its length will be 0
					subsectionLen = 0;
				}
				// Update the Subsection 1 length
				*(unsigned int *)&(newMobj[44]) = subsectionLen;
				// For each remaining subsection (2, 3, and 4)
				for (unsigned int i = 0; i < 3; i++)
				{
					// Calculate the index for the offset
					unsigned int off = (i + 1) * 8;
					// Update the offset to the section
					*(unsigned int *)&(newMobj[off]) = newMobj.size();
					// Check if we are swapping this section
					if (updateRig && (swapMobj & (1 << (i + 2))) != 0)
					{
						if ((excludeMobj & (1 << (i + 2))) == 0)
						{
							// Get the offset and length of the section
							subsectionOff = *(unsigned int *)&(mobjV[off]);
							subsectionLen = *(unsigned int *)&(mobjV[off + 4]);
							// Check if there is data to append
							if (subsectionLen > 0 && mobjVLen >= (subsectionOff + subsectionLen))
							{
								// Append the data
								newMobj += mobjV.substr(subsectionOff, subsectionLen);
							}
							else
							{
								newMobj.append(subsectionLen, 0);
							}
						}
						else
						{
							subsectionLen = 0;
						}
						// Update the length of the section
						*(unsigned int *)&(newMobj[off + 4]) = subsectionLen;
					}
					else
					{
						if ((excludeMobj & (1 << (i + 2))) == 0)
						{
							// Get the offset and length of the section
							subsectionOff = *(unsigned int *)&(mobj[off]);
							subsectionLen = *(unsigned int *)&(mobj[off + 4]);
							// Make sure there is data to append
							if (subsectionLen > 0)
							{
								// Append the data
								newMobj += mobj.substr(subsectionOff, subsectionLen);
							}
						}
						else
						{
							subsectionLen = 0;
						}
						// Update the length of the section
						*(unsigned int *)&(newMobj[off + 4]) = subsectionLen;
					}
					// Check if we need to update the padding between subsections 2 and 3
					if (i == 0)
					{
						// Update the padding offset
						*(unsigned int *)&(newMobj[48]) = newMobj.size();
						// Add the padding if necessary
						remainder = (*(unsigned int *)&(converted[4]) + newMobj.size()) % 128;
						if (remainder > 0)
						{
							newMobj.append(128 - remainder, 0);
						}
					}
				}
				// Update the length of the new mobj
				*(unsigned int *)&(newMobj[4]) = newMobj.size() - 8;
				// Check if we are excluding the mobj section (please don't do this)
				if ((excludeMdls & 0x01) == 0)
				{
					// Append the new mobj to the converted file
					converted += newMobj;
				}
				// Add the KN5 Header
				converted.append("_KN5", 4);
				converted.append(124, 0);
				// For each of the remaining sections (1 - 7)
				for (unsigned int i = 0; i < numSections; i++)
				{
					// Get the section and next section indices
					unsigned int section = (i + 1), nextSection = (i + 2), sectionOff, nextSectionOff, sectionLen;
					bool useVertexTemplate = updateRig && ((swapMdls & (1 << section)) != 0);
					// Get the section offset
					if (section < numSections)
					{
						// Update the section offset
						*(unsigned int *)&(converted[(i + 2) * 4]) = converted.size();
						if ((excludeMdls & (1 << section)) == 0)
						{
							// Determine which template to get the offset from
							if (useVertexTemplate)
							{
								// Get the section offset
								sectionOff = *(unsigned int *)&(templateVertexData[(section + 1) * 4]);
							}
							else
							{
								// Get the section offset
								sectionOff = *(unsigned int *)&(templateRigData[(section + 1) * 4]);
							}
							// Check if the next section is a real section
							if (nextSection < numSections)
							{
								// Determine which template to get the offset from
								if (useVertexTemplate)
								{
									// Get the next section offset
									nextSectionOff = *(unsigned int *)&(templateVertexData[(nextSection + 1) * 4]);
								}
								else
								{
									// Get the next section offset
									nextSectionOff = *(unsigned int *)&(templateRigData[(nextSection + 1) * 4]);
								}
							}
							else
							{
								// Determine which template to get the offset from and use the end of the file
								if (useVertexTemplate)
								{
									nextSectionOff = templateVertexData.size();
								}
								else
								{
									nextSectionOff = templateRigData.size();
								}
							}
							// Determine the section length
							if (nextSectionOff > sectionOff)
							{
								sectionLen = nextSectionOff - sectionOff;
								if (useVertexTemplate && ((sectionOff + sectionLen) > templateVertexLen))
								{
									sectionLen = templateVertexLen - sectionOff;
								}
								else if (!useVertexTemplate && ((sectionOff + sectionLen) > templateRigLen))
								{
									sectionLen = templateRigLen - sectionOff;
								}
							}
							else
							{
								sectionLen = 0;
							}
						}
						else
						{
							sectionLen = 0;
						}
					}
					else
					{
						// Check if the file is byte aligned
						unsigned int fakeLen = converted.size();
						remainder = converted.size() % 128;
						// Set the section length to 0, because this isn't a section
						sectionLen = 0;
						// Append any padding if necessary
						if (remainder != 0)
						{
							fakeLen += 128 - remainder;
						}
						// Update the section offset
						*(unsigned int *)&(converted[(i + 2) * 4]) = fakeLen;
						// Update the offsets in nonexistant sections
						for (unsigned int prevSection = i; prevSection < numSections; prevSection--)
						{
							if (*(unsigned int *)&(converted[(prevSection + 2) * 4]) == converted.size())
							{
								*(unsigned int *)&(converted[(prevSection + 2) * 4]) = fakeLen;
							}
						}
					}
					// Check if there is data to append
					if (sectionLen != 0)
					{
						// Determine which template to copy from
						if (useVertexTemplate)
						{
							if (templateVertexLen >= (sectionOff + sectionLen))
							{
								// Append the section
								converted += templateVertexData.substr(sectionOff, sectionLen);
							}
							else
							{
								converted.append(sectionLen, 0);
							}
						}
						else
						{
							if (templateRigLen >= (sectionOff + sectionLen))
							{
								// Append the section
								converted += templateRigData.substr(sectionOff, sectionLen);
							}
							else
							{
								converted.append(sectionLen, 0);
							}
						}
					}
				}
			}
			else
			{
				cout << "Error: Invalid MOBJ Header! The template is not an MDLS" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid Number of Sections! The template is not an MDLS" << endl;
		}
	}
	else
	{
		cout << "Error: Invalid Length! The template is not an MDLS" << endl;
	}
	return converted;
}

string convertMeshToWpn(vector<VertexRelative> verts, vector<Face> faces, float adjustments[], float max)
{
	string converted = "";
	cout << "Warning: Converting to WPN without a template is currently not supported" << endl;
	return converted;
}

string convertMeshToWpn(vector<VertexRelative> verts, vector<Face> faces, string templateData, float adjustments[], float max)
{
	vector<vector<vector<vector<unsigned int>>>> polygonCollections;
	// The copy variables determine if we copy their sections from the templates (true), or if the tool generates those sections (false)
	string converted = "", menv, newMenv;
	unsigned int templateLen = templateData.size(), numSections, menvOff, menvLen, sectionOff, nextSectionOff, sectionLen, subsectionOff, subsectionLen, numPolygonCollections, numJoints, numVerts = verts.size(), textureInd, dimOff, origins[numOrigins], reset[numOrigins], remainder;
	// Flags for excluding and/or swapping sections of the mobj and mdls (0 = include section, 1 = exclude section)
	const unsigned char excludeWpn = 0x00, excludeMenv = 0x02;
	// Set all of the vertices to use joint 2
	for (unsigned int i = 0; i < numVerts; i++)
	{
		verts[i].originJointIndex = 2;
	}
	// Make sure the template is long enough to have the MDLS header
	if (templateLen > 16)
	{
		// Get the number of section in the joint template
		numSections = *(unsigned int *)&(templateData[0]);
		// A WPN starts with 2 offsets, so verify it has the correct number
		if (numSections == 2)
		{
			// Create the MDLS Header (we will need to update the offsets later)
			converted.append((char *)&(numSections), sizeof(unsigned int));	// Number of Sections
			converted.append(124, 0);										// Offsets and Padding
			// Update the offset to the first section
			*(unsigned int *)&(converted[4]) = converted.size();
			// Copy section 0 into converted
			if ((excludeWpn & 1) == 0)
			{
				// Get the section offset
				sectionOff = *(unsigned int *)&(templateData[4]);
				// Get the next section offset
				nextSectionOff = *(unsigned int *)&(templateData[8]);
				// Determine the section length
				if (nextSectionOff > sectionOff)
				{
					sectionLen = nextSectionOff - sectionOff;
				}
				else
				{
					sectionLen = 0;
				}
			}
			else
			{
				sectionLen = 0;
			}
			// Check if there is data to append
			if (sectionLen != 0)
			{
				if (templateLen >= (sectionOff + sectionLen))
				{
					// Append the section
					converted += templateData.substr(sectionOff, sectionLen);
				}
				else
				{
					converted.append(sectionLen, 0);
				}
			}
			// Update the offset to the second section
			*(unsigned int *)&(converted[8]) = converted.size();
			// Get the offset to the MENV section
			menvOff = *(unsigned int *)&(templateData[8]);
			// Make sure the menv section has a header
			if (templateLen >= (menvOff + menvHeaderLen))
			{
				// Get the length of the MENV subsection
				menvLen = *(unsigned int *)&(templateData[menvOff + 4]) + 8;
				// Make sure the menv section exists
				if (templateLen >= (menvOff + menvLen))
				{
					// Separate the MENV section from the rest of the WPN
					menv = templateData.substr(menvOff, menvLen);
					// Begin building the new menv section
					newMenv = "MENV";
					newMenv.append(menvHeaderLen - 4, 0); // Fill in the lengths and offsets later
					// Fill in the subsection 0 offset
					*(unsigned int *)&(newMenv[32]) = menvHeaderLen;
					// Sort the faces to optimize polygon collections
					faces = sortFaces(faces, verts);
					// Convert the faces into polygon collections
					if (getPolygonCollections(verts, faces, polygonCollections))
					{
						// Save the number of polygon collections
						numPolygonCollections = polygonCollections.size();
					}
					else
					{
						// There are no polygon collections
						numPolygonCollections = 0;
					}
					// Make sure the template has the num joints value
					if (menvLen >= (menvHeaderLen + 4))
					{
						// Store the number of joints used in the template
						numJoints = *(unsigned int *)&(menv[mobjHeaderLen]);
					}
					else
					{
						// Just set the number of joints to zero I guess, let it fail somewhere else
						numJoints = 0;
					}
					// Create model data section header
					newMenv.append((char *)&numJoints, sizeof(unsigned int));
					newMenv.append(8, (char)255); // Just set these offsets to not included
					newMenv.append((char *)&numPolygonCollections, sizeof(unsigned int));
					// Create the polygon collection table
					for (unsigned int i = 0; i < numPolygonCollections; i++)
					{
						if (polygonCollections[i].size() > 0 && polygonCollections[i][0].size() > 0 && polygonCollections[i][0][0].size() > 0)
						{
							textureInd = verts[polygonCollections[i][0][0][0]].textureIndex;
						}
						else
						{
							textureInd = 0;
						}
						newMenv.append(4, 0);
						newMenv.append((char *)&textureInd, sizeof(unsigned int));
						newMenv.append((char *)&textureInd, sizeof(unsigned int));
						newMenv.append(4, 0); // Need to fill in this offset later
					}
					// Append 32 bytes padding
					newMenv.append(32, 0);

					// Skip the hitbox table and the joint data subsections
					
					// Check if we are excluding the model data from the mobj
					if ((excludeMenv & 0x01) == 0)
					{
						// Handle Polygon Collections and Joint References
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							// Update offset in the polygon collection table
							subsectionOff = newMenv.size() - menvHeaderLen;
							*(unsigned int *)&(newMenv[menvHeaderLen + i * 16 + 28]) = subsectionOff;

							// Set origin joint indices to uninitialized and clear the reset
							for (unsigned int j = 0; j < numOrigins; j++)
							{
								origins[j] = absoluteMaxJoints;
								reset[j] = absoluteMaxJoints;
							}
							bool initialized = false, reinitialize = false;
							// For each polygon sub-collection in the polygon collection
							for (unsigned int j = 0; j < polygonCollections[i].size(); j++)
							{

								// Store the current size of the mobj for later so we can calculate the length of the polygon collection
								subsectionOff = newMenv.size();
								// Padding
								newMenv.append(8, 0); // Fill in the length field later
								// Unknown Stuff
								newMenv.append("\x01\x01\x00\x01\x00\x80\x00\x6C", 8);	// Fill in ?2 field later (length - 1)

								// For each triangle list in the polygon sub-collection
								for (unsigned int k = 0; k < polygonCollections[i][j].size(); k++)
								{
									// For each vertex in the triangle list
									for (unsigned int l = 0; initialized && l < polygonCollections[i][j][k].size(); l++)
									{
										// Check if we need to reinitialize the origins
										for (unsigned int m = 0; initialized && m < numOrigins; m++)
										{
											// Check if this vertex's origin joint matches any of the stored origin joints
											if (verts[polygonCollections[i][j][k][l]].originJointIndex == origins[m])
											{
												// The origin joint is valid, we can continue without reinitializing
												break;
											}
											// If we didn't find it
											else if (m == (numOrigins - 1))
											{
												// We need to reinitialize
												initialized = false;
											}
										}
									}
									// Check if the origins have been initialized with real indices
									if (!initialized)
									{
										// Store the previous values of the origins array in the reset array and clear the origins array
										for (unsigned int l = 0; l < numOrigins; l++)
										{
											reset[l] = origins[l];
											origins[l] = absoluteMaxJoints;
										}
										// Start from the current triangle list
										for (unsigned int k2 = k; !initialized && k2 < polygonCollections[i][j].size(); k2++)
										{
											// For each vertex in the triangle list
											for (unsigned int l = 0; !initialized && l < polygonCollections[i][j][k2].size(); l++)
											{
												// For each origin we can reference simultaneously
												for (unsigned int m = 0; m < numOrigins; m++)
												{
													// Check if this vertex has a new joint we need to reference
													if (verts[polygonCollections[i][j][k2][l]].originJointIndex != origins[m] && origins[m] == absoluteMaxJoints)
													{
														// Store the new index
														origins[m] = verts[polygonCollections[i][j][k2][l]].originJointIndex;
														// Check if we just filled the origins array
														if (m == (numOrigins - 1))
														{
															// We are done initializing
															initialized = true;
														}
														// Break from the loop so we don't duplicate the entry
														break;
													}
													else if (verts[polygonCollections[i][j][k2][l]].originJointIndex == origins[m])
													{
														// This is a repeat, just break from the loop to try the next vertex
														break;
													}
													// Check if this vertex matches the joint from the previous initialization
													else //if (verts[polygonCollections[i][j][k2][l]].originJointIndex == reset[m] && origins[m] == absoluteMaxJoints)
													{
														// Check if this vertex matches the joint from a previous initialization
														bool repeat = false;
														if (origins[m] == absoluteMaxJoints)
														{
															for (unsigned int n = 0; !repeat && (n < numOrigins); n++)
															{
																if (verts[polygonCollections[i][j][k2][l]].originJointIndex == reset[n])
																{
																	// Store the previous index
																	origins[m] = reset[n];
																	repeat = true;
																}
															}
														}
														// Store the previous index
														//origins[m] = reset[m];
														if (repeat)
														{
															// Check if we just filled the origins array
															if (m == (numOrigins - 1))
															{
																// We are done initializing
																initialized = true;
															}
															// Break from the loop so we don't duplicate the entry
															break;
														}
													}
												}
											}
										}
										initialized = true;
										reinitialize = true;
									}
									// At this point we should have the correct origins
									if (reinitialize)
									{
										// For each origin
										for (unsigned int l = 0; l < numOrigins; l++)
										{
											bool writeOrigin = origins[l] != absoluteMaxJoints;
											for (unsigned int m = 0; writeOrigin && (m < numOrigins); m++)
											{
												writeOrigin = writeOrigin && (origins[l] != reset[m]);
											}
											// Check if we need to update the origin by comparing with reset
											//if (writeOrigin)
											if (origins[l] != absoluteMaxJoints && origins[l] != reset[l])
											{
												// Write the origin update subsection
												newMenv.append(4, 0);						// Origin Update identifier
												newMenv.append((char *)&(origins[l]), 4);	// Origin Joint Index
												newMenv.append((char *)&(l), 4);			// Origin Index
												newMenv.append(116, 0);						// Padding (used for computation)
											}
										}

										// Clear the reinitialize flag
										reinitialize = false;
									}

									// Write triangle list header
									numVerts = polygonCollections[i][j][k].size();
									// Calculate the length of the triangle list section
									subsectionLen = numVerts * 48 + 32;
									// Triangle list identifier
									newMenv.append("\x01\x00\x00\x00", 4);
									// Length of triangle list
									newMenv.append((char *)&(subsectionLen), sizeof(unsigned int));
									// Number of vertices in the triangle list
									newMenv.append((char *)&(numVerts), sizeof(unsigned int));
									// Initial culling direction flag (0 - Anticlockwise, 1 - Clockwise), We assume Anticlockwise 
									newMenv.append(4, 0);
									// Copy of the number of vertices
									newMenv.append((char *)&(numVerts), sizeof(unsigned int));
									// Unknown stuff
									newMenv.append("\x00\x40\x3E\x30\x12\x04\x00\x00", 8);
									// Padding?
									newMenv.append(4, 0);

									// For each vertex in the triangle list
									for (unsigned int l = 0; l < polygonCollections[i][j][k].size(); l++)
									{
										VertexRelative v = verts[polygonCollections[i][j][k][l]];
										// Write the normals
										for (unsigned int m = 0; m < numDims; m++)
										{
											dimOff = (m + 1) % numDims;
											newMenv.append((char *)&(v.normal[dimOff]), sizeof(float));
										}
										// Find and write the origin index
										for (unsigned int m = 0; m < numOrigins; m++)
										{
											// Find the corresponding origin
											if (origins[m] == v.originJointIndex)
											{
												// Write the origin index
												newMenv.append((char *)&(m), sizeof(unsigned int));
												break;
											}
											else if (m == (numOrigins - 1))
											{
												// Fail safe
												newMenv.append(4, 0);
											}
										}
										// For each dimension
										for (unsigned int m = 0; m < numDims; m++)
										{
											dimOff = (m + 1) % numDims;
											// Write the vertex's coordinates
											newMenv.append((char *)&(v.coordinates[dimOff]), sizeof(float));
										}
										// Write the vertex's coordinate scale factor
										newMenv.append((char *)&(v.coordinatesScaleFactor), sizeof(float));
										// For each UV coordinate
										for (unsigned int m = 0; m < 2; m++)
										{
											// Get the UV coordinate
											float texVal = v.textureMap[m];
											// Invert the V coordinate, because mdls format is special
											if (m == 1)
											{
												texVal = 1 - texVal;
											}
											// Write the vertex's UV coordinates
											newMenv.append((char *)&(texVal), sizeof(float));
										}
										// Write the vertex's texture scale factor
										newMenv.append((char *)&(v.textureScaleFactor), sizeof(float));
										// Padding
										newMenv.append(4, 0);
									}
								}

								// Write the polygon (sub) collection footer
								newMenv.append("\x00\x80\x00\x00", 4);
								if (j == (polygonCollections[i].size() - 1))
								{
									// This is the end of the polygon collection
									newMenv.append(1, 0);
								}
								else
								{
									// We still have more to do in this polygon collection
									newMenv.append(1, 48);
								}
								newMenv.append(11, 0);
								newMenv.append("\x00\x00\x00\x17", 4);
								newMenv.append(12, 0);
								// Update the polygon collection's length field
								*(unsigned int *)&(newMenv[subsectionOff]) = ((newMenv.size() - subsectionOff) / 16) - 1;
								newMenv[subsectionOff + 3] = 16;
								// Update ?2 field
								newMenv[subsectionOff + 14] = newMenv[subsectionOff] - 1;

							}

							// To-Do: End of Polygon Collection?

						}
						subsectionLen = newMenv.size() - menvHeaderLen;
					}
					else
					{
						// Don't do this, this is really dumb
						subsectionLen = 0;
					}

					// Update the model data section length
					*(unsigned int *)&(newMenv[36]) = subsectionLen;
					// To-Do: Subsection 1
					subsectionOff = newMenv.size();
					// Update the Subsection 1 offset
					*(unsigned int *)&(newMenv[40]) = subsectionOff;
					// Check if we are including this section
					if ((excludeMenv & 0x02) == 0)
					{
						// For now just copy Subsection 1 from the template
						// Get the offset and length of the subsection
						subsectionOff = *(unsigned int *)&(menv[40]);
						subsectionLen = *(unsigned int *)&(menv[44]);
						// Make sure there is data to copy
						if (subsectionLen > 0 && menvLen >= (subsectionOff + subsectionLen))
						{
							newMenv += menv.substr(subsectionOff, subsectionLen);
						}
						else
						{
							newMenv.append(subsectionLen, 0);
						}
					}
					else
					{
						// Were aren't including this section, so its length will be 0
						subsectionLen = 0;
					}
					// Update the Subsection 1 length
					*(unsigned int *)&(newMenv[44]) = subsectionLen;
					// For each remaining subsection (2, 3, and 4)
					for (unsigned int i = 0; i < 3; i++)
					{
						// Calculate the index for the offset
						unsigned int off = (i + 1) * 8;
						// Update the offset to the section
						*(unsigned int *)&(newMenv[off]) = newMenv.size();
						if ((excludeMenv & (1 << (i + 2))) == 0)
						{
							// Get the offset and length of the section
							subsectionOff = *(unsigned int *)&(menv[off]);
							subsectionLen = *(unsigned int *)&(menv[off + 4]);
							// Make sure there is data to append
							if (subsectionLen > 0)
							{
								// Append the data
								newMenv += menv.substr(subsectionOff, subsectionLen);
							}
						}
						else
						{
							subsectionLen = 0;
						}
						// Update the length of the section
						*(unsigned int *)&(newMenv[off + 4]) = subsectionLen;
						// Check if we need to update the padding between subsections 2 and 3
						if (i == 0)
						{
							// Update the padding offset
							*(unsigned int *)&(newMenv[48]) = newMenv.size();
							// Add the padding if necessary
							remainder = (*(unsigned int *)&(converted[4]) + newMenv.size()) % 128;
							if (remainder > 0)
							{
								newMenv.append(128 - remainder, 0);
							}
						}
					}
					// Update the length of the new mobj
					*(unsigned int *)&(newMenv[4]) = newMenv.size() - 8;
					// Check if we are excluding the mobj section (please don't do this)
					if ((excludeWpn & 0x02) == 0)
					{
						// Append the new mobj to the converted file
						converted += newMenv;
					}

					// To-Do: TEXA Section
					unsigned int texaOff = menvOff + menvLen, texaLen = 0;
					if (templateLen >= (texaOff + 8))
					{
						texaLen = *(unsigned int *)&(templateData[texaOff + 4]) + 8;
						if ((texaLen > 0) && ((excludeWpn & 4) == 0))
						{
							if (templateLen >= (texaOff + texaLen))
							{
								converted += templateData.substr(texaOff, texaLen);
							}
						}
					}

					// Add the KN5 Header
					converted.append("_KN5", 4);

					// Check if the file is byte aligned
					unsigned int fakeLen = converted.size();
					remainder = converted.size() % 128;
					// Append any padding if necessary
					if (remainder != 0)
					{
						fakeLen += 128 - remainder;
					}
					// Update the section offset
					*(unsigned int *)&(converted[12]) = fakeLen;
					// Update the offsets in nonexistant sections
					for (unsigned int prevSection = (numSections - 1); prevSection < numSections; prevSection--)
					{
						if (*(unsigned int *)&(converted[(prevSection + 1) * 4]) == (converted.size() - texaLen - 4))
						{
							*(unsigned int *)&(converted[(prevSection + 1) * 4]) = fakeLen;
						}
					}
				}
				else
				{
					cout << "Error: Invalid MENV section! The template is not a valid WPN" << endl;
				}
			}
			else
			{
				cout << "Error: Invalid MENV Header! The template is not a WPN" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid Number of Sections! The template is not a WPN" << endl;
		}
	}
	else
	{
		cout << "Error: Invalid Length! The template is not a WPN" << endl;
	}
	return converted;
}

string convertMeshToDae(vector<JointRelative> joints, vector<VertexGlobal> verts, vector<Face> faces, vector<Animation> anims, float adjustments[], float max, string name)
{
	string converted = "", numAsStr, tabbed;
	stringstream ss;
	// Convert the joint positions into relative vertices
	vector<VertexRelative> jointsR = convertJointsToVerts(joints);
	vector<VertexGlobal> jointsG;
	vector<vector<float>> transformMat, translateMat, rotateMat;
	vector<unsigned int> indexStack;
	vector<bool> jointStatusStack;
	unsigned int numTextures = getNumTextures(verts), numFacesOfTexture, rootJointIndex = getRootJointIndex(joints), counter, jointDepth, jointIndex, parentJointIndex, childJointIndex, numChildren, dimOff;
	// The translation for the bind shape matrix
	float translate[numDims];
	// Get the root joint index
	if (rootJointIndex < joints.size() && rootJointIndex < absoluteMaxJoints)
	{
		// Convert the relative positions to global positions
		if (getVerticesGlobal(joints, jointsR, jointsG))
		{
			// Declare xml version and encoding
			converted += "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
			// Declare Collada version
			converted += "<COLLADA xmlns=\"http://www.collada.org/2005/11/COLLADASchema\" version=\"1.4.1\">\n";
			// Include the author, authoring tool, meter unit, and up axis
			converted += "\t<asset>\n";
			converted += "\t\t<contributor>\n";
			converted += "\t\t\t<author>Hypercrown User</author>\n";
			converted += "\t\t\t<authoring_tool>" + prog + " " + version + "</authoring_tool>\n";
			converted += "\t\t</contributor>\n";
			// Label the z axis as up
			converted += "\t\t<up_axis>Z_UP</up_axis>\n";
			converted += "\t</asset>\n";
			// Create the image library for holding the textures
			converted += "\t<library_images>\n";
			for (unsigned int i = 0; i < numTextures; i++)
			{
				// Convert the texture index to a string
				ss.clear();
				ss.str("");
				ss << i;
				ss >> numAsStr;
				// Create an image for texture i in the image library
				converted += "\t\t<image id=\"texture" + numAsStr + "\">\n";
				converted += "\t\t\t<init_from>" + name + "-" + numAsStr + ".png" + "</init_from>\n";
				converted += "\t\t</image>\n";
			}
			converted += "\t</library_images>\n";
			// Create the effect library for holding the material effects
			converted += "\t<library_effects>\n";
			for (unsigned int i = 0; i < numTextures; i++)
			{
				ss.clear();
				ss.str("");
				ss << i;
				ss >> numAsStr;
				converted += "\t\t<effect id=\"mat" + numAsStr + "-effect\">\n";
				converted += "\t\t\t<profile_COMMON>\n";
				converted += "\t\t\t\t<newparam sid=\"mat" + numAsStr + "-surface\">\n";
				converted += "\t\t\t\t\t<surface type=\"2D\">\n";
				converted += "\t\t\t\t\t\t<init_from>texture" + numAsStr + "</init_from>\n";
				converted += "\t\t\t\t\t</surface>\n";
				converted += "\t\t\t\t</newparam>\n";
				converted += "\t\t\t\t<newparam sid=\"mat" + numAsStr + "-sampler\">\n";
				converted += "\t\t\t\t\t<sampler2D>\n";
				converted += "\t\t\t\t\t\t<source>mat" + numAsStr + "-surface</source>\n";
				converted += "\t\t\t\t\t</sampler2D>\n";
				converted += "\t\t\t\t</newparam>\n";
				converted += "\t\t\t\t<technique sid=\"common\">\n";
				converted += "\t\t\t\t\t<lambert>\n";
				converted += "\t\t\t\t\t\t<diffuse>\n";
				converted += "\t\t\t\t\t\t\t<texture texture=\"mat" + numAsStr + "-sampler\" texcoord=\"UVMap\"/>\n";
				converted += "\t\t\t\t\t\t</diffuse>\n";
				converted += "\t\t\t\t\t</lambert>\n";
				converted += "\t\t\t\t</technique>\n";
				converted += "\t\t\t</profile_COMMON>\n";
				converted += "\t\t</effect>\n";
			}
			converted += "\t</library_effects>\n";
			// Create material library for holding the materials
			converted += "\t<library_materials>\n";
			for (unsigned int i = 0; i < numTextures; i++)
			{
				ss.clear();
				ss.str("");
				ss << i;
				ss >> numAsStr;
				converted += "\t\t<material id=\"mat" + numAsStr + "\">\n";
				converted += "\t\t\t<instance_effect url=\"#mat" + numAsStr + "-effect\"/>\n";
				converted += "\t\t</material>\n";
			}
			converted += "\t</library_materials>\n";
			// Create geometry library for holding the mesh data
			converted += "\t<library_geometries>\n";
			converted += "\t\t<geometry id=\"" + name + "-mesh\" name=\"" + name + "\">\n";
			converted += "\t\t\t<mesh>\n";
			converted += "\t\t\t\t<source id=\"" + name + "-mesh-pose\">\n";
			ss.clear();
			ss.str("");
			ss << verts.size() * 3;
			ss >> numAsStr;
			converted += "\t\t\t\t\t<float_array id=\"" + name + "-mesh-pose-array\" count=\"" + numAsStr + "\"> ";
			for (unsigned int i = 0; i < verts.size(); i++)
			{
				for (unsigned int j = 0; j < numDims; j++)
				{
					ss.clear();
					ss.str("");
					ss << (verts[i].coordinates[j] - adjustments[j]) / max;
					ss >> numAsStr;
					converted += numAsStr + " ";
				}
			}
			converted += "</float_array>\n";
			converted += "\t\t\t\t\t<technique_common>\n";
			ss.clear();
			ss.str("");
			ss << verts.size();
			ss >> numAsStr;
			converted += "\t\t\t\t\t\t<accessor source=\"#" + name + "-mesh-pose-array\" count=\"" + numAsStr + "\" stride=\"3\">\n";
			converted += "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t</accessor>\n";
			converted += "\t\t\t\t\t</technique_common>\n";
			converted += "\t\t\t\t</source>\n";
			converted += "\t\t\t\t<source id=\"" + name + "-normals\">\n";
			ss.clear();
			ss.str("");
			ss << verts.size() * 3;
			ss >> numAsStr;
			converted += "\t\t\t\t\t<float_array id=\"" + name + "-normals-array\" count=\"" + numAsStr + "\"> ";
			for (unsigned int i = 0; i < verts.size(); i++)
			{
				for (unsigned int j = 0; j < numDims; j++)
				{
					ss.clear();
					ss.str("");
					ss << verts[i].normal[j];
					ss >> numAsStr;
					converted += numAsStr + " ";
				}
			}
			converted += "</float_array>\n";
			converted += "\t\t\t\t\t<technique_common>\n";
			ss.clear();
			ss.str("");
			ss << verts.size();
			ss >> numAsStr;
			converted += "\t\t\t\t\t\t<accessor source=\"#" + name + "-normals-array\" count=\"" + numAsStr + "\" stride=\"3\">\n";
			converted += "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t</accessor>\n";
			converted += "\t\t\t\t\t</technique_common>\n";
			converted += "\t\t\t\t</source>\n";
			converted += "\t\t\t\t<source id=\"" + name + "-texcoords\">\n";
			ss.clear();
			ss.str("");
			ss << verts.size() * 2;
			ss >> numAsStr;
			converted += "\t\t\t\t\t<float_array id=\"" + name + "-texcoords-array\" count=\"" + numAsStr + "\"> ";
			for (unsigned int i = 0; i < verts.size(); i++)
			{
				for (unsigned int j = 0; j < 2; j++)
				{
					ss.clear();
					ss.str("");
					ss << verts[i].textureMap[j];
					ss >> numAsStr;
					converted += numAsStr + " ";
				}
			}
			converted += "</float_array>\n";
			converted += "\t\t\t\t\t<technique_common>\n";
			ss.clear();
			ss.str("");
			ss << verts.size();
			ss >> numAsStr;
			converted += "\t\t\t\t\t\t<accessor source=\"#" + name + "-texcoords-array\" count=\"" + numAsStr + "\" stride=\"2\">\n";
			converted += "\t\t\t\t\t\t\t<param name=\"S\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t\t<param name=\"T\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t</accessor>\n";
			converted += "\t\t\t\t\t</technique_common>\n";
			converted += "\t\t\t\t</source>\n";
			converted += "\t\t\t\t<vertices id=\"" + name + "-mesh-verts\">\n";
			converted += "\t\t\t\t\t<input semantic=\"POSITION\" source=\"#" + name + "-mesh-pose\"/>\n";
			converted += "\t\t\t\t</vertices>\n";
			counter = 0;
			for (unsigned int i = 0; i < numTextures; i++)
			{
				ss.clear();
				ss.str("");
				ss << i;
				ss >> numAsStr;
				converted += "\t\t\t\t<triangles material=\"mat" + numAsStr + "\" count=\"";
				numFacesOfTexture = getNumFacesOfTexture(verts, faces, i);
				ss.clear();
				ss.str("");
				ss << numFacesOfTexture;
				ss >> numAsStr;
				converted += numAsStr + "\">\n";
				converted += "\t\t\t\t\t<input semantic=\"VERTEX\" source=\"#" + name + "-mesh-verts\" offset=\"0\"/>\n";
				converted += "\t\t\t\t\t<input semantic=\"NORMAL\" source=\"#" + name + "-normals\" offset=\"0\"/>\n";
				converted += "\t\t\t\t\t<input semantic=\"TEXCOORD\" source=\"#" + name + "-texcoords\" offset=\"0\"/>\n";
				converted += "\t\t\t\t\t<p> ";
				for (unsigned int j = 0; j < faces.size(); j++)
				{
					if (faces[j].vertexIndices[0] < verts.size() && verts[faces[j].vertexIndices[0]].textureIndex == i)
					{
						for (unsigned int k = 0; k < numVertsPerFace; k++)
						{
							// Offset 0
							ss.clear();
							ss.str("");
							ss << faces[j].vertexIndices[k];
							ss >> numAsStr;
							converted += numAsStr + " ";
						}
					}
				}
				converted += "</p>\n";
				converted += "\t\t\t\t</triangles>\n";
			}
			converted += "\t\t\t</mesh>\n";
			converted += "\t\t</geometry>\n";
			converted += "\t</library_geometries>\n";
			// Create controller library to hold the rig
			converted += "\t<library_controllers>\n";
			converted += "\t\t<controller id=\"" + name + "-skin\" name=\"" + name + "-skin\">\n";
			converted += "\t\t\t<skin source=\"#" + name + "-mesh\">\n";
			// Create bind shape matrix for overlaying the normalized model on the skeleton
			converted += "\t\t\t\t<bind_shape_matrix> ";
			// Construct the bind translation array assuming the model has been scaled to the range [-1, 1]
			for (unsigned int i = 0; i < numDims; i++)
			{
				translate[i] = 0;
			}
			// Get the translation matrix using the bind translation
			transformMat = getTranslationMatrix(translate);
			for (unsigned int i = 0; i < 4; i++)
			{
				for (unsigned int j = 0; j < 4; j++)
				{
					ss.clear();
					ss.str("");
					ss << transformMat[i][j];
					ss >> numAsStr;
					converted += numAsStr + " ";
				}
			}
			converted += "</bind_shape_matrix>\n";
			// Create joint names
			converted += "\t\t\t\t<source id=\"" + name + "-skin-joints\">\n";
			ss.clear();
			ss.str("");
			ss << joints.size();
			ss >> numAsStr;
			converted += "\t\t\t\t\t<Name_array id=\"" + name + "-skin-joints-array\" count=\"" + numAsStr + "\"> ";
			for (unsigned int i = 0; i < joints.size(); i++)
			{
				ss.clear();
				ss.str("");
				ss << setfill('0') << setw(3) << joints[i].ind;
				ss >> numAsStr;
				converted += "bone" + numAsStr + " ";
			}
			converted += "</Name_array>\n";
			converted += "\t\t\t\t\t<technique_common>\n";
			ss.clear();
			ss.str("");
			ss << joints.size();
			ss >> numAsStr;
			converted += "\t\t\t\t\t\t<accessor source=\"#" + name + "-skin-joints-array\" count=\"" + numAsStr + "\" stride=\"1\">\n";
			converted += "\t\t\t\t\t\t\t<param name=\"JOINT\" type=\"name\"/>\n";
			converted += "\t\t\t\t\t\t</accessor>\n";
			converted += "\t\t\t\t\t</technique_common>\n";
			converted += "\t\t\t\t</source>\n";
			// Create the inverse bind matrices for each joint
			converted += "\t\t\t\t<source id=\"" + name + "-skin-bind-poses\">\n";
			ss.clear();
			ss.str("");
			ss << joints.size() * 16;
			ss >> numAsStr;
			converted += "\t\t\t\t\t<float_array id=\"" + name + "-skin-bind-poses-array\" count=\"" + numAsStr + "\"> \n";
			tabbed = "\t\t\t\t\t\t";
			for (unsigned int i = 0; i < joints.size(); i++)
			{
				// Reset the transform mat
				if (getInverseBindMatrix(joints, i, transformMat, max))
				{
					converted += tabbed;
					for (unsigned int j = 0; j < transformMat.size(); j++)
					{
						for (unsigned int k = 0; k < transformMat[j].size(); k++)
						{
							ss.clear();
							ss.str("");
							ss << transformMat[j][k];
							ss >> numAsStr;
							converted += numAsStr + " ";
						}
					}
					converted += "\n";
				}
				else
				{
					cout << "Error: Failed to get inverse bind matrix of joint " << i << endl;
				}
			}
			converted += "\t\t\t\t\t</float_array>\n";
			converted += "\t\t\t\t\t<technique_common>\n";
			ss.clear();
			ss.str("");
			ss << joints.size();
			ss >> numAsStr;
			converted += "\t\t\t\t\t\t<accessor source=\"#" + name + "-skin-bind-poses-array\" count=\"" + numAsStr + "\" stride=\"16\">\n";
			converted += "\t\t\t\t\t\t\t<param name=\"TRANSFORM\" type=\"float4x4\"/>\n";
			converted += "\t\t\t\t\t\t</accessor>\n";
			converted += "\t\t\t\t\t</technique_common>\n";
			converted += "\t\t\t\t</source>\n";
			// Create skin weights
			converted += "\t\t\t\t<source id=\"" + name + "-skin-weights\">\n";
			converted += "\t\t\t\t\t<float_array id=\"" + name + "-skin-weights-array\" count=\"1\">1</float_array>\n";
			converted += "\t\t\t\t\t<technique_common>\n";
			converted += "\t\t\t\t\t\t<accessor source=\"#" + name + "-skin-weights-array\" count=\"1\" stride=\"1\">\n";
			converted += "\t\t\t\t\t\t\t<param name=\"WEIGHT\" type=\"float\"/>\n";
			converted += "\t\t\t\t\t\t</accessor>\n";
			converted += "\t\t\t\t\t</technique_common>\n";
			converted += "\t\t\t\t</source>\n";
			// Define the joints
			converted += "\t\t\t\t<joints>\n";
			converted += "\t\t\t\t\t<input semantic=\"JOINT\" source=\"#" + name + "-skin-joints\"/>\n";
			converted += "\t\t\t\t\t<input semantic=\"INV_BIND_MATRIX\" source=\"#" + name + "-skin-bind-poses\"/>\n";
			converted += "\t\t\t\t</joints>\n";
			// Create VertexRelative Weights
			ss.clear();
			ss.str("");
			ss << verts.size();
			ss >> numAsStr;
			converted += "\t\t\t\t<vertex_weights count=\"" + numAsStr + "\">\n";
			converted += "\t\t\t\t\t<input semantic=\"JOINT\" source=\"#" + name + "-skin-joints\" offset=\"0\"/>\n";
			converted += "\t\t\t\t\t<input semantic=\"WEIGHT\" source=\"#" + name + "-skin-weights\" offset=\"1\"/>\n";
			converted += "\t\t\t\t\t<vcount> ";
			for (unsigned int i = 0; i < verts.size(); i++)
			{
				converted += "1 ";
			}
			converted += "</vcount>\n";
			converted += "\t\t\t\t\t<v> ";
			for (unsigned int i = 0; i < verts.size(); i++)
			{
				ss.clear();
				ss.str("");
				ss << verts[i].originJointIndex;
				ss >> numAsStr;
				converted += numAsStr + " 0 ";
			}
			converted += "</v>\n";
			converted += "\t\t\t\t</vertex_weights>\n";
			converted += "\t\t\t</skin>\n";
			converted += "\t\t</controller>\n";
			converted += "\t</library_controllers>\n";
			// Check if we need to add an animations library
			if (anims.size() > 0)
			{
				converted += "\t<library_animations>\n";
				for (unsigned int i = 0; i < anims.size(); i++)
				{
					Animation ai = anims[i];
					unsigned int numKeyframes;
					string numKeyframesAsStr, numAsStr2;
					if (ai.keyframes.size() < ai.times.size())
					{
						numKeyframes = ai.keyframes.size();
					}
					else
					{
						numKeyframes = ai.times.size();
					}
					ss.clear();
					ss.str("");
					ss << numKeyframes;
					ss >> numKeyframesAsStr;
					ss.clear();
					ss.str("");
					ss << setfill('0') << setw(3) << i;
					ss >> numAsStr;
					converted += "\t\t<animation id=\"Anim" + numAsStr + "-anim\" name=\"Anim" + numAsStr + "\">\n";
					converted += "\t\t\t<animation>\n";
					converted += "\t\t\t\t<source id=\"Anim" + numAsStr + "-Matrix-animation-input\">\n";
					converted += "\t\t\t\t\t<float_array id=\"Anim" + numAsStr + "-Matrix-animation-input-array\" count=\"" + numKeyframesAsStr + "\">\n";
					for (unsigned int j = 0; j < numKeyframes; j++)
					{
						ss.clear();
						ss.str("");
						ss << ai.times[j];
						ss >> numAsStr2;
						converted += "\t\t\t\t\t\t" + numAsStr2 + "\n";
					}
					converted += "\t\t\t\t\t</float_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					converted += "\t\t\t\t\t\t<accessor source=\"#Anim" + numAsStr + "-Matrix-animation-input-array\" count=\"" + numKeyframesAsStr + "\">\n";
					converted += "\t\t\t\t\t\t\t<param name=\"TIME\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					converted += "\t\t\t\t<source id=\"Anim" + numAsStr + "-Matrix-animation-output-transform\">\n";
					ss.clear();
					ss.str("");
					ss << numKeyframes * 16;
					ss >> numAsStr2;
					converted += "\t\t\t\t\t<float_array id=\"Anim" + numAsStr + "-Matrix-animation-output-transform-array\" count=\"" + numAsStr2 + "\">\n";
					for (unsigned int j = 0; j < numKeyframes; j++)
					{
						converted += "\t\t\t\t\t\t";
						for (unsigned int k = 0; k < 4; k++)
						{
							for (unsigned int l = 0; l < 4; l++)
							{
								ss.clear();
								ss.str("");
								if (ai.keyframes[j].size() == 4 && ai.keyframes[j][0].size() == 4 && ai.keyframes[j][1].size() == 4 && ai.keyframes[j][2].size() == 4 && ai.keyframes[j][3].size() == 4)
								{
									ss << ai.keyframes[j][k][l];
									ss >> numAsStr2;
								}
								else
								{
									ss << (k == l) ? 1 : 0;
									ss >> numAsStr2;
								}
								converted += numAsStr2 + " ";
							}
						}
						converted += "\n";
					}
					converted += "\t\t\t\t\t</float_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					converted += "\t\t\t\t\t\t<accessor source=\"#Anim" + numAsStr + "-Matrix-animation-output-transform-array\" count=\"" + numKeyframesAsStr + "\" stride=\"16\">\n";
					converted += "\t\t\t\t\t\t\t<param type=\"float4x4\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					converted += "\t\t\t\t<source id=\"Anim" + numAsStr + "-Interpolations\">\n";
					converted += "\t\t\t\t\t<Name_array id=\"Anim" + numAsStr + "-Interpolations-array\" count=\"" + numKeyframesAsStr + "\">\n";
					for (unsigned int j = 0; j < numKeyframes; j++)
					{
						converted += "\t\t\t\t\t\tLINEAR\n";
					}
					converted += "\t\t\t\t\t</Name_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					converted += "\t\t\t\t\t\t<accessor source=\"#Anim" + numAsStr + "-Interpolations-array\" count=\"" + numKeyframesAsStr + "\">\n";
					converted += "\t\t\t\t\t\t\t<param type=\"name\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					converted += "\t\t\t\t<sampler id=\"Anim" + numAsStr + "-Matrix-animation-transform\">\n";
					converted += "\t\t\t\t\t<input semantic=\"INPUT\" source=\"#Anim" + numAsStr + "-Matrix-animation-input\"/>\n";
					converted += "\t\t\t\t\t<input semantic=\"OUTPUT\" source=\"#Anim" + numAsStr + "-Matrix-animation-output-transform\"/>\n";
					converted += "\t\t\t\t\t<input semantic=\"INTERPOLATION\" source=\"#Anim" + numAsStr + "-Interpolations\"/>\n";
					converted += "\t\t\t\t</sampler>\n";
					ss.clear();
					ss.str("");
					ss << setfill('0') << setw(3) << ai.jointInd;
					ss >> numAsStr2;
					converted += "\t\t\t\t<channel source=\"#Anim" + numAsStr + "-Matrix-animation-transform\" target=\"bone" + numAsStr2 + "/matrix\"/>\n";
					converted += "\t\t\t</animation>\n";
					converted += "\t\t</animation>\n";
				}
				converted += "\t</library_animations>\n";
			}
			// Create visual scene library to hold the scene
			converted += "\t<library_visual_scenes>\n";
			converted += "\t\t<visual_scene id=\"scene\" name=\"Scene\">\n";
			// Create Skeleton Nodes
			converted += "\t\t\t<node id=\"Armature\" name=\"Armature\" type=\"NODE\">\n";
			// Add the root node to the stack
			indexStack.push_back(rootJointIndex);
			jointStatusStack.push_back(false);
			// Reset the counter and jointDepth
			counter = 0;
			jointDepth = 0;
			// Process the stack
			while (indexStack.size() > 0) // Fix error with joint 291 / 292
			{
				// Check if this is a joint or a depth update
				if (indexStack.back() < joints.size())
				{
					// Get the current joint index
					jointIndex = indexStack.back();
					// Get the parent joint index
					parentJointIndex = joints[jointIndex].jointInfo.parentIndex;
					// Construct tabs string
					tabbed = "";
					tabbed.append(jointDepth + 4, '\t');
					// Check if we have already visited this joint
					if (jointStatusStack.back())
					{
						// Add the extra techniques
						converted += tabbed + "\t<extra>\n";
						converted += tabbed + "\t\t<technique profile=\"blender\">\n";
						// Get the parent's connected child index
						if (parentJointIndex != absoluteMaxJoints)
						{
							childJointIndex = joints[parentJointIndex].jointInfo.childIndex;
						}
						else
						{
							childJointIndex = absoluteMaxJoints;
						}
						// Check if the parent is connected to this bone
						if (childJointIndex == jointIndex)
						{
							converted += tabbed + "\t\t\t<connect sid=\"connect\" type=\"bool\">1</connect>\n";
						}
						// Get this joint's connected child index
						childJointIndex = joints[jointIndex].jointInfo.childIndex;
						if (joints[jointIndex].childrenIndices.size() == 0 || childJointIndex < joints.size() && joints[childJointIndex].childrenIndices.size() == 0)
						{	
							for (unsigned int i = 0; i < numDims; i++)
							{
								char dimDisp = 'x' + i;
								ss.clear();
								ss.str("");
								if (joints[jointIndex].childrenIndices.size() == 0)
								{
									ss << 0;
								}
								else
								{
									ss << (jointsG[childJointIndex].coordinates[i] - jointsG[jointIndex].coordinates[i]) / max;
								}
								ss >> numAsStr;
								converted += tabbed + "\t\t\t<tip_" + dimDisp + " sid=\"tip_" + dimDisp + "\" type=\"float\">" + numAsStr + "</tip_" + dimDisp + ">\n";
							}
						}
						converted += tabbed + "\t\t</technique>\n";
						converted += tabbed + "\t\t<technique_common/>\n";
						converted += tabbed + "\t</extra>\n";
						// Close the node
						converted += tabbed + "</node>\n";
						// Remove the joint from the stacks
						indexStack.pop_back();
						jointStatusStack.pop_back();
					}
					else
					{
						// Mark this joint as visited
						jointStatusStack.back() = true;
						// Get the number of children this joint has
						numChildren = joints[jointIndex].childrenIndices.size();
						// Create node for this joint
						ss.clear();
						ss.str("");
						ss << setfill('0') << setw(3) << joints[jointIndex].ind;
						ss >> numAsStr;
						converted += tabbed + "<node id=\"bone" + numAsStr + "\" name=\"bone" + numAsStr + "\" sid=\"bone" + numAsStr + "\" type=\"JOINT\">\n";

						// To-Do: Add matrix transform
						// Construct the bind pose matrix for each joint in the following manner: trans{X, Y, Z} * rot{X} * rot{Z} * rot{Y} 
						transformMat = eye(4);
						// For each axis...
						for (unsigned int i = 0; i < numDims; i++)
						{
							// Get the dim offset so that we access the dims in Y, Z, X order
							dimOff = (i + 1) % numDims;
							// Get the rotation matrix
							rotateMat = getRotationMatrix(dimOff, joints[jointIndex].rotations[dimOff], 4);
							// Combine the rotation matrix with the transform matrix
							transformMat = matrixMultiply(rotateMat, transformMat);
						}
						// For each dimension
						for (unsigned int i = 0; i < numDims; i++)
						{
							// Get the joint's relative coordinates
							translate[i] = joints[jointIndex].coordinates[i] / max;
						}
						translateMat = getTranslationMatrix(translate);
						transformMat = matrixMultiply(translateMat, transformMat);
						converted += tabbed + "\t<matrix> ";
						for (unsigned int i = 0; i < transformMat.size(); i++)
						{
							for (unsigned int j = 0; j < transformMat[i].size(); j++)
							{
								ss.clear();
								ss.str("");
								ss << transformMat[i][j];
								ss >> numAsStr;
								converted += numAsStr + " ";
							}
						}
						converted += "</matrix>\n";
						// Update the counter
						counter++;
						// Check if there are chidren
						if (numChildren > 0)
						{
							// Push an invalid index to mark we need to decrement the depth
							indexStack.push_back(joints.size());
							// Update the depth
							jointDepth++;
						}
						// Add the joint's children to the stack
						for (unsigned int i = numChildren - 1; i < numChildren; i--)
						{
							indexStack.push_back(joints[jointIndex].childrenIndices[i]);
							jointStatusStack.push_back(false);
						}
					}
				}
				else
				{
					// Update the depth
					jointDepth--;
					// Remove the depth update from the index stack
					indexStack.pop_back();
				}
			}
			converted += "\t\t\t</node>\n";
			// Create Mesh Node
			converted += "\t\t\t<node id=\"" + name + "\" name=\"" + name + "\" type=\"NODE\">\n";
			//converted += "\t\t\t\t<instance_geometry url=\"#" + name + "-mesh\" name=\"" + name + "\">\n";
			converted += "\t\t\t\t<instance_controller url=\"#" + name + "-skin\" name=\"" + name + "\">\n";
			converted += "\t\t\t\t\t<skeleton>#Joint0</skeleton>\n";
			converted += "\t\t\t\t\t<bind_material>\n";
			converted += "\t\t\t\t\t\t<technique_common>\n";
			for (unsigned int i = 0; i < numTextures; i++)
			{
				ss.clear();
				ss.str("");
				ss << i;
				ss >> numAsStr;
				converted += "\t\t\t\t\t\t\t<instance_material symbol=\"mat" + numAsStr + "\" target=\"#mat" + numAsStr + "\">\n";
				converted += "\t\t\t\t\t\t\t\t<bind_vertex_input semantic=\"UVMap\" input_semantic=\"TEXCOORD\"/>\n";
				converted += "\t\t\t\t\t\t\t</instance_material>\n";
			}
			converted += "\t\t\t\t\t\t</technique_common>\n";
			converted += "\t\t\t\t\t</bind_material>\n";
			//converted += "\t\t\t\t</instance_geometry>\n";
			converted += "\t\t\t\t</instance_controller>\n";
			converted += "\t\t\t</node>\n";
			converted += "\t\t</visual_scene>\n";
			converted += "\t</library_visual_scenes>\n";
			// Instantiate the scene
			converted += "\t<scene>\n";
			converted += "\t\t<instance_visual_scene url=\"#scene\"/>\n";
			converted += "\t</scene>\n";
			converted += "</COLLADA>\n";
		}
		else
		{
			cout << "Error: Unable to get global positions of joints" << endl;
		}
	}
	else
	{
		cout << "Error: Unable to find root joint" << endl;
	}
	return converted;
}

string convertSkeletonToObj(vector<JointRelative> joints, unsigned int maxJoints)
{
	string converted = "";
	return converted;
}

string createMTL(unsigned int numTextures, string filenamePrefix)
{
	string mtl = "", numAsStr;
	stringstream ss;
	for (unsigned int i = 0; i < numTextures; i++)
	{
		ss.clear();
		ss.str("");
		ss << i;
		ss >> numAsStr;
		mtl += "newmtl mat" + numAsStr + "\n";
		mtl += "Kd 1.0 1.0 1.0\n";
		mtl += "map_Kd " + filenamePrefix + "-" + numAsStr + ".png\n";
		// Place a blank numAsStr between maps
		if (i < numTextures - 1)
		{
			mtl += "\n";
		}
	}
	// Return the mtl file
	return mtl;
}

void textureToRaw(Texture t, unsigned char *&rawTexture)
{
	const unsigned int bytesPerPixel = 4;
	rawTexture = new unsigned char[t.height * t.width * bytesPerPixel];
	for (unsigned int i = 0; i < t.indices.size(); i++)
	{
		unsigned int colorInd = t.indices[i];
		if (colorInd < t.palette.numColors)
		{
			rawTexture[i * 4] = t.palette.r[colorInd];
			rawTexture[i * 4 + 1] = t.palette.g[colorInd];
			rawTexture[i * 4 + 2] = t.palette.b[colorInd];
			rawTexture[i * 4 + 3] = t.palette.a[colorInd];
		}
		else
		{
			rawTexture[i * 4] = 0;
			rawTexture[i * 4 + 1] = 0;
			rawTexture[i * 4 + 2] = 0;
			rawTexture[i * 4 + 3] = 0;
		}
	}
	return;
}

vector<unsigned char> textureToRaw(Texture t)
{
	vector<unsigned char> rawTexture;
	const unsigned int bytesPerPixel = 4;
	rawTexture.reserve(t.height * t.width * bytesPerPixel);
	for (unsigned int i = 0; i < t.indices.size(); i++)
	{
		unsigned int colorInd = t.indices[i];
		if (colorInd < t.palette.numColors)
		{
			rawTexture.push_back(t.palette.r[colorInd]);
			rawTexture.push_back(t.palette.g[colorInd]);
			rawTexture.push_back(t.palette.b[colorInd]);
			rawTexture.push_back(t.palette.a[colorInd]);
		}
		else
		{
			rawTexture.push_back(0);
			rawTexture.push_back(0);
			rawTexture.push_back(0);
			rawTexture.push_back(0);
		}
	}
	return rawTexture;
}

// Returns the number of textures in an mdls file, returns 0 if a different file is passed
unsigned int getNumTextures(vector<VertexRelative> verts)
{
	unsigned int numTextures = 0, numVerts = verts.size(), textureInd;
	for (unsigned int i = 0; i < numVerts; i++)
	{
		textureInd = verts[i].textureIndex;
		if (textureInd >= numTextures)
		{
			numTextures = textureInd + 1;
		}
	}
	return numTextures;
}

// This function doesn't care about the vertex coordinates
unsigned int getNumTextures(vector<VertexGlobal> verts)
{
	return getNumTextures(*(vector<VertexRelative> *)&(verts));
}

unsigned int getNumMdlsTextures(string raw)
{
	unsigned int dataLen = raw.size(), mobjOff, tableOff, numPolygonCollections, textureInd, numTextures = 0;
	if (dataLen >= 8)
	{
		// Get the mobj offset
		mobjOff = *(unsigned int *)&(raw[4]);
		// Make sure the file contains the mobj header
		if (dataLen >= mobjOff + mobjHeaderLen)
		{
			// Determine the polygon collection table offset
			tableOff = mobjOff + *(unsigned int *)&(raw[mobjOff + 32]);
			// Make sure the file contains a polygon collection table
			if (dataLen >= tableOff + modelTableHeaderLen)
			{
				// Get the number of polygon collections
				numPolygonCollections = *(unsigned int *)&(raw[tableOff + 12]);
				// Make sure the polygon collection table is the correct size
				if (dataLen >= tableOff + (numPolygonCollections * 16))
				{
					// Store the offsets to each polygon collection
					for (unsigned int i = 0; i < numPolygonCollections; i++)
					{
						textureInd = *(unsigned int *)&(raw[tableOff + (i * 16) + 24]);
						if (textureInd >= numTextures)
						{
							numTextures = textureInd + 1;
						}
					}
				}
			}
		}
	}
	return numTextures;
}

unsigned int getNumWpnTextures(string raw)
{
	unsigned int dataLen = raw.size(), numOffs, menvOff, texInfoOff, texInfoLen, numTextures = 0;
	if (dataLen >= 12)
	{
		numOffs = *(unsigned int *)&(raw[0]);
		if (numOffs == 2)
		{
			// Get the mobj offset
			menvOff = *(unsigned int *)&(raw[8]);
			// Make sure the file contains the mobj header
			if (dataLen >= (menvOff + menvHeaderLen))
			{
				// Determine the polygon collection table offset
				texInfoOff = menvOff + *(unsigned int *)&(raw[menvOff + 8]);
				texInfoLen = *(unsigned int *)&(raw[menvOff + 12]);
				// Make sure the file contains a polygon collection table
				if (dataLen >= (texInfoOff + texInfoLen))
				{
					numTextures = texInfoLen / 16;
				}
				else
				{
					cout << "Error: The Texture Info subsection of the WPN file is invalid" << endl;
				}
			}
			else
			{
				cout << "Error: The MENV section of the WPN file contains an invalid header" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid number of sections in WPN header" << endl;
		}
	}
	else
	{
		cout << "Error: The WPN header is too short" << endl;
	}
	return numTextures;
}

vector<string> getMdlsSpecialEffects(string raw)
{
	vector<string> effects;
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16, numSubsections;
	string specialEffectSection, effect;
	// Make sure there is data to read
	if (raw.size() >= (readOffset + 4))
	{
		// Get the number of sections
		numSections = *(unsigned int *)&(raw[readOffset]);
		// Make sure this file has the correct number of sections
		if (numSections == 8)
		{
			// Update the read offset
			readOffset = 8;
			// Make sure there is data to read
			if (raw.size() >= 16)
			{
				// Get the offset to the Special Effects section
				sectionOffset = *(unsigned int *)&(raw[readOffset]);
				// Move to the HUD Image section offset
				readOffset += 4;
				// Determine the section length
				sectionLen = (*(unsigned int *)&(raw[readOffset])) - sectionOffset;
				// Make sure the section isn't empty
				if (sectionLen > 0)
				{
					// Make sure there is data to read and that there is a section header
					if (raw.size() >= (sectionOffset + sectionLen) && (sectionLen >= 16))
					{
						specialEffectSection = raw.substr(sectionOffset, sectionLen);
						// Move to the start of the section
						readOffset = 0;
						// Check the section id
						if (*(unsigned int *)&(specialEffectSection[readOffset]) == 130)
						{
							unsigned int numEntries = 0;
							vector<unsigned int> entryOffs;
							vector<string> entryMetaData;
							// Move to number of subsection type 0 entries
							readOffset += 12;
							// Get the number of entries
							numEntries = *(unsigned int *)&(specialEffectSection[readOffset]);
							readOffset += 4;
							entryOffs.reserve(numEntries);
							entryMetaData.reserve(numEntries);
							// For each entry, read the meta data and add it to the effect string
							for (unsigned int i = 0; i < numEntries; i++)
							{
								entryOffs.push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
								readOffset += 4;
								entryMetaData.push_back(specialEffectSection.substr(readOffset, 28));
								readOffset += 28;
							}
							if (sectionLen >= (readOffset + 4))
							{
								vector<unsigned int> subsectionOffsets;
								numSubsections = *(unsigned int *)&(specialEffectSection[readOffset]);
								readOffset += 4;
								subsectionOffsets.reserve(numSubsections);
								if (sectionLen >= (readOffset + (numSubsections * 4)))
								{
									// Store each subsection offset
									for (unsigned int i = 0; i < numSubsections; i++)
									{
										subsectionOffsets.push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
										readOffset += 4;
									}
									// Parse each subsection
									for (unsigned int i = 0; i < numSubsections; i++)
									{
										unsigned int finalSubsubsectionType = 5, effectLen = 0, associatedEntries = 0;
										for (unsigned int j = 0; j < numEntries; j++)
										{
											if (entryOffs[j] == subsectionOffsets[i])
											{
												associatedEntries++;
											}
										}
										// Add the spe meta data to the start of the effect string
										effect = "SPFX";
										effect.append((char *)&(associatedEntries), sizeof(unsigned int));
										for (unsigned int j = 0; j < numEntries; j++)
										{
											if (entryOffs[j] == subsectionOffsets[i])
											{
												effect += entryMetaData[j];
											}
										}
										// Word align the meta data
										if (effect.size() % 16 > 0)
										{
											effect.append(16 - (effect.size() % 16), 0);
										}
										// Move to the start of the subsection
										readOffset = subsectionOffsets[i];
										// Make sure the offset is valid
										if (sectionLen >= (readOffset + 4))
										{
											if (*(unsigned int *)&(specialEffectSection[readOffset]) == 150)
											{
												vector<vector<unsigned int>> subsubsectionOffsets;
												subsubsectionOffsets.reserve(5);
												// Move to the number of type 0 subsubsections
												readOffset += 4;
												// For each subsubsection type
												for (unsigned int j = 0; j < 5; j++)
												{
													if (sectionLen >= (readOffset + 4))
													{
														unsigned int numSubsubsections = *(unsigned int *)&(specialEffectSection[readOffset]);
														readOffset += 4;
														subsubsectionOffsets.push_back(vector<unsigned int>());
														subsubsectionOffsets[j].reserve(numSubsubsections);
														if (sectionLen >= (readOffset + (numSubsubsections * 4)))
														{
															for (unsigned int k = 0; k < numSubsubsections; k++)
															{
																subsubsectionOffsets[j].push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
																readOffset += 4;
															}
														}
														else
														{
															cout << "Error: Invalid number of subsubsections in subsection " << i << endl;
														}
													}
													else
													{
														cout << "Error: Invalid Subsection " << i << endl;
													}
												}
												for (unsigned int j = 4; j < 5; j--)
												{
													if (subsubsectionOffsets[j].size() != 0)
													{
														finalSubsubsectionType = j;
														break;
													}
												}
												if (finalSubsubsectionType < 5)
												{
													// Get the offset to the final subsubsection
													unsigned int subsubsectionLen = 0, finalSubsubsectionOffset = subsubsectionOffsets[finalSubsubsectionType].back();
													// Set the length equal to the last offset
													effectLen = finalSubsubsectionOffset;
													// Move to the start of the subsubsection
													readOffset = subsectionOffsets[i] + finalSubsubsectionOffset;
													// Add the length of the final subsubsection by parsing it
													switch (finalSubsubsectionType)
													{
													case 0:
														// Make sure there is enough data for the header
														if (sectionLen >= (readOffset + 288))
														{
															// Get the offset to the last part of this subsubsection
															unsigned int lastOffset = *(unsigned int *)&(raw[readOffset + 12]);
															// Initialize the subsubsection length
															subsubsectionLen = lastOffset + 272;
															// Move to the start of the last section
															readOffset += subsubsectionLen;
															if (sectionLen >= readOffset + 4)
															{
																// Get the number of values in the last section
																unsigned int numVals = *(unsigned int *)&(specialEffectSection[readOffset]), remainder;
																// Add the length of the last part to the subsubsection length
																subsubsectionLen += ((numVals + 1) * 4);
																// Determine if the subsubsection needs to be padded
																remainder = subsubsectionLen % 16;
																if (remainder > 0)
																{
																	// Pad the length to word align the subsubsection
																	subsubsectionLen += (16 - remainder);
																}
																if (sectionLen >= (readOffset + subsubsectionLen))
																{
																	effectLen += subsubsectionLen;
																}
																else
																{
																	cout << "Error: Missing values" << endl;
																}
															}
															else
															{
																cout << "Error: Missing the number of values" << endl;
															}
														}
														else
														{
															cout << "Error: Missing Subsubsection header" << endl;
														}
														break;
													case 1:
														// Make sure there is enough data for the header
														if (sectionLen >= (readOffset + 32))
														{
															unsigned int indsLen;
															unsigned short paletteLen = 0x400;
															indsLen = *(unsigned int *)&(specialEffectSection[readOffset + 27]) & 0xFFFFFF;
															readOffset += 32;
															subsubsectionLen = indsLen + paletteLen + 32;
															// Make sure the data for the indices exists
															if (sectionLen >= subsubsectionLen)
															{
																effectLen += subsubsectionLen;
															}
															else
															{
																cout << "Error: Missing image data" << endl;
															}
														}
														else
														{
															cout << "Error: Missing Subsubsection header" << endl;
														}
														break;
													case 2:
														// Make sure there is enough data for the header
														if (sectionLen >= (readOffset + 32))
														{
															subsubsectionLen = ((*(unsigned short *)&(specialEffectSection[readOffset + 30])) + 16);
															if (sectionLen >= (readOffset + subsubsectionLen))
															{
																effectLen += subsubsectionLen;
															}
															else
															{
																cout << "Error: Invalid subsubsection length" << endl;
															}
														}
														else
														{
															cout << "Error: Missing Subsubsection header" << endl;
														}
														break;
													case 3:
														// To-Do: Parse Type 3 Entry
														cout << "Error: Effect " << i << " ends with a Type 3 Subsubsection which currently is unable to be parsed" << endl;
														break;
													case 4:
														// Make sure there is enough data for the header
														if (sectionLen >= (readOffset + 12))
														{
															unsigned int numVals = *(unsigned int *)&(specialEffectSection[readOffset + 8]), remainder;
															subsubsectionLen = (numVals * 2) + 12;
															// Check if the subsubsection requires padding
															remainder = subsubsectionLen % 16;
															if (remainder > 0)
															{
																// Add the padding to the length
																subsubsectionLen += (16 - remainder);
															}
															if (sectionLen >= (readOffset + subsubsectionLen))
															{
																effectLen += subsubsectionLen;
															}
															else
															{
																cout << "Error: Invalid subsubsection length" << endl;
															}
														}
														else
														{
															cout << "Error: Missing Subsubsection header" << endl;
														}
														break;
													default:
														cout << "You shouldn't be able to get here" << endl;
														effectLen = 64;
													}
												}
												else
												{
													// There are no subsubsections? Just give the minimum length of the header?
													effectLen = 64;
												}
												if (effectLen > 0)
												{
													effect += specialEffectSection.substr(subsectionOffsets[i], effectLen);
													effects.push_back(effect);
												}
											}
											else
											{
												cout << "Error: Subsection ID is incorrect" << endl;
											}
										}
										else
										{
											cout << "Error: Invalid subsection offset for subsection " << i << endl;
										}
									}
								}
							}
							else
							{
								cout << "Error: Section Header isn't complete" << endl;
							}
						}
						else
						{
							cout << "Error: Section ID is incorrect" << endl;
						}
					}
					else
					{
						cout << "Error: Invalid section length" << endl;
					}
				}
			}
			else
			{
				cout << "Error: Invalid MDLS - Missing full header" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid MDLS - Incorrect number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: Invalid MDLS - Missing number of sections" << endl;
	}
	return effects;
}

vector<string> getWpnSpecialEffects(string raw)
{
	vector<string> effects;
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16, numSubsections;
	string specialEffectSection, effect;
	// Make sure there is data to read
	if (raw.size() >= (readOffset + 4))
	{
		// Get the number of sections
		numSections = *(unsigned int *)&(raw[readOffset]);
		// Make sure this file has the correct number of sections
		if (numSections == 2)
		{
			// Update the read offset
			readOffset = 4;
			// Make sure there is data to read
			if (raw.size() >= 12)
			{
				// Get the offset to the Special Effects section
				sectionOffset = *(unsigned int *)&(raw[readOffset]);
				// Move to the MENV section offset
				readOffset += 4;
				// Determine the section length
				sectionLen = (*(unsigned int *)&(raw[readOffset])) - sectionOffset;
				// Make sure the section isn't empty
				if (sectionLen > 0)
				{
					// Make sure there is data to read and that there is a section header
					if (raw.size() >= (sectionOffset + sectionLen) && (sectionLen >= 16))
					{
						specialEffectSection = raw.substr(sectionOffset, sectionLen);
						// Move to the start of the section
						readOffset = 0;
						// Check the section id
						if (*(unsigned int *)&(specialEffectSection[readOffset]) == 130)
						{
							unsigned int numEntries = 0;
							vector<unsigned int> entryOffs;
							vector<string> entryMetaData;
							// Move to number of subsection type 0 entries
							readOffset += 12;
							// Get the number of entries
							numEntries = *(unsigned int *)&(specialEffectSection[readOffset]);
							readOffset += 4;
							entryOffs.reserve(numEntries);
							entryMetaData.reserve(numEntries);
							// For each entry, read the meta data and add it to the effect string
							for (unsigned int i = 0; i < numEntries; i++)
							{
								entryOffs.push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
								readOffset += 4;
								entryMetaData.push_back(specialEffectSection.substr(readOffset, 28));
								readOffset += 28;
							}
							if (sectionLen >= (readOffset + 4))
							{
								vector<unsigned int> subsectionOffsets;
								numSubsections = *(unsigned int *)&(specialEffectSection[readOffset]);
								readOffset += 4;
								subsectionOffsets.reserve(numSubsections);
								if (sectionLen >= (readOffset + (numSubsections * 4)))
								{
									// Store each subsection offset
									for (unsigned int i = 0; i < numSubsections; i++)
									{
										subsectionOffsets.push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
										readOffset += 4;
									}
									// Parse each subsection
									for (unsigned int i = 0; i < numSubsections; i++)
									{
										unsigned int finalSubsubsectionType = 5, effectLen = 0, associatedEntries = 0;
										for (unsigned int j = 0; j < numEntries; j++)
										{
											if (entryOffs[j] == subsectionOffsets[i])
											{
												associatedEntries++;
											}
										}
										// Add the spe meta data to the start of the effect string
										effect = "SPFX";
										effect.append((char *)&(associatedEntries), sizeof(unsigned int));
										for (unsigned int j = 0; j < numEntries; j++)
										{
											if (entryOffs[j] == subsectionOffsets[i])
											{
												effect += entryMetaData[j];
											}
										}
										// Word align the meta data
										if (effect.size() % 16 > 0)
										{
											effect.append(16 - (effect.size() % 16), 0);
										}
										// Move to the start of the subsection
										readOffset = subsectionOffsets[i];
										// Make sure the offset is valid
										if (sectionLen >= (readOffset + 4))
										{
											if (*(unsigned int *)&(specialEffectSection[readOffset]) == 150)
											{
												vector<vector<unsigned int>> subsubsectionOffsets;
												subsubsectionOffsets.reserve(5);
												// Move to the number of type 0 subsubsections
												readOffset += 4;
												// For each subsubsection type
												for (unsigned int j = 0; j < 5; j++)
												{
													if (sectionLen >= (readOffset + 4))
													{
														unsigned int numSubsubsections = *(unsigned int *)&(specialEffectSection[readOffset]);
														readOffset += 4;
														subsubsectionOffsets.push_back(vector<unsigned int>());
														subsubsectionOffsets[j].reserve(numSubsubsections);
														if (sectionLen >= (readOffset + (numSubsubsections * 4)))
														{
															for (unsigned int k = 0; k < numSubsubsections; k++)
															{
																subsubsectionOffsets[j].push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
																readOffset += 4;
															}
														}
														else
														{
															cout << "Error: Invalid number of subsubsections in subsection " << i << endl;
														}
													}
													else
													{
														cout << "Error: Invalid Subsection " << i << endl;
													}
												}
												for (unsigned int j = 4; j < 5; j--)
												{
													if (subsubsectionOffsets[j].size() != 0)
													{
														finalSubsubsectionType = j;
														break;
													}
												}
												if (finalSubsubsectionType < 5)
												{
													// Get the offset to the final subsubsection
													unsigned int subsubsectionLen = 0, finalSubsubsectionOffset = subsubsectionOffsets[finalSubsubsectionType].back();
													// Set the length equal to the last offset
													effectLen = finalSubsubsectionOffset;
													// Move to the start of the subsubsection
													readOffset = subsectionOffsets[i] + finalSubsubsectionOffset;
													// Add the length of the final subsubsection by parsing it
													switch (finalSubsubsectionType)
													{
													case 0:
														// Make sure there is enough data for the header
														if (sectionLen >= (readOffset + 288))
														{
															// Get the offset to the last part of this subsubsection
															unsigned int lastOffset = *(unsigned int *)&(specialEffectSection[readOffset + 12]);
															// Initialize the subsubsection length
															subsubsectionLen = lastOffset + 272;
															// Move to the start of the last section
															readOffset += subsubsectionLen;
															if (sectionLen >= readOffset + 4)
															{
																// Get the number of values in the last section
																unsigned int numVals = *(unsigned int *)&(specialEffectSection[readOffset]), remainder;
																// Add the length of the last part to the subsubsection length
																subsubsectionLen += ((numVals + 1) * 4);
																// Determine if the subsubsection needs to be padded
																remainder = subsubsectionLen % 16;
																if (remainder > 0)
																{
																	// Pad the length to word align the subsubsection
																	subsubsectionLen += (16 - remainder);
																}
																if (sectionLen >= (readOffset + subsubsectionLen))
																{
																	effectLen += subsubsectionLen;
																}
																else
																{
																	cout << "Error: Missing values" << endl;
																}
															}
															else
															{
																cout << "Error: Missing the number of values" << endl;
															}
														}
														else
														{
															cout << "Error: Missing Subsubsection header" << endl;
														}
														break;
													case 1:
														// Make sure there is enough data for the header
														if (sectionLen >= (readOffset + 32))
														{
															unsigned int indsLen;
															unsigned short paletteLen = 0x400;
															indsLen = *(unsigned int *)&(specialEffectSection[readOffset + 27]) & 0xFFFFFF;
															readOffset += 32;
															subsubsectionLen = indsLen + paletteLen + 32;
															// Make sure the data for the indices exists
															if (sectionLen >= subsubsectionLen)
															{
																effectLen += subsubsectionLen;
															}
															else
															{
																cout << "Error: Missing image data" << endl;
															}
														}
														else
														{
															cout << "Error: Missing Subsubsection header" << endl;
														}
														break;
													case 2:
														// Make sure there is enough data for the header
														if (sectionLen >= (readOffset + 32))
														{
															subsubsectionLen = ((*(unsigned short *)&(specialEffectSection[readOffset + 30])) + 16);
															if (sectionLen >= (readOffset + subsubsectionLen))
															{
																effectLen += subsubsectionLen;
															}
															else
															{
																cout << "Error: Invalid subsubsection length" << endl;
															}
														}
														else
														{
															cout << "Error: Missing Subsubsection header" << endl;
														}
														break;
													case 3:
														// To-Do: Parse Type 3 Entry
														cout << "Error: Effect " << i << " ends with a Type 3 Subsubsection which currently is unable to be parsed" << endl;
														break;
													case 4:
														// Make sure there is enough data for the header
														if (sectionLen >= (readOffset + 12))
														{
															unsigned int numVals = *(unsigned int *)&(specialEffectSection[readOffset + 8]), remainder;
															subsubsectionLen = (numVals * 2) + 12;
															// Check if the subsubsection requires padding
															remainder = subsubsectionLen % 16;
															if (remainder > 0)
															{
																// Add the padding to the length
																subsubsectionLen += (16 - remainder);
															}
															if (sectionLen >= (readOffset + subsubsectionLen))
															{
																effectLen += subsubsectionLen;
															}
															else
															{
																cout << "Error: Invalid subsubsection length" << endl;
															}
														}
														else
														{
															cout << "Error: Missing Subsubsection header" << endl;
														}
														break;
													default:
														cout << "You shouldn't be able to get here" << endl;
														effectLen = 64;
													}
												}
												else
												{
													// There are no subsubsections? Just give the minimum length of the header?
													effectLen = 64;
												}
												if (effectLen > 0)
												{
													effect += specialEffectSection.substr(subsectionOffsets[i], effectLen);
													effects.push_back(effect);
												}
											}
											else
											{
												cout << "Error: Subsection ID is incorrect" << endl;
											}
										}
										else
										{
											cout << "Error: Invalid subsection offset for subsection " << i << endl;
										}
									}
								}
							}
							else
							{
								cout << "Error: Section Header isn't complete" << endl;
							}
						}
						else
						{
							cout << "Error: Section ID is incorrect" << endl;
						}
					}
					else
					{
						cout << "Error: Invalid section length" << endl;
					}
				}
			}
			else
			{
				cout << "Error: Invalid WPN - Missing full header" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid WPN - Incorrect number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: Invalid WPN - Missing number of sections" << endl;
	}
	return effects;
}

vector<Texture> getMdlsTextures(string raw)
{
	vector<Texture> textures;
	Texture t;
	unsigned int numTextures, numSections, readOffset = 0, mobjOffset, subsectionOffsets[3], subsectionLengths[3];
	// Make sure there is data to read
	if (raw.size() >= (readOffset + 4))
	{
		// Get the number of sections
		numSections = *(unsigned int *)&(raw[readOffset]);
		// Update the read offset
		readOffset += 4;
		// Make sure this file has the correct number of sections
		if (numSections == 8)
		{
			// Make sure there is data to read
			if (raw.size() >= (readOffset + 4))
			{
				// Get the offset to the MOBJ section
				readOffset = *(unsigned int *)&(raw[readOffset]);
				// Make sure there is data to read
				if (raw.size() >= (readOffset + mobjHeaderLen))
				{
					// Store the mobj offset
					mobjOffset = readOffset;
					// Move to the first section offset
					readOffset += 8;
					for (unsigned int i = 0; i < 3; i++)
					{
						subsectionOffsets[i] = mobjOffset + *(unsigned int *)&(raw[readOffset]);
						readOffset += 4;
						subsectionLengths[i] = *(unsigned int *)&(raw[readOffset]);
						readOffset += 4;
					}
					readOffset = subsectionOffsets[0];
					// Parse subsection 2
					if (raw.size() >= (readOffset + subsectionLengths[0]))
					{
						numTextures = subsectionLengths[0] / 16;
						for (unsigned int i = 0; i < numTextures; i++)
						{
							// Get the palette length
							unsigned short indicesLen = (*(unsigned short *)&(raw[readOffset])) * 16;
							readOffset += sizeof(unsigned short);
							// Extract the height and with exponents
							t.wExp = *(unsigned char *)&(raw[readOffset]);
							readOffset++;
							t.hExp = *(unsigned char *)&(raw[readOffset]);
							readOffset++;
							// The number of colors should always be 256
							t.palette.numColors = 256;
							// Get the height of the image
							t.width = *(unsigned short *)&(raw[readOffset]);
							readOffset += sizeof(unsigned short);
							// Get the width of the image
							t.height = *(unsigned short *)&(raw[readOffset]);
							readOffset += sizeof(unsigned short) + 8;
							// Reserve space for the indices
							t.indices.reserve(t.height * t.width);
							// Store the texture
							textures.push_back(t);
						}
					}
					// Parse subsection 3
					readOffset = subsectionOffsets[1];
					if (raw.size() >= (readOffset + subsectionLengths[1]))
					{
						unsigned int totalLen = 0;
						for (unsigned int i = 0; i < numTextures; i++)
						{
							t = textures[i];
							unsigned int indicesLen = t.height * t.width;
							totalLen += indicesLen;
							// Make sure there is enough data for texture i's indices
							if (subsectionLengths[1] >= totalLen)
							{
								// For each pixel
								for (unsigned int j = 0; j < indicesLen; j++)
								{
									// Store the index
									unsigned char colorInd, remainder;
									colorInd = *(unsigned char *)&(raw[readOffset]);
									readOffset++;
									remainder = colorInd % 32;
									if (remainder >= 8 && remainder < 16)
									{
										colorInd += 8;
									}
									else if(remainder >= 16 && remainder < 24)
									{
										colorInd -= 8;
									}
									textures[i].indices.push_back(colorInd);
								}
							}
						}
					}
					// Parse subsection 4
					readOffset = subsectionOffsets[2];
					if (raw.size() >= (readOffset + subsectionLengths[2]))
					{
						unsigned int totalLen = 0;
						for (unsigned int i = 0; i < numTextures; i++)
						{
							const unsigned int colorLen = 4;
							t = textures[i];
							totalLen += t.palette.numColors * colorLen;
							if (subsectionLengths[2] >= totalLen)
							{
								for (unsigned int j = 0; j < t.palette.numColors; j++)
								{
									unsigned char colorVal = raw[readOffset];
									readOffset++;
									textures[i].palette.r.push_back(colorVal);
									colorVal = raw[readOffset];
									readOffset++;
									textures[i].palette.g.push_back(colorVal);
									colorVal = raw[readOffset];
									readOffset++;
									textures[i].palette.b.push_back(colorVal);
									colorVal = raw[readOffset];
									readOffset++;
									if (colorVal > 0)
									{
										colorVal = 2 * colorVal - 1;
									}
									textures[i].palette.a.push_back(colorVal);
								}
							}
						}
					}
				}
			}
		}
	}
	return textures;
}

vector<Texture> getWpnTextures(string raw)
{
	vector<Texture> textures;
	Texture t;
	unsigned int numTextures, numSections, readOffset = 0, menvOffset, subsectionOffsets[3], subsectionLengths[3];
	// Make sure there is data to read
	if (raw.size() >= (readOffset + 4))
	{
		// Get the number of sections
		numSections = *(unsigned int *)&(raw[readOffset]);
		// Update the read offset
		readOffset += 8;
		// Make sure this file has the correct number of sections
		if (numSections == 2)
		{
			// Make sure there is data to read
			if (raw.size() >= (readOffset + 4))
			{
				// Get the offset to the MENV section
				readOffset = *(unsigned int *)&(raw[readOffset]);
				// Make sure there is data to read
				if (raw.size() >= (readOffset + menvHeaderLen))
				{
					// Store the menv offset
					menvOffset = readOffset;
					// Move to the first section offset
					readOffset += 8;
					for (unsigned int i = 0; i < 3; i++)
					{
						subsectionOffsets[i] = menvOffset + *(unsigned int *)&(raw[readOffset]);
						readOffset += 4;
						subsectionLengths[i] = *(unsigned int *)&(raw[readOffset]);
						readOffset += 4;
					}
					// Parse subsection 2
					readOffset = subsectionOffsets[0];
					if (raw.size() >= (readOffset + subsectionLengths[0]))
					{
						// Get the number of textures
						numTextures = subsectionLengths[0] / 16;
						// For each texture...
						for (unsigned int i = 0; i < numTextures; i++)
						{
							// Get the palette length
							unsigned short indicesLen = (*(unsigned short *)&(raw[readOffset])) * 16;
							readOffset += sizeof(unsigned short);
							// Extract the height and with exponents
							t.wExp = *(unsigned char *)&(raw[readOffset]);
							readOffset += sizeof(unsigned char);
							t.hExp = *(unsigned char *)&(raw[readOffset]);
							readOffset += sizeof(unsigned char);
							// The number of colors should always be 256
							t.palette.numColors = 256;
							// Get the height of the image
							t.width = *(unsigned short *)&(raw[readOffset]);
							readOffset += sizeof(unsigned short);
							// Get the width of the image
							t.height = *(unsigned short *)&(raw[readOffset]);
							readOffset += sizeof(unsigned short) + 8;
							// Reserve space for the indices
							t.indices.reserve(t.height * t.width);
							// Store the texture
							textures.push_back(t);
						}
					}
					// Parse subsection 3
					readOffset = subsectionOffsets[1];
					if (raw.size() >= (readOffset + subsectionLengths[1]))
					{
						unsigned int totalLen = 0;
						for (unsigned int i = 0; i < numTextures; i++)
						{
							t = textures[i];
							unsigned int indicesLen = t.height * t.width;
							totalLen += indicesLen;
							// Make sure there is enough data for texture i's indices
							if (subsectionLengths[1] >= totalLen)
							{
								// For each pixel
								for (unsigned int j = 0; j < indicesLen; j++)
								{
									// Store the index
									unsigned char colorInd, remainder;
									colorInd = *(unsigned char *)&(raw[readOffset]);
									readOffset++;
									remainder = colorInd % 32;
									if (remainder >= 8 && remainder < 16)
									{
										colorInd += 8;
									}
									else if (remainder >= 16 && remainder < 24)
									{
										colorInd -= 8;
									}
									textures[i].indices.push_back(colorInd);
								}
							}
						}
					}
					// Parse subsection 4
					readOffset = subsectionOffsets[2];
					if (raw.size() >= (readOffset + subsectionLengths[2]))
					{
						unsigned int totalLen = 0;
						for (unsigned int i = 0; i < numTextures; i++)
						{
							const unsigned int colorLen = 4;
							t = textures[i];
							totalLen += t.palette.numColors * colorLen;
							if (subsectionLengths[2] >= totalLen)
							{
								for (unsigned int j = 0; j < t.palette.numColors; j++)
								{
									unsigned char colorVal = raw[readOffset];
									readOffset++;
									textures[i].palette.r.push_back(colorVal);
									colorVal = raw[readOffset];
									readOffset++;
									textures[i].palette.g.push_back(colorVal);
									colorVal = raw[readOffset];
									readOffset++;
									textures[i].palette.b.push_back(colorVal);
									colorVal = raw[readOffset];
									readOffset++;
									if (colorVal > 0)
									{
										colorVal = 2 * colorVal - 1;
									}
									textures[i].palette.a.push_back(colorVal);
								}
							}
						}
					}
				}
			}
		}
	}
	return textures;
}

string setMdlsSpecialEffects(string raw, vector<string> effects)
{
	string updatedMdls = "";
	unsigned int mdlsLen = raw.size(), sectionLen, remainder, fakeSectionOff;
	// Make sure the mdls data contains the number of sections
	if (mdlsLen >= 4)
	{
		unsigned int readOffset = 0, numSections = *(unsigned int *)&(raw[readOffset]);
		readOffset += 4;
		// Make sure the mdls data contains the proper number of sections
		if (numSections == 8)
		{
			// Make sure the mdls data contains the section offsets
			if (mdlsLen >= 128)
			{
				// Get the section offsets
				unsigned int sectionOffsets[8];
				for (unsigned int i = 0; i < numSections; i++)
				{
					sectionOffsets[i] = *(unsigned int *)&(raw[readOffset]);
					readOffset += 4;
				}
				// Copy the mdls header into the return value (We update the offsets later)
				updatedMdls = raw.substr(0, 128);
				// Copy the mobj section from the original and update the offset to the section
				sectionLen = sectionOffsets[1] - sectionOffsets[0];
				// Update the offset to section i
				readOffset = 4;
				*(unsigned int *)&(updatedMdls[readOffset]) = updatedMdls.size();
				// Copy the section
				if (sectionLen > 0)
				{
					updatedMdls += raw.substr(sectionOffsets[0], sectionLen);
				}

				// Generate the Special Effect Section
				string specialEffectSection = "\x82";
				unsigned int numEffects = effects.size(), numEntries = 0, headerLen = 0, effectOffset;
				specialEffectSection.append(15, 0);
				for (unsigned int i = 0; i < numEffects; i++)
				{
					string e = effects[i];
					unsigned int eLen = e.size();
					if (eLen >= 8)
					{
						if (e.substr(0, 4) == "SPFX")
						{
							unsigned int numEntriesI = *(unsigned int *)&(e[4]);
							if (eLen >= ((numEntriesI * 28) + 8))
							{
								for (unsigned int j = 0; j < numEntriesI; j++)
								{
									specialEffectSection.append(4, 0);											// Effect Offset (will update later)
									specialEffectSection += e.substr((j * 28) + 8, 28);							// Effect Meta Data
									numEntries++;
								}
							}
							else
							{
								cout << "Error: Effect " << i << " is missing effect meta data" << endl;
							}
						}
						else
						{
							cout << "Error: Effect " << i << " is not a .spi file" << endl;
						}
					}
					else
					{
						cout << "Error: Effect " << i << " is not long enough to be a .spi file" << endl;
					}
				}
				// Update the number of entries
				*(unsigned int *)&(specialEffectSection[12]) = numEntries;
				// Append the number of effects to the section
				specialEffectSection.append((char *)&(numEffects), sizeof(unsigned int));
				// Determine what the length of the header will be
				headerLen = specialEffectSection.size() + (numEffects * 4) + 96;
				remainder = headerLen % 16;
				if (remainder > 0)
				{
					headerLen += (16 - remainder);
				}
				effectOffset = headerLen;
				readOffset = 16;
				for (unsigned int i = 0; i < numEffects; i++)
				{
					string e = effects[i];
					unsigned int eLen = e.size();
					if (eLen >= 8)
					{
						unsigned int numEntriesI = *(unsigned int *)&(e[4]), effectStart = ((numEntriesI * 28) + 8), dLen = eLen;
						remainder = effectStart % 16;
						if (remainder > 0)
						{
							effectStart += (16 - remainder);
						}
						// Determine the length of just the effect data (excludes the meta data)
						dLen -= effectStart;
						// Update each entry offset
						for (unsigned int j = 0; j < numEntriesI; j++)
						{
							*(unsigned int *)&(specialEffectSection[readOffset]) = effectOffset;
							readOffset += 32;
						}
						// Append the effect offset
						specialEffectSection.append((char *)&(effectOffset), sizeof(unsigned int));
						// Update the effect offset
						effectOffset += dLen;
					}
				}
				// Check if we need padding to word align the header
				remainder = specialEffectSection.size() % 16;
				if (remainder > 0)
				{
					// Append padding
					specialEffectSection.append(16 - remainder, 0);
				}
				// Append the end of header stuff
				specialEffectSection.append(64, 0);
				specialEffectSection.append(32, (char)255);
				// Append each effect
				for (unsigned int i = 0; i < numEffects; i++)
				{
					string e = effects[i];
					unsigned int eLen = e.size();
					if (eLen >= 8)
					{
						unsigned int numEntriesI = *(unsigned int *)&(e[4]), effectStart = ((numEntriesI * 28) + 8), dLen = eLen;
						remainder = effectStart % 16;
						if (remainder > 0)
						{
							effectStart += (16 - remainder);
						}
						// Determine the length of just the effect data (excludes the meta data)
						dLen -= effectStart;
						// Append the effect data
						specialEffectSection += e.substr(effectStart, dLen);
					}
				}
				// Check if we need to add padding between sections
				remainder = specialEffectSection.size() % 128;
				if (remainder > 0)
				{
					// Add padding
					specialEffectSection.append(128 - remainder, 0);
				}
				// Update the offset to the start of the special effect section
				*(unsigned int *)&(updatedMdls[8]) = updatedMdls.size();
				// Append the special effect section to the updated mdls
				updatedMdls += specialEffectSection;

				readOffset = 12;
				// Copy each remaining section from the original and update the offset to the section
				for (unsigned int i = 2; i < numSections; i++)
				{
					unsigned int sectionLen;
					if (i < (numSections - 1))
					{
						sectionLen = sectionOffsets[i + 1] - sectionOffsets[i];
					}
					else
					{
						if (sectionOffsets[i] < raw.size())
						{
							sectionLen = raw.size() - sectionOffsets[i];
						}
						else
						{
							sectionLen = 0;
						}
					}
					// Update the offset to section i
					*(unsigned int *)&(updatedMdls[readOffset]) = updatedMdls.size();
					readOffset += 4;
					// Copy the section
					if (sectionLen > 0)
					{
						updatedMdls += raw.substr(sectionOffsets[i], sectionLen);
					}
				}
				// Update the mdls (fake) length field
				remainder = updatedMdls.size() % 128;
				fakeSectionOff = updatedMdls.size();
				if (remainder != 0)
				{
					fakeSectionOff += 128 - remainder;
				}
				*(unsigned int *)&(updatedMdls[readOffset]) = fakeSectionOff;
				readOffset -= 4;
				for (unsigned int i = numSections - 1; i < numSections; i--)
				{
					unsigned int *sectionOff = (unsigned int *)&(updatedMdls[readOffset]);
					if (*sectionOff == updatedMdls.size())
					{
						*sectionOff = fakeSectionOff;
						readOffset -= 4;
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				cout << "Error: The mdls file is invalid! It doesn't contain the section offsets" << endl;
			}
		}
		else
		{
			cout << "Error: The mdls file is invalid! It doesn't contain the correct number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: The mdls file is invalid! It doesn't contain the number of sections" << endl;
	}
	return updatedMdls;
}

string setWpnSpecialEffects(string raw, vector<string> effects)
{
	string updatedWpn = "";
	unsigned int wpnLen = raw.size();
	// Make sure the mdls data contains the number of sections
	if (wpnLen >= 4)
	{
		unsigned int readOffset = 0, numSections = *(unsigned int *)&(raw[readOffset]), sectionLen, remainder, fakeSectionOff;
		readOffset += 4;
		// Make sure the mdls data contains the proper number of sections
		if (numSections == 2)
		{
			// Make sure the mdls data contains the section offsets
			if (wpnLen >= 16)
			{
				// Get the section offsets
				unsigned int sectionOffsets[2];
				for (unsigned int i = 0; i < numSections; i++)
				{
					sectionOffsets[i] = *(unsigned int *)&(raw[readOffset]);
					readOffset += 4;
				}
				// Copy the wpn header into the return value (We update the offsets later)
				updatedWpn = raw.substr(0, sectionOffsets[0]);
				// Make sure the mdls data contains a menv header
				if (wpnLen >= (sectionOffsets[0] + 16))
				{
					// Generate the special effect section
					string specialEffectSection = "\x82";
					unsigned int numEffects = effects.size(), numEntries = 0, headerLen = 0, effectOffset;
					specialEffectSection.append(15, 0);
					for (unsigned int i = 0; i < numEffects; i++)
					{
						string e = effects[i];
						unsigned int eLen = e.size();
						if (eLen >= 8)
						{
							if (e.substr(0, 4) == "SPFX")
							{
								unsigned int numEntriesI = *(unsigned int *)&(e[4]);
								if (eLen >= ((numEntriesI * 28) + 8))
								{
									for (unsigned int j = 0; j < numEntriesI; j++)
									{
										specialEffectSection.append(4, 0);											// Effect Offset (will update later)
										specialEffectSection += e.substr((j * 28) + 8, 28);							// Effect Meta Data
										numEntries++;
									}
								}
								else
								{
									cout << "Error: Effect " << i << " is missing effect meta data" << endl;
								}
							}
							else
							{
								cout << "Error: Effect " << i << " is not a .spi file" << endl;
							}
						}
						else
						{
							cout << "Error: Effect " << i << " is not long enough to be a .spi file" << endl;
						}
					}
					// Update the number of entries
					*(unsigned int *)&(specialEffectSection[12]) = numEntries;
					// Append the number of effects to the section
					specialEffectSection.append((char *)&(numEffects), sizeof(unsigned int));
					// Determine what the length of the header will be
					headerLen = specialEffectSection.size() + (numEffects * 4) + 96;
					remainder = headerLen % 16;
					if (remainder > 0)
					{
						headerLen += (16 - remainder);
					}
					effectOffset = headerLen;
					readOffset = 16;
					for (unsigned int i = 0; i < numEffects; i++)
					{
						string e = effects[i];
						unsigned int eLen = e.size();
						if (eLen >= 8)
						{
							unsigned int numEntriesI = *(unsigned int *)&(e[4]), effectStart = ((numEntriesI * 28) + 8), dLen = eLen;
							remainder = effectStart % 16;
							if (remainder > 0)
							{
								effectStart += (16 - remainder);
							}
							// Determine the length of just the effect data (excludes the meta data)
							dLen -= effectStart;
							// Update each entry offset
							for (unsigned int j = 0; j < numEntriesI; j++)
							{
								*(unsigned int *)&(specialEffectSection[readOffset]) = effectOffset;
								readOffset += 32;
							}
							// Append the effect offset
							specialEffectSection.append((char *)&(effectOffset), sizeof(unsigned int));
							// Update the effect offset
							effectOffset += dLen;
						}
					}
					// Check if we need padding to word align the header
					remainder = specialEffectSection.size() % 16;
					if (remainder > 0)
					{
						// Append padding
						specialEffectSection.append(16 - remainder, 0);
					}
					// Append the end of header stuff
					specialEffectSection.append(64, 0);
					specialEffectSection.append(32, (char)255);
					// Append each effect
					for (unsigned int i = 0; i < numEffects; i++)
					{
						string e = effects[i];
						unsigned int eLen = e.size();
						if (eLen >= 8)
						{
							unsigned int numEntriesI = *(unsigned int *)&(e[4]), effectStart = ((numEntriesI * 28) + 8), dLen = eLen;
							remainder = effectStart % 16;
							if (remainder > 0)
							{
								effectStart += (16 - remainder);
							}
							// Determine the length of just the effect data (excludes the meta data)
							dLen -= effectStart;
							// Append the effect data
							specialEffectSection += e.substr(effectStart, dLen);
						}
					}
					// Check if we need to add padding between sections
					remainder = specialEffectSection.size() % 128;
					if (remainder > 0)
					{
						// Add padding
						specialEffectSection.append(128 - remainder, 0);
					}

					// Update the offset to the special effect section
					readOffset = 4;
					*(unsigned int *)&(updatedWpn[readOffset]) = updatedWpn.size();
					// Append the generated special effect section and update the offset to it
					updatedWpn += specialEffectSection;

					// Copy the menv section from the original and update the offset to the section
					sectionLen = wpnLen - sectionOffsets[1];
					// Update the offset to section i
					readOffset = 8;
					*(unsigned int *)&(updatedWpn[readOffset]) = updatedWpn.size();
					// Copy the section
					if (sectionLen > 0)
					{
						updatedWpn += raw.substr(sectionOffsets[1], sectionLen);
					}
					// Reset the offset to the wpn (fake) length field
					readOffset = 12;
					// Update the wpn (fake) length field
					remainder = updatedWpn.size() % 128;
					fakeSectionOff = updatedWpn.size();
					if (remainder > 0)
					{
						fakeSectionOff += 128 - remainder;
					}
					*(unsigned int *)&(updatedWpn[readOffset]) = fakeSectionOff;
					readOffset -= 4;
					for (unsigned int i = numSections - 1; i < numSections; i--)
					{
						unsigned int *sectionOff = (unsigned int *)&(updatedWpn[readOffset]);
						if (*sectionOff == updatedWpn.size())
						{
							*sectionOff = fakeSectionOff;
							readOffset -= 4;
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					cout << "Error: The wpn file is invalid! It doesn't contain the menv header" << endl;
				}
			}
			else
			{
				cout << "Error: The wpn file is invalid! It doesn't contain the section offsets" << endl;
			}
		}
		else
		{
			cout << "Error: The wpn file is invalid! It doesn't contain the correct number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: The wpn file is invalid! It doesn't contain the number of sections" << endl;
	}
	return updatedWpn;
}

string setMdlsTextures(string raw, vector<Texture> textures)
{
	string updatedMdls = "";
	unsigned int mdlsLen = raw.size();
	// Make sure the mdls data contains the number of sections
	if (mdlsLen >= 4)
	{
		unsigned int readOffset = 0, numSections = *(unsigned int *)&(raw[readOffset]);
		readOffset += 4;
		// Make sure the mdls data contains the proper number of sections
		if (numSections == 8)
		{
			// Make sure the mdls data contains the section offsets
			if (mdlsLen >= 128)
			{
				// Get the section offsets
				unsigned int sectionOffsets[8];
				for (unsigned int i = 0; i < numSections; i++)
				{
					sectionOffsets[i] = *(unsigned int *)&(raw[readOffset]);
					readOffset += 4;
				}
				// Copy the mdls header into the return value (We update the offsets later)
				updatedMdls = raw.substr(0, 128);
				// Make sure the mdls data contains a mobj header
				if (mdlsLen >= (sectionOffsets[0] + mobjHeaderLen))
				{
					unsigned int mobjLen;
					// Move to the mobj length field
					readOffset = sectionOffsets[0] + 4;
					// Extract the length of the mobj
					mobjLen = *(unsigned int *)&(raw[readOffset]) + 8;
					readOffset += 4;
					// Make sure the mdls data contains the entire mobj section
					if (mdlsLen >= (sectionOffsets[0] + mobjLen))
					{
						// Get the mobj section from the mdls
						string mobj = raw.substr(sectionOffsets[0], mobjLen), updatedMobj = "";
						// Get the subsection offsets
						unsigned int subsectionOffsets[5], subsectionLengths[5], numTextures = textures.size(), numTexturesOriginal, remainder, updatedSubsectionLen, fakeSectionOff;
						// Reset the read offset since we isolated the mobj section
						readOffset = 8;
						for (unsigned int i = 2; i < 5; i++)
						{
							subsectionOffsets[i] = *(unsigned int *)&(mobj[readOffset]);
							readOffset += 4;
							subsectionLengths[i] = *(unsigned int *)&(mobj[readOffset]);
							readOffset += 4;
						}
						for (unsigned int i = 0; i < 2; i++)
						{
							subsectionOffsets[i] = *(unsigned int *)&(mobj[readOffset]);
							readOffset += 4;
							subsectionLengths[i] = *(unsigned int *)&(mobj[readOffset]);
							readOffset += 4;
						}
						// Copy the header from the original mobj (We update the offsets and lengths later)
						updatedMobj = mobj.substr(0, mobjHeaderLen);
						// Copy the subsections 0 and 1 from the original mobj
						for (unsigned int i = 0; i < 2; i++)
						{
							// Make sure the data for the section exists
							if (mobjLen >= (subsectionOffsets[i] + subsectionLengths[i]) && subsectionLengths[i] > 0)
							{
								// Copy the data for this section
								updatedMobj += mobj.substr(subsectionOffsets[i], subsectionLengths[i]);
							}
						}
						// Reset the readOffset back to the length of subsection 2
						readOffset = 12;
						// Generate the texture info subsection
						for (unsigned int i = 0; i < numTextures; i++)
						{
							unsigned short paletteLen = textures[i].palette.numColors * 4, height = textures[i].height, hExp = textures[i].hExp, width = textures[i].width, wExp = textures[i].wExp;
							updatedMobj.append((char *)&(paletteLen), sizeof(unsigned short));
							updatedMobj.append((char *)&(wExp), sizeof(unsigned char));
							updatedMobj.append((char *)&(hExp), sizeof(unsigned char));
							updatedMobj.append((char *)&(width), sizeof(unsigned short));
							updatedMobj.append((char *)&(height), sizeof(unsigned short));
							updatedMobj.append(8, 0); // Padding
						}
						// Update the subsection 2 length
						updatedSubsectionLen = numTextures * 16;
						*(unsigned int *)&(updatedMobj[readOffset]) = updatedSubsectionLen;
						readOffset += 4;
						// Determine the number of textures in the original mobj
						numTexturesOriginal = subsectionLengths[2] / 16;
						// Check if the number of new textures is less than the original number of textures
						if (numTextures < numTexturesOriginal)
						{
							// Display a warning to make sure the user didn't mess up
							cout << "Warning: The original MDLS contained more textures than were provided!" << endl;
							cout << "         If this is unexpected, then the output model is invalid." << endl;
						}
						// Update the padding offset
						*(unsigned int *)&(updatedMobj[48]) = updatedMobj.size();
						// Add padding (because MDLS is special)
						remainder = updatedMobj.size() % 128;
						updatedMobj.append(128 - remainder, 0);
						// Update the subsection 3 offset
						*(unsigned int *)&(updatedMobj[readOffset]) = updatedMobj.size();
						readOffset += 4;
						// Initialize the subsection length to 0
						updatedSubsectionLen = 0;
						// Generate the texture indices subsection
						for (unsigned int i = 0; i < numTextures; i++)
						{
							// Increment the subsection length by the number of indices in texture i
							updatedSubsectionLen += textures[i].indices.size();
							for (unsigned int j = 0; j < textures[i].indices.size(); j++)
							{
								// Get the color index for pixel (i, j)
								unsigned char index = textures[i].indices[j];
								// Use modulo function to obfuscate the color index
								remainder = index % 32;
								if (remainder >= 8 && remainder < 16)
								{
									index += 8;
								}
								else if (remainder >= 16 && remainder < 24)
								{
									index -= 8;
								}
								// Append the obfuscated color index
								updatedMobj.append(1, index);
							}
						}
						// Update the texture indices section length
						*(unsigned int *)&(updatedMobj[readOffset]) = updatedSubsectionLen;
						readOffset += 4;
						// Update the subsection 4 offset
						*(unsigned int *)&(updatedMobj[readOffset]) = updatedMobj.size();
						readOffset += 4;
						// Reset the subsection length
						updatedSubsectionLen = 0;
						// Generate the texture palette subsection
						for (unsigned int i = 0; i < numTextures; i++)
						{
							updatedSubsectionLen += textures[i].palette.numColors * 4;
							for (unsigned int j = 0; j < textures[i].palette.numColors; j++)
							{
								// Scale the alpha value to the range [0, 128]
								unsigned char alpha = textures[i].palette.a[j];
								if (alpha == 255)
								{
									alpha = 128;
								}
								else
								{
									alpha /= 2;
								}
								updatedMobj.append(1, textures[i].palette.r[j]);
								updatedMobj.append(1, textures[i].palette.g[j]);
								updatedMobj.append(1, textures[i].palette.b[j]);
								updatedMobj.append(1, alpha);
							}
						}
						// Update the subsection 4 length
						*(unsigned int *)&(updatedMobj[readOffset]) = updatedSubsectionLen;
						// Update the mobj length
						*(unsigned int *)&(updatedMobj[4]) = updatedMobj.size() - 8;
						// Reset the read offset to the section 1 offset location
						readOffset = 4;
						// Copy the mobj section to the return value and update the meta data
						*(unsigned int *)&(updatedMdls[readOffset]) = updatedMdls.size();
						readOffset += 4;
						updatedMdls += updatedMobj;
						// Add the KN5 Header
						updatedMdls.append("_KN5", 4);
						updatedMdls.append(124, 0);
						// Copy each remaining section from the original and update the offset to the section
						for (unsigned int i = 1; i < numSections; i++)
						{
							unsigned int sectionLen;
							if (i < (numSections - 1))
							{
								sectionLen = sectionOffsets[i + 1] - sectionOffsets[i];
							}
							else
							{
								if (sectionOffsets[i] < raw.size())
								{
									sectionLen = raw.size() - sectionOffsets[i];
								}
								else
								{
									sectionLen = 0;
								}
							}
							// Update the offset to section i
							*(unsigned int *)&(updatedMdls[readOffset]) = updatedMdls.size();
							readOffset += 4;
							// Copy the section
							if (sectionLen > 0)
							{
								updatedMdls += raw.substr(sectionOffsets[i], sectionLen);
							}
						}
						// Update the mdls (fake) length field
						remainder = updatedMdls.size() % 128;
						fakeSectionOff = updatedMdls.size();
						if (remainder != 0)
						{
							fakeSectionOff += 128 - remainder;
						}
						*(unsigned int *)&(updatedMdls[readOffset]) = fakeSectionOff;
						readOffset -= 4;
						for (unsigned int i = numSections - 1; i < numSections; i--)
						{
							unsigned int *sectionOff = (unsigned int *)&(updatedMdls[readOffset]);
							if (*sectionOff == updatedMdls.size())
							{
								*sectionOff = fakeSectionOff;
								readOffset -= 4;
							}
							else
							{
								break;
							}
						}
					}
					else
					{
						cout << "Error: The mdls file is invalid! It doesn't contain the mobj section" << endl;
					}
				}
				else
				{
					cout << "Error: The mdls file is invalid! It doesn't contain the mobj header" << endl;
				}
			}
			else
			{
				cout << "Error: The mdls file is invalid! It doesn't contain the section offsets" << endl;
			}
		}
		else
		{
			cout << "Error: The mdls file is invalid! It doesn't contain the correct number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: The mdls file is invalid! It doesn't contain the number of sections" << endl;
	}
	return updatedMdls;
}

string setWpnTextures(string raw, vector<Texture> textures)
{
	string updatedWpn = "";
	unsigned int wpnLen = raw.size();
	// Make sure the mdls data contains the number of sections
	if (wpnLen >= 4)
	{
		unsigned int readOffset = 0, numSections = *(unsigned int *)&(raw[readOffset]), texaLen = 0;
		readOffset += 4;
		// Make sure the mdls data contains the proper number of sections
		if (numSections == 2)
		{
			// Make sure the mdls data contains the section offsets
			if (wpnLen >= 16)
			{
				// Get the section offsets
				unsigned int sectionOffsets[2];
				for (unsigned int i = 0; i < numSections; i++)
				{
					sectionOffsets[i] = *(unsigned int *)&(raw[readOffset]);
					readOffset += 4;
				}
				// Copy the wpn header into the return value (We update the offsets later)
				updatedWpn = raw.substr(0, sectionOffsets[0]);
				// Make sure the mdls data contains a menv header
				if (wpnLen >= (sectionOffsets[1] + menvHeaderLen))
				{
					unsigned int menvLen;
					// Move to the mobj length field
					readOffset = sectionOffsets[1] + 4;
					// Extract the length of the mobj
					menvLen = *(unsigned int *)&(raw[readOffset]) + 8;
					readOffset += 4;
					// Make sure the mdls data contains the entire menv section
					if (wpnLen >= (sectionOffsets[1] + menvLen))
					{
						// Get the mobj section from the mdls
						string menv = raw.substr(sectionOffsets[1], menvLen), updatedMenv = "";
						// Get the subsection offsets
						unsigned int subsectionOffsets[5], subsectionLengths[5], numTextures = textures.size(), numTexturesOriginal, remainder, updatedSubsectionLen, fakeSectionOff;
						// Reset the read offset since we isolated the mobj section
						readOffset = 8;
						for (unsigned int i = 2; i < 5; i++)
						{
							subsectionOffsets[i] = *(unsigned int *)&(menv[readOffset]);
							readOffset += 4;
							subsectionLengths[i] = *(unsigned int *)&(menv[readOffset]);
							readOffset += 4;
						}
						for (unsigned int i = 0; i < 2; i++)
						{
							subsectionOffsets[i] = *(unsigned int *)&(menv[readOffset]);
							readOffset += 4;
							subsectionLengths[i] = *(unsigned int *)&(menv[readOffset]);
							readOffset += 4;
						}
						// Copy the header from the original mobj (We update the offsets and lengths later)
						updatedMenv = menv.substr(0, menvHeaderLen);
						// Copy the subsections 0 and 1 from the original mobj
						for (unsigned int i = 0; i < 2; i++)
						{
							// Make sure the data for the section exists
							if (menvLen >= (subsectionOffsets[i] + subsectionLengths[i]) && (subsectionLengths[i] > 0))
							{
								// Copy the data for this section
								updatedMenv += menv.substr(subsectionOffsets[i], subsectionLengths[i]);
							}
						}
						// Reset the readOffset back to the length of subsection 2
						readOffset = 12;
						// Generate the texture info subsection
						for (unsigned int i = 0; i < numTextures; i++)
						{
							unsigned short paletteLen = textures[i].palette.numColors * 4, height = textures[i].height, hExp = textures[i].hExp, width = textures[i].width, wExp = textures[i].wExp;
							updatedMenv.append((char *)&(paletteLen), sizeof(unsigned short));
							updatedMenv.append((char *)&(wExp), sizeof(unsigned char));
							updatedMenv.append((char *)&(hExp), sizeof(unsigned char));
							updatedMenv.append((char *)&(width), sizeof(unsigned short));
							updatedMenv.append((char *)&(height), sizeof(unsigned short));
							updatedMenv.append(8, 0); // Padding
						}
						// Update the subsection 2 length
						updatedSubsectionLen = numTextures * 16;
						*(unsigned int *)&(updatedMenv[readOffset]) = updatedSubsectionLen;
						readOffset += 4;
						// Determine the number of textures in the original mobj
						numTexturesOriginal = subsectionLengths[2] / 16;
						// Check if the number of new textures is less than the original number of textures
						if (numTextures < numTexturesOriginal)
						{
							// Display a warning to make sure the user didn't mess up
							cout << "Warning: The original WPN contained more textures than were provided!" << endl;
							cout << "         If this is unexpected, then the output model is invalid." << endl;
						}
						// Update the padding offset
						*(unsigned int *)&(updatedMenv[48]) = updatedMenv.size();
						// Add padding (because WPN is special)
						remainder = updatedMenv.size() % 128;
						updatedMenv.append(128 - remainder, 0);
						// Update the subsection 3 offset
						*(unsigned int *)&(updatedMenv[readOffset]) = updatedMenv.size();
						readOffset += 4;
						// Initialize the subsection length to 0
						updatedSubsectionLen = 0;
						// Generate the texture indices subsection
						for (unsigned int i = 0; i < numTextures; i++)
						{
							// Increment the subsection length by the number of indices in texture i
							updatedSubsectionLen += textures[i].indices.size();
							for (unsigned int j = 0; j < textures[i].indices.size(); j++)
							{
								// Get the color index for pixel (i, j)
								unsigned char index = textures[i].indices[j];
								// Use modulo function to obfuscate the color index
								remainder = index % 32;
								if (remainder >= 8 && remainder < 16)
								{
									index += 8;
								}
								else if (remainder >= 16 && remainder < 24)
								{
									index -= 8;
								}
								// Append the obfuscated color index
								updatedMenv.append(1, index);
							}
						}
						// Update the texture indices section length
						*(unsigned int *)&(updatedMenv[readOffset]) = updatedSubsectionLen;
						readOffset += 4;
						// Update the subsection 4 offset
						*(unsigned int *)&(updatedMenv[readOffset]) = updatedMenv.size();
						readOffset += 4;
						// Reset the subsection length
						updatedSubsectionLen = 0;
						// Generate the texture palette subsection
						for (unsigned int i = 0; i < numTextures; i++)
						{
							updatedSubsectionLen += textures[i].palette.numColors * 4;
							for (unsigned int j = 0; j < textures[i].palette.numColors; j++)
							{
								// Scale the alpha value to the range [0, 128]
								unsigned char alpha = textures[i].palette.a[j];
								if (alpha == 255)
								{
									alpha = 128;
								}
								else
								{
									alpha /= 2;
								}
								updatedMenv.append(1, textures[i].palette.r[j]);
								updatedMenv.append(1, textures[i].palette.g[j]);
								updatedMenv.append(1, textures[i].palette.b[j]);
								updatedMenv.append(1, alpha);
							}
						}
						// Update the subsection 4 length
						*(unsigned int *)&(updatedMenv[readOffset]) = updatedSubsectionLen;
						// Update the menv length
						*(unsigned int *)&(updatedMenv[4]) = updatedMenv.size() - 8;
						// Reset the read offset to the section 1 offset location
						readOffset = 4;
						// Copy the special effect section from the original and update the offset to the section
						for (unsigned int i = 0; i < (numSections - 1); i++)
						{
							unsigned int sectionLen = sectionOffsets[i + 1] - sectionOffsets[i];
							// Update the offset to section i
							*(unsigned int *)&(updatedWpn[readOffset]) = updatedWpn.size();
							readOffset += 4;
							// Copy the section
							if (sectionLen > 0)
							{
								updatedWpn += raw.substr(sectionOffsets[i], sectionLen);
							}
						}
						// Copy the menv section to the return value and update the meta data
						*(unsigned int *)&(updatedWpn[readOffset]) = updatedWpn.size();
						readOffset += 4;
						updatedWpn += updatedMenv;
						// Get the offset to the start of the TEXA section
						readOffset = sectionOffsets[numSections - 1] + menv.length();
						if (wpnLen >= (readOffset + 8))
						{
							// Get the length of the TEXA section
							texaLen = *(unsigned int *)&(raw[readOffset + 4]) + 8;
							if (wpnLen >= (readOffset + texaLen))
							{
								updatedWpn += raw.substr(readOffset, texaLen);
							}
							else
							{
								cout << "Error: Invalid WPN - Invalid TEXA length" << endl;
							}
						}
						else
						{
							cout << "Error: Invalid WPN - Missing TEXA section" << endl;
						}
						// Add the KN5 Header
						updatedWpn.append("_KN5", 4);
						// Reset the offset to the wpn (fake) length field
						readOffset = 12;
						// Update the wpn (fake) length field
						remainder = updatedWpn.size() % 128;
						fakeSectionOff = updatedWpn.size();
						if (remainder > 0)
						{
							fakeSectionOff += 128 - remainder;
						}
						*(unsigned int *)&(updatedWpn[readOffset]) = fakeSectionOff;
						readOffset -= 4;
						for (unsigned int i = numSections - 1; i < numSections; i--)
						{
							unsigned int *sectionOff = (unsigned int *)&(updatedWpn[readOffset]);
							if (*sectionOff == (updatedWpn.size() - texaLen - 4))
							{
								*sectionOff = fakeSectionOff;
								readOffset -= 4;
							}
							else
							{
								break;
							}
						}
					}
					else
					{
						cout << "Error: The wpn file is invalid! It doesn't contain the menv section" << endl;
					}
				}
				else
				{
					cout << "Error: The wpn file is invalid! It doesn't contain the menv header" << endl;
				}
			}
			else
			{
				cout << "Error: The wpn file is invalid! It doesn't contain the section offsets" << endl;
			}
		}
		else
		{
			cout << "Error: The wpn file is invalid! It doesn't contain the correct number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: The wpn file is invalid! It doesn't contain the number of sections" << endl;
	}
	return updatedWpn;
}

vector<Texture> getMdlsSpecialEffectTextures(string raw)
{
	vector<Texture> textures;
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16, numSubsections;
	string specialEffectSection;
	// Make sure there is data to read
	if (raw.size() >= (readOffset + 4))
	{
		// Get the number of sections
		numSections = *(unsigned int *)&(raw[readOffset]);
		// Make sure this file has the correct number of sections
		if (numSections == 8)
		{
			// Update the read offset
			readOffset = 8;
			// Make sure there is data to read
			if (raw.size() >= 40)
			{
				// Get the offset to the Special Effects section
				sectionOffset = *(unsigned int *)&(raw[readOffset]);
				// Move to the HUD Image section offset
				readOffset += 4;
				// Determine the section length
				sectionLen = (*(unsigned int *)&(raw[readOffset])) - sectionOffset;
				// Make sure the section isn't empty
				if (sectionLen > 0)
				{
					// Make sure there is data to read and that there is a section header
					if (raw.size() >= (sectionOffset + sectionLen) && (sectionLen >= 16))
					{
						specialEffectSection = raw.substr(sectionOffset, sectionLen);
						// Move to the start of the section
						readOffset = 0;
						// Check the section id
						if (*(unsigned int *)&(specialEffectSection[readOffset]) == 130)
						{
							// Move to number of subsection type 0 entries
							readOffset += 12;
							readOffset += ((*(unsigned int *)&(specialEffectSection[readOffset])) * 32) + 4;
							if (sectionLen >= (readOffset + 4))
							{
								vector<unsigned int> subsectionOffsets;
								numSubsections = *(unsigned int *)&(specialEffectSection[readOffset]);
								readOffset += 4;
								subsectionOffsets.reserve(numSubsections);
								if (sectionLen >= (readOffset + (numSubsections * 4)))
								{
									// Store each subsection offset
									for (unsigned int i = 0; i < numSubsections; i++)
									{
										subsectionOffsets.push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
										readOffset += 4;
									}
									// Parse each subsection
									for (unsigned int i = 0; i < numSubsections; i++)
									{
										// Move to the start of the subsection
										readOffset = subsectionOffsets[i];
										// Make sure the offset is valid
										if (sectionLen >= (readOffset + 4))
										{
											if (*(unsigned int *)&(specialEffectSection[readOffset]) == 150)
											{
												vector<vector<unsigned int>> subsubsectionOffsets;
												subsubsectionOffsets.reserve(5);
												// Move to the number of type 0 subsubsections
												readOffset += 4;
												// For each subsubsection type
												for (unsigned int j = 0; j < 5; j++)
												{
													if (sectionLen >= (readOffset + 4))
													{
														unsigned int numSubsubsections = *(unsigned int *)&(specialEffectSection[readOffset]);
														readOffset += 4;
														subsubsectionOffsets.push_back(vector<unsigned int>());
														subsubsectionOffsets[j].reserve(numSubsubsections);
														if (sectionLen >= (readOffset + (numSubsubsections * 4)))
														{
															for (unsigned int k = 0; k < numSubsubsections; k++)
															{
																subsubsectionOffsets[j].push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
																readOffset += 4;
															}
														}
														else
														{
															cout << "Error: Invalid number of subsubsections in subsection " << i << endl;
														}
													}
													else
													{
														cout << "Error: Invalid Subsection " << i << endl;
													}
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[0].size(); j++)
												{
													// To-Do: Parse Type 0 Subsubsections
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[1].size(); j++)
												{
													// To-Do: Parse Type 1 Subsubsections
													// Move to the start of the subsubsection
													readOffset = subsectionOffsets[i] + subsubsectionOffsets[1][j];
													// Make sure there is enough data for the header
													if (sectionLen >= (readOffset + 32))
													{
														Texture t;
														unsigned int x, indsLen;
														unsigned short paletteLen = 0x400;
														t.width = *(unsigned short *)&(specialEffectSection[readOffset + 12]);
														x = 0;
														while ((t.width >> x) > 1)
														{
															x++;
														}
														t.wExp = x;
														t.height = *(unsigned short *)&(specialEffectSection[readOffset + 14]);
														x = 0;
														while ((t.height >> x) > 1)
														{
															x++;
														}
														t.hExp = x;
														indsLen = *(unsigned int *)&(specialEffectSection[readOffset + 27]) & 0xFFFFFF;
														t.indices.reserve(indsLen);
														paletteLen = *(unsigned short *)&(specialEffectSection[readOffset + 29]);
														t.palette.numColors = paletteLen / 4;
														t.palette.r.reserve(t.palette.numColors);
														t.palette.g.reserve(t.palette.numColors);
														t.palette.b.reserve(t.palette.numColors);
														t.palette.a.reserve(t.palette.numColors);
														readOffset += 32;
														// Make sure the data for the indices exists
														if (sectionLen >= (readOffset + indsLen))
														{
															// Get each index
															for (unsigned int k = 0; k < indsLen; k++)
															{
																unsigned char ind = (unsigned char)specialEffectSection[readOffset + k], remainder = ind % 32;
																if (remainder >= 8 && remainder < 16)
																{
																	ind += 8;
																}
																else if (remainder >= 16 && remainder < 24)
																{
																	ind -= 8;
																}
																t.indices.push_back(ind);
															}
															readOffset += indsLen;
															// Make sure the data for the palette exists
															if (sectionLen >= (readOffset + paletteLen))
															{
																for (unsigned int k = 0; k < t.palette.numColors; k++)
																{
																	unsigned char colorVal = (unsigned char)specialEffectSection[readOffset + (k * 4)];
																	t.palette.r.push_back(colorVal);
																	colorVal = (unsigned char)specialEffectSection[readOffset + (k * 4) + 1];
																	t.palette.g.push_back(colorVal);
																	colorVal = (unsigned char)specialEffectSection[readOffset + (k * 4) + 2];
																	t.palette.b.push_back(colorVal);
																	colorVal = (unsigned char)specialEffectSection[readOffset + (k * 4) + 3];
																	if (colorVal > 0)
																	{
																		colorVal = (colorVal * 2 - 1);
																	}
																	t.palette.a.push_back(colorVal);
																}
																// Add the image to the vector to return
																textures.push_back(t);
															}
															else
															{
																cout << "Error: Missing palette colors in subsubsection" << endl;
															}
														}
														else
														{
															cout << "Error: Missing image indices in subsubsection" << endl;
														}
													}
													else
													{
														cout << "Error: Missing Subsubsection header" << endl;
													}
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[2].size(); j++)
												{
													// To-Do: Parse Type 2 Subsubsections
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[3].size(); j++)
												{
													// To-Do: Parse Type 3 Subsubsections
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[4].size(); j++)
												{
													// To-Do: Parse Type 4 subsubsections
												}
											}
											else
											{
												cout << "Error: Subsection ID is incorrect" << endl;
											}
										}
										else
										{
											cout << "Error: Invalid subsection offset for subsection " << i << endl;
										}
									}									
								}
							}
							else
							{
								cout << "Error: Section Header isn't complete" << endl;
							}
						}
						else
						{
							cout << "Error: Section ID is incorrect" << endl;
						}
					}
					else
					{
						cout << "Error: Invalid section length" << endl;
					}
				}
			}
			else
			{
				cout << "Error: Invalid MDLS - Missing full header" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid MDLS - Incorrect number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: Invalid MDLS - Missing number of sections" << endl;
	}
	return textures;
}

vector<Texture> getWpnSpecialEffectTextures(string raw)
{
	vector<Texture> textures;
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16, numSubsections;
	string specialEffectSection;
	// Make sure there is data to read
	if (raw.size() >= (readOffset + 4))
	{
		// Get the number of sections
		numSections = *(unsigned int *)&(raw[readOffset]);
		// Make sure this file has the correct number of sections
		if (numSections == 2)
		{
			// Update the read offset
			readOffset = 4;
			// Make sure there is data to read
			if (raw.size() >= 16)
			{
				// Get the offset to the Special Effects section
				sectionOffset = *(unsigned int *)&(raw[readOffset]);
				// Move to the HUD Image section offset
				readOffset += 4;
				// Determine the section length
				sectionLen = (*(unsigned int *)&(raw[readOffset])) - sectionOffset;
				// Make sure the section isn't empty
				if (sectionLen > 0)
				{
					// Make sure there is data to read and that there is a section header
					if (raw.size() >= (sectionOffset + sectionLen) && (sectionLen >= 16))
					{
						specialEffectSection = raw.substr(sectionOffset, sectionLen);
						// Move to the start of the section
						readOffset = 0;
						// Check the section id
						if (*(unsigned int *)&(specialEffectSection[readOffset]) == 130)
						{
							// Move to number of subsection type 0 entries
							readOffset += 12;
							readOffset += ((*(unsigned int *)&(specialEffectSection[readOffset])) * 32) + 4;
							if (sectionLen >= (readOffset + 4))
							{
								vector<unsigned int> subsectionOffsets;
								numSubsections = *(unsigned int *)&(specialEffectSection[readOffset]);
								readOffset += 4;
								subsectionOffsets.reserve(numSubsections);
								if (sectionLen >= (readOffset + (numSubsections * 4)))
								{
									// Store each subsection offset
									for (unsigned int i = 0; i < numSubsections; i++)
									{
										subsectionOffsets.push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
										readOffset += 4;
									}
									// Parse each subsection
									for (unsigned int i = 0; i < numSubsections; i++)
									{
										// Move to the start of the subsection
										readOffset = subsectionOffsets[i];
										// Make sure the offset is valid
										if (sectionLen >= (readOffset + 4))
										{
											if (*(unsigned int *)&(specialEffectSection[readOffset]) == 150)
											{
												vector<vector<unsigned int>> subsubsectionOffsets;
												subsubsectionOffsets.reserve(5);
												// Move to the number of type 0 subsubsections
												readOffset += 4;
												// For each subsubsection type
												for (unsigned int j = 0; j < 5; j++)
												{
													if (sectionLen >= (readOffset + 4))
													{
														unsigned int numSubsubsections = *(unsigned int *)&(specialEffectSection[readOffset]);
														readOffset += 4;
														subsubsectionOffsets.push_back(vector<unsigned int>());
														subsubsectionOffsets[j].reserve(numSubsubsections);
														if (sectionLen >= (readOffset + (numSubsubsections * 4)))
														{
															for (unsigned int k = 0; k < numSubsubsections; k++)
															{
																subsubsectionOffsets[j].push_back(*(unsigned int *)&(specialEffectSection[readOffset]));
																readOffset += 4;
															}
														}
														else
														{
															cout << "Error: Invalid number of subsubsections in subsection " << i << endl;
														}
													}
													else
													{
														cout << "Error: Invalid Subsection " << i << endl;
													}
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[0].size(); j++)
												{
													// To-Do: Parse Type 0 Subsubsections
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[1].size(); j++)
												{
													// To-Do: Parse Type 1 Subsubsections
													// Move to the start of the subsubsection
													readOffset = subsectionOffsets[i] + subsubsectionOffsets[1][j];
													// Make sure there is enough data for the header
													if (sectionLen >= (readOffset + 32))
													{
														Texture t;
														unsigned int x, indsLen;
														unsigned short paletteLen = 0x400;
														t.width = *(unsigned short *)&(specialEffectSection[readOffset + 12]);
														x = 0;
														while ((t.width >> x) > 1)
														{
															x++;
														}
														t.wExp = x;
														t.height = *(unsigned short *)&(specialEffectSection[readOffset + 14]);
														x = 0;
														while ((t.height >> x) > 1)
														{
															x++;
														}
														t.hExp = x;
														indsLen = *(unsigned int *)&(specialEffectSection[readOffset + 27]) & 0xFFFFFF;
														t.indices.reserve(indsLen);
														paletteLen = *(unsigned short *)&(specialEffectSection[readOffset + 29]);
														t.palette.numColors = paletteLen / 4;
														t.palette.r.reserve(t.palette.numColors);
														t.palette.g.reserve(t.palette.numColors);
														t.palette.b.reserve(t.palette.numColors);
														t.palette.a.reserve(t.palette.numColors);
														readOffset += 32;
														// Make sure the data for the indices exists
														if (sectionLen >= (readOffset + indsLen))
														{
															// Get each index
															for (unsigned int k = 0; k < indsLen; k++)
															{
																unsigned char ind = (unsigned char)specialEffectSection[readOffset + k], remainder = ind % 32;
																if (remainder >= 8 && remainder < 16)
																{
																	ind += 8;
																}
																else if (remainder >= 16 && remainder < 24)
																{
																	ind -= 8;
																}
																t.indices.push_back(ind);
															}
															readOffset += indsLen;
															// Make sure the data for the palette exists
															if (sectionLen >= (readOffset + paletteLen))
															{
																for (unsigned int k = 0; k < t.palette.numColors; k++)
																{
																	unsigned char colorVal = (unsigned char)specialEffectSection[readOffset + (k * 4)];
																	t.palette.r.push_back(colorVal);
																	colorVal = (unsigned char)specialEffectSection[readOffset + (k * 4) + 1];
																	t.palette.g.push_back(colorVal);
																	colorVal = (unsigned char)specialEffectSection[readOffset + (k * 4) + 2];
																	t.palette.b.push_back(colorVal);
																	colorVal = (unsigned char)specialEffectSection[readOffset + (k * 4) + 3];
																	if (colorVal > 0)
																	{
																		colorVal = (colorVal * 2 - 1);
																	}
																	t.palette.a.push_back(colorVal);
																}
																// Add the image to the vector to return
																textures.push_back(t);
															}
															else
															{
																cout << "Error: Missing palette colors in subsubsection" << endl;
															}
														}
														else
														{
															cout << "Error: Missing image indices in subsubsection" << endl;
														}
													}
													else
													{
														cout << "Error: Missing Subsubsection header" << endl;
													}
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[2].size(); j++)
												{
													// To-Do: Parse Type 2 Subsubsections
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[3].size(); j++)
												{
													// To-Do: Parse Type 3 Subsubsections
												}
												for (unsigned int j = 0; j < subsubsectionOffsets[4].size(); j++)
												{
													// To-Do: Parse Type 4 subsubsections
												}
											}
											else
											{
												cout << "Error: Subsection ID is incorrect" << endl;
											}
										}
										else
										{
											cout << "Error: Invalid subsection offset for subsection " << i << endl;
										}
									}
								}
							}
							else
							{
								cout << "Error: Section Header isn't complete" << endl;
							}
						}
						else
						{
							cout << "Error: Section ID is incorrect" << endl;
						}
					}
					else
					{
						cout << "Error: Invalid section length" << endl;
					}
				}
			}
			else
			{
				cout << "Error: Invalid WPN - Missing full header" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid WPN - Incorrect number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: Invalid WPN - Missing number of sections" << endl;
	}
	return textures;
}

unsigned int getNumFacesOfTexture(vector<VertexRelative> verts, vector<Face> faces, unsigned int textureInd)
{
	unsigned int numVertsOfTexture = 0, numVerts = verts.size(), numFaces = faces.size(), vertexInd;
	for (unsigned int i = 0; i < numFaces; i++)
	{
		vertexInd = faces[i].vertexIndices[0];
		if (vertexInd < numVerts && verts[vertexInd].textureIndex == textureInd)
		{
			numVertsOfTexture++;
		}
	}
	return numVertsOfTexture;
}

// This function doesn't care about coordinates
unsigned int getNumFacesOfTexture(vector<VertexGlobal> verts, vector<Face> faces, unsigned int textureInd)
{
	return getNumFacesOfTexture(*(vector<VertexRelative> *)&(verts), faces, textureInd);
}

unsigned int getFaceOrientation(Face f, vector<VertexGlobal> verts)
{
	unsigned int orientation = COLINEAR, vInd;
	float orientationVal = 0;
	ai_real orientationFloat;
	aiMatrix4x4 mat;
	VertexGlobal v;
	// Add the face vertices to the matrix
	for (unsigned int i = 0; i < numVertsPerFace; i++)
	{
		for (unsigned int j = 0; j < numDims; j++)
		{
			vInd = f.vertexIndices[i];
			if (vInd < verts.size())
			{
				v = verts[vInd];
				mat[i][j] = v.coordinates[j];
				// Add the view point on the first iteration using the first vertex and its normal
				if (i == 0)
				{
					mat[numVertsPerFace][j] = v.coordinates[j] + v.normal[j];
				}
			}
		}
		mat[i][numDims] = 1;
		// View point
		if (i == 0)
		{
			mat[numVertsPerFace][numDims] = 1;
		}
	}
	// Get the determinate to quantify the orientation
	orientationFloat = mat.Determinant();
	if (orientationFloat > 0)
	{
		orientation = CLOCKWISE;
	}
	else if (orientationFloat < 0)
	{
		orientation = COUNTERCLOCKWISE;
	}
	return orientation;
}

unsigned int getRootJointIndex(vector<JointRelative> joints)
{
	unsigned int rootInd = 0, numJoints = joints.size();
	for (rootInd; rootInd < numJoints; rootInd++)
	{
		if (joints[rootInd].jointInfo.parentIndex == absoluteMaxJoints)
		{
			break;
		}
	}
	// Return invalid index if there is no root
	if (rootInd == numJoints)
	{
		rootInd = absoluteMaxJoints;
	}
	return rootInd;
}

bool getDaeJoints(const aiScene *scene, vector<JointRelative> &joints, unsigned int startDepth)
{
	bool valid = scene;
	// Clear the joints vector
	joints.clear();
	aiNode *currentNode = NULL, *parentNode, *nextNode, *root;
	stringstream ss;
	vector<JointRelative> copy;
	// Make sure the assimp scene is valid
	if (valid)
	{
		// Determine the node to use as the skeleton
		if (scene->mNumMeshes > 0 && scene->mMeshes[0]->mNumBones > 0)
		{
			// Use the first bone of mesh 0 to determine the skeleton to use
			currentNode = scene->mRootNode->FindNode(scene->mMeshes[0]->mBones[0]->mName.data);
			for (nextNode = currentNode; nextNode && nextNode != scene->mRootNode;)
			{
				parentNode = currentNode->mParent;
				nextNode = parentNode;
				for (unsigned int i = 0; nextNode && i < (startDepth + 1); i++)
				{
					nextNode = nextNode->mParent;
				}
				if (nextNode)
				{
					currentNode = parentNode;
				}
			}
		}
		// Make sure the root node was found
		valid = currentNode;
		if (valid)
		{
			root = currentNode;
			cout << "The root joint of the skeleton is: " << root->mName.data << endl;
			// Visit each node in the skeleton and add it to the joints vector
			do
			{
				// Store the node in the joints vector
				JointRelative tj;
				tj.name = currentNode->mName.data;
				//tj.ind = joints.size();
				ss.clear();
				ss.str(tj.name.substr(4, tj.name.size()));
				ss >> tj.ind;
				if (joints.size() > 0)
				{
					tj.jointInfo.parentIndex = 0x3FF;
					for (unsigned int i = 0; i < joints.size(); i++)
					{
						if (currentNode->mParent->mName.C_Str() == joints[i].name)
						{
							tj.jointInfo.parentIndex = joints[i].ind;
							break;
						}
					}
				}
				else
				{
					// This is the root node
					tj.jointInfo.parentIndex = 0x3FF;
				}
				tj.jointInfo.unknownFlag = true;
				tj.jointInfo.unknownIndex = 0x3FF;
				tj.jointInfo.childIndex = 0x3FF;
				// Decompose the node transformation
				aiVector3D scale, rotate, translate;
				mdlsDecompose(currentNode->mTransformation, scale, rotate, translate);
				for (unsigned int i = 0; i < numDims; i++)
				{
					tj.coordinates[i] = translate[i];
					tj.rotations[i] = rotate[i];
					tj.scaleFactors[i] = scale[i];
				}
				tj.special = 0;
				joints.push_back(tj);
				// Find the next node
				if (currentNode->mNumChildren > 0)
				{
					currentNode = currentNode->mChildren[0];
				}
				else
				{
					nextNode = currentNode;
					while (nextNode == currentNode)
					{
						parentNode = currentNode->mParent;
						if (parentNode)
						{
							unsigned int currentChildInd;
							for (currentChildInd = 0; currentNode == nextNode && currentChildInd < (parentNode->mNumChildren - 1); currentChildInd++)
							{
								if (currentNode == parentNode->mChildren[currentChildInd])
								{
									nextNode = parentNode->mChildren[currentChildInd + 1];
								}
							}
							if (currentNode == nextNode && currentChildInd == (parentNode->mNumChildren - 1))
							{	
								nextNode = parentNode;
								if (nextNode != root)
								{
									currentNode = parentNode;
								}
							}
						}
						else
						{
							nextNode = parentNode;
						}
					}
					currentNode = nextNode;
				}
			} while (currentNode && currentNode != root);
			copy = joints;
			for (unsigned int i = 0; i < joints.size(); i++)
			{
				if (copy[i].ind < joints.size())
				{
					joints[copy[i].ind] = copy[i];
				}
				else
				{
					// Invalid index
					joints.clear();
					valid = false;
					break;
				}
			}
			for (unsigned int i = 0; i < joints.size(); i++)
			{
				JointRelative tj = joints[i];
				if (tj.jointInfo.parentIndex < joints.size())
				{
					// Update the parent's child joint info
					if (joints[tj.jointInfo.parentIndex].childrenIndices.size() == 0)
					{
						// This is the joint's first child, so make it the favorite
						joints[tj.jointInfo.parentIndex].jointInfo.childIndex = tj.ind;
					}
					// Add this joint to the parent's list of children
					joints[tj.jointInfo.parentIndex].childrenIndices.push_back(tj.ind);
				}
			}
		}
		//cout << "Number of joints discovered: " << joints.size() << endl;
	}
	return valid;
}

bool getDaeVerticesAndFaces(const aiScene *scene, vector<VertexGlobal> &verts, vector<Face> &faces, vector<JointRelative> joints)
{
	bool valid = scene;
	unsigned int numVerts, numFaces, totalNumVerts = 0, totalNumFaces = 0, previousNumVerts, boneIndex, jointIndex;
	VertexGlobal tempVert;
	// Clear the verts and faces vectors
	verts.clear();
	faces.clear();
	// Make sure the assimp scene is valid
	if (valid)
	{
		// For each mesh
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			//cout << "Starting Mesh " << i << endl;
			// Get the number of vertices and faces the mesh has
			numVerts = scene->mMeshes[i]->mNumVertices;
			numFaces = scene->mMeshes[i]->mNumFaces;
			// Store the previous total number of vertices the model had
			previousNumVerts = totalNumVerts;
			// Get the potential number of total vertices and faces the model can have
			totalNumVerts += numVerts;
			totalNumFaces += numFaces;
			// Reserve space for the vertices and faces
			verts.reserve(totalNumVerts);
			faces.reserve(totalNumFaces);
			// For each vertex in the mesh
			for (unsigned int j = 0; j < numVerts; j++)
			{
				// Make sure the mesh has a position
				if (scene->mMeshes[i]->HasPositions())
				{
					tempVert.coordinates[0] = scene->mMeshes[i]->mVertices[j].x;
					tempVert.coordinates[1] = scene->mMeshes[i]->mVertices[j].y;
					tempVert.coordinates[2] = scene->mMeshes[i]->mVertices[j].z;
					tempVert.coordinatesScaleFactor = 1;
				}
				else
				{
					// Default to origin
					tempVert.coordinates[0] = 0;
					tempVert.coordinates[1] = 0;
					tempVert.coordinates[2] = 0;
				}
				// Make sure the mesh has normals
				if (scene->mMeshes[i]->HasNormals())
				{
					tempVert.normal[0] = scene->mMeshes[i]->mNormals[j].x;
					tempVert.normal[1] = scene->mMeshes[i]->mNormals[j].y;
					tempVert.normal[2] = scene->mMeshes[i]->mNormals[j].z;
				}
				else
				{
					// Default to (1, 1, 1)
					tempVert.normal[0] = 1;
					tempVert.normal[1] = 1;
					tempVert.normal[2] = 1;
				}
				// Make sure the mesh has bones and that joints were provided
				if (scene->mMeshes[i]->HasBones() && joints.size() > 0)
				{
					boneIndex = UINT_MAX;
					jointIndex = UINT_MAX;
					// For each bone the mesh references
					for (unsigned int k = 0; k < scene->mMeshes[i]->mNumBones; k++)
					{
						// For each vertex the bone affects
						for (unsigned int l = 0; l < scene->mMeshes[i]->mBones[k]->mNumWeights; l++)
						{
							// Check if the bone affects this vertex
							if (scene->mMeshes[i]->mBones[k]->mWeights[l].mVertexId == j)
							{
								// It does, so store the bone index
								boneIndex = k;
								// Update k to end the outer loop
								k = scene->mMeshes[i]->mNumBones;
								// Break from this loop
								break;
							}
						}
					}
					// Make sure a valid bone index was found
					if (boneIndex < scene->mMeshes[i]->mNumBones)
					{
						// For each joint provided
						for (unsigned int k = 0; k < joints.size(); k++)
						{
							// Make sure the joint's name matches the bone's name
							if (joints[k].name == scene->mMeshes[i]->mBones[boneIndex]->mName.data)
							{
								// Store the joint index
								jointIndex = k;
								// Break from the loop
								break;
							}
						}
					}
					// Make sure a valid joint index was found
					if (jointIndex < joints.size())
					{
						// Store the joint index
						tempVert.originJointIndex = jointIndex;
					}
					else
					{
						// Default to the root joint
						tempVert.originJointIndex = getRootJointIndex(joints);
					}
				}
				else
				{
					// Check if we have joints
					if (joints.size() > 0)
					{
						// The joint index defaults to the root joint index
						tempVert.originJointIndex = getRootJointIndex(joints);
					}
					else
					{
						// The joint index defaults to joint 0 (which hopefully is the root)
						tempVert.originJointIndex = 0;
					}
				}
				// Make sure the model has materials and at least 1 UV Channel
				if (scene->mNumMaterials > 0 && scene->mMeshes[i]->GetNumUVChannels() > 0)
				{
					// Store the material index
					tempVert.textureIndex = scene->mMeshes[i]->mMaterialIndex;
					// Assume we are using the first UV Channel
					tempVert.textureMap[0] = scene->mMeshes[i]->mTextureCoords[0][j].x;
					tempVert.textureMap[1] = scene->mMeshes[i]->mTextureCoords[0][j].y;
				}
				else
				{
					// Default the texture to invalid
					tempVert.textureIndex = 0xFFFFFFFF;
					tempVert.textureMap[0] = 0;
					tempVert.textureMap[1] = 0;
				}
				tempVert.textureScaleFactor = 1;
				// Check if we already have this joint in the vertices vector
				vector<VertexGlobal>::iterator iter = find(verts.begin(), verts.end(), tempVert);
				if (iter == verts.end())
				{
					// It is a new vertex, so add it to the vector
					//cout << "Adding Vertex " << verts.size() << endl;
					verts.push_back(tempVert);
				}
				else
				{
					//cout << "skipping duplicate of " << (iter - verts.begin()) << endl;
				}
			}
			// For each face
			for (unsigned int j = 0; j < numFaces; j++)
			{
				if (scene->mMeshes[i]->mFaces[j].mNumIndices == numDims)
				{
					Face f;
					f.orientation = COUNTERCLOCKWISE;
					for (unsigned int k = 0; k < numDims; k++)
					{
						// Include the offset (Note: This can get sketchy with our code that removes duplicates, but the update importer settings removes duplicates prior to this point)
						f.vertexIndices[k] = scene->mMeshes[i]->mFaces[j].mIndices[k] + previousNumVerts;
					}
					faces.push_back(f);
				}
			}
			// Update the number of vertices and faces to match the actual amount found
			totalNumVerts = verts.size();
			totalNumFaces = faces.size();
		}
	}
	return valid;
}

/* Extract the joints from the mdls data and return them as a vector */
vector<JointRelative> getMdlsJoints(string raw, unsigned int maxJoints)
{
	unsigned int mobjOff, tableOff, numJoints, jointsOff, dimOff, parentInd;
	vector<JointRelative> joints;
	// Make sure the header exists
	if (raw.size() >= 8)
	{
		// Get the mobj offset
		mobjOff = *(unsigned int *)&(raw[4]);
		// Determine the meta-data offset
		tableOff = mobjOff + 64;
		// Make sure the file contains a the joint meta-data
		if (raw.size() >= tableOff + 8)
		{
			// Get the number of joints
			numJoints = *(unsigned int *)&(raw[tableOff]);
			if (numJoints > maxJoints)
			{
				numJoints = maxJoints;
			}
			// Make sure the maximum number of joints isn't exceeded
			if (numJoints <= absoluteMaxJoints)
			{
				// Reserve space for all of the joints
				joints.reserve(numJoints);
				// Get the offset to the start of the joints
				jointsOff = tableOff + *(unsigned int *)&(raw[tableOff + 4]);
				// Make sure the joints section is the correct size
				if (raw.size() >= jointsOff + (numJoints * 48))
				{
					// For each joint in the skeleton
					for (unsigned int i = 0; i < numJoints; i++)
					{
						// Add a new joint to the joints vector
						joints.push_back(JointRelative());
						// Fill in the joint's index
						joints[i].ind = *(unsigned int *)&(raw[jointsOff + (i * 48) + 12]);
						// Fill in the joint's special
						joints[i].special = *(float *)&(raw[jointsOff + (i * 48) + 28]);
						// Fill in the joint's jointInfo
						joints[i].jointInfo.unknownFlag = (bool)((*(unsigned int *)&(raw[jointsOff + (i * 48) + 44])) >> 30);
						joints[i].jointInfo.childIndex = ((*(unsigned int *)&(raw[jointsOff + (i * 48) + 44])) >> 20) & absoluteMaxJoints;
						joints[i].jointInfo.unknownIndex = ((*(unsigned int *)&(raw[jointsOff + (i * 48) + 44])) >> 10) & absoluteMaxJoints;
						joints[i].jointInfo.parentIndex = (*(unsigned int *)&(raw[jointsOff + (i * 48) + 44])) & absoluteMaxJoints;
						// For each dimension
						for (unsigned int j = 0; j < numDims; j++)
						{
							// The coorindates are stored as Y, Z, X but we will store them as X, Y, Z
							dimOff = (j + 2) % 3;
							// Fill in dimension joints's scale factor
							joints[i].scaleFactors[j] = *(float *)&(raw[jointsOff + (i * 48) + (dimOff * 4)]);
							// Fill in dimension joints's axis rotation
							joints[i].rotations[j] = *(float *)&(raw[jointsOff + (i * 48) + (dimOff * 4) + 16]);
							// Fill in dimension joints's coordinate
							joints[i].coordinates[j] = *(float *)&(raw[jointsOff + (i * 48) + (dimOff * 4) + 32]);
						}
					}
					// Go through each joint and update its children index vector
					for (unsigned int i = 0; i < joints.size(); i++)
					{
						// Get joint i's parent index
						parentInd = joints[i].jointInfo.parentIndex;
						// Make sure it is valid (i.e. not the root node)
						if (parentInd != absoluteMaxJoints && parentInd < joints.size())
						{
							// Add i to the parent's child index vector
							joints[parentInd].childrenIndices.push_back(i);
						}
					}
				}
			}
		}
	}
	return joints;
}

bool getMdlsVerticesAndFaces(string raw, vector<JointRelative> joints, vector<VertexRelative> &verts, vector<Face> &faces)
{
	// Declare variables
	unsigned int mobjOff, tableOff, modelSectionLen, numPolygonCollections, *polygonCollectionTextures, *polygonCollectionOffs, polygonCollectionLen, polygonEntryType, polygonLen, numPolygonVerts, originIndex, off, jointIndex, origins[numOrigins] = { 0, 0, 0, 0, 0, 0 };
	VertexRelative tempVert;
	Face tempFace;
	bool valid = true;
	// Clear the return values
	verts.clear();
	faces.clear();
	// Make sure the header exists
	if (raw.size() >= 8)
	{
		// Get the mobj offset
		mobjOff = *(unsigned int *)&(raw[4]);
		// Make sure the file contains the mobj header
		if (raw.size() > mobjOff + mobjHeaderLen)
		{
			// Determine the polygon collection table offset
			tableOff = mobjOff + *(unsigned int *)&(raw[mobjOff + 32]);
			// Determine the length of the 3D Model section
			modelSectionLen = *(unsigned int *)&(raw[mobjOff + 36]);
			// Make sure the file contains a polygon collection table
			if (raw.size() >= tableOff + modelTableHeaderLen)
			{
				// Get the number of polygon collections
				numPolygonCollections = *(unsigned int *)&(raw[tableOff + 12]);
				// Initialize the texture and offset arrays
				polygonCollectionTextures = new unsigned int[numPolygonCollections];
				polygonCollectionOffs = new unsigned int[numPolygonCollections];
				// Make sure the polygon collection table is the correct size
				if (raw.size() >= tableOff + (numPolygonCollections * 16))
				{
					// Store the offsets to each polygon collection
					for (unsigned int i = 0; i < numPolygonCollections; i++)
					{
						polygonCollectionTextures[i] = *(unsigned int *)&(raw[tableOff + (i * 16) + 24]);
						polygonCollectionOffs[i] = tableOff + *(unsigned int *)&(raw[tableOff + (i * 16) + 28]);
					}
					// Make sure the file contains each polygon collection
					if (raw.size() >= polygonCollectionOffs[numPolygonCollections - 1] + 16)
					{
						// For each polygon collection
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							// Get the polygon collection length
							if (i < numPolygonCollections - 1)
							{
								polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
							}
							else
							{
								polygonCollectionLen = modelSectionLen - polygonCollectionOffs[i];
							}
							// Make sure the polygon collection exists (really only necessary on the last one since we did a prior check)
							if (raw.size() >= polygonCollectionOffs[i] + polygonCollectionLen)
							{
								// For each polygon in the polygon collection
								for (unsigned int j = polygonCollectionOffs[i] + 16; valid && ((j + 12) < (polygonCollectionOffs[i] + polygonCollectionLen)); j += polygonLen)
								{
									// Get the entry type
									polygonEntryType = *(unsigned int *)&(raw[j]);
									// Check the entry type and perform the appropriate actions
									if (polygonEntryType == 0) // This is an origin update
									{
										// Update the polygon length to be the joint reference size
										polygonLen = 128;
										// Get the joint index
										jointIndex = *(unsigned int *)&(raw[j + 4]);
										// Get the origin index
										originIndex = *(unsigned int *)&(raw[j + 8]);
										// Make sure the joint index references a valid joint
										if (jointIndex < joints.size() && originIndex < numOrigins)
										{
											// Update the origin at originJointIndex
											origins[originIndex] = jointIndex;
										}
										else // Invalid origin
										{
											valid = false;
										}
									}
									else if (polygonEntryType == 1) // This is a collection of faces
									{
										// Get the length of the polygon
										polygonLen = *(unsigned int *)&(raw[j + 4]);
										// Get the number of vertices that make up this polygon
										numPolygonVerts = *(unsigned int *)&(raw[j + 8]);
										// Make sure the polygon length is not 0
										if (polygonLen > 0)
										{
											// Make sure the polygon is in the data
											if (raw.size() >= j + polygonLen)
											{
												unsigned int numWeirds = 0;
												// For each vertex that makes up the polygon
												for (unsigned int k = 0; k < numPolygonVerts; k++)
												{
													// Extract the origin index
													originIndex = *(unsigned int *)&(raw[j + (k * 48) + (numWeirds * 16) + 44]);
													// Make sure that the origin index is valid
													if (originIndex < numOrigins)
													{
														// Get the joint index for the vertex
														tempVert.originJointIndex = origins[originIndex];
														// Store the vertex's texture index
														tempVert.textureIndex = polygonCollectionTextures[i];
														// Extract the coordinate scale factor
														tempVert.coordinatesScaleFactor = *(float *)&(raw[j + (k * 48) + (numWeirds * 16) + 60]);
														// Check for the padding
														while(*(unsigned int *)&(raw[j + (k * 48) + (numWeirds * 16) + 76]) != 0)
														{
															numWeirds++;
														}
														// Extract the texture scale factor
														tempVert.textureScaleFactor = *(float *)&(raw[j + (k * 48) + (numWeirds * 16) + 72]);
														// Extract the coordinates for the vertex for each dimension
														for (unsigned int l = 0; l < numDims; l++)
														{
															// Get dimension offset
															off = (l + 2) % numDims;
															// Extract the normal values
															tempVert.normal[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 32]);
															// Get the vertex's relative coordinate
															tempVert.coordinates[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 48]);
														}
														// Extract the texture u and v values of the vertex
														for (unsigned int l = 0; l < 2; l++)
														{
															tempVert.textureMap[l] = *(float *)&(raw[j + (k * 48) + (l * 4) + (numWeirds * 16) + 64]);
															// Invert the v coordinate, because KH has to be special
															if (l == 1)
															{
																tempVert.textureMap[l] = 1 - tempVert.textureMap[l];
															}
														}
														// Check if we already have tempVert in verts
														if (find(verts.begin(), verts.end(), tempVert) == verts.end())
														{
															// This is a new vertex, add tempVert to verts
															verts.push_back(tempVert);
														}
													}
													else // Invalid origin?
													{
														valid = false;
													}
												}
											}
											else // Failed to parse polygon
											{
												valid = false;
											}
										}
										else // The polygon is empty?
										{
											valid = false;
										}
									}
									else if (polygonEntryType == 2) // Unknown entry type from Sora's High Poly model, skip for now
									{
										// Get the length of the entry
										polygonLen = *(unsigned int *)&(raw[j + 4]);
									}
									else if (polygonEntryType == 32768) // This is the end of a sub-collection
									{
										// Get the length field
										polygonLen = *(unsigned int *)&(raw[j + 4]);
										// If the length is 0, then it is the end of the polygon collection
										if (polygonLen == 0)
										{
											break;
										}
									}
									else // Invalid polygon entry type?
									{
										polygonLen = 0;
										valid = false;
									}
								}
							}
							else // Failed to parse polygon collection header
							{
								valid = false;
							}
						}
						// Reset origins
						for (unsigned int i = 0; i < numDims; i++)
						{
							origins[i] = 0;
						}
						// Now that we have a list of vertices, revisit each polygon to get the faces
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							// Get the polygon collection length
							if (i < numPolygonCollections - 1)
							{
								polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
							}
							else
							{
								polygonCollectionLen = modelSectionLen - polygonCollectionOffs[i];
							}
							// Make sure the polygon collection exists (really only necessary on the last one since we did a prior check)
							if (raw.size() >= polygonCollectionOffs[i] + polygonCollectionLen)
							{
								// For each polygon in the polygon collection
								for (unsigned int j = polygonCollectionOffs[i] + 16; valid && ((j + 12) < (polygonCollectionOffs[i] + polygonCollectionLen)); j += polygonLen)
								{
									// Get the polygon entry type
									polygonEntryType = *(unsigned int *)&(raw[j]);
									if (polygonEntryType == 0) // This is an origin update
									{
										// Update the polygon length to be the joint reference size
										polygonLen = 128;
										// Get the joint index
										jointIndex = *(unsigned int *)&(raw[j + 4]);
										// Get the origin index
										originIndex = *(unsigned int *)&(raw[j + 8]);
										// Make sure the joint index references a valid joint
										if (jointIndex < joints.size() && originIndex < numOrigins)
										{
											// Update the origin at originJointIndex
											origins[originIndex] = jointIndex;
										}
										else // Invalid origin
										{
											valid = false;
										}
									}
									else if (polygonEntryType == 1) // This is a collection of vertices
									{
										// Get the length of the polygon
										polygonLen = *(unsigned int *)&(raw[j + 4]);
										// Get the number of vertices that make up this polygon
										numPolygonVerts = *(unsigned int *)&(raw[j + 8]);
										bool orientation = raw[j + 12];
										// Make sure the polygon length is not 0
										if (polygonLen > 0)
										{
											// Make sure the polygon is in the data
											if (raw.size() >= j + polygonLen)
											{
												unsigned int numWeirds = 0;
												// For each vertex in the polygon that is the start of a face
												for (unsigned int k = 0; (k + numVertsPerFace) <= numPolygonVerts; k++)
												{
													// Extract the origin index
													originIndex = *(unsigned int *)&(raw[j + (k * 48) + (numWeirds * 16) + 44]);
													// Make sure that the origin index is valid
													if (originIndex < numOrigins)
													{
														// Get the joint index for the vertex
														tempVert.originJointIndex = origins[originIndex];
														// Store the vertex's texture index
														tempVert.textureIndex = polygonCollectionTextures[i];
														// Extract the coordinate scale factor
														tempVert.coordinatesScaleFactor = *(float *)&(raw[j + (k * 48) + (numWeirds * 16) + 60]);
														// Check for padding
														while (*(unsigned int *)&(raw[j + (k * 48) + (numWeirds * 16) + 76]) != 0)
														{
															numWeirds++;
														}
														// Extract the texture scale factor
														tempVert.textureScaleFactor = *(float *)&(raw[j + (k * 48) + (numWeirds * 16) + 72]);
														// Extract the coordinates for the vertex for each dimension
														for (unsigned int l = 0; l < numDims; l++)
														{
															// Get dimension offset
															off = (l + 2) % numDims;
															// To-Do: Extract the normal values
															tempVert.normal[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 32]);
															// Get the vertex's relative coordinate
															tempVert.coordinates[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 48]);
														}
														// Extract the texture u and vr values of the vertex
														for (unsigned int l = 0; l < 2; l++)
														{
															tempVert.textureMap[l] = *(float *)&(raw[j + (k * 48) + (l * 4) + (numWeirds * 16) + 64]);
															if (l == 1)
															{
																tempVert.textureMap[l] = 1 - tempVert.textureMap[l];
															}
														}
														// Get the index of this vertex in the verts vector
														unsigned int currentVertInd = (find(verts.begin(), verts.end(), tempVert) - verts.begin()), nextVertInd;
														// Make sure we got a valid index
														if (currentVertInd < verts.size())
														{
															// Save the currentVertInd in the tempFace
															tempFace.vertexIndices[0] = currentVertInd;
															unsigned int numWeirds2 = numWeirds;
															// For each numAsStr that a vertex can have
															for (unsigned int l = 1; l <= numLinesPerVertex; l++)
															{
																// Check if we need to add the lth numAsStr segment
																if ((k + l) < numPolygonVerts)
																{
																	// Get Origin Index for vertex k + l
																	originIndex = *(unsigned int *)&(raw[j + ((k + l) * 48) + (numWeirds2 * 16) + 44]);
																	// Make sure the origin index is valid
																	if (originIndex < numOrigins)
																	{
																		// Get the joint index for the vertex
																		tempVert.originJointIndex = origins[originIndex];
																		// Store the vertex's texture index
																		tempVert.textureIndex = polygonCollectionTextures[i];
																		// Extract the coordinate scale factor
																		tempVert.coordinatesScaleFactor = *(float *)&(raw[j + ((k + l) * 48) + (numWeirds2 * 16) + 60]);
																		while (*(unsigned int *)&(raw[j + ((k + l) * 48) + (numWeirds2 * 16) + 76]) != 0)
																		{
																			numWeirds2++;
																		}
																		// Extract the texture scale factor
																		tempVert.textureScaleFactor = *(float *)&(raw[j + ((k + l) * 48) + (numWeirds2 * 16) + 72]);
																		// For each dimension
																		for (unsigned int m = 0; m < numDims; m++)
																		{
																			// Find the offset since it is {Y, Z, X} in the file, but we want {X, Y, Z}
																			off = ((m + 2) % numDims);
																			// Extract the normal values
																			tempVert.normal[m] = *(float *)&(raw[j + ((k + l) * 48) + (off * 4) + (numWeirds2 * 16) + 32]);
																			// Extract the vertex coordinate
																			tempVert.coordinates[m] = *(float *)&(raw[j + ((k + l) * 48) + (off * 4) + (numWeirds2 * 16) + 48]);
																		}
																		// Extract the texture u and v values of the vertex
																		for (unsigned int m = 0; m < 2; m++)
																		{
																			tempVert.textureMap[m] = *(float *)&(raw[j + ((k + l) * 48) + (m * 4) + (numWeirds2 * 16) + 64]);
																			if (m == 1)
																			{
																				tempVert.textureMap[m] = 1 - tempVert.textureMap[m];
																			}
																		}
																		// Get the index of vertex k + l
																		nextVertInd = (find(verts.begin(), verts.end(), tempVert) - verts.begin());
																		// Make sure we got a valid index
																		if (nextVertInd < verts.size())
																		{
																			// Save the nextVertInd in the tempFace
																			tempFace.vertexIndices[l] = nextVertInd;
																		}
																		else // Referenced an unknown vertex
																		{
																			valid = false;
																			// Break from the loop
																			break;
																		}
																	}
																}
															}
															// Make sure we got a valid face
															if (valid)
															{
																// Check if we need to flip the order
																if (orientation)
																{
																	unsigned int swap = tempFace.vertexIndices[0];
																	tempFace.vertexIndices[0] = tempFace.vertexIndices[1];
																	tempFace.vertexIndices[1] = swap;
																}
																tempFace.orientation = CLOCKWISE;
																// Add the face <k, k + 1, k + 2> to the faces vector
																faces.push_back(tempFace);
																orientation = !orientation;
															}
														}
														else // Referenced an unknown vertex?
														{
															valid = false;
														}
													}
													else // Invalid origin?
													{
														valid = false;
													}
												}
											}
											else // Failed to parse polygon
											{
												valid = false;
											}
										}
										else // The polygon is empty?
										{
											valid = false;
										}
									}
									else if (polygonEntryType == 2) // Unknown entry type from Sora's High Poly model, skip over for now
									{
										// Get the length of the entry
										polygonLen = *(unsigned int *)&(raw[j + 4]);
									}
									else if (polygonEntryType == 32768) // This is the end of a sub polygon collection
									{
										// Get the length field
										polygonLen = *(unsigned int *)&(raw[j + 4]);
										// If the length is 0, then it is the end of the polygon collection
										if (polygonLen == 0)
										{
											break;
										}
									}
									else // Invalid polygon entry type?
									{
										polygonLen = 0;
										valid = false;
									}
								}
							}
							else // Failed to parse polygon collection
							{
								valid = false;
							}
						}
					}
					else // Failed to parse polygon collection header
					{
						valid = false;
					}
				}
				else // Failed to parse mobj polygon collection table
				{
					valid = false;
				}
				// Free the polygon collection textures and offsets
				delete[]polygonCollectionTextures;
				delete[]polygonCollectionOffs;
			}
			else // The file does not contain the mobj table
			{
				valid = false;
			}
		}
		else // The file does not contain the mobj header
		{
			valid = false;
		}
	}
	else // The file does not contain the mdls header
	{
		valid = false;
	}
	return valid;
}

bool getWpnVerticesAndFaces(string raw, /*vector<JointRelative> joints, */vector<VertexRelative> &verts, vector<Face> &faces)
{
	// Declare variables
	unsigned int numSections, menvOff, tableOff, modelSectionLen, numPolygonCollections, *polygonCollectionTextures, *polygonCollectionOffs, polygonCollectionLen, polygonEntryType, polygonLen, numPolygonVerts, originIndex, off, jointIndex, origins[numOrigins] = { 0, 0, 0, 0, 0, 0 };
	VertexRelative tempVert;
	Face tempFace;
	bool valid = true;
	// Clear the return values
	verts.clear();
	faces.clear();
	// Make sure the header exists
	if (raw.size() >= 16)
	{
		numSections = *(unsigned int *)&(raw[0]);
		if (numSections == 2)
		{
			// Get the menv offset
			menvOff = *(unsigned int *)&(raw[8]);
			// Make sure the file contains the mobj header
			if (raw.size() > (menvOff + menvHeaderLen))
			{
				// Determine the polygon collection table offset
				tableOff = menvOff + *(unsigned int *)&(raw[menvOff + 32]);
				// Determine the length of the 3D Model section
				modelSectionLen = *(unsigned int *)&(raw[menvOff + 36]);
				// Make sure the file contains a polygon collection table
				if (raw.size() >= tableOff + modelTableHeaderLen)
				{
					// Get the number of polygon collections
					numPolygonCollections = *(unsigned int *)&(raw[tableOff + 12]);
					// Initialize the texture and offset arrays
					polygonCollectionTextures = new unsigned int[numPolygonCollections];
					polygonCollectionOffs = new unsigned int[numPolygonCollections];
					// Make sure the polygon collection table is the correct size
					if (raw.size() >= (tableOff + ((numPolygonCollections + 1) * 16)))
					{
						// Store the offsets to each polygon collection
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							polygonCollectionTextures[i] = *(unsigned int *)&(raw[tableOff + (i * 16) + 24]);
							polygonCollectionOffs[i] = tableOff + *(unsigned int *)&(raw[tableOff + (i * 16) + 28]);
						}
						// Make sure the file contains each polygon collection
						if (raw.size() >= polygonCollectionOffs[numPolygonCollections - 1] + 16)
						{
							// For each polygon collection
							for (unsigned int i = 0; i < numPolygonCollections; i++)
							{
								// Get the polygon collection length
								if (i < numPolygonCollections - 1)
								{
									polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
								}
								else
								{
									polygonCollectionLen = modelSectionLen - (polygonCollectionOffs[i] - menvOff);
								}
								// Make sure the polygon collection exists (really only necessary on the last one since we did a prior check)
								if (raw.size() >= polygonCollectionOffs[i] + polygonCollectionLen)
								{
									// For each polygon in the polygon collection
									for (unsigned int j = polygonCollectionOffs[i] + 16; valid && ((j + 12) < (polygonCollectionOffs[i] + polygonCollectionLen)); j += polygonLen)
									{
										// Get the entry type
										polygonEntryType = *(unsigned int *)&(raw[j]);
										// Check the entry type and perform the appropriate actions
										if (polygonEntryType == 0) // This is an origin update
										{
											// Update the polygon length to be the joint reference size
											polygonLen = 128;
											// Get the joint index
											jointIndex = *(unsigned int *)&(raw[j + 4]);
											// Get the origin index
											originIndex = *(unsigned int *)&(raw[j + 8]);
											// Make sure the joint index references a valid joint
											//if (jointIndex < joints.size() && originIndex < numOrigins)
											{
												// Update the origin at originJointIndex
												origins[originIndex] = jointIndex;
											}
											//else // Invalid origin
											{
											//	valid = false;
											}
										}
										else if (polygonEntryType == 1) // This is a collection of faces
										{
											// Get the length of the polygon
											polygonLen = *(unsigned int *)&(raw[j + 4]);
											// Get the number of vertices that make up this polygon
											numPolygonVerts = *(unsigned int *)&(raw[j + 8]);
											// Make sure the polygon length is not 0
											if (polygonLen > 0)
											{
												// Make sure the polygon is in the data
												if (raw.size() >= j + polygonLen)
												{
													unsigned int numWeirds = 0;
													// For each vertex that makes up the polygon
													for (unsigned int k = 0; k < numPolygonVerts; k++)
													{
														// Extract the origin index
														originIndex = *(unsigned int *)&(raw[j + (k * 48) + (numWeirds * 16) + 44]);
														// Make sure that the origin index is valid
														if (originIndex < numOrigins)
														{
															// Get the joint index for the vertex
															tempVert.originJointIndex = origins[originIndex];
															// Store the vertex's texture index
															tempVert.textureIndex = polygonCollectionTextures[i];
															// Extract the coordinate scale factor
															tempVert.coordinatesScaleFactor = *(float *)&(raw[j + (k * 48) + (numWeirds * 16) + 60]);
															// Check for the padding
															while (*(unsigned int *)&(raw[j + (k * 48) + (numWeirds * 16) + 76]) != 0)
															{
																numWeirds++;
															}
															// Extract the texture scale factor
															tempVert.textureScaleFactor = *(float *)&(raw[j + (k * 48) + (numWeirds * 16) + 72]);
															// Extract the coordinates for the vertex for each dimension
															for (unsigned int l = 0; l < numDims; l++)
															{
																// Get dimension offset
																off = (l + 2) % numDims;
																// Extract the normal values
																tempVert.normal[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 32]);
																// Get the vertex's relative coordinate
																tempVert.coordinates[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 48]);
															}
															// Extract the texture u and v values of the vertex
															for (unsigned int l = 0; l < 2; l++)
															{
																tempVert.textureMap[l] = *(float *)&(raw[j + (k * 48) + (l * 4) + (numWeirds * 16) + 64]);
																// Invert the v coordinate, because KH has to be special
																if (l == 1)
																{
																	tempVert.textureMap[l] = 1 - tempVert.textureMap[l];
																}
															}
															// Check if we already have tempVert in verts
															if (find(verts.begin(), verts.end(), tempVert) == verts.end())
															{
																// This is a new vertex, add tempVert to verts
																verts.push_back(tempVert);
															}
														}
														else // Invalid origin?
														{
															valid = false;
														}
													}
												}
												else // Failed to parse polygon
												{
													valid = false;
												}
											}
											else // The polygon is empty?
											{
												valid = false;
											}
										}
										else if (polygonEntryType == 2) // Unknown entry type from Sora's High Poly model, skip for now
										{
											// Get the length of the entry
											polygonLen = *(unsigned int *)&(raw[j + 4]);
										}
										else if (polygonEntryType == 32768) // This is the end of a sub-collection
										{
											// Get the length field
											polygonLen = *(unsigned int *)&(raw[j + 4]);
											// If the length is 0, then it is the end of the polygon collection
											if (polygonLen == 0)
											{
												break;
											}
										}
										else // Invalid polygon entry type?
										{
											polygonLen = 0;
											valid = false;
										}
									}
								}
								else // Failed to parse polygon collection header
								{
									valid = false;
								}
							}
							// Reset origins
							for (unsigned int i = 0; i < numDims; i++)
							{
								origins[i] = 0;
							}
							// Now that we have a list of vertices, revisit each polygon to get the faces
							for (unsigned int i = 0; i < numPolygonCollections; i++)
							{
								// Get the polygon collection length
								if (i < numPolygonCollections - 1)
								{
									polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
								}
								else
								{
									polygonCollectionLen = modelSectionLen - (polygonCollectionOffs[i] - menvOff);
								}
								// Make sure the polygon collection exists (really only necessary on the last one since we did a prior check)
								if (raw.size() >= polygonCollectionOffs[i] + polygonCollectionLen)
								{
									// For each polygon in the polygon collection
									for (unsigned int j = polygonCollectionOffs[i] + 16; valid && ((j + 12) < (polygonCollectionOffs[i] + polygonCollectionLen)); j += polygonLen)
									{
										// Get the polygon entry type
										polygonEntryType = *(unsigned int *)&(raw[j]);
										if (polygonEntryType == 0) // This is an origin update
										{
											// Update the polygon length to be the joint reference size
											polygonLen = 128;
											// Get the joint index
											jointIndex = *(unsigned int *)&(raw[j + 4]);
											// Get the origin index
											originIndex = *(unsigned int *)&(raw[j + 8]);
											// Make sure the joint index references a valid joint
											//if (jointIndex < joints.size() && originIndex < numOrigins)
											{
												// Update the origin at originJointIndex
												origins[originIndex] = jointIndex;
											}
											//else // Invalid origin
											{
											//	valid = false;
											}
										}
										else if (polygonEntryType == 1) // This is a collection of vertices
										{
											// Get the length of the polygon
											polygonLen = *(unsigned int *)&(raw[j + 4]);
											// Get the number of vertices that make up this polygon
											numPolygonVerts = *(unsigned int *)&(raw[j + 8]);
											bool orientation = raw[j + 12];
											// Make sure the polygon length is not 0
											if (polygonLen > 0)
											{
												// Make sure the polygon is in the data
												if (raw.size() >= j + polygonLen)
												{
													unsigned int numWeirds = 0;
													// For each vertex in the polygon that is the start of a face
													for (unsigned int k = 0; (k + numVertsPerFace) <= numPolygonVerts; k++)
													{
														// Extract the origin index
														originIndex = *(unsigned int *)&(raw[j + (k * 48) + (numWeirds * 16) + 44]);
														// Make sure that the origin index is valid
														if (originIndex < numOrigins)
														{
															// Get the joint index for the vertex
															tempVert.originJointIndex = origins[originIndex];
															// Store the vertex's texture index
															tempVert.textureIndex = polygonCollectionTextures[i];
															// Extract the coordinate scale factor
															tempVert.coordinatesScaleFactor = *(float *)&(raw[j + (k * 48) + (numWeirds * 16) + 60]);
															// Check for padding
															while (*(unsigned int *)&(raw[j + (k * 48) + (numWeirds * 16) + 76]) != 0)
															{
																numWeirds++;
															}
															// Extract the texture scale factor
															tempVert.textureScaleFactor = *(float *)&(raw[j + (k * 48) + (numWeirds * 16) + 72]);
															// Extract the coordinates for the vertex for each dimension
															for (unsigned int l = 0; l < numDims; l++)
															{
																// Get dimension offset
																off = (l + 2) % numDims;
																// To-Do: Extract the normal values
																tempVert.normal[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 32]);
																// Get the vertex's relative coordinate
																tempVert.coordinates[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 48]);
															}
															// Extract the texture u and vr values of the vertex
															for (unsigned int l = 0; l < 2; l++)
															{
																tempVert.textureMap[l] = *(float *)&(raw[j + (k * 48) + (l * 4) + (numWeirds * 16) + 64]);
																if (l == 1)
																{
																	tempVert.textureMap[l] = 1 - tempVert.textureMap[l];
																}
															}
															// Get the index of this vertex in the verts vector
															unsigned int currentVertInd = (find(verts.begin(), verts.end(), tempVert) - verts.begin()), nextVertInd;
															// Make sure we got a valid index
															if (currentVertInd < verts.size())
															{
																// Save the currentVertInd in the tempFace
																tempFace.vertexIndices[0] = currentVertInd;
																unsigned int numWeirds2 = numWeirds;
																// For each numAsStr that a vertex can have
																for (unsigned int l = 1; l <= numLinesPerVertex; l++)
																{
																	// Check if we need to add the lth numAsStr segment
																	if ((k + l) < numPolygonVerts)
																	{
																		// Get Origin Index for vertex k + l
																		originIndex = *(unsigned int *)&(raw[j + ((k + l) * 48) + (numWeirds2 * 16) + 44]);
																		// Make sure the origin index is valid
																		if (originIndex < numOrigins)
																		{
																			// Get the joint index for the vertex
																			tempVert.originJointIndex = origins[originIndex];
																			// Store the vertex's texture index
																			tempVert.textureIndex = polygonCollectionTextures[i];
																			// Extract the coordinate scale factor
																			tempVert.coordinatesScaleFactor = *(float *)&(raw[j + ((k + l) * 48) + (numWeirds2 * 16) + 60]);
																			while (*(unsigned int *)&(raw[j + ((k + l) * 48) + (numWeirds2 * 16) + 76]) != 0)
																			{
																				numWeirds2++;
																			}
																			// Extract the texture scale factor
																			tempVert.textureScaleFactor = *(float *)&(raw[j + ((k + l) * 48) + (numWeirds2 * 16) + 72]);
																			// For each dimension
																			for (unsigned int m = 0; m < numDims; m++)
																			{
																				// Find the offset since it is {Y, Z, X} in the file, but we want {X, Y, Z}
																				off = ((m + 2) % numDims);
																				// Extract the normal values
																				tempVert.normal[m] = *(float *)&(raw[j + ((k + l) * 48) + (off * 4) + (numWeirds2 * 16) + 32]);
																				// Extract the vertex coordinate
																				tempVert.coordinates[m] = *(float *)&(raw[j + ((k + l) * 48) + (off * 4) + (numWeirds2 * 16) + 48]);
																			}
																			// Extract the texture u and v values of the vertex
																			for (unsigned int m = 0; m < 2; m++)
																			{
																				tempVert.textureMap[m] = *(float *)&(raw[j + ((k + l) * 48) + (m * 4) + (numWeirds2 * 16) + 64]);
																				if (m == 1)
																				{
																					tempVert.textureMap[m] = 1 - tempVert.textureMap[m];
																				}
																			}
																			// Get the index of vertex k + l
																			nextVertInd = (find(verts.begin(), verts.end(), tempVert) - verts.begin());
																			// Make sure we got a valid index
																			if (nextVertInd < verts.size())
																			{
																				// Save the nextVertInd in the tempFace
																				tempFace.vertexIndices[l] = nextVertInd;
																			}
																			else // Referenced an unknown vertex
																			{
																				valid = false;
																				// Break from the loop
																				break;
																			}
																		}
																	}
																}
																// Make sure we got a valid face
																if (valid)
																{
																	// Check if we need to flip the order
																	if (orientation)
																	{
																		unsigned int swap = tempFace.vertexIndices[0];
																		tempFace.vertexIndices[0] = tempFace.vertexIndices[1];
																		tempFace.vertexIndices[1] = swap;
																	}
																	tempFace.orientation = CLOCKWISE;
																	// Add the face <k, k + 1, k + 2> to the faces vector
																	faces.push_back(tempFace);
																	orientation = !orientation;
																}
															}
															else // Referenced an unknown vertex?
															{
																valid = false;
															}
														}
														else // Invalid origin?
														{
															valid = false;
														}
													}
												}
												else // Failed to parse polygon
												{
													valid = false;
												}
											}
											else // The polygon is empty?
											{
												valid = false;
											}
										}
										else if (polygonEntryType == 2) // Unknown entry type from Sora's High Poly model, skip over for now
										{
											// Get the length of the entry
											polygonLen = *(unsigned int *)&(raw[j + 4]);
										}
										else if (polygonEntryType == 32768) // This is the end of a sub polygon collection
										{
											// Get the length field
											polygonLen = *(unsigned int *)&(raw[j + 4]);
											// If the length is 0, then it is the end of the polygon collection
											if (polygonLen == 0)
											{
												break;
											}
										}
										else // Invalid polygon entry type?
										{
											polygonLen = 0;
											valid = false;
										}
									}
								}
								else // Failed to parse polygon collection
								{
									valid = false;
								}
							}
						}
						else // Failed to parse polygon collection header
						{
							valid = false;
						}
					}
					else // Failed to parse menv polygon collection table
					{
						valid = false;
					}
					// Free the polygon collection textures and offsets
					delete[]polygonCollectionTextures;
					delete[]polygonCollectionOffs;
				}
				else // The file does not contain the menv table
				{
					valid = false;
				}
			}
			else // The file does not contain the menv header
			{
				valid = false;
			}
		}
		else // The file doesn't contain the appropriate number of sections
		{
			valid = false;
		}
	}
	else // The file does not contain the wpn header
	{
		valid = false;
	}
	return valid;
}

void removeDuplicateFaces(vector<Face> &faces, bool ignoreOrientation)
{
	bool dupe, foundCopy;
	unsigned int numFaces = faces.size();
	for (unsigned int i = 0; i < numFaces; i++)
	{
		for (unsigned int j = numFaces - 1; j > i; j--)
		{
			// Reset the duplicate flag
			dupe = true;
			// Determine if j is a duplicate of i
			for (unsigned int k = 0; k < numVertsPerFace; k++)
			{
				if (ignoreOrientation)
				{
					foundCopy = false;
					for (unsigned int l = 0; l < numVertsPerFace; l++)
					{
						if (faces[i].vertexIndices[k] == faces[j].vertexIndices[l])
						{
							foundCopy = true;
							break;
						}
					}
					if (!foundCopy)
					{
						dupe = false;
						break;
					}
				}
				else
				{
					if (faces[i].vertexIndices[k] != faces[j].vertexIndices[k])
					{
						dupe = false;
						break;
					}
				}
			}
			// Check if face j is a duplicate of i
			if (dupe)
			{
				// Remove face j
				faces.erase(faces.begin() + j);
				// Update the number of faces
				numFaces--;
			}
		}
	}
	return;
}

bool getJointGlobal(vector<JointRelative> joints, unsigned int jrInd, JointGlobal &jg)
{
	// Initialize variables
	unsigned int parentInd = absoluteMaxJoints, recursiveInd = jrInd, numJoints = joints.size(), rotationOrder[numDims] = { 1, 2, 0 };
	bool valid = true;
	if (jrInd < numJoints)
	{
		JointRelative jr = joints[jrInd];
		aiMatrix4x4 transform, scale, rotate, translate;
		aiVector3D scaling, rotation, translation;
		// Copy the contents of jr int jg
		jg = *(JointGlobal *)&(jr);
		// Make sure the index isn't out of range
		if (recursiveInd < numJoints && recursiveInd != absoluteMaxJoints)
		{
			// While not at the root node and the loop hasn't timed out
			for (unsigned int depth = 0; recursiveInd < numJoints && recursiveInd != absoluteMaxJoints && depth < loopTimeOut; depth++)
			{
				JointRelative parent = joints[recursiveInd];
				scaling = aiVector3D(parent.scaleFactors[0], parent.scaleFactors[1], parent.scaleFactors[2]);
				rotation = aiVector3D(parent.rotations[0], parent.rotations[1], parent.rotations[2]);
				translation = aiVector3D(parent.coordinates[0], parent.coordinates[1], parent.coordinates[2]);
				// Get the scaling matrix
				aiMatrix4x4::Scaling(scaling, scale);
				// Peform scaling
				transform = scale * transform;
				// Perform the rotations for each dimension
				for (unsigned int i = 0; i < numDims; i++)
				{
					// Get the rotation axis
					unsigned int rotationDim = rotationOrder[i];
					// Make sure the rotation order is not out of bounds and that there is a rotation to perform
					if (rotationDim < numDims && rotation[rotationDim] != 0)
					{
						// Get the rotation matrix
						switch (rotationDim)
						{
						case 0:
							aiMatrix4x4::RotationX(rotation[0], rotate);
							break;
						case 1:
							aiMatrix4x4::RotationY(rotation[1], rotate);
							break;
						case 2:
							aiMatrix4x4::RotationZ(rotation[2], rotate);
							break;
						}
						// Peform the rotation
						transform = rotate * transform;
					}
				}
				// Get the translation matrix
				aiMatrix4x4::Translation(translation, translate);
				// Peform the translation
				transform = translate * transform;
				// Get the this joint's parent
				recursiveInd = jr.jointInfo.parentIndex;
			}
			// Check if we exited the loop normally
			if (recursiveInd == absoluteMaxJoints)
			{
				// Decompose the cumulative transform
				mdlsDecompose(transform, scaling, rotation, translation);
				// Copy the final pose into the pose array to return
				for (unsigned int i = 0; i < numDims; i++)
				{
					// Copy the values for dim i into the global joint
					jg.scaleFactors[i] = scaling[i];
					jg.rotations[i] = rotation[i];
					jg.coordinates[i] = translation[i];
				}
			}
			else // The loop timed out or we got an invalid index
			{
				valid = false;
			}
		}
	}
	else
	{
		valid = false;
	}
	return valid;
}

bool getJointsGlobal(vector<JointRelative> jr, vector<JointGlobal> &jg)
{
	bool valid = true;
	unsigned int numJoints = jr.size();
	JointGlobal jgi;
	jg.clear();
	jg.reserve(numJoints);
	for (unsigned int i = 0; valid && (i < numJoints); i++)
	{
		valid = getJointGlobal(jr, i, jgi);
		if (valid)
		{
			jg.push_back(jgi);
		}
	}
	return valid;
}

/* Traverse the Skeleton to get the global position of the vertex */
bool getVertexGlobal(vector<JointRelative> joints, VertexRelative vr, VertexGlobal &vg)
{
	// Initialize variables
	unsigned int parentInd = absoluteMaxJoints, recursiveInd = vr.originJointIndex, unknownJointInd, numJoints = joints.size(), rotationOrder[numDims] = { 1, 2, 0 };
	float rotations[numDims];
	vector<vector<float>> rotMat, globalPosVec = zeros(numDims, 1), jointPosVec = zeros(numDims, 1);
	bool valid = true, unknownFlag;
	// Copy the contents of vr int vg
	vg = *(VertexGlobal *)&(vr);
	// Initialize the global pose vector to the vertex's relative coordinates
	for (unsigned int i = 0; i < numDims; i++)
	{
		globalPosVec[i][0] = vr.coordinates[i];
	}
	// Make sure the index isn't out of range
	if (recursiveInd < numJoints && recursiveInd != absoluteMaxJoints)
	{
		// While not at the root node and the loop hasn't timed out
		for (unsigned int depth = 0; recursiveInd < numJoints && recursiveInd != absoluteMaxJoints && depth < loopTimeOut; depth++)
		{
			JointRelative jr = joints[recursiveInd];
			// Get the current joint's unknownFlag flag
			unknownFlag = jr.jointInfo.unknownFlag;
			// Get the current JointRelative's unknown index
			unknownJointInd = jr.jointInfo.unknownIndex;
			// Store the current joint's rotation values and pose
			for (unsigned int i = 0; i < numDims; i++)
			{
				// Get the parent rotation values
				rotations[i] = jr.rotations[i];
				// Get the parent pose
				jointPosVec[i][0] = (*(float *)&(jr.coordinates[i]));
			}
			// Perform the rotations for each dimension
			for (unsigned int i = 0; i < numDims; i++)
			{
				// Make sure the rotation order is not out of bounds and that there is a rotation to perform
				if (rotationOrder[i] < numDims && rotations[rotationOrder[i]] != 0)
				{
					// Get the rotation matrix
					rotMat = getRotationMatrix(rotationOrder[i], rotations[rotationOrder[i]]);
					// Perform the rotation on the current pose
					globalPosVec = matrixMultiply(rotMat, globalPosVec);
				}
			}
			// Combine the current joint's pose and global pose
			for (unsigned int i = 0; i < numDims; i++)
			{
				globalPosVec[i][0] += jointPosVec[i][0];
			}
			// Get the this joint's parent
			recursiveInd = jr.jointInfo.parentIndex;
		}
		// Check if we exited the loop normally
		if (recursiveInd == absoluteMaxJoints)
		{
			// Copy the final pose into the pose array to return
			for (unsigned int i = 0; i < numDims; i++)
			{
				// Copy the value for dim i into the pose array
				vg.coordinates[i] = globalPosVec[i][0];
			}
		}
		else // The loop timed out or we got an invalid index
		{
			valid = false;
		}
	}
	return valid;
}

bool getVerticesGlobal(vector<JointRelative> joints, vector<VertexRelative> vr, vector<VertexGlobal> &vg)
{
	bool valid = true;
	unsigned int numVerts = vr.size();;
	// Copy the relative vertices into vg
	vg = *(vector<VertexGlobal> *)&(vr);
	// Update the coordinates
	for (unsigned int i = 0; valid && i < numVerts; i++)
	{
		valid = getVertexGlobal(joints, vr[i], vg[i]);
	}
	return valid;
}

bool getVertexRelative(vector<JointRelative> joints, VertexGlobal vg, VertexRelative &vr)
{
	bool valid = true;
	unsigned int recursiveInd = vg.originJointIndex, numJoints = joints.size(), rotationOrder[numDims] = {0, 2, 1};
	float rotations[numDims];
	vector<unsigned int> path;
	vector<vector<float>> rotMat, relativePosVec = zeros(numDims, 1), jointRelativePosVec = zeros(numDims, 1);
	JointRelative jr = joints[recursiveInd];
	// Add this joint to the path
	path.push_back(recursiveInd);
	// Update recursiveInd to be the target joint's parent
	recursiveInd = jr.jointInfo.parentIndex;
	// Traverse the skeleton to find the path to the root and calculate the joint's global pose
	for (unsigned int depth = 0; recursiveInd < numJoints && recursiveInd != absoluteMaxJoints && depth < loopTimeOut; depth++)
	{
		// Add this joint to the path
		path.push_back(recursiveInd);
		// Update the recurrsive index
		recursiveInd = joints[recursiveInd].jointInfo.parentIndex;
	}
	// Check if we exited the loop normaly
	if (recursiveInd == absoluteMaxJoints)
	{
		// Copy the contents of the global vertex position into the relative vertex position
		vr = *(VertexRelative *)&(vg);
		// Update the relative vertex's coordinates to actually be relative to the joint
		for (unsigned int i = 0; i < numDims; i++)
		{
			//vr.coordinates[i] = vg.coordinates[i] - globalJointPose.coordinates[i];
			relativePosVec[i][0] = vg.coordinates[i];
		}
		// Traverse the saved path of joints
		while (path.size() > 0)
		{
			// Get the next joint in the path
			jr = joints[path.back()];
			// Save the joint's inverse rotations
			for (unsigned int i = 0; i < numDims; i++)
			{
				rotations[i] = -jr.rotations[i];
			}
			// Perform the inverse translations
			for (unsigned int i = 0; i < numDims; i++)
			{
				relativePosVec[i][0] -= jr.coordinates[i];
			}
			// Perform the inverse rotations
			for (unsigned int i = 0; i < numDims; i++)
			{
				rotMat = getRotationMatrix(rotationOrder[i], rotations[rotationOrder[i]]);
				relativePosVec = matrixMultiply(rotMat, relativePosVec);
			}
			// Remove the joint from the path
			path.pop_back();
		}
		// Update the relative vertex with the new value
		for (unsigned int i = 0; i < numDims; i++)
		{
			vr.coordinates[i] = relativePosVec[i][0];
		}
	}
	else
	{
		// We either timed out in the loop or we hit an invalid joint index
		valid = false;
	}
	return valid;
}

bool getVerticesRelative(vector<JointRelative> joints, vector<VertexGlobal> vg, vector<VertexRelative> &vr)
{
	bool valid = true;
	// Copy the global vertices into vr
	vr = *(vector<VertexRelative> *)&(vg);
	// Update the coordinates
	for (unsigned int i = 0; valid && i < vr.size(); i++)
	{
		valid = getVertexRelative(joints, vg[i], vr[i]);
	}
	return valid;
}

bool getFace(FaceEx fe, vector<VertexRelative> verts, Face &f)
{
	bool valid = true;
	unsigned int numVerts = verts.size();
	f.orientation = fe.orientation;
	for (unsigned int i = 0; valid && (i < numVertsPerFace); i++)
	{
		unsigned int j;
		for (j = 0; valid && (j < numVerts); j++)
		{
			if (fe.vertices[i] == verts[j])
			{
				f.vertexIndices[i] = j;
				break;
			}
		}
		if (j == numVerts)
		{
			valid = false;
		}
	}
	return valid;
}

bool getFaces(vector<FaceEx> facesEx, vector<VertexRelative> verts, vector<Face> &faces)
{
	bool valid = true;
	unsigned int numFaces = facesEx.size();
	// Clear the return vector
	faces.clear();
	// Allocate space for the faces
	faces.reserve(numFaces);
	// Get each face
	for (unsigned int i = 0; valid && (i < numFaces); i++)
	{
		Face f;
		valid = getFace(facesEx[i], verts, f);
		faces.push_back(f);
	}
	return valid;
}

bool getFaceEx(Face f, vector<VertexRelative> verts, FaceEx &fe)
{
	bool valid = true;
	unsigned int numVerts = verts.size();
	fe.orientation = f.orientation;
	for (unsigned int i = 0; valid && (i < numVertsPerFace); i++)
	{
		if (f.vertexIndices[i] < numVerts)
		{
			fe.vertices[i] = verts[f.vertexIndices[i]];
		}
		else
		{
			valid = false;
		}
	}
	return valid;
}

bool getFacesEx(vector<Face> faces, vector<VertexRelative> verts, vector<FaceEx> &facesEx)
{
	bool valid = true;
	unsigned int numFaces = faces.size();
	facesEx.clear();
	facesEx.reserve(numFaces);
	for (unsigned int i = 0; valid && (i < numFaces); i++)
	{
		FaceEx fe;
		valid = getFaceEx(faces[i], verts, fe);
		facesEx.push_back(fe);
	}
	return valid;
}

bool getInverseBindMatrix(vector<JointRelative> joints, unsigned int jointIndex, vector<vector<float>> &ibm, float max)
{
	bool valid = jointIndex < joints.size();
	unsigned int dimOff;
	float translation[numDims];
	vector<vector<float>> transformMat;
	// Initialize the inverse bind matrix
	ibm = eye(4);
	// Make sure the joint index is valid
	if (valid)
	{
		// Make sure the joint index is valid and that we aren't at the root joint
		while (valid && jointIndex != absoluteMaxJoints)
		{
			// Get inverse rotations
			for (unsigned int i = 0; i < numDims; i++)
			{
				// Get the dimension offset to access the dimensions in the order Y, Z, X
				dimOff = (i + 1) % numDims;
				// Get the inverse rotation matrix for dim dimOffset
				transformMat = getRotationMatrix(dimOff, -joints[jointIndex].rotations[dimOff], 4);
				// Incorperate the inverse rotation matrix for dim dimOffset into the inverse bind matrix
				ibm = matrixMultiply(ibm, transformMat);
				// Store the inverse translation for dim dimOffset in the translation array
				translation[dimOff] = -(joints[jointIndex].coordinates[dimOff]) / max;
			}
			// Get inverse translation matrix
			transformMat = getTranslationMatrix(translation);
			// Incorperate the inverse tranlation matrix into the inverse bind matrix
			ibm = matrixMultiply(ibm, transformMat);
			// Update the joint index to be the parent and make sure it is still valid
			jointIndex = joints[jointIndex].jointInfo.parentIndex;
			valid = jointIndex < joints.size();
		}
		// If the while loop terminated because we reached the root node, the output is still valid
		if (jointIndex == absoluteMaxJoints)
		{
			valid = true;
		}
	}
	return valid;
}

bool getPolygonCollections(vector<VertexRelative> verts, vector<Face> faces, vector<vector<vector<vector<unsigned int>>>> &polygonCollections)
{
	bool valid = true, canConnectTL, canConnectPSC = true, foundVertex, vertStatus[numDims], swap = false;
	const unsigned int lengthThreshold = 4096;
	unsigned int numTextures = getNumTextures(verts), currentTextureIndex = numTextures, vertIndex, subCollectionLen, triangleListLen;
	vector<vector<vector<unsigned int>>> polygonCollection;
	vector<vector<unsigned int>> polygonSubCollection;
	vector<unsigned int> triangleList;
	// Clear the return value
	polygonCollections.clear();
	// Parse the faces to separate into polygon collections based on texture index
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		// Get Face i
		Face f = faces[i], fp;
		// Reset the can connect flag
		canConnectTL = true;
		// Get the index of the first vertex of the face
		vertIndex = f.vertexIndices[0];
		// Make sure the vertex is valid
		if (vertIndex < verts.size())
		{
			// Check if we are still using the same polygon collection
			if ((verts[vertIndex].textureIndex != currentTextureIndex))
			{
				if (triangleList.size() > 0)
				{
					// Add the finished triangle list to the polygon sub-collection
					polygonSubCollection.push_back(triangleList);
					// Clear the triangle list to be used for the new polygon sub-collection
					triangleList.clear();
				}
				if (polygonSubCollection.size() > 0)
				{
					// Add the finished polygon sub-collection the the polygon collection
					polygonCollection.push_back(polygonSubCollection);
					// Clear the polygon sub-collection for the new polygon list
					polygonSubCollection.clear();
					canConnectPSC = true;
				}
				if (polygonCollection.size() > 0)
				{
					// Add the finished polygon collection to the output vector
					polygonCollections.push_back(polygonCollection);
					// Clear the polygon collection to be used for the new polygon collection
					polygonCollection.clear();
				}
				// Update the current texture index
				currentTextureIndex = verts[vertIndex].textureIndex;
			}
		}
		// If this isn't the first face, check if we can connect it to the existing triangle list
		if (triangleList.size() >= 3)
		{
			// Get the previous face
			fp = faces[i - 1];
			unsigned int ind;
			// Check which face should be converted to clockwise for the comparison
			if (swap)
			{
				ind = f.vertexIndices[0];
				f.vertexIndices[0] = f.vertexIndices[1];
				f.vertexIndices[1] = ind;
			}
			else
			{
				ind = fp.vertexIndices[0];
				fp.vertexIndices[0] = fp.vertexIndices[1];
				fp.vertexIndices[1] = ind;
			}
			// For each vertex in the face...
			for (unsigned int j = 0; j < numVertsPerFace; j++)
			{
				// Get the index of vertex j
				vertIndex = f.vertexIndices[j];
				// Make sure the vertex is valid
				if (vertIndex < verts.size())
				{
					// Reset the found vertex flag
					foundVertex = false;
					// For each vertex that makes up the previous face...
					for (unsigned int k = 0; !foundVertex && (k < numDims); k++)
					{
						// Check if it matches a vertex from the previous face
						if (vertIndex == fp.vertexIndices[k])
						{
							// It does, set the found vertex flag
							foundVertex = true;
						}
					}
					// Save the status of the vertex
					vertStatus[j] = foundVertex;
					// Check if we found the vertex in the previous face
					if (!foundVertex)
					{
						// We didn't, so now check if this was the only different vertex
						for (unsigned int k = 0; k < j && canConnectTL; k++)
						{
							canConnectTL = canConnectTL && vertStatus[k];
						}
					}
				}
				else
				{
					cout << "Error: Invalid Vertex Index referenced by Face " << i << endl;
					valid = false;
				}
			}
		}
		// Make sure the face was valid
		if (valid)
		{
			// Check if this is the first face
			if (triangleList.size() < 3)
			{
				// Add all 3 vertices to the triangle list
				for (unsigned int j = 0; j < numVertsPerFace; j++)
				{
					// Get the index of vertex j
					vertIndex = f.vertexIndices[j];
					// Make sure the vertex is valid
					if (vertIndex < verts.size())
					{
						// This is the first face, so all we have to do is add the first 3 verts
						triangleList.push_back(vertIndex);
					}
					else
					{
						cout << "Error: Invalid Vertex Index referenced by Face " << i << endl;
						valid = false;
					}
				}
				// The next face will be clockwise
				swap = true;
			}
			else
			{
				// Check if we should transition to a new polygon sub-collection by estimating the current subcollection size
				subCollectionLen = 48;
				for (unsigned int j = 0; j < polygonSubCollection.size(); j++)
				{
					subCollectionLen += polygonSubCollection[j].size() * 176 + 32;
				}
				triangleListLen = (triangleList.size() + 1) * 176 + 32;
				// Use the estimated sub-collection size to see if we should start a new sub-collection
				if ((subCollectionLen + triangleListLen) > lengthThreshold)
				{
					// Adding this face to the current triangle list could push it over the size limit
					canConnectTL = false;
					canConnectPSC = false;
				}
				// Check if this face can connect to the previous face
				if (canConnectTL)
				{
					if (triangleList.size() == 3)
					{
						Face fr = f, fpr = fp;
						// We can play with the order of the original face to append the second face
						for (unsigned int j = 0; j < numVertsPerFace; j++)
						{
							if (vertStatus[0] && (f.vertexIndices[0] == fp.vertexIndices[j]))
							{
								// Use vertex 0 neighbors
								if (f.vertexIndices[1] == (fp.vertexIndices[(j + 1) % numVertsPerFace]))
								{
									fpr.vertexIndices[0] = fp.vertexIndices[(j + 2) % numVertsPerFace];
									fpr.vertexIndices[1] = fp.vertexIndices[j];
									fpr.vertexIndices[2] = fp.vertexIndices[(j + 1) % numVertsPerFace];
									vertStatus[2] = false;
								}
								else if (f.vertexIndices[2] == (fp.vertexIndices[(j + 2) % numVertsPerFace]))
								{
									fr.vertexIndices[0] = f.vertexIndices[2];
									fr.vertexIndices[1] = f.vertexIndices[0];
									fr.vertexIndices[2] = f.vertexIndices[1];
									fpr.vertexIndices[0] = fp.vertexIndices[(j + 1) % numVertsPerFace];
									fpr.vertexIndices[1] = fp.vertexIndices[(j + 2) % numVertsPerFace];
									fpr.vertexIndices[2] = fp.vertexIndices[j];
									vertStatus[1] = true;
									vertStatus[2] = false;
								}
								else
								{
									// This face cannot connect, because it is the incorrect orientation
									canConnectTL = false;
								}
								break;
							}
							else if (f.vertexIndices[1] == fp.vertexIndices[j])
							{
								// Use vertex 1 neighbors
								if (f.vertexIndices[2] == (fp.vertexIndices[(j + 1) % numVertsPerFace]))
								{
									fr.vertexIndices[0] = f.vertexIndices[1];
									fr.vertexIndices[1] = f.vertexIndices[2];
									fr.vertexIndices[2] = f.vertexIndices[0];
									fpr.vertexIndices[0] = fp.vertexIndices[(j + 2) % numVertsPerFace];
									fpr.vertexIndices[1] = fp.vertexIndices[j];
									fpr.vertexIndices[2] = fp.vertexIndices[(j + 1) % numVertsPerFace];
									vertStatus[0] = true;
									vertStatus[2] = false;
								}
								else if (f.vertexIndices[0] == (fp.vertexIndices[(j + 2) % numVertsPerFace]))
								{
									fpr.vertexIndices[0] = fp.vertexIndices[(j + 1) % numVertsPerFace];
									fpr.vertexIndices[1] = fp.vertexIndices[(j + 2) % numVertsPerFace];
									fpr.vertexIndices[2] = fp.vertexIndices[j];
									vertStatus[2] = false;
								}
								else
								{
									// This face cannot connect because it is the incorrect orientation
									canConnectTL = false;
								}
								break;
							}
						}
						// Update the face order if we need to
						if (canConnectTL)
						{
							// Update f and fp
							f = fr;
							fp = fpr;
							// Check if we need to swap fpr
							if (!swap)
							{
								unsigned int ind = fpr.vertexIndices[0];
								fpr.vertexIndices[0] = fpr.vertexIndices[1];
								fpr.vertexIndices[1] = ind;
							}
							// Update the stored previous face in the triangle list
							for (unsigned int j = 0; j < numVertsPerFace; j++)
							{
								triangleList[j] = fpr.vertexIndices[j];
							}
						}
					}
					else
					{
						// We must enforce the established order
						canConnectTL = f.vertexIndices[0] == fp.vertexIndices[1] && f.vertexIndices[1] == f.vertexIndices[2];
					}
				}
				// Check if the face is valid to connect to the triangle list
				if (canConnectTL)
				{
					// Check the status of the vertices to find the new vertex
					for (unsigned int j = 0; j < numDims; j++)
					{
						if (!vertStatus[j])
						{
							// Add the new vertex to the triangle list
							triangleList.push_back(f.vertexIndices[j]);
							break;
						}
						// Just in case...You shouldn't be able to achieve this condition
						if (j == 2)
						{
							// Add the first vertex from the face
							triangleList.push_back(f.vertexIndices[0]);
						}
					}
					// The next face should be the opposite orientation
					swap = !swap;
				}
				else
				{
					// Add the triangle list to the polygon sub-collection
					polygonSubCollection.push_back(triangleList);
					// Clear the triangle list to use for the new list
					triangleList.clear();
					// Check if we should transition to a new polygon sub-collection
					if (!canConnectPSC)
					{
						// Add the polygon sub-collection to the polygon collection
						polygonCollection.push_back(polygonSubCollection);
						// Clear the polygon sub-collection to use for the next sub-collection
						polygonSubCollection.clear();
						canConnectPSC = true;
					}
					// Add all of the vertices from the current face to the new triangle list
					for (unsigned int j = 0; j < numDims; j++)
					{
						triangleList.push_back(faces[i].vertexIndices[j]);
					}
					// The next face will be clockwise
					swap = true;
				}
			}
		}
	}
	// Check if we need to add the last triangle list to the current polygon sub-collection
	if (triangleList.size() > 0)
	{
		polygonSubCollection.push_back(triangleList);
	}
	// Check if we need to add the last polygon sub-collection to the current polygon collection
	if (polygonSubCollection.size() > 0)
	{
		polygonCollection.push_back(polygonSubCollection);
	}
	// Check if we need to add the last polygon collection to the output
	if (polygonCollection.size() > 0)
	{
		polygonCollections.push_back(polygonCollection);
	}
	return valid;
}

unsigned int countFaces(vector<vector<vector<vector<unsigned int>>>> polygonCollections)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < polygonCollections.size(); i++)
	{
		vector<vector<vector<unsigned int>>> *polygonCollection = &(polygonCollections[i]);
		for (unsigned int j = 0; j < polygonCollection->size(); j++)
		{
			vector<vector<unsigned int>> *subCollection = &((*polygonCollection)[j]);
			for (unsigned int k = 0; k < subCollection->size(); k++)
			{
				vector<unsigned int> *triangleList = &((*subCollection)[k]);
				unsigned int numVerts = triangleList->size();
				if (numVerts >= 3)
				{
					count += numVerts - 2;
				}
				else
				{
					cout << "Warning: Invalid Triangle List!" << endl;
				}
			}
		}
	}
	return count;
}

bool compareFacesToPolygonCollections(vector<Face> faces, vector<vector<vector<vector<unsigned int>>>> polygonCollections)
{
	vector<Face> collectionFaces;
	unsigned int numFaces = faces.size(), numPolygonCollectionFaces = countFaces(polygonCollections);
	bool result = numFaces == numPolygonCollectionFaces, swap = false;
	if (result)
	{
		unsigned int polygonCollectionInd = 0, subCollectionInd = 0, triangleListInd = 0, vertexInd = 0;
		collectionFaces.reserve(numFaces);
		for (unsigned int i = 0; result && i < numFaces; i++)
		{
			Face fi = faces[i], fpci;
			bool matchingFace = true;
			if (polygonCollectionInd < polygonCollections.size())
			{
				vector<vector<vector<unsigned int>>> pc = polygonCollections[polygonCollectionInd];
				if (subCollectionInd < pc.size())
				{
					vector<vector<unsigned int>> sc = pc[subCollectionInd];
					if (triangleListInd < sc.size())
					{
						vector<unsigned int> tl = sc[triangleListInd];
						unsigned int j;
						for (j = 0; vertexInd < tl.size() && j < numVertsPerFace; j++)
						{
							fpci.vertexIndices[j] = tl[vertexInd];
							vertexInd++;
						}
						if (j != numVertsPerFace)
						{
							matchingFace = false;
						}
						else
						{
							collectionFaces.push_back(fpci);
						}
						if (swap)
						{
							unsigned int val = fpci.vertexIndices[1];
							fpci.vertexIndices[1] = fpci.vertexIndices[2];
							fpci.vertexIndices[2] = val;
						}
						for (j = 0; matchingFace && j < numVertsPerFace; j++)
						{
							if (fi.vertexIndices[j] != fpci.vertexIndices[j])
							{
								matchingFace = false;
							}
						}
						result = result && matchingFace;
						if (vertexInd == tl.size())
						{
							vertexInd = 0;
							triangleListInd++;
							swap = false;
							if (triangleListInd == sc.size())
							{
								triangleListInd = 0;
								subCollectionInd++;
								if (subCollectionInd == pc.size())
								{
									subCollectionInd = 0;
									polygonCollectionInd++;
								}
							}
						}
						else
						{
							vertexInd -= 2;
							swap = !swap;
						}
					}
					else
					{
						result = false;
					}
				}
				else
				{
					result = false;
				}
			}
			else
			{
				result = false;
			}
		}
	}
	return result;
}

VertexRelative convertJointToVert(JointRelative joint)
{
	VertexRelative v;
	// Make the vertex relative to joint i
	v.originJointIndex = joint.ind;
	// Dummy variables
	v.coordinatesScaleFactor = 1;
	v.textureScaleFactor = 1;
	for (unsigned int j = 0; j < numDims; j++)
	{
		// Dummy variables
		v.normal[j] = 1;
		// Set the vertex's position to { 0, 0, 0 } so that it matches the joint's position
		v.coordinates[j] = 0;
	}
	for (unsigned int j = 0; j < 2; j++)
	{
		// Dummy variables
		v.textureMap[j] = 0;
	}
	return v;
}

vector<VertexRelative> convertJointsToVerts(vector<JointRelative> joints)
{
	VertexRelative v;
	vector<VertexRelative> verts;
	// Reserve space for each vertex
	verts.reserve(joints.size());
	// Create a vertex for each joint
	for (unsigned int i = 0; i < joints.size(); i++)
	{
		v = convertJointToVert(joints[i]);
		// Add the vertex to the verts vector
		verts.push_back(v);
	}
	return verts;
}

Animation convertMsetStaticAnimationToAnimation(vector<JointRelative> joints, MsetStaticAnimation mAnim)
{
	Animation anim;
	// Make sure the joint id is valid
	if (mAnim.jointID < joints.size())
	{
		float scaleFactors[numDims], rotations[numDims], translations[numDims];
		vector<vector<float>> s, r = eye(4), t, h = eye(4);
		for(unsigned int i = 0; i < mAnim.transforms.size() && i < mAnim.hasTransform.size() && i < 9; i++)
		{
			unsigned int ind = (i + 1) % 3;
			if (mAnim.hasTransform[i])
			{
				if (i < 3)
				{
					scaleFactors[ind] = mAnim.transforms[i];
				}
				else if (i < 6)
				{
					rotations[ind] = mAnim.transforms[i];
				}
				else
				{
					translations[ind] = mAnim.transforms[i];
				}
			}
			else
			{
				if (i < 3)
				{
					scaleFactors[ind] = joints[mAnim.jointID].scaleFactors[ind];
				}
				else if (i < 6)
				{
					rotations[ind] = joints[mAnim.jointID].rotations[ind];
				}
				else
				{
					translations[ind] = joints[mAnim.jointID].coordinates[ind];
				}
			}
		}
		// Convert the scale, rotate, and translate arrays into 4x4 matrices
		s = getScaleMatrix(scaleFactors, 4);
		for (unsigned int i = 0; i < numDims; i++)
		{
			unsigned int off = (i + 1) % numDims;
			r = matrixMultiply(getRotationMatrix(off, rotations[off], 4), r);
		}
		t = getTranslationMatrix(translations);
		// Combine the matrices to form the homogenous matrix
		h = matrixMultiply(s, h);
		h = matrixMultiply(r, h);
		h = matrixMultiply(t, h);
		// Add everything to the animation
		anim.jointInd = mAnim.jointID;
		anim.times.push_back(0);
		anim.keyframes.push_back(h);
	}
	return anim;
}

vector<Animation> convertMsetStaticAnimationsToAnimations(vector<JointRelative> joints, vector<MsetStaticAnimation> mAnims)
{
	Animation anim;
	vector<Animation> anims;
	// Reserve space for each vertex
	anims.reserve(mAnims.size());
	// Convert each static anim
	for (unsigned int i = 0; i < mAnims.size(); i++)
	{
		anim = convertMsetStaticAnimationToAnimation(joints, mAnims[i]);
		// Add the vertex to the verts vector
		anims.push_back(anim);
	}
	return anims;
}

bool normalizePosition(vector<VertexGlobal> vertices, float adjustments[], float &max)
{
	bool valid = true;
	// Initialize max
	max = numeric_limits<float>::min();
	float min = numeric_limits<float>::max(), tempCoord, maxes[numDims] = { max, max, max }, mins[numDims] = { min, min, min };
	// Find the min and max values for the coordinates
	for (unsigned int i = 0; i < vertices.size(); i++)
	{
		for (unsigned int j = 0; j < numDims; j++)
		{
			tempCoord = vertices[i].coordinates[j];
			if (tempCoord < mins[j])
			{
				mins[j] = tempCoord;
			}
			if (tempCoord > maxes[j])
			{
				maxes[j] = tempCoord;
			}
		}
	}
	// Translate the model to be centered around the origin
	for (unsigned int i = 0; i < numDims; i++)
	{
		// Determine the middle value for the dim
		adjustments[i] = (maxes[i] + mins[i]) / 2;
		// Adjust the max and min values for this dim to reflect the transform
		maxes[i] -= adjustments[i];
		mins[i] -= adjustments[i];
		// Determine the absolute min and max values
		if (mins[i] < min)
		{
			min = mins[i];
		}
		if (maxes[i] > max)
		{
			max = maxes[i];
		}
	}
	// Determine max magnitude for scaling
	if ((-min) > max)
	{
		max = -min;
	}
	return valid;
}

// Parse through the template vertex data and modify the origin update sections to match the rigging from vertices
bool updateRigging(vector<JointRelative> joints, vector<JointRelative> templateJoints, vector<VertexRelative> vertices, vector<Face> faces, string templateVertexData, string &vertexData)
{
	bool valid;
	unsigned int readIndex = 0, type, jointIndex, originIndex, vertexInd, vertexIndInd, faceInd = 0, numJoints = joints.size(), numVerts = vertices.size(), numFaces = faces.size(), numTemplateJoints = templateJoints.size(), templateLen = templateVertexData.size(), originJointInds[numOrigins] = { 0, 0, 0, 0, 0, 0 }, updatedOrigins[numOrigins] = { 0, 0, 0, 0, 0, 0 };
	vector<vector<unsigned int>> jointRelations = getJointRelations(templateJoints, joints);
	vector<VertexGlobal> vertsG;
	// Make sure we got valid relations and get the global vertices
	valid = (jointRelations.size() != 0) && getVerticesGlobal(joints, vertices, vertsG);
	// Parse the template vertex data
	while (valid && (readIndex + 3) < templateLen)
	{
		// Get the type of entry
		type = *(unsigned int *)&(templateVertexData[readIndex]);
		// Check the type
		if (type == 0 && (readIndex + 127) < templateLen)
		{
			// It is an origin update, get the joint index and origin index
			jointIndex = *(unsigned int *)&(templateVertexData[readIndex + 4]);
			originIndex = *(unsigned int *)&(templateVertexData[readIndex + 8]);
			// Validate the origin index
			if (originIndex >= numOrigins)
			{
				originIndex = numOrigins - 1;
			}
			// Save the origin joint index
			originJointInds[originIndex] = jointIndex;
			// Make sure the index is valid
			if (jointIndex < numTemplateJoints)
			{
				// Store the template joint
				unsigned int relation = jointRelations[0][jointIndex];
				// Make sure a match was found
				if (relation != numJoints)
				{
					unsigned int newJointInd = joints[relation].ind;
					*(unsigned int *)&(templateVertexData[readIndex + 4]) = newJointInd;
					updatedOrigins[originIndex] = newJointInd;
				}
				else
				{
					// A match wasn't found, notify the user
					cout << "Warning: Match not found for joint" << templateJoints[jointIndex].ind << endl;
				}
			}
			else
			{
				valid = false;
			}
			// Move to the next entry
			readIndex += 128;
		}
		else if (type == 1 && (readIndex + 31) < templateLen)
		{
			// This entry is a triangle list
			// Get the section length and the number of vertices in the list
			unsigned int sectionLen = *(unsigned int *)&(templateVertexData[readIndex + 4]), numTemplateVerts = *(unsigned int *)&(templateVertexData[readIndex + 8]);
			// Reset the vertexIndInd
			vertexIndInd = 0;
			// Move to the vertex data
			readIndex += 32;
			// For each vertex
			for (unsigned int i = 0; i < numTemplateVerts && (readIndex + 47) < templateLen; i++)
			{
				// Extract vertex i from the template subsection
				if (faceInd < numFaces)
				{
					Face f = faces[faceInd];
					if (faceInd > 0 && vertexIndInd == 0)
					{
						Face fp = faces[faceInd - 1];
						if (fp.vertexIndices[1] == f.vertexIndices[0] && fp.vertexIndices[2] == f.vertexIndices[1])
						{
							vertexIndInd = 2;
						}
					}
					vertexInd = f.vertexIndices[vertexIndInd];
					if (vertexInd < numVerts)
					{
						VertexGlobal vg, vg2;
						VertexRelative customVert, vr, vr2;
						// Get the origin index
						originIndex = *(unsigned int *)&(templateVertexData[readIndex + 12]);
						// Fill out vr
						vr.originJointIndex = originJointInds[originIndex];
						vr.coordinatesScaleFactor = *(float *)&(templateVertexData[readIndex + 28]);
						vr.textureScaleFactor = *(float *)&(templateVertexData[readIndex + 40]);
						for (unsigned int j = 0; j < numDims; j++)
						{
							unsigned int swap = (j + 2) % numDims;
							vr.normal[j] = *(float *)&(templateVertexData[readIndex + (swap * 4)]);
							vr.coordinates[j] = *(float *)&(templateVertexData[readIndex + (swap * 4) + 16]);
							if (j == 0)
							{
								vr.textureMap[0] = *(float *)&(templateVertexData[readIndex + 32]);
							}
							else if (j == 1)
							{
								vr.textureMap[1] = 1 - *(float *)&(templateVertexData[readIndex + 36]);
							}
						}
						if (getVertexGlobal(templateJoints, vr, vg))
						{
							vg2 = vg;
							vg2.originJointIndex = updatedOrigins[originIndex];
							if (getVertexRelative(joints, vg2, vr2))
							{
								if (!compareVertices(vr, vr2))
								{
									customVert = vr2;
								}
								else
								{
									customVert = vr;
								}
							}
							else
							{
								customVert = vr;
							}
						}
						else
						{
							customVert = vr;
						}
						*(float *)&(templateVertexData[readIndex + 28]) = customVert.coordinatesScaleFactor;
						*(float *)&(templateVertexData[readIndex + 40]) = customVert.textureScaleFactor;
						for (unsigned int j = 0; j < numDims; j++)
						{
							unsigned int swap = (j + 2) % numDims;
							*(float *)&(templateVertexData[readIndex + (swap * 4)]) = customVert.normal[j];
							*(float *)&(templateVertexData[readIndex + (swap * 4) + 16]) = customVert.coordinates[j];
							if (j == 0)
							{
								*(float *)&(templateVertexData[readIndex + 32]) = customVert.textureMap[0];
							}
							else if (j == 1)
							{
								*(float *)&(templateVertexData[readIndex + 36]) = 1 - customVert.textureMap[1];
							}
						}
						vertexIndInd++;
						if (vertexIndInd == 3)
						{
							faceInd++;
							vertexIndInd = 0;
						}
					}
				}
				// Compare to vertices
				readIndex += 48;
			}
		}
		else if (type == 0 || type == 1)
		{
			valid = false;
		}
		else
		{
			// Skip
			readIndex += 16;
		}
	}
	vertexData = templateVertexData;
	return valid;
}

bool compareJoints(JointGlobal a, JointGlobal b, float threshold)
{
	bool result, matchingCoords = true, matchingRotations = true, matchingChildren = (a.childrenIndices.size() == b.childrenIndices.size());
	float poseA, poseB, rotationA, rotationB, diff;
	for (unsigned int i = 0; i < numDims; i++)
	{
		poseA = a.coordinates[i] * a.scaleFactors[i];
		poseB = a.coordinates[i] * b.scaleFactors[i];
		diff = poseA - poseB;
		matchingCoords = matchingCoords && (diff < threshold) && (diff > -threshold);
		rotationA = a.rotations[i];
		rotationB = b.rotations[i];
		diff = rotationA - rotationB;
		matchingRotations = matchingRotations && (diff < threshold) && (diff > -threshold);
	}
	result = matchingCoords && matchingRotations && matchingChildren;
	return result;
}

bool compareVertices(VertexGlobal a, VertexGlobal b, float threshold)
{
	return compareVertices(*(VertexRelative *)&a, *(VertexRelative *)&b, threshold);
}

bool compareVertices(VertexRelative a, VertexRelative b, float threshold)
{
	bool result, matchingPoses = true, matchingNormals = true, matchingTextures = (a.textureIndex == b.textureIndex);
	float poseA, poseB, normalA, normalB, texMapA[2], texMapB[2], diff;
	for (unsigned int i = 0; i < numDims; i++)
	{
		poseA = a.coordinates[i] * a.coordinatesScaleFactor;
		poseB = b.coordinates[i] * b.coordinatesScaleFactor;
		diff = poseA - poseB;
		matchingPoses = matchingPoses && (diff < threshold) && (diff > -threshold);
		normalA = a.normal[i];
		normalB = b.normal[i];
		diff = normalA - normalB;
		matchingNormals = matchingNormals && (diff < threshold) && (diff > -threshold);
		if (i < 2)
		{
			texMapA[i] = a.textureMap[i] * a.textureScaleFactor;
			texMapB[i] = b.textureMap[i] * b.textureScaleFactor;
			diff = texMapA[i] - texMapB[i];
			matchingTextures = matchingTextures && (diff < threshold) && (diff > -threshold);
		}
	}
	result = matchingPoses && matchingTextures;
	return result;
}

vector<vector<unsigned int>> getJointRelations(vector<JointRelative> jointsI, vector<JointRelative> jointsJ, float threshold)
{
	// Get the sizes of the input vectors
	unsigned int numJointsI = jointsI.size(), numJointsJ = jointsJ.size();
	vector<vector<unsigned int>> relations;
	relations.reserve(2);
	// Convert the joints to relative vertices
	vector<VertexRelative> jir = convertJointsToVerts(jointsI), jvr = convertJointsToVerts(jointsJ);
	vector<VertexGlobal> jig, jvg;
	// Convert the relative vertices to the global frame of reference
	if (getVerticesGlobal(jointsI, jir, jig) && getVerticesGlobal(jointsJ, jvr, jvg))
	{
		relations.push_back(vector<unsigned int>());
		relations.push_back(vector<unsigned int>());
		// Reserve space in the output
		relations[0].reserve(numJointsI);
		relations[1].reserve(numJointsJ);
		for (unsigned int i = 0; i < numJointsI; i++)
		{
			relations[0].push_back(numJointsJ);
		}
		for (unsigned int i = 0; i < numJointsJ; i++)
		{
			relations[1].push_back(numJointsI);
		}
		// For each vertex in the first vector
		for (unsigned int i = 0; i < numJointsI; i++)
		{
			// For each vertex in the second vector
			for (unsigned int j = 0; j < numJointsJ; j++)
			{
				// Compare the vertices
				if ((relations[0][i] == numJointsJ) && (relations[1][j] == numJointsI) && compareVertices(jig[i], jvg[j], threshold) && compareVertices(jir[i], jvr[j], threshold))
				{
					// They match, so store the relation and break from the inner loop
					relations[0][i] = j;
					relations[1][j] = i;
					break;
				}
			}
		}
	}
	return relations;
}

template<typename TReal>
void mdlsDecompose(aiMatrix4x4t<TReal> transform, aiVector3t<TReal> &pScaling, aiVector3t<TReal> &pRotation, aiVector3t<TReal> &pPosition)
{
	// Extract the columns of the vector
	aiVector3t<TReal> vCols[3] = {	aiVector3t<TReal>(transform[0][0], transform[1][0], transform[2][0]), 
									aiVector3t<TReal>(transform[0][1], transform[1][1], transform[2][1]), 
									aiVector3t<TReal>(transform[0][2], transform[1][2], transform[2][2])	};
	// Extract the scaling factors
	pScaling.x = vCols[0].Length();
	pScaling.y = vCols[1].Length();
	pScaling.z = vCols[2].Length();
	// Extract the sign of the scaling
	if (transform.Determinant() < 0)
	{
		pScaling = -pScaling;
	}
	// Extract the translation from the homogenous matrix
	pPosition.x = transform[0][3];
	pPosition.y = transform[1][3];
	pPosition.z = transform[2][3];
	// Remove any scaling from the matrix
	if (pScaling.x)
	{
		vCols[0] /= pScaling.x;
	}
	if (pScaling.y)
	{
		vCols[1] /= pScaling.y;
	}
	if (pScaling.z)
	{
		vCols[2] /= pScaling.z;
	}
	// Use small epsilon to account for small floating point inaccuracies
	const TReal epsilon = 10e-3f;
	pRotation.z = asin(-vCols[1].x);	// F, angle around z
	TReal E = cos(pRotation.z);			// E
	if (fabs(E) > epsilon)
	{
		// Find the angle around x
		TReal tan_x = vCols[1].y / E;	// A
		TReal tan_y = vCols[1].z / E;	// B
		pRotation.x = atan2(tan_y, tan_x);
		// Find the angle around y
		tan_x = vCols[0].x / E;			// C
		tan_y = vCols[2].x / E;			// D
		pRotation.y = atan2(tan_y, tan_x);
	}
	else
	{
		pRotation.y = 0;	// Set angle around y to 0. => C = 1, D = 0, E = 0, F = 1
		// Find the angle around z
		TReal tan_x = vCols[2].z;		// BDF+AC => A
		TReal tan_y = -vCols[2].y;		// ADF-BC => -B
		pRotation.x = atan2(tan_y, tan_x);
	}
	return;
}

bool readTexture(string filename, Texture &t)
{
	bool result = true, dispWarning = false;
	unsigned int width, height, targetNumColors = 256;
	vector<unsigned char> pngData, rawData;
	// Load the contents of the file
	result = lodepng::load_file(pngData, filename) == 0;
	// Make sure we loaded the png data successfully
	if (result)
	{
		// Decode the png data into raw RGBA data
		result = lodepng::decode(rawData, width, height, pngData, LodePNGColorType::LCT_RGBA) == 0;
		// Make sure the decode was successful
		if (result)
		{
			vector<unsigned char> inds;
			Palette p;
			// Initialize the texture with known values
			t.width = width;
			t.height = height;
			t.wExp = 0;
			t.hExp = 0;
			while ((t.width >> t.wExp) > 1)
			{
				t.wExp++;
			}
			while ((t.height >> t.hExp) > 1)
			{
				t.hExp++;
			}
			p.numColors = 0;
			// For each pixel
			for (unsigned int i = 0; i < rawData.size(); i += 4)
			{
				// Store this pixel's color values
				unsigned char r = rawData[i], g = rawData[i + 1], b = rawData[i + 2], a = rawData[i + 3], j;
				// Check the palette for this color
				for (j = 0; j < p.numColors; j++)
				{
					// Check if we found the color
					if (r == p.r[j] && g == p.g[j] && b == p.b[j] && a == p.a[j])
					{
						// Add the index
						inds.push_back(j);
						break;
					}
				}
				if ((j == p.numColors) && (p.numColors < targetNumColors))
				{
					// This color isn't in the palette, so add the color to the palette
					p.r.push_back(r);
					p.g.push_back(g);
					p.b.push_back(b);
					p.a.push_back(a);
					p.numColors++;
					// Add the index
					inds.push_back(j);
				}
				else if (j == p.numColors)
				{
					// The palette overflowed
					if (!dispWarning)
					{
						cout << "Warning: The palette overflowed!" << endl << "Make sure to use 256 colors or less" << endl;
						dispWarning = true;
					}
					inds.push_back(0);
				}
			}
			// Fill any empty slots in the palette
			for (p.numColors; p.numColors < targetNumColors; p.numColors++)
			{
				p.r.push_back(0);
				p.g.push_back(0);
				p.b.push_back(0);
				p.a.push_back(0);
			}
			// Store the texture's indices and palette
			t.indices = inds;
			t.palette = p;
		}
	}
	return result;
}

bool writeTexture(Texture t, string filename)
{
	bool result;
	vector<unsigned char> pngData, rawData = textureToRaw(t);
	result = lodepng::encode(pngData, rawData, t.width, t.height, LodePNGColorType::LCT_RGBA) == 0;
	if (result)
	{
		result = lodepng::save_file(pngData, filename) == 0;
	}
	return result;
}

vector<MsetStaticAnimation> getMsetAnimations(string raw)
{
	vector<MsetStaticAnimation> anims;
	unsigned int readOffset = 0, msetLen = raw.size();
	// Make sure the data is long enough to have the number of section offsets
	if (msetLen >= 4)
	{
		// Extract the number of section offsets
		unsigned int numSections = *(unsigned int *)&(raw[readOffset]);
		readOffset += 4;
		// Make sure the number of sections is 3 (That identifies it as an MSET)
		if (numSections == 3)
		{
			// Make sure there is enough data for the MSET header
			if (msetLen >= 16)
			{
				// Extract the mmtn offset
				unsigned int mmtnOffset = *(unsigned int *)&(raw[readOffset]), mmtnLen = *(unsigned int *)&(raw[readOffset + 4]) - mmtnOffset;
				if (msetLen >= (mmtnOffset + mmtnLen))
				{
					string mmtn = raw.substr(mmtnOffset, mmtnLen);
					readOffset = 0;
					// Make sure the mmtn header exists
					if (mmtnLen >= 64)
					{
						// Check the magic id
						if (mmtn.substr(readOffset, 4) == "MMTN")
						{
							readOffset += 4;
							unsigned int actualMmtnLen = *(unsigned int *)&(mmtn[readOffset]) + 8;
							if (actualMmtnLen <= mmtnLen)
							{
								unsigned int animationsOffset, animationsLen;
								readOffset = 48;
								animationsOffset = *(unsigned int *)&(mmtn[readOffset]);
								readOffset += 4;
								animationsLen = *(unsigned int *)&(mmtn[readOffset]);
								if (actualMmtnLen >= (animationsOffset + animationsLen))
								{
									unsigned int numAnims, remainder;
									vector<unsigned int> animationOffsets;
									// Get each animation offset
									readOffset = animationsOffset;
									while (animationsLen >= ((readOffset - animationsOffset) + 4))
									{
										unsigned int animOff = *(unsigned int *)&(mmtn[readOffset]);
										// Move to the next word
										readOffset += 4;
										// Check if we reached the end of the animation offsets
										if (animOff == UINT_MAX)
										{
											// We are done so break from the loop
											break;
										}
										else
										{
											// Store the animation offset
											animationOffsets.push_back(animOff);
										}
									}
									// Store the number of animations
									numAnims = animationOffsets.size();
									// Move to the start of the first animation
									remainder = readOffset % 16;
									if (remainder > 0)
									{
										readOffset += (16 - remainder);
									}
									readOffset += 16;
									
									// For testing, lets limit ourselves to only a single animation
									if (numAnims > 0)
									{
										numAnims = 1;
									}

									for (unsigned int i = 0; i < numAnims; i++)
									{
										if (animationsLen >= ((readOffset - animationsOffset) + 80))
										{
											unsigned int numJoints, numTransforms, transformsOffset;
											readOffset += 16;
											numJoints = *(unsigned int *)&(mmtn[readOffset]);
											readOffset += 36;
											numTransforms = *(unsigned int *)&(mmtn[readOffset]);
											readOffset += 4;
											transformsOffset = *(unsigned int *)&(mmtn[readOffset]);
											readOffset = animationsOffset + animationOffsets[i] + transformsOffset;
											if (animationsLen >= ((readOffset - animationsOffset) + (numTransforms * 8)))
											{
												// Construct joint transfrom vector to hold all potential joint transforms
												vector<vector<float>> jointTransforms;
												vector<vector<bool>> foundTransform;
												jointTransforms.reserve(numJoints);
												for (unsigned int j = 0; j < numJoints; j++)
												{
													vector<float> transforms;
													vector<bool> statuses;
													transforms.reserve(9);
													statuses.reserve(9);
													for (unsigned int k = 0; k < 9; k++)
													{
														transforms.push_back((float)0xFFFFFF);
														statuses.push_back(false);
													}
													jointTransforms.push_back(transforms);
													foundTransform.push_back(statuses);
												}
												for (unsigned int j = 0; j < numTransforms; j++)
												{
													unsigned short jointID, transformType;
													float transformVal;
													jointID = *(unsigned short *)&(mmtn[readOffset]);
													transformType = *(unsigned short *)&(mmtn[readOffset + 2]);
													transformVal = *(float *)&(mmtn[readOffset + 4]);
													if (jointID < numJoints && transformType > 0 && transformType < 10)
													{
														jointTransforms[jointID][transformType - 1] = transformVal;
														foundTransform[jointID][transformType - 1] = true;
													}
													// Move to the next transform
													readOffset += 8;
												}
												for (unsigned int j = 0; j < numJoints; j++)
												{
													for (unsigned int k = 0; k < 9; k++)
													{
														if (foundTransform[j][k])
														{
															MsetStaticAnimation a;
															a.jointID = j;
															a.transforms = jointTransforms[j];
															a.hasTransform = foundTransform[j];
															anims.push_back(a);
															break;
														}
													}
												}
											}
											else
											{
												cout << "Error: Animation " << i << " transforms are longer than the length of the animations section" << endl;
											}
										}
										else
										{
											cout << "Error: Animation " << i << " is outside the length of the animations section" << endl;
										}
									}
								}
								else
								{
									cout << "Error: The MMTN header has invalid offsets and lengths" << endl;
								}
							}
							else
							{
								cout << "Error: The length of the MMTN section is invalid" << endl;
							}
						}
						else
						{
							cout << "Error: Section 0 is not an MMTN section" << endl;
						}
					}
					else
					{
						cout << "Error: The MMTN section does not contain the full header" << endl;
					}
				}
				else
				{
					cout << "Error: The section offsets are invalid" << endl;
				}
			}
			else
			{
				cout << "Error: This MSET is missing the section offsets" << endl;
			}
		}
		else
		{
			cout << "Error: This file is not an MSET" << endl;
		}
	}
	else
	{
		cout << "Error: This file doesn't contain enough data to be a valid MSET" << endl;
	}
	return anims;
}

/* 
    To-Do: Fix this sort function to minimize the number of origin updates required
	Ideas:
	  * Create a graph where each origin joint is a node and each vertex represents an edge
	    Find a path to visit every node, and then sort thebased on that path
*/
vector<Face> sortFaces(vector<Face> faces, vector<VertexRelative> verts)
{
	// Create a copy of faces
	vector<Face> sorted = faces;
	vector<FaceEx> toSort;
	vector<unsigned int> minJointVertInds;
	// Get the number of faces
	unsigned int numFaces = faces.size();
	// For each face...
	for (unsigned int i = 0; i < numFaces; i++)
	{
		// Get face i
		Face f = faces[i];
		// Initialize the minimum joint vertex index to 0
		minJointVertInds.push_back(0);
		// Check vertices 1 and 2 to see if they have smaller joint indices than vertex 0
		for (unsigned int j = 1; j < numVertsPerFace; j++)
		{
			if (verts[f.vertexIndices[j]] < verts[f.vertexIndices[minJointVertInds[i]]])
			{
				minJointVertInds[i] = j;
			}
		}
		// Shift the order of the vertices so that the minimum vertex is listed first while maintaining the orientation
		for (unsigned int j = 0; j < numVertsPerFace; j++)
		{
			sorted[i].vertexIndices[j] = faces[i].vertexIndices[(minJointVertInds[i] + j) % numVertsPerFace];
		}
		// Check if vertex 2 has a smaller origin joint index than vertex 1
		if (verts[sorted[i].vertexIndices[1]] > verts[sorted[i].vertexIndices[2]])
		{
			// It does so swap the orientation so that the origin joint indices are in ascending order
			unsigned int temp = sorted[i].vertexIndices[1];
			sorted[i].vertexIndices[1] = sorted[i].vertexIndices[2];
			sorted[i].vertexIndices[2] = temp;
			if (sorted[i].orientation == CLOCKWISE)
			{
				sorted[i].orientation = COUNTERCLOCKWISE;
			}
			else if (sorted[i].orientation == COUNTERCLOCKWISE)
			{
				sorted[i].orientation = CLOCKWISE;
			}
		}
	}
	// Convert the faces to the explicit form
	if (getFacesEx(sorted, verts, toSort))
	{
		// Sort the faces
		std::sort(toSort.begin(), toSort.end());
		// Convert the faces back to the general form
		if (getFaces(toSort, verts, sorted))
		{
			// Reset all the faces to a clockwise orientation
			for (unsigned int i = 0; i < numFaces; i++)
			{
				if (sorted[i].orientation == CLOCKWISE)
				{
					unsigned int temp = sorted[i].vertexIndices[1];
					sorted[i].vertexIndices[1] = sorted[i].vertexIndices[2];
					sorted[i].vertexIndices[2] = temp;
					sorted[i].orientation = COUNTERCLOCKWISE;
				}
			}
		}
		else
		{
			sorted.clear();
		}
	}
	else
	{
		sorted.clear();
	}
	return sorted;
}

// Overload comparison operators for VertexGlobal structs
bool operator==(const VertexGlobal a, const VertexGlobal b)
{
	bool eq = (a.originJointIndex == b.originJointIndex) && (a.textureIndex == b.textureIndex) && (a.coordinatesScaleFactor == b.coordinatesScaleFactor) && (a.textureScaleFactor == b.textureScaleFactor);
	if (eq)
	{
		for (unsigned int i = 0; i < numDims; i++)
		{
			if ((a.normal[i] != b.normal[i]) || (a.coordinates[i] != b.coordinates[i]))
			{
				eq = false;
				break;
			}
		}
		if (eq)
		{
			for (unsigned int i = 0; i < 2; i++)
			{
				if (a.textureMap[i] != b.textureMap[i])
				{
					eq = false;
					break;
				}
			}
		}
	}
	return eq;
}

bool operator!=(const VertexGlobal a, const VertexGlobal b)
{
	return !(a == b);
}

// Overload comparison operators for VertexRelative structs
bool operator==(const VertexRelative a, const VertexRelative b)
{
	bool eq = (a.originJointIndex == b.originJointIndex) && (a.textureIndex == b.textureIndex) && (a.coordinatesScaleFactor == b.coordinatesScaleFactor) && (a.textureScaleFactor == b.textureScaleFactor);
	if (eq)
	{
		for (unsigned int i = 0; i < numDims; i++)
		{
			if ((a.normal[i] != b.normal[i]) || (a.coordinates[i] != b.coordinates[i]))
			{
				eq = false;
				break;
			}
		}
		if (eq)
		{
			for (unsigned int i = 0; i < 2; i++)
			{
				if (a.textureMap[i] != b.textureMap[i])
				{
					eq = false;
					break;
				}
			}
		}
	}
	return eq;
}

bool operator!=(const VertexRelative a, const VertexRelative b)
{
	return !(a == b);
}

bool operator<(const VertexRelative a, const VertexRelative b)
{
	return a.originJointIndex < b.originJointIndex;
}

bool operator>(const VertexRelative a, const VertexRelative b)
{
	return a.originJointIndex > b.originJointIndex;
}

bool operator<=(const VertexRelative a, const VertexRelative b)
{
	return !(a > b);
}

bool operator>=(const VertexRelative a, const VertexRelative b)
{
	return !(a < b);
}

bool operator<(const FaceEx a, const FaceEx b)
{
	bool retVal = a.vertices[0].textureIndex < b.vertices[0].textureIndex;
	if (a.vertices[0].textureIndex == b.vertices[0].textureIndex)
	{
		for (unsigned int i = 0; i < numVertsPerFace; i++)
		{
			if (a.vertices[i].originJointIndex != b.vertices[i].originJointIndex)
			{
				retVal = a.vertices[i] < b.vertices[i];
				break;
			}
		}
	}
	return retVal;
}

bool operator>(const FaceEx a, const FaceEx b)
{
	bool retVal = !(a < b);
	for (unsigned int i = 0; retVal && (i < numVertsPerFace); i++)
	{
		retVal = retVal && (a.vertices[i].originJointIndex != b.vertices[i].originJointIndex);
	}
	return retVal;
}

bool operator<=(const FaceEx a, const FaceEx b)
{
	return !(a > b);
}

bool operator>=(const FaceEx a, const FaceEx b)
{
	return !(a < b);
}
