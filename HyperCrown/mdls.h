#pragma once

#define USE_MATH_DEFINES // Enables math defines

// Prototype structs
struct AddInfo;
struct Animation;
struct DaeImage;
struct Face;
struct FaceEx;
struct JointGlobal;
struct JointRelative;
struct JointInfo;
struct MsetStaticAnimation;
struct Palette;
struct Texture;
struct VertexGlobal;
struct VertexRelative;

// Include Standard Libraries
#include<iomanip>
#include<iostream>
#include<fstream>
#include<math.h>
#include<sstream>
#include<string>
#include<vector>

// Include Assimp Libraries
#include<assimp\Importer.hpp>
#include<assimp\postprocess.h>
#include<assimp\scene.h>
#include<assimp\types.h>

#define PP_RVC_FLAGS aiComponent_COLORS | aiComponent_LIGHTS | aiComponent_CAMERAS

// Include LodePNG Libraries
#include"lodepng.h"

#define COLINEAR			0
#define CLOCKWISE			1
#define COUNTERCLOCKWISE	2

#include"matmul.h"
#include"progDetails.h"

using namespace std;

// Prototype Functions
bool assimpTest();
string convertMesh(string raw, string inputFormat, string outputFormat, AddInfo optionalVals);
string convertSkeleton(string raw, string format = ".obj", unsigned int maxJoints = UINT_MAX);
string convertMeshToDae(vector<JointRelative> joints, vector<VertexGlobal> verts, vector<Face> faces, vector<Animation> anims, float adjustments[], float max, string name = "model");
string convertMeshToMdls(vector<JointRelative> joints, vector<VertexRelative> verts, vector<Face> faces, float adjustments[], float max);
string convertMeshToMdls(vector<JointRelative> joints, vector<VertexRelative> verts, vector<Face> faces, string templateRigData, string templateVertexData, float adjustments[], float max);
string convertMeshToObj(vector<VertexGlobal> verts, vector<Face> faces, float adjustments[], float max, string mtlName = "");
string convertMeshToWpn(vector<VertexRelative> verts, vector<Face> faces, float adjustments[], float max);
string convertMeshToWpn(vector<VertexRelative> verts, vector<Face> faces, string templateData, float adjustments[], float max);
string convertSkeletonToObj(vector<JointRelative> joints, unsigned int maxJoints = UINT_MAX);
string createMTL(unsigned int numTextures, string filenamePrefix);
void textureToRaw(Texture t, unsigned char *&raw);
vector<unsigned char> textureToRaw(Texture t);
unsigned int getNumMdlsTextures(string raw);
unsigned int getNumWpnTextures(string raw);
vector<Texture> getMdlsTextures(string raw);
vector<Texture> getWpnTextures(string  raw);
string setMdlsSpecialEffects(string raw, vector<string> effects);
string setWpnSpecialEffects(string raw, vector<string> effects);
string setMdlsTextures(string raw, vector<Texture> textures);
string setWpnTextures(string raw, vector<Texture> textures);
vector<string> getMdlsSpecialEffects(string raw);
vector<string> getWpnSpecialEffects(string raw);
vector<Texture> getMdlsSpecialEffectTextures(string raw);
vector<Texture> getWpnSpecialEffectTextures(string raw);
unsigned int getNumTextures(vector<VertexRelative> verts);
unsigned int getNumTextures(vector<VertexGlobal> verts);
unsigned int getNumFacesOfTexture(vector<VertexRelative> verts, vector<Face> faces, unsigned int textureInd);
unsigned int getNumFacesOfTexture(vector<VertexGlobal> verts, vector<Face> faces, unsigned int textureInd);
unsigned int getFaceOrientation(Face f, vector<VertexGlobal> verts);
unsigned int getRootJointIndex(vector<JointRelative> joints);
bool getDaeJoints(const aiScene *scene, vector<JointRelative> &joints, unsigned int startDepth = 0);
bool getDaeVerticesAndFaces(const aiScene *scene, vector<VertexGlobal> &verts, vector<Face> &faces, vector<JointRelative> joints = vector<JointRelative>());
vector<JointRelative> getMdlsJoints(string raw, unsigned int maxJoints = UINT_MAX);
bool getMdlsVerticesAndFaces(string raw, vector<JointRelative> joints, vector<VertexRelative> &verts, vector<Face> &faces);
bool getWpnVerticesAndFaces(string raw, vector<VertexRelative> &verts, vector<Face> &faces);
void removeDuplicateFaces(vector<Face> &faces, bool ignoreOrientation);
bool getJointGlobal(vector<JointRelative> joints, unsigned int jrInd, JointGlobal &jg);
bool getJointsGlobal(vector<JointRelative> jr, vector<JointGlobal> &jg);
bool getVertexGlobal(vector<JointRelative> joints, VertexRelative vr, VertexGlobal &vg);
bool getVerticesGlobal(vector<JointRelative> joints, vector<VertexRelative> vr, vector<VertexGlobal> &vg);
bool getVertexRelative(vector<JointRelative> joints, VertexGlobal vg, VertexRelative &vr);
bool getVerticesRelative(vector<JointRelative> joints, vector<VertexGlobal> vg, vector<VertexRelative> &vr);
bool getFace(FaceEx fe, vector<VertexRelative> verts, Face &f);
bool getFaces(vector<FaceEx> facesEx, vector<VertexRelative> verts, vector<Face> &faces);
bool getFaceEx(Face f, vector<VertexRelative> verts, FaceEx &fe);
bool getFacesEx(vector<Face> faces, vector<VertexRelative> verts, vector<FaceEx> &facesEx);
bool getInverseBindMatrix(vector<JointRelative> joints, unsigned int jointIndex, vector<vector<float>> &ibm, float max = 1);
bool getPolygonCollections(vector<VertexRelative> verts, vector<Face> faces, vector<vector<vector<vector<unsigned int>>>> &polygonCollections);
unsigned int countFaces(vector<vector<vector<vector<unsigned int>>>> polygonCollections);
bool compareFacesToPolygonCollections(vector<Face> faces, vector<vector<vector<vector<unsigned int>>>> polygonCollections);
VertexRelative convertJointToVert(JointRelative joint);
vector<VertexRelative> convertJointsToVerts(vector<JointRelative> joints);
bool normalizePosition(vector<VertexGlobal> vertices, float adjustments[], float &max);
bool updateRigging(vector<JointRelative> joints, vector<JointRelative> templateJoints, vector<VertexRelative> vertices, vector<Face> faces, string templateVertexData, string &vertexData);
bool compareJoints(JointGlobal a, JointGlobal b, float threshold = 0.5f);
bool compareVertices(VertexGlobal a, VertexGlobal b, float threshold = 0.5f);
bool compareVertices(VertexRelative a, VertexRelative b, float threshold = 0.5f);
vector<vector<unsigned int>> getJointRelations(vector<JointRelative> jointsI, vector<JointRelative> jointsJ, float threshold = 0.75f);
template <typename TReal>
void mdlsDecompose(aiMatrix4x4t<TReal> transform, aiVector3t<TReal> &pScaling, aiVector3t<TReal> &pRotation, aiVector3t<TReal> &pPosition);
bool readTexture(string filename, Texture &t);
bool writeTexture(Texture t, string filename);
vector<MsetStaticAnimation> getMsetAnimations(string raw);
Animation convertMsetStaticAnimationToAnimation(vector<JointRelative> joints, MsetStaticAnimation mAnim);
vector<Animation> convertMsetStaticAnimationsToAnimations(vector<JointRelative> joints, vector<MsetStaticAnimation> mAnims);
vector<Face> sortFaces(vector<Face> faces, vector<VertexRelative> verts);
// Prototype equivalence operators
bool operator==(const VertexGlobal a, const VertexGlobal b);
bool operator!=(const VertexGlobal a, const VertexGlobal b);
bool operator==(const VertexRelative a, const VertexRelative b);
bool operator!=(const VertexRelative a, const VertexRelative b);
bool operator<(const VertexRelative a, const VertexRelative b);
bool operator>(const VertexRelative a, const VertexRelative b);
bool operator<=(const VertexRelative a, const VertexRelative b);
bool operator>=(const VertexRelative a, const VertexRelative b);
bool operator<(const FaceEx a, const FaceEx b);
bool operator>(const FaceEx a, const FaceEx b);
bool operator<=(const FaceEx a, const FaceEx b);
bool operator>=(const FaceEx a, const FaceEx b);

