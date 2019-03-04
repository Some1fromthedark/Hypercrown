#include"mdls.h"

// Read the contents of a binary file and store it as a string
bool binaryFileRead(string filename, string &dat)
{
	bool valid = true, eof;
	char byte;
	ifstream in;
	// Clear the output
	dat = "";
	// Open the input file
	in.open(filename, ifstream::binary);
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
	}
	else
	{
		valid = false;
	}
	return valid;
}

/* Calls the appropriate convert functions based on the input and output formats */
string convertMesh(string raw, string inputFormat, string outputFormat, AddInfo optionalVals)
{
	// Declare variables
	bool valid, normalize, flipUVs, combineMesh, swapChiral, shadow;
	string converted;
	vector<JointRelative> joints;
	vector<vector<VertexRelative>> vertsR;
	vector<vector<VertexGlobal>> vertsG;
	vector<vector<Face>> faces;
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
	// Check if there is a value for the flipUVs flag provided
	if (optionalVals.additionalBools.size() > addBoolIndex)
	{
		// Store the provided value
		flipUVs = optionalVals.additionalBools[addBoolIndex++];
	}
	else
	{
		// Default to not flipping
		flipUVs = false;
	}
	// Check if there is a value for the combineMeshes flag provided
	if (optionalVals.additionalBools.size() > addBoolIndex)
	{
		// Store the provided value
		combineMesh = optionalVals.additionalBools[addBoolIndex++];
	}
	else
	{
		// Default to not flipping
		combineMesh = false;
	}
	// Check if there is a value for the swapChirality flag provided
	if (optionalVals.additionalBools.size() > addBoolIndex)
	{
		// Store the provided value
		swapChiral = optionalVals.additionalBools[addBoolIndex++];
	}
	else
	{
		// Default to not flipping
		swapChiral = false;
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
		// Check if there is a value for the shadow flag provided
		if (optionalVals.additionalBools.size() > addBoolIndex)
		{
			// Store the provided value
			shadow = optionalVals.additionalBools[addBoolIndex++];
		}
		else
		{
			// Default to not flipping
			shadow = false;
		}
		// Get the joint positions
		joints = getMdlsJoints(raw);
		if (shadow)
		{
			valid = getMdlsShadowVerticesAndFaces(raw, joints, vertsR, faces);
		}
		else
		{
			// Get the vertex positions and face configuration
			valid = getMdlsVerticesAndFaces(raw, joints, vertsR, faces);
		}
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
		// Get the joints
		joints = getWpnJoints(raw, absoluteMaxJoints);
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
		unsigned int numTextures = getNumTextures(vertsG);
		if (numTextures > 0)
		{
			vector<vector<VertexGlobal>> vgs;
			vector<vector<VertexRelative>> vrs;
			vector<vector<Face>> fs;
			vgs.reserve(vertsG.size());
			vrs.reserve(vertsR.size());
			fs.reserve(faces.size());
			for (unsigned int i = 0; i < numTextures; i++)
			{
				for (unsigned int j = 0; j < vertsG.size(); j++)
				{
					if (vertsG[j][0].textureIndex == i)
					{
						vgs.push_back(vertsG[j]);
						vrs.push_back(vertsR[j]);
						fs.push_back(faces[j]);
					}
				}
			}
			vertsG = vgs;
			vertsR = vrs;
			faces = fs;
		}

		// Check if wee need to normalize the model
		if (normalize)
		{
			// Limit the coordinates to the range [-1, 1]
			valid = normalizePosition(vertsG, vertsG, joints, &joints);
			if (valid)
			{
				valid = getVerticesRelative(joints, vertsG, vertsR);
			}
			if (!valid)
			{
				cout << "Error: Failed to normalize model" << endl;
			}
		}
		// Check if we need to flip the UV coordinates
		if (flipUVs)
		{
			// Flip the vertices' UV Coordinates
			vector<vector<VertexGlobal>> flippedVerts;
			vector<vector<VertexRelative>> flippedVerts2;
			valid = flipUVCoordinates(vertsG, flippedVerts);
			// Save the result as vertsG
			vertsG = flippedVerts;
			valid = valid && flipUVCoordinates(vertsR, flippedVerts2);
			vertsR = flippedVerts2;
		}
		// Check if we need to combine the meshes
		if (combineMesh)
		{
			if (combineMeshes(vertsG, faces))
			{
				getVerticesRelative(joints, vertsG, vertsR);
			}
			else
			{
				cout << "Error: Failed to combine meshes" << endl;
			}
		}
		// Check if we need to swap chirality
		if (swapChiral)
		{
			if (swapChirality(joints, vertsG))
			{
				getVerticesRelative(joints, vertsG, vertsR);
			}
			else
			{
				cout << "Error: Failed to swap the mesh's chirality" << endl;
			}
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
			converted = convertMeshToObj(vertsG, faces, mtlFilename);
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
				converted = convertMeshToDae(joints, vertsG, faces, anims, texturePrefix);
			}
			else
			{
				converted = convertMeshToDae(joints, vertsG, faces, anims);
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
				converted = convertMeshToMdls(joints, vertsR, faces, exampleRig, exampleVertices);
			}
			else
			{
				converted = convertMeshToMdls(joints, vertsR, faces);
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
				converted = convertMeshToWpn(vertsR, faces, exampleVertices);
			}
			else
			{
				converted = convertMeshToWpn(vertsR, faces);
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

string convertMeshToObj(vector<vector<VertexGlobal>> verts, vector<vector<Face>> faces, string mtlName)
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
		for (unsigned int j = 0; j < verts[i].size(); j++)
		{
			// VertexRelative Identifier
			converted += "v";
			for (unsigned int k = 0; k < numDims; k++)
			{
				//  Convert each coordinate from float to string
				ss.clear();
				ss.str("");
				// Use dim order to swap y and z since most obj imports use y as the vertical axis
				ss << dimScalars[dimOrder[k]] * verts[i][j].coordinates[dimOrder[k]];
				ss >> numAsStr;
				// Append the coordinate to the numAsStr
				converted += " " + numAsStr;
			}
			// Go to the next numAsStr
			converted += "\n";
			// VertexRelative Texture Identifier
			converted += "vt";
			for (unsigned int k = 0; k < 2; k++)
			{
				// Convert the U and V values from float to string
				ss.clear();
				ss.str("");
				ss << verts[i][j].textureMap[k];
				ss >> numAsStr;
				// Append the texture map to the numAsStr
				converted += " " + numAsStr;
			}
			// Go to the next numAsStr
			converted += "\n";
		}
	}
	// Add another blank line to separate the vertices and faces for read-ability
	converted += "\n";
	// Add each face into the file
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		for (unsigned int j = 0; j < faces[i].size(); j++)
		{
			// Check if the texture index changed
			if (textureInd != verts[i][faces[i][j].vertexIndices[0]].textureIndex)
			{
				// Update the texture index
				textureInd = verts[i][faces[i][j].vertexIndices[0]].textureIndex;
				// Change the material being used
				ss.clear();
				ss.str("");
				ss << textureInd;
				ss >> numAsStr;
				converted += "usemtl mat" + numAsStr + "\n";
			}
			// Face Identifier
			converted += "f";
			for (unsigned int k = 0; k < numVertsPerFace; k++)
			{
				// Convert each index from unsigned int to string
				ss.clear();
				ss.str("");
				ss << (faces[i][j].vertexIndices[k] + 1); // .obj indices begin at 1, not 0
				ss >> numAsStr;
				// Append the index to the numAsStr
				converted += " " + numAsStr + "/" + numAsStr;
			}
			// Go to the next line
			converted += "\n";
		}
	}
	return converted;
}

string convertMeshToMdls(vector<JointRelative> joints, vector<vector<VertexRelative>> verts, vector<vector<Face>> faces) 
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