// Declare constants
const unsigned int	numOrigins = 6,
numDims = 3,
additionalVals = 2,
numLinesPerVertex = 2,
numVertsPerFace = 3,
absoluteMaxJoints = 1023,
loopTimeOut = 10000,
menvHeaderLen = 64,
mobjHeaderLen = 64,
modelTableHeaderLen = 16;

// Define structs
struct AddInfo
{
	vector<string> additionalStrs;
	vector<unsigned int> additionalInts;
	vector<float> additionalFloats;
	vector<bool> additionalBools;
};

struct Animation
{
	unsigned int jointInd;
	vector<float> times;
	vector<vector<vector<float>>> keyframes;
};

struct DaeImage
{
	string id;
	string filename;
};

struct Face
{
	unsigned char orientation;
	unsigned int vertexIndices[numVertsPerFace];
};

struct JointInfo
{
	bool unknownFlag;
	unsigned int childIndex, unknownIndex, parentIndex;
};

struct JointGlobal
{
	string name;
	float scaleFactors[numDims];
	unsigned int ind;
	float special;
	float rotations[numDims];
	float coordinates[numDims];
	JointInfo jointInfo;
	vector<unsigned int> childrenIndices;
};

struct JointRelative
{
	string name;
	float scaleFactors[numDims];
	unsigned int ind;
	float special;
	float rotations[numDims];
	float coordinates[numDims];
	JointInfo jointInfo;
	vector<unsigned int> childrenIndices;
};

struct MsetStaticAnimation
{
	unsigned int jointID;
	vector<float> transforms;
	vector<bool> hasTransform;
};

struct Palette
{
	unsigned short numColors;
	vector<unsigned char> r;
	vector<unsigned char> g;
	vector<unsigned char> b;
	vector<unsigned char> a;

};

struct Texture
{
	unsigned short height;
	unsigned short width;
	unsigned char hExp;
	unsigned char wExp;
	vector<unsigned char> indices;
	Palette palette;
};

struct VertexGlobal
{
	unsigned int textureIndex;
	float normal[numDims];
	unsigned int originJointIndex;
	float coordinates[numDims];
	float coordinatesScaleFactor;
	float textureMap[2];
	float textureScaleFactor;
};

struct VertexRelative
{
	unsigned int textureIndex; // Indicates which texture to use
	float normal[numDims];
	unsigned int originJointIndex;
	float coordinates[numDims];
	float coordinatesScaleFactor;
	float textureMap[2]; // U and V
	float textureScaleFactor;
};

struct FaceEx
{
	unsigned char orientation;
	VertexRelative vertices[numVertsPerFace];
};