string convertMeshToMdls(vector<JointRelative> joints, vector<vector<VertexRelative>> verts, vector<vector<Face>> faces, string templateRigData, string templateVertexData)
{
	// Declare constants
	const unsigned int numOrigins = 3;
	// Flags for excluding and/or swapping sections of the mobj and mdls (0 = copy from joint template, 1 = exclude section/copy from vertex template, swapping only occurs if updateRig is true)
	const unsigned char excludeMdls = 0x00, excludeMobj = 0x02, swapMdls = 0x01, swapMobj = 0x1F;
	// Declare and initialize variables
	vector<PolygonCollection> polygonCollections;
	// The copy variables determine if we copy their sections from the templates (true), or if the tool generates those sections (false)
	bool updateRig, copyVertexData = false, copyJointData = false;
	string converted = "", mobj, mobjV, newMobj;
	unsigned int templateRigLen = templateRigData.size(), templateVertexLen = templateVertexData.size(), numSections, mobjOff, mobjVOff, mobjEnd, mobjVEnd, mobjLen, mobjVLen, subsectionOff, subsectionLen, numJoints, numTemplateJoints, numPolygonCollections, numVerts, textureInd, dimOff, jointInfo, origins[numOrigins], reset[numOrigins], remainder;
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
				// Create MOBJ Header
				newMobj = "MOBJ";
				newMobj.append(mobjHeaderLen - 4, 0); // Fill in the lengths and offsets later
				// Fill in the subsection 0 offset
				*(unsigned int *)&(newMobj[32]) = mobjHeaderLen;
				// Check if we are copying the vertex data from a template
				if (!copyVertexData)
				{
					// Sort the faces to optimize polygon collections
					//faces = sortFaces(faces, verts);
					// Convert the faces into polygon collections
					if (getPolygonCollections(verts, faces, polygonCollections))
					{
						// Save the number of polygon collections
						numPolygonCollections = polygonCollections.size();
						if(!compareFacesToPolygonCollections(faces, polygonCollections))
						{
							cout << "Error: PolygonCollections doesn't match faces" << endl;
						}
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
						if (i < verts.size())
						{
							if (polygonCollections[i].subcollections.size() > 0
								&& polygonCollections[i].subcollections[0].triangleLists.size() > 0
								&& polygonCollections[i].subcollections[0].triangleLists[0].vertexIndices.size() > 0)
							{
								unsigned int vertexIndex = polygonCollections[i].subcollections[0].triangleLists[0].vertexIndices[0];
								textureInd = verts[i][vertexIndex].textureIndex;
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
						else
						{
							// Invalid vertex vector or polygon collection vector
							numPolygonCollections = 0;
							cout << "Error: The vertices vector does not correspond with the polygon collection vector" << endl;
						}
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
					vector<JointRelative> templateJoints = getMdlsJoints(templateRigData);

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
					for (unsigned int i = 0; i < numJoints && i < templateJoints.size(); i++)
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
						//newMobj.append((char *)&(joints[i].special), 4);
						newMobj.append((char *)&(templateJoints[i].special), 4);
						// Add the joint position
						for (unsigned int j = 0; j < numDims; j++)
						{
							dimOff = (j + 1) % numDims;
							newMobj.append((char *)&(joints[i].coordinates[dimOff]), 4);
						}
						//jointInfo = (joints[i].jointInfo.unknownFlag << 30) | (joints[i].jointInfo.childIndex << 20) | (joints[i].jointInfo.unknownIndex << 10) | joints[i].jointInfo.parentIndex;
						jointInfo = (templateJoints[i].jointInfo.unknownFlag << 30) | (templateJoints[i].jointInfo.childIndex << 20) | (templateJoints[i].jointInfo.unknownIndex << 10) | templateJoints[i].jointInfo.parentIndex;
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
						unsigned int numFilledOrigins, numFilledPreviousOrigins;
						// Handle Polygon Collections and Joint References
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							PolygonCollection polygonCollection = polygonCollections[i];
							// Update offset in the polygon collection table
							subsectionOff = newMobj.size() - mobjHeaderLen;
							*(unsigned int *)&(newMobj[mobjHeaderLen + i * 16 + 28]) = subsectionOff;
							// Reset the origin counts
							numFilledOrigins = 0;
							numFilledPreviousOrigins = 0;
							// Set origin joint indices to uninitialized and clear the reset
							for (unsigned int j = 0; j < numOrigins; j++)
							{
								origins[j] = absoluteMaxJoints;
								reset[j] = absoluteMaxJoints;
							}
							bool initialized = false, reinitialize = false;
							// For each polygon sub-collection in the polygon collection
							for (unsigned int j = 0; j < polygonCollection.subcollections.size(); j++)
							{
								PolygonSubCollection subcollection = polygonCollection.subcollections[j];
								// Store the current size of the mobj for later so we can calculate the length of the polygon collection
								subsectionOff = newMobj.size();
								// Padding
								newMobj.append(8, 0); // Fill in the length field later
								// Unknown Stuff
								newMobj.append("\x01\x01\x00\x01\x00\x80\x00\x6C", 8);	// Fill in ?2 field later [(length - 16) / 16]

								// For each triangle list in the polygon sub-collection
								for (unsigned int k = 0; k < subcollection.triangleLists.size(); k++)
								{
									unsigned int vertexIndex;
									TriangleList triangleList = subcollection.triangleLists[k];
									// For each vertex in the triangle list
									for (unsigned int l = 0; initialized && l < triangleList.vertexIndices.size(); l++)
									{
										vertexIndex = triangleList.vertexIndices[l];
										// Check if we need to reinitialize the origins
										if (numFilledOrigins > 0)
										{
											for (unsigned int m = 0; initialized && m < numFilledOrigins; m++)
											{
												// Check if this vertex's origin joint matches any of the stored origin joints
												if (verts[i][vertexIndex].originJointIndex == origins[m])
												{
													// The origin joint is valid, we can continue without reinitializing
													break;
												}
												// If we didn't find it
												else if (m == (numFilledOrigins - 1))
												{
													// We need to reinitialize
													initialized = false;
												}
											}
										}
										else
										{
											// We need to initialize
											initialized = false;
										}
									}
									// Check if the origins have been initialized
									if (!initialized)
									{
										unsigned int numNewOrigins;
										vector<unsigned int> newOrigins;
										newOrigins.reserve(numOrigins);
										// Store the previous values of the origins array in the reset array and clear the origins array
										for (unsigned int l = 0; l < numFilledOrigins && l < numOrigins; l++)
										{
											reset[l] = origins[l];
											origins[l] = absoluteMaxJoints;
										}
										// Update the number of previously filled origins
										numFilledPreviousOrigins = numFilledOrigins;
										// Reset the number of filled origins
										numFilledOrigins = 0;

										// For each vertex in the triangle list
										for (unsigned int l = 0; l < triangleList.vertexIndices.size(); l++)
										{
											bool isReset = false;
											vertexIndex = triangleList.vertexIndices[l];
											for (unsigned int m = 0; m < numFilledPreviousOrigins; m++)
											{
												if (reset[m] == verts[i][vertexIndex].originJointIndex)
												{
													isReset = true;
													break;
												}
											}
											if (!isReset && find(newOrigins.begin(), newOrigins.end(), verts[i][vertexIndex].originJointIndex) == newOrigins.end())
											{
												newOrigins.push_back(verts[i][vertexIndex].originJointIndex);
											}
										}
										numNewOrigins = newOrigins.size();

										for (unsigned int k2 = k; (numFilledOrigins + numNewOrigins) < numOrigins && k2 < subcollection.triangleLists.size(); k2++)
										{
											// For each vertex in the triangle list
											for (unsigned int l = 0; (numFilledOrigins + numNewOrigins) < numOrigins && l < subcollection.triangleLists[k2].vertexIndices.size(); l++)
											{
												vertexIndex = subcollection.triangleLists[k2].vertexIndices[l];
												// For each origin we can reference simultaneously
												for (unsigned int m = 0; m < numFilledPreviousOrigins; m++)
												{
													// Check if this vertex has a new joint we need to reference
													if (verts[i][vertexIndex].originJointIndex == reset[m] && origins[m] == absoluteMaxJoints)
													{
														// Store the new index
														origins[m] = reset[m];
														numFilledOrigins++;
														break;
													}
												}
											}
										}
										for(unsigned int l = 0; l < numNewOrigins; l++)
										{
											// For each origin we can reference simultaneously
											for (unsigned int m = 0; m < numOrigins; m++)
											{
												// Check if this vertex has a new joint we need to reference
												if (origins[m] == absoluteMaxJoints)
												{
													// Store the new index
													origins[m] = newOrigins[l];
													numFilledOrigins++;
													// Break from the loop so we don't duplicate the entry
													break;
												}
												else if (m == (numOrigins - 1))
												{
													cout << "Warning: Failed to store new origin i: " << i << " j: " << j << " k: " << k << endl;
												}
											}
										}
										// Copy over the reset origins into any unused slots
										for (unsigned int l = 0; l < numFilledPreviousOrigins; l++)
										{
											if (origins[l] == absoluteMaxJoints)
											{
												origins[l] = reset[l];
												numFilledOrigins++;
											}
										}
										// Set the initialized flag and the reinitialize flag
										initialized = true;
										reinitialize = true;
									}
									// Check if we need to reinitialize
									if (reinitialize)
									{
										// For each origin
										for (unsigned int l = 0; l < numOrigins; l++)
										{
											// Check if we need to update the origin by comparing with reset
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
									numVerts = triangleList.vertexIndices.size();
									// Calculate the length of the triangle list section
									subsectionLen = numVerts * 48 + 32;
									// Triangle list identifier
									newMobj.append("\x01\x00\x00\x00", 4);
									// Length of triangle list
									newMobj.append((char *)&(subsectionLen), sizeof(unsigned int));
									// Number of vertices in the triangle list
									newMobj.append((char *)&(numVerts), sizeof(unsigned int));
									// Initial culling direction flag (0 - Anticlockwise, 1 - Clockwise)
									newMobj.append(1, triangleList.orientation);
									newMobj.append(3, 0);
									// Copy of the number of vertices
									newMobj.append((char *)&(numVerts), sizeof(unsigned int));
									// Unknown stuff
									newMobj.append("\x00\x40\x3E\x30\x12\x04\x00\x00", 8);
									// Padding?
									newMobj.append(4, 0);

									// For each vertex in the triangle list
									for (unsigned int l = 0; l < triangleList.vertexIndices.size(); l++)
									{
										unsigned int vertexIndex = triangleList.vertexIndices[l];
										VertexRelative v = verts[i][vertexIndex];
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
												cout << "Warning: Unable to find origin joint for i: " << i << " j: " << j << " k: " << k << " l: " << l << endl;
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
											while (texVal < 0 || texVal > 1)
											{
												if (texVal > 1)
												{
													texVal -= 1;
												}
												else
												{
													texVal += 1;
												}
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
								if (j == (polygonCollection.subcollections.size() - 1))
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
								if ((*(unsigned int *)&(newMobj[subsectionOff]) & 0x00FFFFFF) < 256)
								{
									newMobj[subsectionOff + 14] = newMobj[subsectionOff] - 1;
								}
								else
								{
									cout << "Warning: Invalid polygon subcollection length i: " << i << " j: " << j << endl;
									newMobj[subsectionOff + 14] = (char)255;
								}
							}
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
				// Subsection 1
				subsectionOff = newMobj.size();
				// Update the Subsection 1 offset
				*(unsigned int *)&(newMobj[40]) = subsectionOff;
				// Check if we are including this section
				if ((excludeMobj & 0x02) == 0)
				{
					// For now just copy Subsection 1 from the template
					// Check which template to copy from
					if (updateRig && (swapMobj & 0x02) != 0)
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

string convertMeshToWpn(vector<vector<VertexRelative>> verts, vector<vector<Face>> faces)
{
	string converted = "";
	cout << "Warning: Converting to WPN without a template is currently not supported" << endl;
	return converted;
}

string convertMeshToWpn(vector<vector<VertexRelative>> verts, vector<vector<Face>> faces, string templateData)
{
	vector<PolygonCollection> polygonCollections;
	// The copy variables determine if we copy their sections from the templates (true), or if the tool generates those sections (false)
	string converted = "", menv, newMenv;
	const unsigned int numOrigins = 3;
	// Flags for excluding and/or swapping sections of the mobj and mdls (0 = include section, 1 = exclude section)
	const unsigned char excludeWpn = 0x00, excludeMenv = 0x02;
	unsigned int templateLen = templateData.size(), numSections, menvOff, menvLen, sectionOff, nextSectionOff, sectionLen, subsectionOff, subsectionLen, numPolygonCollections, numJoints, numVerts, textureInd, dimOff, origins[numOrigins], reset[numOrigins], remainder;
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
					//faces = sortFaces(faces, verts);
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
						if (polygonCollections[i].subcollections.size() > 0 
							&& polygonCollections[i].subcollections[0].triangleLists.size() > 0 
							&& polygonCollections[i].subcollections[0].triangleLists[0].vertexIndices.size() > 0)
						{
							unsigned int vertexIndex = polygonCollections[i].subcollections[0].triangleLists[0].vertexIndices[0];
							textureInd = verts[i][vertexIndex].textureIndex;
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
						unsigned int numFilledOrigins, numFilledPreviousOrigins;
						// Handle Polygon Collections and Joint References
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							PolygonCollection polygonCollection = polygonCollections[i];
							// Update offset in the polygon collection table
							subsectionOff = newMenv.size() - menvHeaderLen;
							*(unsigned int *)&(newMenv[menvHeaderLen + i * 16 + 28]) = subsectionOff;
							// Reset the origin counts
							numFilledOrigins = 0;
							numFilledPreviousOrigins = 0;
							// Set origin joint indices to uninitialized and clear the reset
							for (unsigned int j = 0; j < numOrigins; j++)
							{
								origins[j] = absoluteMaxJoints;
								reset[j] = absoluteMaxJoints;
							}
							bool initialized = false, reinitialize = false;
							// For each polygon sub-collection in the polygon collection
							for (unsigned int j = 0; j < polygonCollection.subcollections.size(); j++)
							{
								PolygonSubCollection subcollection = polygonCollection.subcollections[j];
								// Store the current size of the mobj for later so we can calculate the length of the polygon collection
								subsectionOff = newMenv.size();
								// Padding
								newMenv.append(8, 0); // Fill in the length field later
								// Unknown Stuff
								newMenv.append("\x01\x01\x00\x01\x00\x80\x00\x6C", 8);	// Fill in ?2 field later [(length - 16) / 16]

								// For each triangle list in the polygon sub-collection
								for (unsigned int k = 0; k < subcollection.triangleLists.size(); k++)
								{
									unsigned int vertexIndex;
									TriangleList triangleList = subcollection.triangleLists[k];
									// For each vertex in the triangle list
									for (unsigned int l = 0; initialized && l < triangleList.vertexIndices.size(); l++)
									{
										vertexIndex = triangleList.vertexIndices[l];
										// Check if we need to reinitialize the origins
										if (numFilledOrigins > 0)
										{
											for (unsigned int m = 0; initialized && m < numFilledOrigins; m++)
											{
												// Check if this vertex's origin joint matches any of the stored origin joints
												if (verts[i][vertexIndex].originJointIndex == origins[m])
												{
													// The origin joint is valid, we can continue without reinitializing
													break;
												}
												// If we didn't find it
												else if (m == (numFilledOrigins - 1))
												{
													// We need to reinitialize
													initialized = false;
												}
											}
										}
										else
										{
											// We need to initialize
											initialized = false;
										}
									}
									// Check if the origins have been initialized
									if (!initialized)
									{
										unsigned int numNewOrigins;
										vector<unsigned int> newOrigins;
										newOrigins.reserve(numOrigins);
										// Store the previous values of the origins array in the reset array and clear the origins array
										for (unsigned int l = 0; l < numFilledOrigins && l < numOrigins; l++)
										{
											reset[l] = origins[l];
											origins[l] = absoluteMaxJoints;
										}
										// Update the number of previously filled origins
										numFilledPreviousOrigins = numFilledOrigins;
										// Reset the number of filled origins
										numFilledOrigins = 0;

										// For each vertex in the triangle list
										for (unsigned int l = 0; l < triangleList.vertexIndices.size(); l++)
										{
											bool isReset = false;
											vertexIndex = triangleList.vertexIndices[l];
											for (unsigned int m = 0; m < numFilledPreviousOrigins; m++)
											{
												if (reset[m] == verts[i][vertexIndex].originJointIndex)
												{
													isReset = true;
													break;
												}
											}
											if (!isReset && find(newOrigins.begin(), newOrigins.end(), verts[i][vertexIndex].originJointIndex) == newOrigins.end())
											{
												newOrigins.push_back(verts[i][vertexIndex].originJointIndex);
											}
										}
										numNewOrigins = newOrigins.size();

										for (unsigned int k2 = k; (numFilledOrigins + numNewOrigins) < numOrigins && k2 < subcollection.triangleLists.size(); k2++)
										{
											// For each vertex in the triangle list
											for (unsigned int l = 0; (numFilledOrigins + numNewOrigins) < numOrigins && l < subcollection.triangleLists[k2].vertexIndices.size(); l++)
											{
												vertexIndex = subcollection.triangleLists[k2].vertexIndices[l];
												// For each origin we can reference simultaneously
												for (unsigned int m = 0; m < numFilledPreviousOrigins; m++)
												{
													// Check if this vertex has a new joint we need to reference
													if (verts[i][vertexIndex].originJointIndex == reset[m] && origins[m] == absoluteMaxJoints)
													{
														// Store the new index
														origins[m] = reset[m];
														numFilledOrigins++;
														break;
													}
												}
											}
										}
										for (unsigned int l = 0; l < numNewOrigins; l++)
										{
											// For each origin we can reference simultaneously
											for (unsigned int m = 0; m < numOrigins; m++)
											{
												// Check if this vertex has a new joint we need to reference
												if (origins[m] == absoluteMaxJoints)
												{
													// Store the new index
													origins[m] = newOrigins[l];
													numFilledOrigins++;
													// Break from the loop so we don't duplicate the entry
													break;
												}
												else if (m == (numOrigins - 1))
												{
													cout << "Warning: Failed to store new origin i: " << i << " j: " << j << " k: " << k << endl;
												}
											}
										}
										// Copy over the reset origins into any unused slots
										for (unsigned int l = 0; l < numFilledPreviousOrigins; l++)
										{
											if (origins[l] == absoluteMaxJoints)
											{
												origins[l] = reset[l];
												numFilledOrigins++;
											}
										}
										// Set the initialized flag and the reinitialize flag
										initialized = true;
										reinitialize = true;
									}
									// Check if we need to reinitialize
									if (reinitialize)
									{
										// For each origin
										for (unsigned int l = 0; l < numOrigins; l++)
										{
											// Check if we need to update the origin by comparing with reset
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
									numVerts = triangleList.vertexIndices.size();
									// Calculate the length of the triangle list section
									subsectionLen = numVerts * 48 + 32;
									// Triangle list identifier
									newMenv.append("\x01\x00\x00\x00", 4);
									// Length of triangle list
									newMenv.append((char *)&(subsectionLen), sizeof(unsigned int));
									// Number of vertices in the triangle list
									newMenv.append((char *)&(numVerts), sizeof(unsigned int));
									// Initial culling direction flag (0 - Anticlockwise, 1 - Clockwise)
									newMenv.append(1, triangleList.orientation);
									newMenv.append(3, 0);
									// Copy of the number of vertices
									newMenv.append((char *)&(numVerts), sizeof(unsigned int));
									// Unknown stuff
									newMenv.append("\x00\x40\x3E\x30\x12\x04\x00\x00", 8);
									// Padding?
									newMenv.append(4, 0);

									// For each vertex in the triangle list
									for (unsigned int l = 0; l < triangleList.vertexIndices.size(); l++)
									{
										unsigned int vertexIndex = triangleList.vertexIndices[l];
										VertexRelative v = verts[i][vertexIndex];
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
												cout << "Warning: Unable to find origin joint for i: " << i << " j: " << j << " k: " << k << " l: " << l << endl;
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
											while (texVal > 1 || texVal < 0)
											{
												if (texVal > 1)
												{
													texVal -= 1;
												}
												else
												{
													texVal += 1;
												}
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
								if (j == (polygonCollection.subcollections.size() - 1))
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
								if ((*(unsigned int *)&(newMenv[subsectionOff]) & 0x00FFFFFF) < 256)
								{
									newMenv[subsectionOff + 14] = newMenv[subsectionOff] - 1;
								}
								else
								{
									cout << "Warning: Invalid polygon subcollection length i: " << i << " j: " << j << endl;
									newMenv[subsectionOff + 14] = (char)255;
								}
							}
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
					// Subsection 1
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

string convertMeshToDae(vector<JointRelative> joints, vector<vector<VertexGlobal>> verts, vector<vector<Face>> faces, vector<Animation> anims, string name)
{
	string converted = "", numAsStr, tabbed;
	stringstream ss;
	// Convert the joint positions into relative vertices
	vector<vector<VertexRelative>> jointsR = convertJointsToVerts(joints);
	vector<vector<VertexGlobal>> jointsG;
	vector<vector<float>> transformMat, translateMat, rotateMat;
	vector<unsigned int> indexStack;
	vector<bool> jointStatusStack;
	unsigned int numTextures = getNumTextures(verts), numFacesOfTexture, rootJointIndex = getRootJointIndex(joints), counter, jointDepth, jointIndex, parentJointIndex, childJointIndex, numChildren, dimOff, numMeshes = verts.size();
	// The translation for the bind shape matrix
	float translate[numDims];
	unsigned int fnStartInd = (unsigned int)name.find_last_of('\\') + 1;
	// Remove the path from the filename if there is one
	if (fnStartInd < name.size())
	{
		name = name.substr(fnStartInd, name.size() - fnStartInd);
	}
	if (numMeshes == faces.size())
	{
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
				if (numTextures > 0)
				{
					converted += "\t<library_images>\n";
					for (unsigned int i = 0; i < numTextures; i++)
					{
						string imageFilename = name + "-";
						// Convert the texture index to a string
						ss.clear();
						ss.str("");
						ss << i;
						ss >> numAsStr;
						imageFilename += numAsStr + ".png";
						// Create an image for texture i in the image library
						converted += "\t\t<image id=\"texture" + numAsStr + "\">\n";
						converted += "\t\t\t<init_from>" + imageFilename + "</init_from>\n";
						converted += "\t\t</image>\n";
					}
					converted += "\t</library_images>\n";
				}
				// Create the effect library for holding the material effects
				converted += "\t<library_effects>\n";
				for (unsigned int i = 0; i < numTextures || i == 0; i++)
				{
					ss.clear();
					ss.str("");
					ss << i;
					ss >> numAsStr;
					converted += "\t\t<effect id=\"mat" + numAsStr + "-effect\">\n";
					converted += "\t\t\t<profile_COMMON>\n";
					if (numTextures > 0)
					{
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
					}
					converted += "\t\t\t\t<technique sid=\"common\">\n";
					converted += "\t\t\t\t\t<lambert>\n";
					converted += "\t\t\t\t\t\t<diffuse>\n";
					if (numTextures > 0)
					{
						converted += "\t\t\t\t\t\t\t<texture texture=\"mat" + numAsStr + "-sampler\" texcoord=\"UVMap\"/>\n";
					}
					else
					{
						converted += "\t\t\t\t\t\t\t<color>1.0 1.0 1.0 1.0</color>\n";
					}
					converted += "\t\t\t\t\t\t</diffuse>\n";
					converted += "\t\t\t\t\t</lambert>\n";
					converted += "\t\t\t\t</technique>\n";
					converted += "\t\t\t</profile_COMMON>\n";
					converted += "\t\t</effect>\n";
				}
				converted += "\t</library_effects>\n";
				// Create material library for holding the materials
				converted += "\t<library_materials>\n";
				for (unsigned int i = 0; i < numTextures || i == 0; i++)
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
				for (unsigned int i = 0; i < numMeshes; i++)
				{
					string meshName = name + "-mesh";
					ss.clear();
					ss.str("");
					ss << setfill('0') << setw(3) << i;
					ss >> numAsStr;
					meshName += numAsStr;
					converted += "\t\t<geometry id=\"" + meshName + "\" name=\"" + meshName + "\">\n";
					converted += "\t\t\t<mesh>\n";
					converted += "\t\t\t\t<source id=\"" + meshName + "-pose\">\n";
					ss.clear();
					ss.str("");
					ss << verts[i].size() * 3;
					ss >> numAsStr;
					converted += "\t\t\t\t\t<float_array id=\"" + meshName + "-pose-array\" count=\"" + numAsStr + "\"> ";
					for (unsigned int j = 0; j < verts[i].size(); j++)
					{
						for (unsigned int k = 0; k < numDims; k++)
						{
							ss.clear();
							ss.str("");
							ss << verts[i][j].coordinates[k];
							ss >> numAsStr;
							converted += numAsStr + " ";
						}
					}
					converted += "</float_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					ss.clear();
					ss.str("");
					ss << verts[i].size();
					ss >> numAsStr;
					converted += "\t\t\t\t\t\t<accessor source=\"#" + meshName + "-pose-array\" count=\"" + numAsStr + "\" stride=\"3\">\n";
					converted += "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					converted += "\t\t\t\t<source id=\"" + meshName + "-normals\">\n";
					ss.clear();
					ss.str("");
					ss << verts[i].size() * 3;
					ss >> numAsStr;
					converted += "\t\t\t\t\t<float_array id=\"" + meshName + "-normals-array\" count=\"" + numAsStr + "\"> ";
					for (unsigned int j = 0; j < verts[i].size(); j++)
					{
						for (unsigned int k = 0; k < numDims; k++)
						{
							ss.clear();
							ss.str("");
							ss << verts[i][j].normal[k];
							ss >> numAsStr;
							converted += numAsStr + " ";
						}
					}
					converted += "</float_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					ss.clear();
					ss.str("");
					ss << verts[i].size();
					ss >> numAsStr;
					converted += "\t\t\t\t\t\t<accessor source=\"#" + meshName + "-normals-array\" count=\"" + numAsStr + "\" stride=\"3\">\n";
					converted += "\t\t\t\t\t\t\t<param name=\"X\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t\t<param name=\"Y\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t\t<param name=\"Z\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					converted += "\t\t\t\t<source id=\"" + meshName + "-texcoords\">\n";
					ss.clear();
					ss.str("");
					ss << verts[i].size() * 2;
					ss >> numAsStr;
					converted += "\t\t\t\t\t<float_array id=\"" + meshName + "-texcoords-array\" count=\"" + numAsStr + "\"> ";
					for (unsigned int j = 0; j < verts[i].size(); j++)
					{
						for (unsigned int k = 0; k < 2; k++)
						{
							ss.clear();
							ss.str("");
							ss << verts[i][j].textureMap[k];
							ss >> numAsStr;
							converted += numAsStr + " ";
						}
					}
					converted += "</float_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					ss.clear();
					ss.str("");
					ss << verts[i].size();
					ss >> numAsStr;
					converted += "\t\t\t\t\t\t<accessor source=\"#" + meshName + "-texcoords-array\" count=\"" + numAsStr + "\" stride=\"2\">\n";
					converted += "\t\t\t\t\t\t\t<param name=\"S\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t\t<param name=\"T\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					converted += "\t\t\t\t<vertices id=\"" + meshName + "-verts\">\n";
					converted += "\t\t\t\t\t<input semantic=\"POSITION\" source=\"#" + meshName + "-pose\"/>\n";
					converted += "\t\t\t\t\t<input semantic=\"NORMAL\" source=\"#" + meshName + "-normals\"/>\n";
					converted += "\t\t\t\t\t<input semantic=\"TEXCOORD\" source=\"#" + meshName + "-texcoords\"/>\n";
					converted += "\t\t\t\t</vertices>\n";
					counter = 0;
					for (unsigned int j = 0; j < numTextures || j == 0; j++)
					{
						if (numTextures == 0)
						{
							numFacesOfTexture = faces[i].size();
						}
						else
						{
							numFacesOfTexture = getNumFacesOfTexture(verts[i], faces[i], j);
						}
						if (numFacesOfTexture > 0)
						{
							ss.clear();
							ss.str("");
							ss << j;
							ss >> numAsStr;
							converted += "\t\t\t\t<triangles material=\"mat" + numAsStr + "\" count=\"";
							ss.clear();
							ss.str("");
							ss << numFacesOfTexture;
							ss >> numAsStr;
							converted += numAsStr + "\">\n";
							converted += "\t\t\t\t\t<input semantic=\"VERTEX\" source=\"#" + meshName + "-verts\" offset=\"0\"/>\n";
							converted += "\t\t\t\t\t<p> ";
							for (unsigned int k = 0; k < faces[i].size(); k++)
							{
								if (faces[i][j].vertexIndices[0] < verts[i].size() && (verts[i][faces[i][j].vertexIndices[0]].textureIndex == j || numTextures == 0))
								{
									for (unsigned int l = 0; l < numVertsPerFace; l++)
									{
										// Offset 0
										ss.clear();
										ss.str("");
										ss << faces[i][k].vertexIndices[l];
										ss >> numAsStr;
										converted += numAsStr + " ";
									}
								}
							}
							converted += "</p>\n";
							converted += "\t\t\t\t</triangles>\n";
						}
					}
					converted += "\t\t\t</mesh>\n";
					converted += "\t\t</geometry>\n";
				}
				converted += "\t</library_geometries>\n";
				// Create controller library to hold the rig
				converted += "\t<library_controllers>\n";
				for (unsigned int i = 0; i < numMeshes; i++)
				{
					string controllerName = name + "-skin", meshName = name + "-mesh";
					ss.clear();
					ss.str("");
					ss << setfill('0') << setw(3) << i;
					ss >> numAsStr;
					controllerName += numAsStr;
					meshName += numAsStr;
					converted += "\t\t<controller id=\"" + controllerName + "\" name=\"" + controllerName + "\">\n";
					converted += "\t\t\t<skin source=\"#" + meshName + "\">\n";
					// Create bind shape matrix for overlaying the normalized model on the skeleton
					converted += "\t\t\t\t<bind_shape_matrix> ";
					// Construct the bind translation array assuming the model has been scaled to the range [-1, 1]
					for (unsigned int j = 0; j < numDims; j++)
					{
						translate[j] = 0;
					}
					// Get the translation matrix using the bind translation
					transformMat = getTranslationMatrix(translate);
					for (unsigned int j = 0; j < 4; j++)
					{
						for (unsigned int k = 0; k < 4; k++)
						{
							ss.clear();
							ss.str("");
							ss << transformMat[j][k];
							ss >> numAsStr;
							converted += numAsStr + " ";
						}
					}
					converted += "</bind_shape_matrix>\n";
					// Create joint names
					converted += "\t\t\t\t<source id=\"" + controllerName + "-joints\">\n";
					ss.clear();
					ss.str("");
					ss << joints.size();
					ss >> numAsStr;
					converted += "\t\t\t\t\t<Name_array id=\"" + controllerName + "-joints-array\" count=\"" + numAsStr + "\"> ";
					for (unsigned int j = 0; j < joints.size(); j++)
					{
						ss.clear();
						ss.str("");
						ss << setfill('0') << setw(3) << joints[j].ind;
						ss >> numAsStr;
						converted += "bone" + numAsStr + " ";
					}
					converted += "</Name_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					ss.clear();
					ss.str("");
					ss << joints.size();
					ss >> numAsStr;
					converted += "\t\t\t\t\t\t<accessor source=\"#" + controllerName + "-joints-array\" count=\"" + numAsStr + "\" stride=\"1\">\n";
					converted += "\t\t\t\t\t\t\t<param name=\"JOINT\" type=\"name\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					// Create the inverse bind matrices for each joint
					converted += "\t\t\t\t<source id=\"" + controllerName + "-bind-poses\">\n";
					ss.clear();
					ss.str("");
					ss << joints.size() * 16;
					ss >> numAsStr;
					converted += "\t\t\t\t\t<float_array id=\"" + controllerName + "-bind-poses-array\" count=\"" + numAsStr + "\"> \n";
					tabbed = "\t\t\t\t\t\t";
					for (unsigned int j = 0; j < joints.size(); j++)
					{
						// Reset the transform mat
						if (getInverseBindMatrix(joints, j, transformMat))
						{
							converted += tabbed;
							for (unsigned int k = 0; k < transformMat.size(); k++)
							{
								for (unsigned int l = 0; l < transformMat[k].size(); l++)
								{
									ss.clear();
									ss.str("");
									ss << transformMat[k][l];
									ss >> numAsStr;
									converted += numAsStr + " ";
								}
							}
							converted += "\n";
						}
						else
						{
							cout << "Error: Failed to get inverse bind matrix of joint " << j << endl;
						}
					}
					converted += "\t\t\t\t\t</float_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					ss.clear();
					ss.str("");
					ss << joints.size();
					ss >> numAsStr;
					converted += "\t\t\t\t\t\t<accessor source=\"#" + controllerName + "-bind-poses-array\" count=\"" + numAsStr + "\" stride=\"16\">\n";
					converted += "\t\t\t\t\t\t\t<param name=\"TRANSFORM\" type=\"float4x4\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					// Create skin weights
					converted += "\t\t\t\t<source id=\"" + controllerName + "-weights\">\n";
					converted += "\t\t\t\t\t<float_array id=\"" + controllerName + "-weights-array\" count=\"1\">1</float_array>\n";
					converted += "\t\t\t\t\t<technique_common>\n";
					converted += "\t\t\t\t\t\t<accessor source=\"#" + controllerName + "-weights-array\" count=\"1\" stride=\"1\">\n";
					converted += "\t\t\t\t\t\t\t<param name=\"WEIGHT\" type=\"float\"/>\n";
					converted += "\t\t\t\t\t\t</accessor>\n";
					converted += "\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t</source>\n";
					// Define the joints
					converted += "\t\t\t\t<joints>\n";
					converted += "\t\t\t\t\t<input semantic=\"JOINT\" source=\"#" + controllerName + "-joints\"/>\n";
					converted += "\t\t\t\t\t<input semantic=\"INV_BIND_MATRIX\" source=\"#" + controllerName + "-bind-poses\"/>\n";
					converted += "\t\t\t\t</joints>\n";
					// Create VertexRelative Weights
					ss.clear();
					ss.str("");
					ss << verts[i].size();
					ss >> numAsStr;
					converted += "\t\t\t\t<vertex_weights count=\"" + numAsStr + "\">\n";
					converted += "\t\t\t\t\t<input semantic=\"JOINT\" source=\"#" + controllerName + "-joints\" offset=\"0\"/>\n";
					converted += "\t\t\t\t\t<input semantic=\"WEIGHT\" source=\"#" + controllerName + "-weights\" offset=\"1\"/>\n";
					converted += "\t\t\t\t\t<vcount> ";
					for (unsigned int j = 0; j < verts[i].size(); j++)
					{
						converted += "1 ";
					}
					converted += "</vcount>\n";
					converted += "\t\t\t\t\t<v> ";
					for (unsigned int j = 0; j < verts[i].size(); j++)
					{
						ss.clear();
						ss.str("");
						ss << verts[i][j].originJointIndex;
						ss >> numAsStr;
						converted += numAsStr + " 0 ";
					}
					converted += "</v>\n";
					converted += "\t\t\t\t</vertex_weights>\n";
					converted += "\t\t\t</skin>\n";
					converted += "\t\t</controller>\n";
				}
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
										ss << jointsG[0][childJointIndex].coordinates[i] - jointsG[0][jointIndex].coordinates[i];
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
								translate[i] = joints[jointIndex].coordinates[i];
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
				for (unsigned int i = 0; i < numMeshes; i++)
				{
					string meshName = name + "-mesh", controllerName = name + "-skin";
					ss.clear();
					ss.str("");
					ss << setfill('0') << setw(3) << i;
					ss >> numAsStr;
					meshName += numAsStr;
					controllerName += numAsStr;
					converted += "\t\t\t\t<node id=\"" + meshName + "\" name=\"" + meshName + "\" type=\"NODE\">\n";
					converted += "\t\t\t\t\t<instance_controller url=\"#" + controllerName + "\" name=\"" + controllerName + "\">\n";
					converted += "\t\t\t\t\t\t<skeleton>#bone000</skeleton>\n";
					converted += "\t\t\t\t\t\t<bind_material>\n";
					converted += "\t\t\t\t\t\t\t<technique_common>\n";
					for (unsigned int j = 0; j < numTextures || j == 0; j++)
					{
						if (numTextures == 0 || getNumFacesOfTexture(verts[i], faces[i], j) > 0)
						{
							ss.clear();
							ss.str("");
							ss << j;
							ss >> numAsStr;
							converted += "\t\t\t\t\t\t\t\t<instance_material symbol=\"mat" + numAsStr + "\" target=\"#mat" + numAsStr + "\">\n";
							converted += "\t\t\t\t\t\t\t\t\t<bind_vertex_input semantic=\"UVMap\" input_semantic=\"TEXCOORD\"/>\n";
							converted += "\t\t\t\t\t\t\t\t</instance_material>\n";
						}
					}
					converted += "\t\t\t\t\t\t\t</technique_common>\n";
					converted += "\t\t\t\t\t\t</bind_material>\n";
					//converted += "\t\t\t\t</instance_geometry>\n";
					converted += "\t\t\t\t\t</instance_controller>\n";
					converted += "\t\t\t\t</node>\n";
				}
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
	}
	else
	{
		cout << "Error: Invalid mesh" << endl;
	}
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
			rawTexture[i * bytesPerPixel] = t.palette.r[colorInd];
			rawTexture[i * bytesPerPixel + 1] = t.palette.g[colorInd];
			rawTexture[i * bytesPerPixel + 2] = t.palette.b[colorInd];
			rawTexture[i * bytesPerPixel + 3] = t.palette.a[colorInd];
		}
		else
		{
			rawTexture[i * bytesPerPixel] = 0;
			rawTexture[i * bytesPerPixel + 1] = 0;
			rawTexture[i * bytesPerPixel + 2] = 0;
			rawTexture[i * bytesPerPixel + 3] = 0;
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
unsigned int getNumTextures(vector<vector<VertexRelative>> verts)
{
	unsigned int numTextures = 0, numMeshes = verts.size(), numVerts, textureInd;
	for (unsigned int i = 0; i < numMeshes; i++)
	{
		numVerts = verts[i].size();
		for (unsigned int j = 0; j < numVerts; j++)
		{
			textureInd = verts[i][j].textureIndex;
			if (textureInd >= numTextures)
			{
				numTextures = textureInd + 1;
			}
		}
	}
	return numTextures;
}

// This function doesn't care about the vertex coordinates
unsigned int getNumTextures(vector<vector<VertexGlobal>> verts)
{
	return getNumTextures(*(vector<vector<VertexRelative>> *)&(verts));
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
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16;
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
					if (raw.size() >= (sectionOffset + sectionLen) && (sectionLen >= sectionHeaderLen))
					{
						specialEffectSection = raw.substr(sectionOffset, sectionLen);
						readOffset = 0;
						// Check the section id
						if (*(unsigned int *)&(specialEffectSection[readOffset]) == 130)
						{
							effects = getSPFXSpecialEffects(specialEffectSection);
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
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16;
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
					if (raw.size() >= (sectionOffset + sectionLen) && (sectionLen >= sectionHeaderLen))
					{
						specialEffectSection = raw.substr(sectionOffset, sectionLen);
						effects = getSPFXSpecialEffects(specialEffectSection);
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

vector<Texture> getSPFXTextures(string raw)
{
	vector<Texture> textures;
	unsigned int sectionHeaderLen = 16;
	if (raw.size() >= sectionHeaderLen && *(unsigned int *)&(raw[0]) == 130)
	{
		// Move to number of subsection type 0 entries
		unsigned int readOffset = ((*(unsigned int *)&(raw[12])) * 32) + 16, sectionLen = raw.size();
		if (sectionLen >= (readOffset + 4))
		{
			vector<unsigned int> subsectionOffsets;
			unsigned int numSubsections = *(unsigned int *)&(raw[readOffset]);
			readOffset += 4;
			subsectionOffsets.reserve(numSubsections);
			if (sectionLen >= (readOffset + (numSubsections * 4)))
			{
				// Store each subsection offset
				for (unsigned int i = 0; i < numSubsections; i++)
				{
					subsectionOffsets.push_back(*(unsigned int *)&(raw[readOffset]));
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
						if (*(unsigned int *)&(raw[readOffset]) == 150)
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
									unsigned int numSubsubsections = *(unsigned int *)&(raw[readOffset]);
									readOffset += 4;
									subsubsectionOffsets.push_back(vector<unsigned int>());
									subsubsectionOffsets[j].reserve(numSubsubsections);
									if (sectionLen >= (readOffset + (numSubsubsections * 4)))
									{
										for (unsigned int k = 0; k < numSubsubsections; k++)
										{
											subsubsectionOffsets[j].push_back(*(unsigned int *)&(raw[readOffset]));
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
								// Parse Type 1 Subsubsections
								// Move to the start of the subsubsection
								readOffset = subsectionOffsets[i] + subsubsectionOffsets[1][j];
								// Make sure there is enough data for the header
								if (sectionLen >= (readOffset + 32))
								{
									Texture t;
									unsigned int x, indsLen;
									unsigned short paletteLen = 0x400;
									t.width = *(unsigned short *)&(raw[readOffset + 12]);
									x = 0;
									while ((t.width >> x) > 1)
									{
										x++;
									}
									t.wExp = x;
									t.height = *(unsigned short *)&(raw[readOffset + 14]);
									x = 0;
									while ((t.height >> x) > 1)
									{
										x++;
									}
									t.hExp = x;
									indsLen = *(unsigned int *)&(raw[readOffset + 27]) & 0xFFFFFF;
									t.indices.reserve(indsLen);
									paletteLen = *(unsigned short *)&(raw[readOffset + 29]);
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
											unsigned char ind = (unsigned char)raw[readOffset + k], remainder = ind % 32;
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
												unsigned char colorVal = (unsigned char)raw[readOffset + (k * 4)];
												t.palette.r.push_back(colorVal);
												colorVal = (unsigned char)raw[readOffset + (k * 4) + 1];
												t.palette.g.push_back(colorVal);
												colorVal = (unsigned char)raw[readOffset + (k * 4) + 2];
												t.palette.b.push_back(colorVal);
												colorVal = (unsigned char)raw[readOffset + (k * 4) + 3];
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
		cout << "Error: This section is not a special effect section" << endl;
	}
	return textures;
}

vector<string> getSPFXSpecialEffects(string raw)
{
	vector<string> effects;
	unsigned int readOffset = 0, sectionLen = raw.size();
	// Check the section id
	if (*(unsigned int *)&(raw[readOffset]) == 130)
	{
		unsigned int numEntries = 0;
		vector<unsigned int> entryOffs;
		vector<string> entryMetaData;
		// Move to number of subsection type 0 entries
		readOffset += 12;
		// Get the number of entries
		numEntries = *(unsigned int *)&(raw[readOffset]);
		readOffset += 4;
		entryOffs.reserve(numEntries);
		entryMetaData.reserve(numEntries);
		// For each entry, read the meta data and add it to the effect string
		for (unsigned int i = 0; i < numEntries; i++)
		{
			entryOffs.push_back(*(unsigned int *)&(raw[readOffset]));
			readOffset += 4;
			entryMetaData.push_back(raw.substr(readOffset, 28));
			readOffset += 28;
		}
		if (sectionLen >= (readOffset + 4))
		{
			vector<unsigned int> subsectionOffsets;
			unsigned int numSubsections = *(unsigned int *)&(raw[readOffset]);
			readOffset += 4;
			subsectionOffsets.reserve(numSubsections);
			if (sectionLen >= (readOffset + (numSubsections * 4)))
			{
				// Store each subsection offset
				for (unsigned int i = 0; i < numSubsections; i++)
				{
					subsectionOffsets.push_back(*(unsigned int *)&(raw[readOffset]));
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
					string effect = "SPFX";
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
						if (*(unsigned int *)&(raw[readOffset]) == 150)
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
									unsigned int numSubsubsections = *(unsigned int *)&(raw[readOffset]);
									readOffset += 4;
									subsubsectionOffsets.push_back(vector<unsigned int>());
									subsubsectionOffsets[j].reserve(numSubsubsections);
									if (sectionLen >= (readOffset + (numSubsubsections * 4)))
									{
										for (unsigned int k = 0; k < numSubsubsections; k++)
										{
											subsubsectionOffsets[j].push_back(*(unsigned int *)&(raw[readOffset]));
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
											unsigned int numVals = *(unsigned int *)&(raw[readOffset]), remainder;
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
										indsLen = *(unsigned int *)&(raw[readOffset + 27]) & 0xFFFFFF;
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
										subsubsectionLen = ((*(unsigned short *)&(raw[readOffset + 30])) + 16);
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
										unsigned int numVals = *(unsigned int *)&(raw[readOffset + 8]), remainder;
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
								effect += raw.substr(subsectionOffsets[i], effectLen);
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
									if (colorVal >= 120)
									{
										colorVal = 255;
									}
									else
									{
										colorVal = 0;
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

vector<Texture> getMsetTextures(string raw, vector<Texture> mdlsTextures)
{
	vector<Texture> textures;
	unsigned int numSections, readOffset = 0, msetLen = raw.size();
	// Make sure there is data to read
	if (msetLen >= (readOffset + 4))
	{
		// Get the number of sections
		numSections = *(unsigned int *)&(raw[readOffset]);
		// Update the read offset
		readOffset += 4;
		// Make sure this file has the correct number of sections
		if (numSections == 3)
		{
			// Extract the mmtn offset
			unsigned int mmtnOffset = *(unsigned int *)&(raw[readOffset]);
			readOffset = mmtnOffset;
			// Make sure there is enough data for the MSET header and the MMTN section has the magic value
			if (msetLen >= (readOffset + mmtnHeaderLen) && raw.substr(readOffset, 4) == "MMTN")
			{
				unsigned int mmtnLen = *(unsigned int *)&(raw[readOffset + 4]) + 8;
				readOffset += mmtnLen;
				// Make sure there is enough data for the TEXA header and the TEXA section has the magic value
				if (msetLen >= (readOffset + texaHeaderLen) && raw.substr(readOffset, 4) == "TEXA")
				{
					unsigned int texaOffset = readOffset, texaLen = *(unsigned int *)&(raw[readOffset + 4]) + 8;
					if (msetLen >= (texaOffset + texaLen))
					{
						string texa = raw.substr(texaOffset, texaLen);
						textures = getTexaTextures(texa, mdlsTextures);
					}
				}
			}
		}
	}
	return textures;
}

vector<Texture> getTexaTextures(string raw, vector<Texture> referenceTextures)
{
	vector<Texture> texaTextures;
	unsigned int texaLen = raw.size(), readOffset = 0;
	if (raw.size() >= texaHeaderLen)
	{
		unsigned int numMdlsTextures = *(unsigned int *)&(raw[readOffset + 8]), numTextureSizes = *(unsigned int *)&(raw[readOffset + 12]), numTextures = *(unsigned int *)&(raw[readOffset + 20]);
		vector<unsigned char> originImage;
		vector<unsigned short> widths, heights;
		vector<unsigned int> textureOffsets;
		originImage.reserve(numTextures);
		widths.reserve(numTextures);
		heights.reserve(numTextures);
		textureOffsets.reserve(numTextures);
		readOffset = texaHeaderLen;
		if (numMdlsTextures == referenceTextures.size())
		{
			if (texaLen >= (readOffset + (numTextureSizes * 8) + (numTextures * 4)))
			{
				for (unsigned int i = 0; i < numTextureSizes; i++)
				{
					originImage.push_back(raw[readOffset + 3]);
					if (originImage.back() >= referenceTextures.size())
					{
						cout << "Error: Invalid Origin Texture Index" << endl;
					}
					widths.push_back(*(unsigned short *)&(raw[readOffset + 4]));
					heights.push_back(*(unsigned short *)&(raw[readOffset + 6]));
					readOffset += 8;
				}
				for (unsigned int i = 0; i < numTextures; i++)
				{
					textureOffsets.push_back(*(unsigned int *)&(raw[readOffset]));
					readOffset += 4;
				}
				for (unsigned int i = 0; i < numTextures; i++)
				{
					Texture ti;
					unsigned int textureLen, textureSizeInd;
					// Calculate the size of the texture
					if (i < (numTextures - 1))
					{
						textureLen = textureOffsets[i + 1] - textureOffsets[i];
					}
					else
					{
						textureLen = texaLen - textureOffsets[i];
					}
					// Determine the texture size index
					for (textureSizeInd = 0; textureSizeInd < numTextureSizes; textureSizeInd++)
					{
						if (textureLen == (widths[textureSizeInd] * heights[textureSizeInd]))
						{
							ti.width = widths[textureSizeInd];
							ti.height = heights[textureSizeInd];
							if (originImage[textureSizeInd] < referenceTextures.size())
							{
								ti.palette = referenceTextures[originImage[textureSizeInd]].palette;
							}
							break;
						}
					}
					readOffset = textureOffsets[i];
					if (texaLen >= (readOffset + textureLen))
					{
						for (unsigned int j = 0; j < textureLen; j++)
						{
							unsigned char ind = raw[readOffset + j], indMod = ind % 32;
							if (indMod >= 8 && indMod < 16)
							{
								ind += 8;
							}
							else if (indMod >= 16 && indMod < 24)
							{
								ind -= 8;
							}
							ti.indices.push_back(ind);
						}
						texaTextures.push_back(ti);
					}
				}
			}
		}
		else
		{
			cout << "Error: TEXA does not correspond with Reference Textures" << endl;
		}
	}
	return texaTextures;
}

vector<Texture> getWpnTextures(string raw)
{
	vector<Texture> textures;
	Texture t;
	unsigned int numTextures, numSections, readOffset = 0, subsectionOffsets[3], subsectionLengths[3];
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
				unsigned int menvOffset = *(unsigned int *)&(raw[readOffset]);
				// Make sure there is data to read and that the MENV magic value exists
				if (raw.size() >= (menvOffset + menvHeaderLen) && raw.substr(menvOffset, 4) == "MENV")
				{
					unsigned int menvLen = *(unsigned int *)&(raw[menvOffset + 4]) + 8;
					// Move to the first section offset
					readOffset = menvOffset + 8;
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
									if (colorVal >= 120)
									{
										colorVal = 255;
									}
									else
									{
										colorVal = 0;
									}
									textures[i].palette.a.push_back(colorVal);
								}
							}
						}
					}
					// Check the TEXA section
					unsigned int texaOffset = menvOffset + menvLen;
					if (raw.size() >= (texaOffset + texaHeaderLen) && raw.substr(texaOffset, 4) == "TEXA")
					{
						unsigned int texaLen = *(unsigned int *)&(raw[texaOffset + 4]) + 8;
						if (raw.size() >= (texaOffset + texaLen))
						{
							string texa = raw.substr(texaOffset, texaLen);
							vector<Texture> texaTextures = getTexaTextures(texa, textures);
							for (unsigned int i = 0; i < texaTextures.size(); i++)
							{
								textures.push_back(texaTextures[i]);
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
									specialEffectSection.append(4, 0);						// Effect Offset (will update later)
									specialEffectSection += e.substr((j * 28) + 8, 28);		// Effect Meta Data
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
										specialEffectSection.append(4, 0);						// Effect Offset (will update later)
										specialEffectSection += e.substr((j * 28) + 8, 28);		// Effect Meta Data
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
							if (width != 128 || height != 128)
							{
								cout << "Warning: MDLS Textures that are not 128x128 pixels are unstable" << endl;
							}
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
								unsigned char alpha = textures[i].palette.a[j];
								updatedMobj.append(1, textures[i].palette.r[j]);
								updatedMobj.append(1, textures[i].palette.g[j]);
								updatedMobj.append(1, textures[i].palette.b[j]);
								if (alpha >= 128)
								{
									alpha = 128;
								}
								else
								{
									alpha = 0;
								}
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

string setMsetTextures(string raw, vector<Texture> msetTextures, vector<Texture> mdlsTextures, vector<unsigned int> referenceInds)
{
	string mset = "";
	unsigned int rawLen = raw.size();
	if (rawLen >= 20)
	{
		unsigned int numSections = *(unsigned int *)&(raw[0]);
		if (numSections == 3)
		{
			unsigned int mmtnOff = *(unsigned int *)&(raw[4]), mmtnLen, tableOff = *(unsigned int *)&(raw[8]), unknownOff = *(unsigned int *)&(raw[12]);
			// Copy the header
			mset = raw.substr(0, mmtnOff);
			if (rawLen >= (mmtnOff + mmtnHeaderLen) && raw.substr(mmtnOff, 4) == "MMTN")
			{
				mmtnLen = *(unsigned int *)&(raw[mmtnOff + 4]) + 8;
				unsigned int texaOff = mmtnOff + mmtnLen, texaLen = tableOff - texaOff - 128;
				// Copy MMTN section
				mset += raw.substr(mmtnOff, mmtnLen);
				// Copy TEXA section
				mset += setTexaTextures(raw.substr(texaOff, texaLen), msetTextures, mdlsTextures, referenceInds);
				// Add KN5
				mset += "_KN5";
				mset.append(124, 0);
				// Update table offset
				*(unsigned int *)&(mset[8]) = mset.size();
				// Copy Table Section
				mset += raw.substr(tableOff, unknownOff - tableOff);
				// Update unknown offset
				*(unsigned int *)&(mset[12]) = mset.size();
				// Copy Unknown Section
				mset += raw.substr(unknownOff, raw.size() - unknownOff);
				// Update fake length field
				if (mset.size() % 128 == 0)
				{
					*(unsigned int *)&(raw[16]) = mset.size();
				}
				else
				{
					*(unsigned int *)&(raw[16]) = mset.size() + (128 - (mset.size() % 128));
				}
			}
			else
			{
				cout << "Error: Invalid MMTN subsection" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid number of sections in MSET" << endl;
		}
	}
	else
	{
		cout << "Error: Missing MSET Magic Value" << endl;
	}
	return mset;
}

string setTexaTextures(string raw, vector<Texture> texaTextures, vector<Texture> referenceTextures, vector<unsigned int> referenceInds)
{
	bool valid;
	string texa = "", numAsStr;
	unsigned int texaLen = raw.size(), numTextures = texaTextures.size(), numReferences = referenceTextures.size(), numTextureSizes, readOffset;
	vector<unsigned short> widths;
	vector<unsigned short> heights;
	vector<unsigned int> refInds;
	vector<unsigned int> textureSizeInds;
	// Initialize valid by comparing the number of reference inds to the number of textures
	valid = numTextures == referenceInds.size();
	// Determine the number of 
	for (unsigned int i = 0; i < numTextures; i++)
	{
		Texture t = texaTextures[i];
		for (unsigned int j = 0; j <= widths.size(); j++)
		{
			if (j < widths.size())
			{
				// Check if the texture matches this size
				if (t.width == widths[j] && t.height == heights[j] && referenceInds[i] == refInds[j])
				{
					// Move on to the next texture
					textureSizeInds.push_back(j);
					break;
				}
			}
			else
			{
				// Add the new size
				widths.push_back(t.width);
				heights.push_back(t.height);
				refInds.push_back(referenceInds[i]);
				textureSizeInds.push_back(j);
				break;
			}
		}
	}
	// Store the number of texture sizes
	numTextureSizes = widths.size();
	// Reserve space in the texture size vectors
	widths.reserve(numTextureSizes);
	heights.reserve(numTextureSizes);
	textureSizeInds.reserve(numTextures);
	// Validate the reference indices
	for (unsigned int i = 0; valid && i < numTextureSizes; i++)
	{
		if (refInds[i] >= numReferences)
		{
			valid = false;
		}
	}
	// Check for valid input
	if (valid)
	{
		// Make sure the raw data is a texa valid section
		if (texaLen >= texaHeaderLen && raw.substr(0, 4) == "TEXA" && texaLen == (*(unsigned int *)&(raw[4]) + 8))
		{
			unsigned int rawOffsets[2], subsectionOffsets[2], numRawTS = *(unsigned int *)&(raw[12]), numEntries, numRawTextures;
			// Magic Value
			texa = "TEXA";
			// Initialize size field to 0
			texa.append(4, 0);
			texa.append((char *)&(numReferences), sizeof(unsigned int));
			texa.append((char *)&(numTextureSizes), sizeof(unsigned int));
			texa += raw.substr(16, 4);
			texa.append((char *)&(numTextures), sizeof(unsigned int));
			numRawTextures = *(unsigned int *)&(raw[20]);
			texa += raw.substr(24, 4);
			numEntries = *(unsigned int *)&(raw[28]);
			texa.append((char *)&(numEntries), sizeof(unsigned int));
			readOffset = 32;
			for (unsigned int i = 0; i < 2; i++)
			{
				rawOffsets[i] = *(unsigned int *)&(raw[readOffset]);
				subsectionOffsets[i] = rawOffsets[i] + (numTextureSizes - numRawTS) * 8 + (numTextures - numRawTextures) * 4;
				texa.append((char *)&(subsectionOffsets[i]), sizeof(unsigned int));
				readOffset += 4;
			}
			for (unsigned int i = 0; i < numTextureSizes; i++)
			{
				if (i < numRawTS)
				{
					texa += raw.substr(readOffset, sizeof(unsigned int));
				}
				else
				{
					texa.append(4, 0);
				}
				texa[readOffset + 3] = (unsigned char)(referenceInds[i]);
				texa.append((char *)&(widths[i]), sizeof(unsigned short));
				texa.append((char *)&(heights[i]), sizeof(unsigned short));
				readOffset += 8;
			}
			readOffset = subsectionOffsets[1] + 256;
			if (readOffset % 128 > 0)
			{
				readOffset += (128 - readOffset % 128);
			}
			for (unsigned int i = 0; i < numTextures; i++)
			{
				texa.append((char *)&(readOffset), sizeof(unsigned int));
				readOffset += texaTextures[i].indices.size();
			}
			readOffset = subsectionOffsets[0];
			for (unsigned int i = 0; i < numEntries; i++)
			{
				unsigned int off = *(unsigned int *)&(raw[readOffset]) + (subsectionOffsets[0] - rawOffsets[0]);
				texa.append((char *)&(off), sizeof(unsigned int));
				readOffset += 4;
			}
			readOffset = *(unsigned int *)&(raw[rawOffsets[0]]);
			texa += raw.substr(readOffset, rawOffsets[1] - readOffset + 256);
			// Padding to align with 128 bytes
			if (texa.size() % 128 != 0)
			{
				texa.append(128 - texa.size() % 128, 0);
			}
			for (unsigned int i = 0; i < numTextures; i++)
			{
				Texture t = texaTextures[i], tr = referenceTextures[referenceInds[i]];
				Palette p = t.palette, pr = tr.palette;
				cout << "Texture " << i << "...";
				for (unsigned int j = 0; j < t.indices.size(); j++)
				{
					unsigned char cj = t.indices[j], minInd = 0, minIndMod;
					double minDist = numeric_limits<double>::max();
					for (unsigned int k = 0; k < pr.numColors; k++)
					{
						double kDist = pow(p.r[cj] - pr.r[k], 2) + pow(p.g[cj] - pr.g[k], 2) + pow(p.b[cj] - pr.b[k], 2) + pow(p.a[cj] - pr.a[k], 2);
						if (kDist < minDist)
						{
							minDist = kDist;
							minInd = (unsigned char)k;
						}
					}
					minIndMod = minInd % 32;
					if (minIndMod >= 8 && minIndMod < 16)
					{
						minInd += 8;
					}
					else if (minIndMod >= 16 && minIndMod < 24)
					{
						minInd -= 8;
					}
					texa.append(1, minInd);
				}
				cout << "Success" << endl;
			}
			// Update TEXA Length
			*(unsigned int *)&(texa[4]) = texa.size() - 8;
		}
		else
		{
			cout << "Error: Input is not a TEXA section" << endl;
		}
	}
	else
	{
		cout << "Error: Reference Images do not correspond with Reference Indices" << endl;
	}
	return texa;
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
							if (width != 128 || height != 128)
							{
								cout << "Warning: WPN Textures that are not 128x128 pixels are unstable" << endl;
							}
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
								unsigned char alpha = textures[i].palette.a[j];
								updatedMenv.append(1, textures[i].palette.r[j]);
								updatedMenv.append(1, textures[i].palette.g[j]);
								updatedMenv.append(1, textures[i].palette.b[j]);
								if (alpha >= 128)
								{
									alpha = 128;
								}
								else
								{
									alpha = 0;
								}
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

vector<Texture> getMagSpecialEffectTextures(string raw)
{
	vector<Texture> textures;
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16;
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
			readOffset = 8;
			// Make sure there is data to read
			if (raw.size() >= 40)
			{
				// Get the offset to the Special Effects section
				sectionOffset = *(unsigned int *)&(raw[readOffset]);
				// Move to the HUD Image section offset
				readOffset += 4;
				// Determine the section length
				sectionLen = raw.size() - sectionOffset;
				// Make sure the section isn't empty
				if (sectionLen > 0)
				{
					// Make sure there is data to read and that there is a section header
					if (raw.size() >= (sectionOffset + sectionLen) && (sectionLen >= sectionHeaderLen))
					{
						specialEffectSection = raw.substr(sectionOffset, sectionLen);
						// Move to the start of the section
						readOffset = 0;
						// Check the section id
						if (*(unsigned int *)&(specialEffectSection[readOffset]) == 130)
						{
							textures = getSPFXTextures(specialEffectSection);
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
				cout << "Error: Invalid MAG - Missing full header" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid MAG - Incorrect number of sections" << endl;
		}
	}
	else
	{
		cout << "Error: Invalid MAG - Missing number of sections" << endl;
	}
	return textures;
}

vector<Texture> getMdlsSpecialEffectTextures(string raw)
{
	vector<Texture> textures;
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16;
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
					if (raw.size() >= (sectionOffset + sectionLen) && (sectionLen >= sectionHeaderLen))
					{
						specialEffectSection = raw.substr(sectionOffset, sectionLen);
						// Move to the start of the section
						readOffset = 0;
						// Check the section id
						if (*(unsigned int *)&(specialEffectSection[readOffset]) == 130)
						{
							textures = getSPFXTextures(specialEffectSection);
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
	unsigned int numSections, readOffset = 0, sectionOffset, sectionLen, sectionHeaderLen = 16;
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
			if (raw.size() >= sectionHeaderLen)
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
							textures = getSPFXTextures(specialEffectSection);
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
	unsigned int numVertsOfTexture = 0, numVerts = verts.size(), numMeshes = faces.size(), numFaces = faces.size(), vertexInd;
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
	mat[3][0] = 0;
	mat[3][1] = 0;
	mat[3][2] = 0;
	mat[3][3] = 1;
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
				mat[numVertsPerFace][j] += v.coordinates[j] + v.normal[j];
			}
		}
		mat[i][numDims] = 1;
	}
	mat[3][0] /= numVertsPerFace;
	mat[3][1] /= numVertsPerFace;
	mat[3][2] /= numVertsPerFace;
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
			// Move up the skeleton until you reach the root node - start depth
			for (nextNode = currentNode; nextNode && nextNode != scene->mRootNode;)
			{
				parentNode = currentNode->mParent;
				nextNode = parentNode;
				// Check the start depth
				for (unsigned int i = 0; nextNode && i < (startDepth + 1); i++)
				{
					nextNode = nextNode->mParent;
				}
				// Make sure next node exists
				if (nextNode)
				{
					// Move to the parent node
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
				unsigned int numericStartIndex;
				tj.name = currentNode->mName.data;
				// Find the start index for the numerical portion of the name
				for (numericStartIndex = 0; numericStartIndex < tj.name.size(); numericStartIndex++)
				{
					if (tj.name[numericStartIndex] >= '0' && tj.name[numericStartIndex] <= '9')
					{
						break;
					}
				}
				// Convert the numerical portion to an unsigned int and store it as the joint's ind value
				ss.clear();
				ss.str(tj.name.substr(numericStartIndex, tj.name.size() - numericStartIndex));
				ss >> tj.ind;
				// Check if this is the first joint
				if (joints.size() > 0)
				{
					// It isn't so find the parent node in the list of existing joints
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
				// Initialize unknown values
				tj.jointInfo.unknownFlag = true;
				tj.jointInfo.unknownIndex = 0x3FF;
				// Initialize the child index to none
				tj.jointInfo.childIndex = 0x3FF;
				// Decompose the node transformation
				aiVector3D scale, rotate, translate;
				mdlsDecompose(currentNode->mTransformation, scale, rotate, translate);
				// Store the decomposed values
				for (unsigned int i = 0; i < numDims; i++)
				{
					tj.coordinates[i] = translate[i];
					tj.rotations[i] = rotate[i];
					tj.scaleFactors[i] = scale[i];
				}
				// Initialize another unknown value
				tj.special = 0;
				// Store the joint
				joints.push_back(tj);
				// Find the next node
				if (currentNode->mNumChildren > 0)
				{
					// This node has a child, so just move on to that
					currentNode = currentNode->mChildren[0];
				}
				else
				{
					// Initialize next node to the current node
					nextNode = currentNode;
					// Loop until we found a different node to use
					while (nextNode == currentNode)
					{
						// Store the currentNode's parent
						parentNode = currentNode->mParent;
						// If the parent node exists
						if (parentNode)
						{
							// Get the next child index
							unsigned int currentChildInd;
							for (currentChildInd = 0; currentNode == nextNode && currentChildInd < (parentNode->mNumChildren - 1); currentChildInd++)
							{
								if (currentNode == parentNode->mChildren[currentChildInd])
								{
									nextNode = parentNode->mChildren[currentChildInd + 1];
								}
							}
							// Check if there were no more children to use as the next node
							if (currentNode == nextNode && currentChildInd == (parentNode->mNumChildren - 1))
							{
								// Set the next node to the parent node
								nextNode = parentNode;
								// Make sure that the next node is not the root node
								if (nextNode != root)
								{
									// Set the current node to the parent node to continue the loop
									currentNode = parentNode;
								}
							}
						}
						else
						{
							// Set the next node to non existent to exit
							nextNode = parentNode;
						}
					}
					// Set the current node to next node
					currentNode = nextNode;
				}
			} while (currentNode && currentNode != root);
			// Create a copy of the joints
			copy = joints;
			// Sort the joints by the ind value
			for (unsigned int i = 0; i < joints.size(); i++)
			{
				// Make sure the ind is valid
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
			// Update the child indices values and child index vectors for each joint
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
	}
	return valid;
}

bool getDaeVerticesAndFaces(const aiScene *scene, vector<vector<VertexGlobal>> &verts, vector<vector<Face>> &faces, vector<JointRelative> joints)
{
	bool valid = scene;
	unsigned int numVerts, numFaces, boneIndex, jointIndex;
	VertexGlobal tempVert;
	// Clear the verts and faces vectors
	verts.clear();
	faces.clear();
	// Reserve the space for each mesh
	verts.reserve(scene->mNumMeshes);
	faces.reserve(scene->mNumMeshes);
	// Make sure the assimp scene is valid
	if (valid)
	{
		// For each mesh
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			//cout << "Starting Mesh " << i << endl;
			// Create the vectors for the mesh
			verts.push_back(vector<VertexGlobal>());
			faces.push_back(vector<Face>());
			// Get the number of vertices and faces the mesh has
			numVerts = scene->mMeshes[i]->mNumVertices;
			numFaces = scene->mMeshes[i]->mNumFaces;
			// Reserve space for the vertices and faces
			verts[i].reserve(numVerts);
			faces[i].reserve(numFaces);
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
				vector<VertexGlobal>::iterator iter = find(verts[i].begin(), verts[i].end(), tempVert);
				if (iter == verts[i].end())
				{
					// It is a new vertex, so add it to the vector
					//cout << "Adding Vertex " << verts.size() << endl;
					verts[i].push_back(tempVert);
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
					for (unsigned int k = 0; k < numDims; k++)
					{
						// Include the offset (Note: This can get sketchy with our code that removes duplicates, but the update importer settings removes duplicates prior to this point)
						f.vertexIndices[k] = scene->mMeshes[i]->mFaces[j].mIndices[k];
					}
					//f.orientation = getFaceOrientation(f, verts[i]);
					f.orientation = COUNTERCLOCKWISE;
					faces[i].push_back(f);
				}
			}
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

bool getMdlsVerticesAndFaces(string raw, vector<JointRelative> joints, vector<vector<VertexRelative>> &verts, vector<vector<Face>> &faces)
{
	// Declare variables
	const unsigned int numOrigins = 3;
	unsigned int mobjOff, tableOff, modelSectionLen, numPolygonCollections, *polygonCollectionTextures, *polygonCollectionOffs, polygonCollectionLen, polygonEntryType, polygonLen, numPolygonVerts, originIndex, off, jointIndex, extraFlag, origins[numOrigins] = { 0, 0, 0 };
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
			tableOff = *(unsigned int *)&(raw[mobjOff + 32]);
			// Determine the length of the 3D Model section
			modelSectionLen = *(unsigned int *)&(raw[mobjOff + 36]);
			// Make sure the file contains a polygon collection table
			if (raw.size() >= mobjOff + tableOff + modelTableHeaderLen)
			{
				// Get the number of polygon collections
				numPolygonCollections = *(unsigned int *)&(raw[mobjOff + tableOff + 12]);
				// Reserve space for the meshes
				verts.reserve(numPolygonCollections);
				faces.reserve(numPolygonCollections);
				// Initialize the texture and offset arrays
				polygonCollectionTextures = new unsigned int[numPolygonCollections];
				polygonCollectionOffs = new unsigned int[numPolygonCollections];
				// Make sure the polygon collection table is the correct size
				if (raw.size() >= mobjOff + tableOff + (numPolygonCollections * 16))
				{
					// Store the offsets to each polygon collection
					for (unsigned int i = 0; i < numPolygonCollections; i++)
					{
						polygonCollectionTextures[i] = *(unsigned int *)&(raw[mobjOff + tableOff + (i * 16) + 24]);
						polygonCollectionOffs[i] = mobjOff + tableOff + *(unsigned int *)&(raw[mobjOff + tableOff + (i * 16) + 28]);
					}
					// Make sure the file contains each polygon collection
					if (raw.size() >= polygonCollectionOffs[numPolygonCollections - 1] + 16)
					{
						// For each polygon collection
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							vector<unsigned int> originIndTable, extraJointInds;
							// Create the vector for the meshes
							verts.push_back(vector<VertexRelative>());
							// Get the polygon collection length
							if (i < numPolygonCollections - 1)
							{
								polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
							}
							else
							{
								polygonCollectionLen = modelSectionLen - (polygonCollectionOffs[i] - mobjOff - tableOff);
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
										// Get the extra flag
										extraFlag = *(unsigned int *)&(raw[j + 12]);
										if (extraFlag == 0)
										{
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
										else
										{
											if (extraFlag == 1)
											{
												extraJointInds.clear();
											}
											extraJointInds.push_back(jointIndex);
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
														}
														// Check if we already have tempVert in verts
														if (find(verts[i].begin(), verts[i].end(), tempVert) == verts[i].end())
														{
															// This is a new vertex, add tempVert to verts
															verts[i].push_back(tempVert);
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
										unsigned int numLines = *(unsigned int *)&(raw[j + 8]);
										originIndTable.clear();
										for (unsigned int k = 0; k < numLines; k++)
										{
											for (unsigned int l = 0; l < 4; l++)
											{
												originIndTable.push_back(*(unsigned int *)&(raw[j + (k * 16) + (l * 4) + 16]));
											}
										}
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
							vector<unsigned int> originIndTable, extraJointInds;
							// Create the vector for the mesh
							faces.push_back(vector<Face>());
							// Get the polygon collection length
							if (i < numPolygonCollections - 1)
							{
								polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
							}
							else
							{
								polygonCollectionLen = modelSectionLen - (polygonCollectionOffs[i] - mobjOff - tableOff);
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
										// Get the extra flag
										extraFlag = *(unsigned int *)&(raw[j + 12]);
										if (extraFlag == 0)
										{
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
										else
										{
											if (extraFlag == 1)
											{
												extraJointInds.clear();
											}
											extraJointInds.push_back(jointIndex);
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
															// Extract the normal values
															tempVert.normal[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 32]);
															// Get the vertex's relative coordinate
															tempVert.coordinates[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 48]);
														}
														// Extract the texture u and vr values of the vertex
														for (unsigned int l = 0; l < 2; l++)
														{
															tempVert.textureMap[l] = *(float *)&(raw[j + (k * 48) + (l * 4) + (numWeirds * 16) + 64]);
														}
														// Get the index of this vertex in the verts vector
														unsigned int currentVertInd = (find(verts[i].begin(), verts[i].end(), tempVert) - verts[i].begin()), nextVertInd;
														// Make sure we got a valid index
														if (currentVertInd < verts[i].size())
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
																		}
																		// Get the index of vertex k + l
																		nextVertInd = (find(verts[i].begin(), verts[i].end(), tempVert) - verts[i].begin());
																		// Make sure we got a valid index
																		if (nextVertInd < verts[i].size())
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
																faces[i].push_back(tempFace);
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
										unsigned int numLines = *(unsigned int *)&(raw[j + 8]);
										originIndTable.clear();
										for (unsigned int k = 0; k < numLines; k++)
										{
											for (unsigned int l = 0; l < 4; l++)
											{
												originIndTable.push_back(*(unsigned int *)&(raw[j + (k * 16) + (l * 4) + 16]));
											}
										}
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

bool getMdlsShadowVerticesAndFaces(string raw, vector<JointRelative> joints, vector<vector<VertexRelative>> &verts, vector<vector<Face>> &faces)
{
	// Declare variables
	const unsigned int numOrigins = 3;
	unsigned int mobjOff, tableOff, modelSectionLen, numPolygonCollections, *polygonCollectionOffs, polygonCollectionLen, polygonEntryType, polygonLen, numPolygonVerts, originIndex, off, jointIndex, origins[numOrigins] = { 0, 0, 0 };
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
			tableOff = *(unsigned int *)&(raw[mobjOff + 40]);
			// Determine the length of the 3D Model section
			modelSectionLen = *(unsigned int *)&(raw[mobjOff + 44]);
			// Make sure the file contains a polygon collection table
			if (raw.size() >= mobjOff + tableOff + modelTableHeaderLen)
			{
				// Get the number of polygon collections
				numPolygonCollections = *(unsigned int *)&(raw[mobjOff + tableOff + 12]);
				// Reserve space for the meshes
				verts.reserve(numPolygonCollections);
				faces.reserve(numPolygonCollections);
				// Initialize the offset array
				polygonCollectionOffs = new unsigned int[numPolygonCollections];
				// Make sure the polygon collection table is the correct size
				if (raw.size() >= mobjOff + tableOff + (numPolygonCollections * 16))
				{
					// Store the offsets to each polygon collection
					for (unsigned int i = 0; i < numPolygonCollections; i++)
					{
						polygonCollectionOffs[i] = mobjOff + tableOff + *(unsigned int *)&(raw[mobjOff + tableOff + (i * 16) + 28]);
					}
					// Make sure the file contains each polygon collection
					if (raw.size() >= polygonCollectionOffs[numPolygonCollections - 1] + 16)
					{
						// For each polygon collection
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							// Create the vector for the meshes
							verts.push_back(vector<VertexRelative>());
							// Get the polygon collection length
							if (i < numPolygonCollections - 1)
							{
								polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
							}
							else
							{
								polygonCollectionLen = modelSectionLen - (polygonCollectionOffs[i] - mobjOff - tableOff);
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
										polygonLen = 80;
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
												// For each vertex that makes up the polygon
												for (unsigned int k = 0; k < numPolygonVerts; k++)
												{
													// Extract the origin index
													originIndex = *(unsigned int *)&(raw[j + (k * 16) + 44]);
													// Make sure that the origin index is valid
													if (originIndex < numOrigins)
													{
														// Get the joint index for the vertex
														tempVert.originJointIndex = origins[originIndex];
														// Store the vertex's texture index
														tempVert.textureIndex = UINT_MAX;
														// Extract the coordinate scale factor
														tempVert.coordinatesScaleFactor = 1;
														// Extract the texture scale factor
														tempVert.textureScaleFactor = 1;
														// Extract the coordinates for the vertex for each dimension
														for (unsigned int l = 0; l < numDims; l++)
														{
															// Get dimension offset
															off = (l + 2) % numDims;
															// Extract the normal values
															tempVert.normal[l] = 1;
															// Get the vertex's relative coordinate
															tempVert.coordinates[l] = *(float *)&(raw[j + (k * 16) + (off * 4) + 32]);
														}
														// Extract the texture u and v values of the vertex
														for (unsigned int l = 0; l < 2; l++)
														{
															tempVert.textureMap[l] = 0;
														}
														// Check if we already have tempVert in verts
														if (find(verts[i].begin(), verts[i].end(), tempVert) == verts[i].end())
														{
															// This is a new vertex, add tempVert to verts
															verts[i].push_back(tempVert);
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
							// Create the vector for the mesh
							faces.push_back(vector<Face>());
							// Get the polygon collection length
							if (i < numPolygonCollections - 1)
							{
								polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
							}
							else
							{
								polygonCollectionLen = modelSectionLen - (polygonCollectionOffs[i] - mobjOff - tableOff);
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
										polygonLen = 80;
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
												// For each vertex in the polygon that is the start of a face
												for (unsigned int k = 0; (k + numVertsPerFace) <= numPolygonVerts; k++)
												{
													// Extract the origin index
													originIndex = *(unsigned int *)&(raw[j + (k * 16) + 44]);
													// Make sure that the origin index is valid
													if (originIndex < numOrigins)
													{
														// Get the joint index for the vertex
														tempVert.originJointIndex = origins[originIndex];
														// Store the vertex's texture index
														tempVert.textureIndex = UINT_MAX;
														// Extract the coordinate scale factor
														tempVert.coordinatesScaleFactor = 1;
														// Extract the texture scale factor
														tempVert.textureScaleFactor = 1;
														// Extract the coordinates for the vertex for each dimension
														for (unsigned int l = 0; l < numDims; l++)
														{
															// Get dimension offset
															off = (l + 2) % numDims;
															// Extract the normal values
															tempVert.normal[l] = 1;
															// Get the vertex's relative coordinate
															tempVert.coordinates[l] = *(float *)&(raw[j + (k * 16) + (off * 4) + 32]);
														}
														// Extract the texture u and vr values of the vertex
														for (unsigned int l = 0; l < 2; l++)
														{
															tempVert.textureMap[l] = 0;
														}
														// Get the index of this vertex in the verts vector
														unsigned int currentVertInd = (find(verts[i].begin(), verts[i].end(), tempVert) - verts[i].begin()), nextVertInd;
														// Make sure we got a valid index
														if (currentVertInd < verts[i].size())
														{
															// Save the currentVertInd in the tempFace
															tempFace.vertexIndices[0] = currentVertInd;
															// For each numAsStr that a vertex can have
															for (unsigned int l = 1; l <= numLinesPerVertex; l++)
															{
																// Check if we need to add the lth numAsStr segment
																if ((k + l) < numPolygonVerts)
																{
																	// Get Origin Index for vertex k + l
																	originIndex = *(unsigned int *)&(raw[j + ((k + l) * 16) + 44]);
																	// Make sure the origin index is valid
																	if (originIndex < numOrigins)
																	{
																		// Get the joint index for the vertex
																		tempVert.originJointIndex = origins[originIndex];
																		// Store the vertex's texture index
																		tempVert.textureIndex = UINT_MAX;
																		// Extract the coordinate scale factor
																		tempVert.coordinatesScaleFactor = 1;
																		// Extract the texture scale factor
																		tempVert.textureScaleFactor = 1;
																		// For each dimension
																		for (unsigned int m = 0; m < numDims; m++)
																		{
																			// Find the offset since it is {Y, Z, X} in the file, but we want {X, Y, Z}
																			off = ((m + 2) % numDims);
																			// Extract the normal values
																			tempVert.normal[m] = 1;
																			// Extract the vertex coordinate
																			tempVert.coordinates[m] = *(float *)&(raw[j + ((k + l) * 16) + (off * 4) + 32]);
																		}
																		// Extract the texture u and v values of the vertex
																		for (unsigned int m = 0; m < 2; m++)
																		{
																			tempVert.textureMap[m] = 0;
																		}
																		// Get the index of vertex k + l
																		nextVertInd = (find(verts[i].begin(), verts[i].end(), tempVert) - verts[i].begin());
																		// Make sure we got a valid index
																		if (nextVertInd < verts[i].size())
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
																faces[i].push_back(tempFace);
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
				// Free the polygon collection offsets
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

vector<JointRelative> getWpnJoints(string raw, unsigned int maxJoints)
{
	vector<JointRelative> joints;
	unsigned int numSections, menvOff, tableOff, numJoints = 0;
	// Determine the number of joints the wpn has
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
				if (raw.size() >= (tableOff + 4))
				{
					numJoints = *(unsigned int *)&(raw[tableOff]);
				}
			}
		}
	}
	// Construct the joints
	joints.reserve(numJoints);
	for (unsigned int i = 0; i < numJoints; i++)
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
	return joints;
}

bool getWpnVerticesAndFaces(string raw, vector<vector<VertexRelative>> &verts, vector<vector<Face>> &faces)
{
	// Declare variables
	const unsigned int numOrigins = 3;
	unsigned int numSections, menvOff, tableOff, modelSectionLen, numPolygonCollections, *polygonCollectionTextures, *polygonCollectionOffs, polygonCollectionLen, polygonEntryType, polygonLen, numPolygonVerts, originIndex, off, jointIndex, origins[numOrigins] = { 0, 0, 0 };
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
				tableOff = *(unsigned int *)&(raw[menvOff + 32]);
				// Determine the length of the 3D Model section
				modelSectionLen = *(unsigned int *)&(raw[menvOff + 36]);
				// Make sure the file contains a polygon collection table
				if (raw.size() >= menvOff + tableOff + modelTableHeaderLen)
				{
					// Get the number of polygon collections
					numPolygonCollections = *(unsigned int *)&(raw[menvOff + tableOff + 12]);
					// Reserve space for the meshes
					verts.reserve(numPolygonCollections);
					faces.reserve(numPolygonCollections);
					// Initialize the texture and offset arrays
					polygonCollectionTextures = new unsigned int[numPolygonCollections];
					polygonCollectionOffs = new unsigned int[numPolygonCollections];
					// Make sure the polygon collection table is the correct size
					if (raw.size() >= (menvOff + tableOff + ((numPolygonCollections + 1) * 16)))
					{
						// Store the offsets to each polygon collection
						for (unsigned int i = 0; i < numPolygonCollections; i++)
						{
							polygonCollectionTextures[i] = *(unsigned int *)&(raw[menvOff + tableOff + (i * 16) + 24]);
							polygonCollectionOffs[i] = menvOff + tableOff + *(unsigned int *)&(raw[menvOff + tableOff + (i * 16) + 28]);
						}
						// Make sure the file contains each polygon collection
						if (raw.size() >= polygonCollectionOffs[numPolygonCollections - 1] + 16)
						{
							// For each polygon collection
							for (unsigned int i = 0; i < numPolygonCollections; i++)
							{
								// Create the vector for the mesh
								verts.push_back(vector<VertexRelative>());
								// Get the polygon collection length
								if (i < numPolygonCollections - 1)
								{
									polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
								}
								else
								{
									polygonCollectionLen = tableOff + modelSectionLen - (polygonCollectionOffs[i] - menvOff);
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
											// Update the origin at originJointIndex
											origins[originIndex] = jointIndex;
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
															}
															// Check if we already have tempVert in verts
															if (find(verts[i].begin(), verts[i].end(), tempVert) == verts[i].end())
															{
																// This is a new vertex, add tempVert to verts
																verts[i].push_back(tempVert);
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
								// Create the vector for the mesh
								faces.push_back(vector<Face>());
								// Get the polygon collection length
								if (i < numPolygonCollections - 1)
								{
									polygonCollectionLen = polygonCollectionOffs[i + 1] - polygonCollectionOffs[i];
								}
								else
								{
									polygonCollectionLen = tableOff + modelSectionLen - (polygonCollectionOffs[i] - menvOff);
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
																// Extract the normal values
																tempVert.normal[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 32]);
																// Get the vertex's relative coordinate
																tempVert.coordinates[l] = *(float *)&(raw[j + (k * 48) + (off * 4) + (numWeirds * 16) + 48]);
															}
															// Extract the texture u and vr values of the vertex
															for (unsigned int l = 0; l < 2; l++)
															{
																tempVert.textureMap[l] = *(float *)&(raw[j + (k * 48) + (l * 4) + (numWeirds * 16) + 64]);
															}
															// Get the index of this vertex in the verts vector
															unsigned int currentVertInd = (find(verts[i].begin(), verts[i].end(), tempVert) - verts[i].begin()), nextVertInd;
															// Make sure we got a valid index
															if (currentVertInd < verts[i].size())
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
																			}
																			// Get the index of vertex k + l
																			nextVertInd = (find(verts[i].begin(), verts[i].end(), tempVert) - verts[i].begin());
																			// Make sure we got a valid index
																			if (nextVertInd < verts[i].size())
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
																	faces[i].push_back(tempFace);
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

void removeDuplicateFaces(vector<vector<Face>> &faces, bool ignoreOrientation)
{
	bool dupe, foundCopy;
	unsigned int numMeshes = faces.size(), numFaces;
	for (unsigned int i = 0; i < numMeshes; i++)
	{
		numFaces = faces[i].size();
		for (unsigned int j = 0; j < numFaces; j++)
		{
			for (unsigned int k = numFaces - 1; k > j; k--)
			{
				// Reset the duplicate flag
				dupe = true;
				// Determine if k is a duplicate of j
				for (unsigned int l = 0; l < numVertsPerFace; l++)
				{
					if (ignoreOrientation)
					{
						foundCopy = false;
						for (unsigned int m = (l + 1); m < numVertsPerFace; m++)
						{
							if (faces[i][j].vertexIndices[l] == faces[i][k].vertexIndices[m])
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
						if (faces[i][j].vertexIndices[l] != faces[i][k].vertexIndices[l])
						{
							dupe = false;
							break;
						}
					}
				}
				// Check if face k is a duplicate of j
				if (dupe)
				{
					// Remove face k
					faces.erase(faces.begin() + k);
					// Update the number of faces
					numFaces--;
				}
			}
		}
	}
	return;
}

void removeDuplicateVertices(vector<vector<VertexGlobal>> &verts, vector<vector<Face>> &faces)
{
	unsigned int numMeshes = faces.size(), numFaces, numVerts;
	for (unsigned int i = 0; i < numMeshes; i++)
	{
		numFaces = faces[i].size();
		numVerts = verts[i].size();
		for (unsigned int j = 0; j < numVerts; j++)
		{
			for (unsigned int k = numVerts - 1; k > j; k--)
			{
				// Determine if k is a duplicate of j
				if (verts[i][j] == verts[i][k])
				{
					// Check all of the faces to see if a face contains vertex k
					for (unsigned int l = 0; l < numFaces; l++)
					{
						for (unsigned int m = 0; m < numVertsPerFace; m++)
						{
							if (faces[i][l].vertexIndices[m] == k)
							{
								faces[i][l].vertexIndices[m] = j;
							}
						}
						verts.erase(verts.begin() + k);
						numVerts--;
					}
				}
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
				recursiveInd = parent.jointInfo.parentIndex;
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

bool getVerticesGlobal(vector<JointRelative> joints, vector<vector<VertexRelative>> vr, vector<vector<VertexGlobal>> &vg)
{
	bool valid = true;
	unsigned int numMeshes = vr.size(), numVerts;
	// Copy the relative vertices into vg
	vg = *(vector<vector<VertexGlobal>> *)&(vr);
	// Update the coordinates for each vertex in each mesh
	for (unsigned int i = 0; valid && i < numMeshes; i++)
	{
		// Get the number of vertices in mesh i
		numVerts = vr[i].size();
		for (unsigned int j = 0; j < numVerts; j++)
		{
			// Update the coordinates
			valid = getVertexGlobal(joints, vr[i][j], vg[i][j]);
		}
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

bool getVerticesRelative(vector<JointRelative> joints, vector<vector<VertexGlobal>> vg, vector<vector<VertexRelative>> &vr)
{
	bool valid = true;
	// Copy the global vertices into vr
	vr = *(vector<vector<VertexRelative>> *)&(vg);
	// Update the coordinates
	for (unsigned int i = 0; valid && i < vr.size(); i++)
	{
		for (unsigned int j = 0; valid && j < vr[i].size(); j++)
		{
			valid = getVertexRelative(joints, vg[i][j], vr[i][j]);
		}
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

bool getFaces(vector<vector<FaceEx>> facesEx, vector<vector<VertexRelative>> verts, vector<vector<Face>> &faces)
{
	bool valid = true;
	unsigned int numMeshes = facesEx.size(), numFaces;
	// Clear the return vector
	faces.clear();
	// Make sure the number of meshes matches between the verts and faces
	if (numMeshes == verts.size())
	{
		// Allocate space for the faces
		faces.reserve(numMeshes);
		// Get each face
		for (unsigned int i = 0; valid && (i < numMeshes); i++)
		{
			numFaces = facesEx[i].size();
			faces.push_back(vector<Face>());
			faces[i].reserve(numFaces);
			for (unsigned int j = 0; valid && (j < numFaces); j++)
			{
				Face f;
				valid = getFace(facesEx[i][j], verts[i], f);
				faces[i].push_back(f);
			}
		}
	}
	else
	{
		valid = false;
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

bool getFacesEx(vector<vector<Face>> faces, vector<vector<VertexRelative>> verts, vector<vector<FaceEx>> &facesEx)
{
	bool valid = true;
	unsigned int numMeshes = faces.size(), numFaces;
	facesEx.clear();
	// Make sure the number of meshes matches between the verts and faces
	if (numMeshes == verts.size())
	{
		facesEx.reserve(numMeshes);
		for (unsigned int i = 0; valid && (i < numMeshes); i++)
		{
			numFaces = faces[i].size();
			facesEx.push_back(vector<FaceEx>());
			facesEx[i].reserve(numFaces);
			for (unsigned int j = 0; valid && (j < numFaces); j++)
			{
				FaceEx fe;
				valid = getFaceEx(faces[i][j], verts[i], fe);
				facesEx[i].push_back(fe);
			}
		}
	}
	else
	{
		valid = false;
	}
	return valid;
}

bool combineMeshes(vector<vector<VertexRelative>> &verts, vector<vector<Face>> &faces)
{
	bool valid = true;
	vector<vector<VertexRelative>> combinedVerts;
	vector<VertexRelative> combinedVert;
	vector<vector<Face>> combinedMeshes;
	vector<Face> combinedMesh;
	unsigned int numMeshes = faces.size(), numFaces, totalNumFaces = 0, numVerts, totalNumVerts = 0;
	// Make sure the number of meshes matches between the two vectors
	if (numMeshes == verts.size())
	{
		// Reserve space in the combined vectors
		combinedVerts.reserve(1);
		combinedMeshes.reserve(1);
		// For each mesh
		for (unsigned int i = 0; i < numMeshes; i++)
		{
			// Get the number of vertices in the mesh
			numVerts = verts[i].size();
			// Update the total number of verts
			totalNumVerts += numVerts;
			// Get the number of faces in the mesh
			numFaces = faces[i].size();
			// Update the total number of faces
			totalNumFaces += numFaces;
			// Reserve space in the combined vectors
			combinedVert.reserve(totalNumVerts);
			combinedMesh.reserve(totalNumFaces);
			// Push each vertex into the combined vertices vector
			for (unsigned int j = 0; j < numVerts; j++)
			{
				combinedVert.push_back(verts[i][j]);
			}
			// Push each face into the combined mesh vector
			for (unsigned int j = 0; j < numFaces; j++)
			{
				combinedMesh.push_back(faces[i][j]);
				// Update the vertex indices
				for (unsigned int k = 0; k < numDims; k++)
				{
					combinedMesh.back().vertexIndices[k] += (totalNumVerts - numVerts);
				}
			}
		}
		// Store the combined mesh vectors
		combinedVerts.push_back(combinedVert);
		combinedMeshes.push_back(combinedMesh);
		// Overwrite the outputs
		verts = combinedVerts;
		faces = combinedMeshes;
	}
	else
	{
		// The number of meshes needs to be consistent
		valid = false;
	}
	return valid;
}

// This function doesn't care about coordinates
bool combineMeshes(vector<vector<VertexGlobal>> &verts, vector<vector<Face>> &faces)
{
	return combineMeshes(*(vector<vector<VertexRelative>> *)&(verts), faces);
}

bool swapChirality(vector<JointRelative> &joints, vector<vector<VertexGlobal>> &verts)
{
	bool valid = true;
	unsigned int numJoints = joints.size(), numMeshes = verts.size(), numVerts;
	if (numJoints > 0)
	{
		float tempCoord = joints[0].coordinates[1];
		// Set the rotation amount to -pi/2
		float theta = (float)-M_PI_2;
		// Get the rotation matrix for around the x axis and initalize the pose
		aiMatrix4x4 combinedRotMat, chiralRotMat, jointRotMatX, jointRotMatY, jointRotMatZ;
		aiVector3D pose, scale, rotate, translate;
		// Construct the new rotation matrices
		aiMatrix4x4::RotationX(theta, chiralRotMat);
		aiMatrix4x4::RotationX(joints[0].rotations[0], jointRotMatX);
		aiMatrix4x4::RotationY(joints[0].rotations[1], jointRotMatY);
		aiMatrix4x4::RotationZ(joints[0].rotations[2], jointRotMatZ);
		// Combine the rotations
		combinedRotMat = chiralRotMat * jointRotMatX * jointRotMatZ * jointRotMatY;
		// Decompose the combined rotation matrix
		mdlsDecompose(combinedRotMat, scale, rotate, translate);
		// Modify the root joint to swap its chirality
		joints[0].coordinates[1] = joints[0].coordinates[2];
		joints[0].coordinates[2] = tempCoord;
		for (unsigned int i = 0; i < numDims; i++)
		{
			joints[0].rotations[i] = rotate[i];
		}
		// Rotate the global vertices to match the rotated joints
		for (unsigned int i = 0; i < numMeshes; i++)
		{
			// Get the number of vertices in the mesh
			numVerts = verts[i].size();
			for (unsigned int j = 0; j < numVerts; j++)
			{
				// Get the vertex's coordiantes
				for (unsigned int k = 0; k < numDims; k++)
				{
					pose[k] = verts[i][j].coordinates[k];
				}
				// Rotate them
				pose = chiralRotMat * pose;
				// Store the rotated coordinates
				for (unsigned int k = 0; k < numDims; k++)
				{
					verts[i][j].coordinates[k] = pose[k];
				}
			}
		}
	}
	else
	{
		valid = false;
	}
	return valid;
}

bool swapChirality(vector<JointRelative> &joints, vector<vector<VertexRelative>> &verts)
{
	bool valid = true;
	unsigned int numJoints = joints.size(), numMeshes = verts.size();
	if (numJoints > 0)
	{
		float tempCoord = joints[0].coordinates[1];
		// Set the rotation amount to -pi/2
		float theta = (float)-M_PI_2;
		// Get the rotation matrix for around the x axis and initalize the pose
		aiMatrix4x4 combinedRotMat, chiralRotMat, jointRotMatX, jointRotMatY, jointRotMatZ;
		aiVector3D scale, rotate, translate;
		// Construct the new rotation matrices
		aiMatrix4x4::RotationX(theta, chiralRotMat);
		aiMatrix4x4::RotationX(joints[0].rotations[0], jointRotMatX);
		aiMatrix4x4::RotationY(joints[0].rotations[1], jointRotMatY);
		aiMatrix4x4::RotationZ(joints[0].rotations[2], jointRotMatZ);
		// Combine the rotations
		combinedRotMat = chiralRotMat * jointRotMatX * jointRotMatZ * jointRotMatY;
		// Decompose the combined rotation matrix
		mdlsDecompose(combinedRotMat, scale, rotate, translate);
		// Modify the root joint to swap its chirality
		joints[0].coordinates[1] = joints[0].coordinates[2];
		joints[0].coordinates[2] = tempCoord;
		for (unsigned int i = 0; i < numDims; i++)
		{
			joints[0].rotations[i] = rotate[i];
		}
	}
	else
	{
		valid = false;
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

bool getPolygonCollections(vector<vector<VertexRelative>> verts, vector<vector<Face>> faces, vector<PolygonCollection> &polygonCollections)
{
	bool valid = true, canConnectTL = true, canConnectTLR = true, canConnectPSC = true, foundVertex, foundVertexR, vertStatus[numDims], vertStatusR[numDims], swap;
	const unsigned int lengthThreshold = 4032, numOrigins = 3;
	unsigned int numTextures = getNumTextures(verts), currentTextureIndex = numTextures, subCollectionLen, triangleListLen, triangleListRLen, faceLen, originLen, numFilledOrigins, origins[numOrigins];
	PolygonCollection polygonCollection;
	PolygonSubCollection polygonSubCollection;
	TriangleList triangleList, triangleListR;
	// Clear the return value
	polygonCollections.clear();
	// Set the orientation for the triangle lists
	triangleList.orientation = COUNTERCLOCKWISE;
	triangleListR.orientation = CLOCKWISE;
	// For each mesh
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		// Get the number of verts in mesh i
		unsigned int numVerts = verts[i].size();
		// Reset the sub-collection length and the origins
		subCollectionLen = 0;
		originLen = 0;
		numFilledOrigins = 0;
		swap = false;
		for (unsigned int j = 0; j < numOrigins; j++)
		{
			origins[j] = 0;
		}
		// For each face in the mesh
		for (unsigned int j = 0; j < faces[i].size(); j++)
		{
			// Get Face i, j
			Face f = faces[i][j], fr = f, fp, fpr;
			// Check which faces need to have their orientation flipped
			if (swap && f.orientation == COUNTERCLOCKWISE || !swap && f.orientation == CLOCKWISE)
			{
				unsigned int ind = f.vertexIndices[0];
				f.vertexIndices[0] = f.vertexIndices[1];
				f.vertexIndices[1] = ind;
			}
			if (swap && fr.orientation == CLOCKWISE || !swap && fr.orientation == COUNTERCLOCKWISE)
			{
				unsigned int ind = fr.vertexIndices[0];
				fr.vertexIndices[0] = fr.vertexIndices[1];
				fr.vertexIndices[1] = ind;
			}
			// Reset the can connect flag
			if (!canConnectTL && !canConnectTLR)
			{
				canConnectTL = true;
				canConnectTLR = true;
			}
			// Make sure the vertex is valid
			if (f.vertexIndices[0] < numVerts && f.vertexIndices[1] < numVerts && f.vertexIndices[2] < numVerts)
			{
				// Get the index of the first vertex of the face
				unsigned int vertIndex = f.vertexIndices[0];
				// Check if we need to separate this mesh because it uses multiple textures
				if ((verts[i][vertIndex].textureIndex != currentTextureIndex))
				{
					// Check if we need to add the triangle list to the polygon sub-collection
					if (triangleList.vertexIndices.size() > 0 || triangleListR.vertexIndices.size() > 0)
					{
						// Add the finished triangle list to the polygon sub-collection
						if (triangleList.vertexIndices.size() >= triangleListR.vertexIndices.size())
						{
							polygonSubCollection.triangleLists.push_back(triangleList);
						}
						else
						{
							polygonSubCollection.triangleLists.push_back(triangleListR);
						}
						// Clear the triangle list to be used for the new polygon sub-collection
						triangleList.vertexIndices.clear();
						triangleListR.vertexIndices.clear();
						swap = false;
					}
					// Check if we need to add the polygon sub-collection to the polygon collection
					if (polygonSubCollection.triangleLists.size() > 0)
					{
						// Add the finished polygon sub-collection the the polygon collection
						polygonCollection.subcollections.push_back(polygonSubCollection);
						// Clear the polygon sub-collection for the new polygon list
						polygonSubCollection.triangleLists.clear();
						// Reset the sub-collection length
						subCollectionLen = 0;
						canConnectPSC = true;
					}
					// Check if we need to add the polygon collection to the polygon collections vector
					if (polygonCollection.subcollections.size() > 0)
					{
						// Add the finished polygon collection to the output vector
						polygonCollections.push_back(polygonCollection);
						// Clear the polygon collection to be used for the new polygon collection
						polygonCollection.subcollections.clear();
					}
					// Update the current texture index
					currentTextureIndex = verts[i][vertIndex].textureIndex;
				}
				// If this isn't the first face, check if we can connect it to the existing triangle list
				if (triangleList.vertexIndices.size() >= 3)
				{
					// Get the previous faces
					unsigned int faceOff = 0;
					for (unsigned int k = 0; k < polygonCollection.subcollections.size(); k++)
					{
						for (unsigned int l = 0; l < polygonCollection.subcollections[k].triangleLists.size(); l++)
						{
							faceOff += polygonCollection.subcollections[k].triangleLists[l].vertexIndices.size() - 2;
						}
					}
					for (unsigned int k = 0; k < polygonSubCollection.triangleLists.size(); k++)
					{
						faceOff += polygonSubCollection.triangleLists[k].vertexIndices.size() - 2;
					}
					for (unsigned int k = 0; k < numVertsPerFace; k++)
					{
						if ((j - faceOff + k - 1) < triangleList.vertexIndices.size())
						{
							fp.vertexIndices[k] = triangleList.vertexIndices[j - faceOff + k - 1];
						}
						if ((j - faceOff + k - 1) < triangleListR.vertexIndices.size())
						{
							fpr.vertexIndices[k] = triangleListR.vertexIndices[j - faceOff + k - 1];
						}
					}
					fp.orientation = faces[i][j - 1].orientation;
					fpr.orientation = fp.orientation;
					if (fp.vertexIndices[0] < numVerts && fp.vertexIndices[1] < numVerts && fp.vertexIndices[2] < numVerts)
					{							
						// For each vertex in the face...
						for (unsigned int k = 0; k < numVertsPerFace; k++)
						{
							// Reset the found vertex flags
							foundVertex = false;
							foundVertexR = false;
							// Get the index of vertex k
							vertIndex = f.vertexIndices[k];
							// For each vertex that makes up the previous face...
							for (unsigned int l = 0; !foundVertex && (l < numDims); l++)
							{
								// Check if it matches a vertex from the previous face
								if (vertIndex == fp.vertexIndices[l])
								{
									// It does, set the found vertex flag
									foundVertex = true;
								}
							}
							vertIndex = fr.vertexIndices[k];
							for (unsigned int l = 0; !foundVertexR && (l < numDims); l++)
							{
								// Check if it matches a vertex from the previous face
								if (vertIndex == fpr.vertexIndices[l])
								{
									// It does, set the found vertex flag
									foundVertexR = true;
								}
							}
							// Save the status of the vertex
							vertStatus[k] = foundVertex;
							vertStatusR[k] = foundVertexR;
							// Check if we found the vertex in the previous face
							if (!foundVertex)
							{
								// We didn't, so now check if this was the only different vertex
								for (unsigned int l = 0; l < k && canConnectTL; l++)
								{
									canConnectTL = canConnectTL && vertStatus[l];
								}
							}
							if (!foundVertexR)
							{
								// We didn't, so now check if this was the only different vertex
								for (unsigned int l = 0; l < k && canConnectTLR; l++)
								{
									canConnectTLR = canConnectTLR && vertStatusR[l];
								}
							}
						}
					}
					else
					{
						cout << "Error: Invalid Vertex Index referenced by Mesh " << i << " Face " << j - 1 << endl;
						valid = false;
					}
				}
				// Make sure the face was valid
				if (valid)
				{
					// Check if this is the first face
					if (triangleList.vertexIndices.size() < 3 && triangleListR.vertexIndices.size() < 3)
					{
						// Add all 3 vertices to the triangle list
						for (unsigned int k = 0; k < numVertsPerFace; k++)
						{
							// Get the index of vertex k
							vertIndex = f.vertexIndices[k];
							// This is the first face, so all we have to do is add the first 3 verts (plus offset)
							triangleList.vertexIndices.push_back(vertIndex);
							vertIndex = fr.vertexIndices[k];
							triangleListR.vertexIndices.push_back(vertIndex);
							// Check the origins
							for (unsigned int l = 0; l < numOrigins; l++)
							{
								if (l >= numFilledOrigins)
								{
									origins[l] = verts[i][vertIndex].originJointIndex;
									numFilledOrigins++;
									originLen += 128;
									break;
								}
								else if (origins[l] == verts[i][vertIndex].originJointIndex)
								{
									break;
								}
								else if (l == (numOrigins - 1))
								{
									// A new origin update is required, so find the unused origin
									for (unsigned int m = 0; m < numOrigins; m++)
									{
										if (origins[m] != verts[i][f.vertexIndices[0]].originJointIndex && origins[m] != verts[i][f.vertexIndices[1]].originJointIndex)
										{
											origins[m] = verts[i][vertIndex].originJointIndex;
										}
									}
									// Update the polygon subcollection length
									originLen += 128;
								}
							}
						}
						// The next face will be clockwise
						swap = true;
					}
					else
					{
						// Check if this face can connect to the previous face
						if (canConnectTL)
						{
							for (unsigned int k = 0; k < numVertsPerFace; k++)
							{
								for (unsigned int l = 0; l < numVertsPerFace; l++)
								{
									if (f.vertexIndices[k] == fp.vertexIndices[l] && f.vertexIndices[(k + 1) % numVertsPerFace] == fp.vertexIndices[(l + 1) % numVertsPerFace])
									{
										// Check if we need to reorder the vertices of the first face
										if (triangleList.vertexIndices.size() == 3)
										{
											bool tmpStatus = vertStatus[0];
											unsigned int tmpInd = f.vertexIndices[0];
											// Reorder the vertices
											vertStatus[0] = vertStatus[k];
											f.vertexIndices[0] = f.vertexIndices[k];
											if (k == 1)
											{
												vertStatus[1] = vertStatus[2];
												vertStatus[2] = tmpStatus;
												f.vertexIndices[1] = f.vertexIndices[2];
												f.vertexIndices[2] = tmpInd;
											}
											else if (k == 2)
											{
												vertStatus[2] = vertStatus[1];
												vertStatus[1] = tmpStatus;
												f.vertexIndices[2] = f.vertexIndices[1];
												f.vertexIndices[1] = tmpInd;
											}
											triangleList.vertexIndices[0] = fp.vertexIndices[(l + 2) % numVertsPerFace];
											triangleList.vertexIndices[1] = fp.vertexIndices[l];
											triangleList.vertexIndices[2] = fp.vertexIndices[(l + 1) % numVertsPerFace];
										}
										else if (l != 1)
										{
											canConnectTL = false;
										}
										k = numVertsPerFace - 1;
										break;
									}
									else if (f.vertexIndices[k] == fp.vertexIndices[l] && f.vertexIndices[(k + 2) % numVertsPerFace] == fp.vertexIndices[(l + 1) % numVertsPerFace])
									{
										canConnectTL = false;
										k = numVertsPerFace - 1;
										break;
									}
									else if (k == (numVertsPerFace - 1) && l == (numVertsPerFace - 1))
									{
										// Shouldn't be able to get here
										canConnectTL = false;
									}
								}
							}
						}
						// Check if this face can connect to the previous face
						if (canConnectTLR)
						{
							for (unsigned int k = 0; k < numVertsPerFace; k++)
							{
								for (unsigned int l = 0; l < numVertsPerFace; l++)
								{
									if (fr.vertexIndices[k] == fpr.vertexIndices[l] && fr.vertexIndices[(k + 1) % numVertsPerFace] == fpr.vertexIndices[(l + 1) % numVertsPerFace])
									{
										// Check if we need to reorder the vertices of the first face
										if (triangleListR.vertexIndices.size() == 3)
										{
											bool tmpStatus = vertStatus[0];
											unsigned int tmpInd = fr.vertexIndices[0];
											// Reorder the vertices
											vertStatusR[0] = vertStatusR[k];
											fr.vertexIndices[0] = fr.vertexIndices[k];
											if (k == 1)
											{
												vertStatusR[1] = vertStatusR[2];
												vertStatusR[2] = tmpStatus;
												fr.vertexIndices[1] = fr.vertexIndices[2];
												fr.vertexIndices[2] = tmpInd;
											}
											else if (k == 2)
											{
												vertStatusR[2] = vertStatusR[1];
												vertStatusR[1] = tmpStatus;
												fr.vertexIndices[2] = fr.vertexIndices[1];
												fr.vertexIndices[1] = tmpInd;
											}
											triangleListR.vertexIndices[0] = fpr.vertexIndices[(l + 2) % numVertsPerFace];
											triangleListR.vertexIndices[1] = fpr.vertexIndices[l];
											triangleListR.vertexIndices[2] = fpr.vertexIndices[(l + 1) % numVertsPerFace];
										}
										else if (l != 1)
										{
											canConnectTLR = false;
										}
										k = numVertsPerFace - 1;
										break;
									}
									else if (fr.vertexIndices[k] == fpr.vertexIndices[l] && fr.vertexIndices[(k + 2) % numVertsPerFace] == fpr.vertexIndices[(l + 1) % numVertsPerFace])
									{
										canConnectTLR = false;
										k = numVertsPerFace - 1;
										break;
									}
									else if (k == (numVertsPerFace - 1) && l == (numVertsPerFace - 1))
									{
										// Shouldn't be able to get here
										canConnectTLR = false;
									}
								}
							}
						}
						// Initialize length variables
						faceLen = 48;
						// Check each vertex
						for (unsigned int k = 0; k < numVertsPerFace; k++)
						{
							// Check if it is a new vertex from the previous face
							if (!vertStatus[k])
							{
								// Check each origin
								for (unsigned int l = 0; l < numOrigins; l++)
								{
									if (l >= numFilledOrigins)
									{
										originLen += 128;
										origins[l] = verts[i][f.vertexIndices[k]].originJointIndex;
										numFilledOrigins++;
										break;
									}
									else if (origins[l] == verts[i][f.vertexIndices[k]].originJointIndex)
									{
										break;
									}
									else if (l == (numOrigins - 1))
									{
										for (unsigned int m = 0; m < k; m++)
										{
											if (verts[i][f.vertexIndices[k]].originJointIndex == verts[i][f.vertexIndices[m]].originJointIndex)
											{
												continue;
											}
											else if (m == (k - 1))
											{
												originLen += 128;
											}
										}
										faceLen = 0;
										canConnectTL = false;
										canConnectTLR = false;
									}
								}
							}
						}
						triangleListLen = triangleList.vertexIndices.size() * 48 + 32;
						triangleListRLen = triangleListR.vertexIndices.size() * 48 + 32;
						// Use the estimated sub-collection size to see if we should start a new sub-collection
						if ((subCollectionLen + triangleListLen + faceLen + originLen) > lengthThreshold)
						{
							// Adding this face to the current triangle list could push it over the size limit
							canConnectPSC = false;
						}
						if ((subCollectionLen + triangleListRLen + faceLen + originLen) > lengthThreshold)
						{
							canConnectPSC = false;
						}
						// Check if the face is valid to connect to the triangle list
						if (canConnectTL || canConnectTLR)
						{
							if (canConnectTL)
							{
								// Check the status of the vertices to find the new vertex
								for (unsigned int k = 0; k < numDims; k++)
								{
									if (!vertStatus[k])
									{
										// Add the new vertex to the triangle list
										triangleList.vertexIndices.push_back(f.vertexIndices[k]);
										break;
									}
									// In case you have a duplicate face to orient the triangle list in a specific way
									if (k == 2)
									{
										// Add the last vertex from the face
										triangleList.vertexIndices.push_back(f.vertexIndices[2]);
									}
								}
							}
							if (canConnectTLR)
							{
								// Check the status of the vertices to find the new vertex
								for (unsigned int k = 0; k < numDims; k++)
								{
									if (!vertStatusR[k])
									{
										// Add the new vertex to the triangle list
										triangleListR.vertexIndices.push_back(fr.vertexIndices[k]);
										break;
									}
									// In case you have a duplicate face to orient the triangle list in a specific way
									if (k == 2)
									{
										// Add the last vertex from the face
										triangleListR.vertexIndices.push_back(fr.vertexIndices[2]);
									}
								}
							}
							// The next face should be the opposite orientation
							swap = !swap;
						}
						else
						{
							// Check if we should transition to a new polygon sub-collection
							if (polygonSubCollection.triangleLists.size() > 0 && !canConnectPSC)
							{
								// Add the polygon sub-collection to the polygon collection
								polygonCollection.subcollections.push_back(polygonSubCollection);
								// Clear the polygon sub-collection to use for the next sub-collection
								polygonSubCollection.triangleLists.clear();
								// Reset the sub-collection length
								subCollectionLen = 0;
								canConnectPSC = true;
							}
							if (triangleList.vertexIndices.size() > 0 || triangleListR.vertexIndices.size() > 0)
							{
								if (triangleList.vertexIndices.size() >= triangleListR.vertexIndices.size())
								{
									// Update the sub-collection length
									subCollectionLen += triangleListLen + originLen;
									// Reset the origin length now that it is included in the subcollection length
									originLen = 0;
									// Add the triangle list to the polygon sub-collection
									polygonSubCollection.triangleLists.push_back(triangleList);
								}
								else
								{
									// Update the sub-collection length
									subCollectionLen += triangleListRLen + originLen;
									// Reset the origin length now that it is incldued in the subcollection length
									originLen = 0;
									// Add the triangle list to the polygon sub-collection
									polygonSubCollection.triangleLists.push_back(triangleListR);
								}
								// Clear the triangle list to use for the new list
								triangleList.vertexIndices.clear();
								triangleListR.vertexIndices.clear();
							}
							// Check which faces need to have their orientation flipped
							if (swap && f.orientation == COUNTERCLOCKWISE || !swap && f.orientation == CLOCKWISE)
							{
								unsigned int ind = f.vertexIndices[0];
								f.vertexIndices[0] = f.vertexIndices[1];
								f.vertexIndices[1] = ind;
							}
							if (swap && fr.orientation == COUNTERCLOCKWISE || !swap && fr.orientation == CLOCKWISE)
							{
								unsigned int ind = fr.vertexIndices[0];
								fr.vertexIndices[0] = fr.vertexIndices[1];
								fr.vertexIndices[1] = ind;
							}
							// Add all of the vertices from the current face to the new triangle list
							for (unsigned int k = 0; k < numDims; k++)
							{
								triangleList.vertexIndices.push_back(f.vertexIndices[k]);
								triangleListR.vertexIndices.push_back(fr.vertexIndices[k]);
								for (unsigned int l = 0; l < numOrigins; l++)
								{
									if (l >= numFilledOrigins)
									{
										origins[l] = verts[i][faces[i][j].vertexIndices[k]].originJointIndex;
										numFilledOrigins++;
										originLen += 128;
										break;
									}
									else if (origins[l] == verts[i][faces[i][j].vertexIndices[k]].originJointIndex)
									{
										break;
									}
									else if (l == (numOrigins - 1))
									{
										// Reset the origins and try again
										k = 0;
										numFilledOrigins = 1;
										origins[0] = verts[i][faces[i][j].vertexIndices[0]].originJointIndex;
										originLen = 128;
										triangleList.vertexIndices.clear();
										triangleList.vertexIndices.push_back(f.vertexIndices[0]);
										triangleListR.vertexIndices.clear();
										triangleListR.vertexIndices.push_back(fr.vertexIndices[0]);
									}
								}
							}
							// The next face will be clockwise
							swap = true;
						}
					}
				}
			}
			else
			{
				cout << "Error: Invalid Vertex Index referenced by Mesh " << i << ", Face " << j << endl;
				valid = false;
			}
		}
		// Check if the last triangle list can connect to the polygon sub-collection
		if (polygonSubCollection.triangleLists.size() > 0 && !canConnectPSC)
		{
			polygonCollection.subcollections.push_back(polygonSubCollection);
			polygonSubCollection.triangleLists.clear();
		}
		// Check if we need to add the last triangle list to the current polygon sub-collection
		if (triangleList.vertexIndices.size() > 0 || triangleListR.vertexIndices.size() > 0)
		{
			if (triangleList.vertexIndices.size() >= triangleListR.vertexIndices.size())
			{
				polygonSubCollection.triangleLists.push_back(triangleList);
			}
			else
			{
				polygonSubCollection.triangleLists.push_back(triangleListR);
			}
			triangleList.vertexIndices.clear();
			triangleListR.vertexIndices.clear();
		}
		// Check if we need to add the last polygon sub-collection to the current polygon collection
		if (polygonSubCollection.triangleLists.size() > 0)
		{
			polygonCollection.subcollections.push_back(polygonSubCollection);
			polygonSubCollection.triangleLists.clear();
		}
		// Check if we need to add the last polygon collection to the output
		if (polygonCollection.subcollections.size() > 0)
		{
			polygonCollections.push_back(polygonCollection);
			polygonCollection.subcollections.clear();
		}
	}
	// Condense the polygon collections
	//polygonCollections = condensePolygonCollections(verts, polygonCollections);
	return valid;
}

vector<PolygonCollection> condensePolygonCollections(vector<vector<VertexRelative>> verts, vector<PolygonCollection> polygonCollections)
{
	vector<PolygonCollection> condensed = polygonCollections;
	unsigned int numMeshes = polygonCollections.size();
	// For each mesh
	for (unsigned int meshInd = 0; meshInd < numMeshes; meshInd++)
	{
		PolygonCollection pc = polygonCollections[meshInd];
		vector<TriangleList> triangleLists;
		// Store all of the polygon collection's triangle lists
		for (unsigned int i = 0; i < pc.subcollections.size(); i++)
		{
			PolygonSubCollection psc = pc.subcollections[i];
			for (unsigned int j = 0; j < psc.triangleLists.size(); j++)
			{
				triangleLists.push_back(psc.triangleLists[j]);
			}
		}
		// Check if we can combine any of the triangle lists
		for (unsigned int i = (triangleLists.size() - 2); i < triangleLists.size(); i--)
		{
			TriangleList ti = triangleLists[i];
			if (i == 2)
			{
				cout << "Target" << endl;
			}
			if (ti.vertexIndices.size() > 0)
			{
				for (unsigned int j = (triangleLists.size() - 1); j > i; j--)
				{
					TriangleList tj = triangleLists[j];
					if (tj.vertexIndices.size() > 0)
					{
						const int numOrigins = 3;
						unsigned int numFilledOrigins = 0, origins[numOrigins];
						// Determine the number of origins that will be used if we combine the triangle lists
						for (unsigned int k = 0; k < ti.vertexIndices.size(); k++)
						{
							for (unsigned int l = 0; l < numOrigins; l++)
							{
								if (l == numFilledOrigins)
								{
									if (numFilledOrigins < numOrigins)
									{
										origins[numFilledOrigins] = verts[meshInd][ti.vertexIndices[k]].originJointIndex;
									}
									numFilledOrigins++;
									break;
								}
								else if (origins[l] == verts[meshInd][ti.vertexIndices[k]].originJointIndex)
								{
									break;
								}
							}
						}
						for (unsigned int k = 0; k < tj.vertexIndices.size(); k++)
						{
							for (unsigned int l = 0; l < numOrigins; l++)
							{
								if (l == numFilledOrigins)
								{
									if (numFilledOrigins < numOrigins)
									{
										origins[numFilledOrigins] = verts[meshInd][tj.vertexIndices[k]].originJointIndex;
									}
									numFilledOrigins++;
									break;
								}
								else if (origins[l] == verts[meshInd][tj.vertexIndices[k]].originJointIndex)
								{
									break;
								}
							}
						}
						// Make sure the number of origins is valid before combining
						if (numFilledOrigins <= numOrigins)
						{
							TriangleList comb;
							// Check if we can play with the vertex order
							if (ti.vertexIndices.size() == 3 || tj.vertexIndices.size() == 3)
							{
								TriangleList tb, tr;
								Face fbo, fbn, fro, frn;
								if (ti.vertexIndices.size() == 3)
								{
									tb = tj;
									tr = ti;
									fbo.orientation = tj.orientation;
									fbn.orientation = (tj.orientation + (tj.vertexIndices.size() - 2) % 2) % 2;
									fro.orientation = ti.orientation;
									for (unsigned int k = 0; k < numVertsPerFace; k++)
									{
										fbo.vertexIndices[k] = tj.vertexIndices[k];
										fbn.vertexIndices[k] = tj.vertexIndices[tj.vertexIndices.size() - numVertsPerFace + k];
										fro.vertexIndices[k] = ti.vertexIndices[k];
									}
								}
								else
								{
									tb = ti;
									tr = tj;
									fbo.orientation = ti.orientation;
									fbn.orientation = (ti.orientation + (ti.vertexIndices.size() - 2) % 2) % 2;
									fro.orientation = tj.orientation;
									for (unsigned int k = 0; k < numVertsPerFace; k++)
									{
										fbo.vertexIndices[k] = ti.vertexIndices[k];
										fbn.vertexIndices[k] = ti.vertexIndices[ti.vertexIndices.size() - numVertsPerFace + k];
										fro.vertexIndices[k] = tj.vertexIndices[k];
									}
								}
								frn = fro;
								if (frn.orientation == fbo.orientation)
								{
									unsigned int tmp = frn.vertexIndices[0];
									frn.vertexIndices[0] = frn.vertexIndices[1];
									frn.vertexIndices[1] = tmp;
									frn.orientation = !frn.orientation;
								}
								if (fro.orientation == fbn.orientation)
								{
									unsigned int tmp = fro.vertexIndices[0];
									fro.vertexIndices[0] = fro.vertexIndices[1];
									fro.vertexIndices[1] = tmp;
									fro.orientation = !fro.orientation;
								}
								for (unsigned int k = 0; k < numVertsPerFace; k++)
								{
									for (unsigned int l = 0; l < numVertsPerFace; l++)
									{
										if (frn.vertexIndices[k] == fbo.vertexIndices[l] && frn.vertexIndices[(k + 1) % numVertsPerFace] == fbo.vertexIndices[(l + 1) % numVertsPerFace])
										{
											unsigned int tmp = frn.vertexIndices[0];
											switch (k)
											{
											case 0:
												frn.vertexIndices[0] = frn.vertexIndices[2];
												frn.vertexIndices[2] = frn.vertexIndices[1];
												frn.vertexIndices[1] = tmp;
												break;
											case 2:
												frn.vertexIndices[0] = frn.vertexIndices[1];
												frn.vertexIndices[1] = frn.vertexIndices[2];
												frn.vertexIndices[2] = tmp;
												break;
											}
											tmp = fbo.vertexIndices[0];
											switch (l)
											{
											case 1:
												fbo.vertexIndices[0] = fbo.vertexIndices[1];
												fbo.vertexIndices[1] = fbo.vertexIndices[2];
												fbo.vertexIndices[2] = tmp;
												break;
											case 2:
												fbo.vertexIndices[0] = fbo.vertexIndices[2];
												fbo.vertexIndices[2] = fbo.vertexIndices[1];
												fbo.vertexIndices[1] = tmp;
												break;
											}
											if (tb.vertexIndices.size() == 3 || l == 1)
											{
												comb.orientation = frn.orientation;
												comb.vertexIndices.push_back(frn.vertexIndices[0]);
												if (l == 0)
												{
													for (unsigned int m = 0; m < tb.vertexIndices.size(); m++)
													{
														comb.vertexIndices.push_back(tb.vertexIndices[m]);
													}
												}
												else
												{
													for (unsigned int m = 0; m < numVertsPerFace; m++)
													{
														comb.vertexIndices.push_back(fbo.vertexIndices[m]);
													}
												}
												k = numVertsPerFace;
												break;
											}
										}
										else if (fro.vertexIndices[k] == fbn.vertexIndices[l] && fro.vertexIndices[(k + 1) % numVertsPerFace] == fbn.vertexIndices[(l + 1) % numVertsPerFace])
										{
											unsigned int tmp = fro.vertexIndices[0];
											switch (k)
											{
											case 1:
												fro.vertexIndices[0] = fro.vertexIndices[1];
												fro.vertexIndices[1] = fro.vertexIndices[2];
												fro.vertexIndices[2] = tmp;
												break;
											case 2:
												fro.vertexIndices[0] = fro.vertexIndices[2];
												fro.vertexIndices[2] = fro.vertexIndices[1];
												fro.vertexIndices[1] = tmp;
												break;
											}
											tmp = fbn.vertexIndices[0];
											switch (l)
											{
											case 0:
												fbn.vertexIndices[0] = fbn.vertexIndices[2];
												fbn.vertexIndices[2] = fbn.vertexIndices[1];
												fbn.vertexIndices[1] = tmp;
												break;
											case 2:
												fbn.vertexIndices[0] = fbn.vertexIndices[1];
												fbn.vertexIndices[1] = fbn.vertexIndices[2];
												fbn.vertexIndices[2] = tmp;
												break;
											}
											if (tb.vertexIndices.size() == 3 || l == 1)
											{
												comb.orientation = tb.orientation;
												if (l == 1)
												{
													for (unsigned int m = 0; m < tb.vertexIndices.size(); m++)
													{
														comb.vertexIndices.push_back(tb.vertexIndices[m]);
													}
												}
												else
												{
													for (unsigned int m = 0; m < numVertsPerFace; m++)
													{
														comb.vertexIndices.push_back(fbn.vertexIndices[m]);
													}
												}
												comb.vertexIndices.push_back(fro.vertexIndices[2]);
												k = numVertsPerFace;
												break;
											}
										}
									}
								}
							}
							else
							{
								Face fio, fin, fjo, fjn;
								fio.orientation = ti.orientation;
								fjo.orientation = tj.orientation;
								fin.orientation = (ti.orientation + (ti.vertexIndices.size() - 2) % 2) % 2;
								fjn.orientation = (tj.orientation + (tj.vertexIndices.size() - 2) % 2) % 2;
								for (unsigned int k = 0; k < numVertsPerFace; k++)
								{
									fio.vertexIndices[k] = ti.vertexIndices[k];
									fjo.vertexIndices[k] = tj.vertexIndices[k];
									fin.vertexIndices[k] = ti.vertexIndices[ti.vertexIndices.size() - numVertsPerFace + k];
									fjn.vertexIndices[k] = tj.vertexIndices[tj.vertexIndices.size() - numVertsPerFace + k];
								}
								if (fin.orientation == !fjo.orientation && fin.vertexIndices[1] == fjo.vertexIndices[0] && fin.vertexIndices[2] == fjo.vertexIndices[1])
								{
									comb = ti;
									for (unsigned int k = 2; k < tj.vertexIndices.size(); k++)
									{
										comb.vertexIndices.push_back(tj.vertexIndices[k]);
									}
								}
								else if (fin.orientation == fjn.orientation && fjn.vertexIndices[1] == fin.vertexIndices[2] && fjn.vertexIndices[2] == fin.vertexIndices[1])
								{
									comb = tj;
									for (unsigned int k = ti.vertexIndices.size() - 1; k > 1; k--)
									{
										comb.vertexIndices.push_back(ti.vertexIndices[k]);
									}
								}
								else if (fio.orientation == !fjn.orientation && fjn.vertexIndices[1] == fio.vertexIndices[0] && fjn.vertexIndices[2] == fio.vertexIndices[1])
								{
									comb = tj;
									for (unsigned int k = 2; k < ti.vertexIndices.size(); k++)
									{
										comb.vertexIndices.push_back(ti.vertexIndices[k]);
									}
								}
								else if (fio.orientation == fjo.orientation && fio.vertexIndices[1] == fjo.vertexIndices[0] && fio.vertexIndices[0] == fjo.vertexIndices[1])
								{
									for (unsigned int k = ti.vertexIndices.size() - 1; k > 1; k--)
									{
										comb.vertexIndices.push_back(ti.vertexIndices[k]);
									}
									for (unsigned int k = 0; k < tj.vertexIndices.size(); k++)
									{
										comb.vertexIndices.push_back(tj.vertexIndices[k]);
									}
								}
							}
							if (comb.vertexIndices.size() > 0)
							{
								triangleLists[i] = comb;
								triangleLists.erase(triangleLists.begin() + j);
							}
						}
					}
					else
					{
						triangleLists.erase(triangleLists.begin() + j);
					}
				}
			}
			else
			{
				triangleLists.erase(triangleLists.begin() + i);
			}
		}
		// Convert the triangle lists back into polygon subcollections
		if (triangleLists.size() > 0)
		{
			const unsigned int numOrigins = 3;
			bool canConnectPSC = true;
			unsigned int pscLen = 0, numFilledOrigins = 0, numFilledPrevOrigins = 0, origins[numOrigins], prevOrigins[numOrigins];
			PolygonCollection pc_cond;
			PolygonSubCollection psc;
			for (unsigned int i = 0; i < triangleLists.size(); i++)
			{
				TriangleList ti = triangleLists[i];
				unsigned int tlLen = ti.vertexIndices.size() * 48 + 32, originLen = 0;
				for (unsigned int j = 0; j < ti.vertexIndices.size(); j++)
				{
					for (unsigned int k = 0; k < numOrigins; k++)
					{
						if (k == numFilledOrigins)
						{
							bool isPrev = false;
							for (unsigned int l = 0; l < numFilledPrevOrigins; l++)
							{
								if (verts[meshInd][ti.vertexIndices[j]].originJointIndex == prevOrigins[l])
								{
									isPrev = true;
									origins[k] = prevOrigins[l];
								}
							}
							if (!isPrev)
							{
								if (numFilledOrigins < numOrigins)
								{
									origins[k] = verts[meshInd][ti.vertexIndices[j]].originJointIndex;
									numFilledOrigins++;
									originLen += 128;
								}
								else
								{
									for (unsigned int l = 0; l < numFilledOrigins; l++)
									{
										prevOrigins[l] = origins[l];
									}
									numFilledPrevOrigins = numOrigins;
									origins[0] = verts[meshInd][ti.vertexIndices[j]].originJointIndex;
									numFilledOrigins = 1;
									originLen += 128;
								}
							}
							break;
						}
						else if (verts[meshInd][ti.vertexIndices[j]].originJointIndex == origins[k])
						{
							break;
						}
					}
				}
				if ((pscLen + originLen + tlLen) < 4032)
				{
					psc.triangleLists.push_back(ti);
					pscLen += originLen + tlLen;
				}
				else
				{
					pc_cond.subcollections.push_back(psc);
					psc.triangleLists.clear();
					psc.triangleLists.push_back(ti);
					pscLen = originLen + tlLen;
				}
			}
			if (psc.triangleLists.size() > 0)
			{
				pc_cond.subcollections.push_back(psc);
			}
			if (pc_cond.subcollections.size() > 0)
			{
				condensed[meshInd] = pc_cond;
			}
		}
	}
	return condensed;
}

unsigned int countFaces(vector<PolygonCollection> polygonCollections)
{
	unsigned int count = 0;
	for (unsigned int i = 0; i < polygonCollections.size(); i++)
	{
		PolygonCollection *polygonCollection = &(polygonCollections[i]);
		for (unsigned int j = 0; j < polygonCollection->subcollections.size(); j++)
		{
			PolygonSubCollection *subCollection = &((*polygonCollection).subcollections[j]);
			for (unsigned int k = 0; k < subCollection->triangleLists.size(); k++)
			{
				TriangleList *triangleList = &((*subCollection).triangleLists[k]);
				unsigned int numVerts = triangleList->vertexIndices.size();
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

bool compareFacesToPolygonCollections(vector<vector<Face>> faces, vector<PolygonCollection> polygonCollections)
{
	vector<Face> collectionFaces;
	unsigned int numFaces = 0, numPolygonCollectionFaces = countFaces(polygonCollections);
	bool result, swap = false;
	// Determine the number of faces in the vector
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		numFaces += faces[i].size();
	}
	// Compare the number of faces in the vector to those in the polygon collection
	result = numFaces == numPolygonCollectionFaces;
	/*
	if (result)
	{
		unsigned int polygonCollectionInd = 0, subCollectionInd = 0, triangleListInd = 0, vertexInd = 0, meshVertexOffset = 0;
		collectionFaces.reserve(numFaces);
		for (unsigned int i = 0; result && i < faces.size(); i++)
		{
			if (i > 0)
			{
				meshVertexOffset += faces[i].size();
			}
			for (unsigned int j = 0; j < faces[i].size(); j++)
			{
				Face fi = faces[i][j], fpcj;
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
							unsigned int k;
							for (k = 0; vertexInd < tl.size() && k < numVertsPerFace; k++)
							{
								fpcj.vertexIndices[k] = tl[vertexInd] - meshVertexOffset;
								vertexInd++;
							}
							if (j != numVertsPerFace)
							{
								matchingFace = false;
							}
							else
							{
								collectionFaces.push_back(fpcj);
							}
							if (swap)
							{
								unsigned int val = fpcj.vertexIndices[1];
								fpcj.vertexIndices[1] = fpcj.vertexIndices[2];
								fpcj.vertexIndices[2] = val;
							}
							for (j = 0; matchingFace && j < numVertsPerFace; j++)
							{
								if (fi.vertexIndices[j] != fpcj.vertexIndices[j])
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
	}
	*/
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

vector<vector<VertexRelative>> convertJointsToVerts(vector<JointRelative> joints)
{
	VertexRelative v;
	vector<vector<VertexRelative>> verts;
	verts.reserve(1);
	verts.push_back(vector<VertexRelative>());
	// Reserve space for each vertex
	verts[0].reserve(joints.size());
	// Create a vertex for each joint
	for (unsigned int i = 0; i < joints.size(); i++)
	{
		v = convertJointToVert(joints[i]);
		// Add the vertex to the verts vector
		verts[0].push_back(v);
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

bool normalizePosition(vector<vector<VertexGlobal>> vertices, vector<vector<VertexGlobal>> &vertexOutput, vector<JointRelative> joints, vector<JointRelative> *jointOutput)
{
	bool valid;
	// Initialize max
	float max = numeric_limits<float>::min();
	float min = numeric_limits<float>::max(), tempCoord, maxes[numDims] = { max, max, max }, mins[numDims] = { min, min, min };
	vector<JointGlobal> jointsG;
	// Intialize the outputs to the input
	if (jointOutput == NULL)
	{
		jointOutput = &joints;
	}
	else
	{
		*jointOutput = joints;
	}
	vertexOutput = vertices;
	// Get the global joint coordinates
	valid = getJointsGlobal(joints, jointsG);
	// Make sure the conversion was successful
	if (valid)
	{
		// Find the min and max values for the coordinates
		for (unsigned int i = 0; i < joints.size(); i++)
		{
			for (unsigned int j = 0; j < numDims; j++)
			{
				tempCoord = jointsG[i].coordinates[j];
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
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			for (unsigned int j = 0; j < vertices[i].size(); j++)
			{
				for (unsigned int k = 0; k < numDims; k++)
				{
					tempCoord = vertices[i][j].coordinates[k];
					if (tempCoord < mins[k])
					{
						mins[k] = tempCoord;
					}
					if (tempCoord > maxes[k])
					{
						maxes[k] = tempCoord;
					}
				}
			}
		}
		// Determine the absolute min and max values
		for (unsigned int i = 0; i < numDims; i++)
		{
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
		// Scale the joints
		for (unsigned int i = 0; i < joints.size(); i++)
		{
			for (unsigned int j = 0; j < numDims; j++)
			{
				(*jointOutput)[i].coordinates[j] = joints[i].coordinates[j] / max;
			}
		}
		// Scale the vertices
		for (unsigned int i = 0; i < vertices.size(); i++)
		{
			for (unsigned int j = 0; j < vertices[i].size(); j++)
			{
				for (unsigned int k = 0; k < numDims; k++)
				{
					vertexOutput[i][j].coordinates[k] = vertices[i][j].coordinates[k] / max;
				}
			}
		}
	}
	return valid;
}

bool normalizePosition(vector<vector<VertexRelative>> vertices, vector<vector<VertexRelative>> &vertexOutput, vector<JointRelative> joints, vector<JointRelative>*jointOutput)
{
	bool valid;
	vector<JointGlobal> jointsG;
	vector<vector<VertexGlobal>> vertsG;
	vertexOutput = vertices;
	valid = getVerticesGlobal(joints, vertices, vertsG);
	if (valid)
	{
		valid = normalizePosition(vertsG, vertsG, joints, jointOutput);
		if (valid)
		{
			valid = getVerticesRelative(joints, vertsG, vertexOutput);
		}
	}
	return valid;
}

bool flipUVCoordinates(vector<vector<VertexGlobal>> vertices, vector<vector<VertexGlobal>> &output)
{
	bool valid = true;
	unsigned int numMeshes = vertices.size(), numVerts;
	// Copy the vertices into the output vector
	output = vertices;
	// Flip all of the V coordinates
	for (unsigned int i = 0; i < numMeshes; i++)
	{
		numVerts = vertices[i].size();
		for (unsigned int j = 0; j < numVerts; j++)
		{
			output[i][j].textureMap[1] = 1 - output[i][j].textureMap[1];
		}
	}
	return valid;
}

bool flipUVCoordinates(vector<vector<VertexRelative>> vertices, vector<vector<VertexRelative>> &output)
{
	return flipUVCoordinates(*(vector<vector<VertexGlobal>>*)&(vertices), *(vector<vector<VertexGlobal>>*)&(output));
}

// Parse through the template vertex data and modify the origin update sections to match the rigging from vertices
bool updateRigging(vector<JointRelative> joints, vector<JointRelative> templateJoints, vector<vector<VertexRelative>> vertices, vector<vector<Face>> faces, string templateVertexData, string &vertexData)
{
	bool valid;
	const unsigned int numOrigins = 3;
	unsigned int readIndex = 0, type, jointIndex, originIndex, meshInd = 0, vertexInd, vertexIndInd, faceInd = 0, numJoints = joints.size(), numMeshes = vertices.size(), numVerts, numFaces, numTemplateJoints = templateJoints.size(), templateLen = templateVertexData.size(), originJointInds[numOrigins] = { 0, 0, 0 }, updatedOrigins[numOrigins] = { 0, 0, 0 };
	vector<vector<unsigned int>> jointRelations = getJointRelations(templateJoints, joints);
	vector<vector<VertexGlobal>> vertsG;
	// Make sure we got valid relations and get the global vertices
	valid = (numMeshes == faces.size()) && (jointRelations.size() != 0) && getVerticesGlobal(joints, vertices, vertsG);
	// Intialize the number of faces and vertices
	if (valid && meshInd < numMeshes)
	{
		numFaces = faces[meshInd].size();
		numVerts = vertices[meshInd].size();
	}
	else
	{
		numFaces = 0;
		numVerts = 0;
	}
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
				if (faceInd >= numFaces)
				{
					faceInd = 0;
					meshInd++;
					if (meshInd < numMeshes)
					{
						numFaces = faces[meshInd].size();
						numVerts = vertices[meshInd].size();
					}
					else
					{
						numFaces = 0;
						numVerts = 0;
					}
				}
				if (meshInd < numMeshes && faceInd < numFaces)
				{
					Face f = faces[meshInd][faceInd];
					if (faceInd > 0 && vertexIndInd == 0)
					{
						Face fp = faces[meshInd][faceInd - 1];
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
	vector<vector<VertexRelative>> jir = convertJointsToVerts(jointsI), jvr = convertJointsToVerts(jointsJ);
	vector<vector<VertexGlobal>> jig, jvg;
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
				if ((relations[0][i] == numJointsJ) && (relations[1][j] == numJointsI) && compareVertices(jig[0][i], jvg[0][j], threshold) && compareVertices(jir[0][i], jvr[0][j], threshold))
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
			t.wExp = 7;
			t.hExp = 7;
			/*
			while ((t.width >> t.wExp) > 1)
			{
				t.wExp++;
			}
			while ((t.height >> t.hExp) > 1)
			{
				t.hExp++;
			}
			*/
			p.numColors = 0;
			// For each pixel
			for (unsigned int i = 0; i < rawData.size(); i += 4)
			{
				// Store this pixel's color values
				unsigned char r = rawData[i], g = rawData[i + 1], b = rawData[i + 2], a = rawData[i + 3];
				unsigned short j;
				// Check the palette for this color
				for (j = 0; j < p.numColors; j++)
				{
					// Check if we found the color
					if (r == p.r[j] && g == p.g[j] && b == p.b[j] && a == p.a[j])
					{
						// Add the index
						inds.push_back((unsigned char)j);
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
					inds.push_back((unsigned char)j);
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
vector<vector<Face>> sortFaces(vector<vector<Face>> faces, vector<vector<VertexRelative>> verts)
{
	// Create a copy of faces
	vector<vector<Face>> sorted = faces;
	vector<vector<FaceEx>> toSort;
	vector<vector<unsigned int>> minJointVertInds;
	// Get the number of faces
	unsigned int numMeshes = faces.size(), numFaces;
	if (numMeshes == verts.size())
	{
		// For each mesh...
		for (unsigned int i = 0; i < numMeshes; i++)
		{
			// Get the number of faces
			numFaces = faces[i].size();
			toSort.push_back(vector<FaceEx>());
			minJointVertInds.push_back(vector<unsigned int>());
			toSort.reserve(numFaces);
			minJointVertInds.reserve(numFaces);
			// For each face...
			for (unsigned int j = 0; j < numFaces; j++)
			{
				// Get face j
				Face f = faces[i][j];
				// Initialize the minimum joint vertex index to 0
				minJointVertInds[i].push_back(0);
				// Check vertices 1 and 2 to see if they have smaller joint indices than vertex 0
				for (unsigned int k = 1; k < numVertsPerFace; k++)
				{
					if (verts[i][f.vertexIndices[k]] < verts[i][f.vertexIndices[minJointVertInds[i][j]]])
					{
						minJointVertInds[i][j] = k;
					}
				}
				// Shift the order of the vertices so that the minimum vertex is listed first while maintaining the orientation
				for (unsigned int k = 0; k < numVertsPerFace; k++)
				{
					sorted[i][j].vertexIndices[k] = faces[i][j].vertexIndices[(minJointVertInds[i][j] + k) % numVertsPerFace];
				}
				// Check if vertex 2 has a smaller origin joint index than vertex 1
				if (verts[i][sorted[i][j].vertexIndices[1]] > verts[i][sorted[i][j].vertexIndices[2]])
				{
					// It does so swap the orientation so that the origin joint indices are in ascending order
					unsigned int temp = sorted[i][j].vertexIndices[1];
					sorted[i][j].vertexIndices[1] = sorted[i][j].vertexIndices[2];
					sorted[i][j].vertexIndices[2] = temp;
					if (sorted[i][j].orientation == CLOCKWISE)
					{
						sorted[i][j].orientation = COUNTERCLOCKWISE;
					}
					else if (sorted[i][j].orientation == COUNTERCLOCKWISE)
					{
						sorted[i][j].orientation = CLOCKWISE;
					}
				}
			}
		}
		// Convert the faces to the explicit form
		if (getFacesEx(sorted, verts, toSort))
		{
			// Sort the faces
			//std::sort(toSort.begin(), toSort.end());
			for (unsigned int i = 0; i < numMeshes; i++)
			{
				std::sort(toSort[i].begin(), toSort[i].end());
			}
			// Convert the faces back to the general form
			if (getFaces(toSort, verts, sorted))
			{
				// Reset all the faces to a clockwise orientation
				for (unsigned int i = 0; i < numMeshes; i++)
				{
					numFaces = sorted[i].size();
					for (unsigned int j = 0; j < numFaces; j++)
					{
						if (sorted[i][j].orientation == CLOCKWISE)
						{
							unsigned int temp = sorted[i][j].vertexIndices[1];
							sorted[i][j].vertexIndices[1] = sorted[i][j].vertexIndices[2];
							sorted[i][j].vertexIndices[2] = temp;
							sorted[i][j].orientation = COUNTERCLOCKWISE;
						}
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
