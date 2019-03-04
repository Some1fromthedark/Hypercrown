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
struct PolygonCollection;
struct PolygonSubCollection;
struct TriangleList;

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

#define COUNTERCLOCKWISE	0
#define CLOCKWISE			1
#define COLINEAR			2

#include"matmul.h"
#include"progDetails.h"

using namespace std;

// Prototype Functions
bool binaryFileRead(string filename, string &dat);
string convertMesh(string raw, string inputFormat, string outputFormat, AddInfo optionalVals);
string convertMeshToDae(vector<JointRelative> joints, vector<vector<VertexGlobal>> verts, vector<vector<Face>> faces, vector<Animation> anims, string name = "model");
string convertMeshToMdls(vector<JointRelative> joints, vector<vector<VertexRelative>> verts, vector<vector<Face>> faces);
string convertMeshToMdls(vector<JointRelative> joints, vector<vector<VertexRelative>> verts, vector<vector<Face>> faces, string templateRigData, string templateVertexData);
string convertMeshToObj(vector<vector<VertexGlobal>> verts, vector<vector<Face>> faces, string mtlName = "");
string convertMeshToWpn(vector<vector<VertexRelative>> verts, vector<vector<Face>> faces);
string convertMeshToWpn(vector<vector<VertexRelative>> verts, vector<vector<Face>> faces, string templateData);
string createMTL(unsigned int numTextures, string filenamePrefix);
void textureToRaw(Texture t, unsigned char *&raw);
vector<unsigned char> textureToRaw(Texture t);
unsigned int getNumMdlsTextures(string raw);
unsigned int getNumWpnTextures(string raw);
vector<Texture> getMdlsTextures(string raw);
vector<Texture> getMsetTextures(string raw, vector<Texture> mdlsTextures);
vector<Texture> getTexaTextures(string raw, vector<Texture> referenceTextures);
vector<Texture> getWpnTextures(string  raw);
string setMdlsSpecialEffects(string raw, vector<string> effects);
string setWpnSpecialEffects(string raw, vector<string> effects);
string setMdlsTextures(string raw, vector<Texture> textures);
string setMsetTextures(string raw, vector<Texture> msetTextures, vector<Texture> mdlsTextures, vector<unsigned int> referenceInds);
string setTexaTextures(string raw, vector<Texture> texaTextures, vector<Texture> referenceTextures, vector<unsigned int > referenceInds);
string setWpnTextures(string raw, vector<Texture> textures);
vector<string> getMdlsSpecialEffects(string raw);
vector<string> getWpnSpecialEffects(string raw);
vector<string> getSPFXSpecialEffects(string raw);
vector<Texture> getMagSpecialEffectTextures(string raw);
vector<Texture> getMdlsSpecialEffectTextures(string raw);
vector<Texture> getWpnSpecialEffectTextures(string raw);
vector<Texture> getSPFXTextures(string raw);
unsigned int getNumTextures(vector<vector<VertexRelative>> verts);
unsigned int getNumTextures(vector<vector<VertexGlobal>> verts);
unsigned int getNumFacesOfTexture(vector<VertexRelative> verts, vector<Face> faces, unsigned int textureInd);
unsigned int getNumFacesOfTexture(vector<VertexGlobal> verts, vector<Face> faces, unsigned int textureInd);
unsigned int getFaceOrientation(Face f, vector<VertexGlobal> verts);
unsigned int getRootJointIndex(vector<JointRelative> joints);
bool getDaeJoints(const aiScene *scene, vector<JointRelative> &joints, unsigned int startDepth = 0);
bool getDaeVerticesAndFaces(const aiScene *scene, vector<vector<VertexGlobal>> &verts, vector<vector<Face>> &faces, vector<JointRelative> joints = vector<JointRelative>());
vector<JointRelative> getMdlsJoints(string raw, unsigned int maxJoints = UINT_MAX);
bool getMdlsVerticesAndFaces(string raw, vector<JointRelative> joints, vector<vector<VertexRelative>> &verts, vector<vector<Face>> &faces);
bool getMdlsShadowVerticesAndFaces(string raw, vector<JointRelative> joints, vector<vector<VertexRelative>> &verts, vector<vector<Face>> &faces);
vector<JointRelative> getWpnJoints(string raw, unsigned int maxJoints = UINT_MAX);
bool getWpnVerticesAndFaces(string raw, vector<vector<VertexRelative>> &verts, vector<vector<Face>> &faces);
void removeDuplicateFaces(vector<vector<Face>> &faces, bool ignoreOrientation);
void removeDuplicateVertices(vector<vector<VertexGlobal>> &verts, vector<vector<Face>> &faces);
bool getJointGlobal(vector<JointRelative> joints, unsigned int jrInd, JointGlobal &jg);
bool getJointsGlobal(vector<JointRelative> jr, vector<JointGlobal> &jg);
bool getVertexGlobal(vector<JointRelative> joints, VertexRelative vr, VertexGlobal &vg);
bool getVerticesGlobal(vector<JointRelative> joints, vector<vector<VertexRelative>> vr, vector<vector<VertexGlobal>> &vg);
bool getVertexRelative(vector<JointRelative> joints, VertexGlobal vg, VertexRelative &vr);
bool getVerticesRelative(vector<JointRelative> joints, vector<vector<VertexGlobal>> vg, vector<vector<VertexRelative>> &vr);
bool getFace(FaceEx fe, vector<VertexRelative> verts, Face &f);
bool getFaces(vector<vector<FaceEx>> facesEx, vector<vector<VertexRelative>> verts, vector<vector<Face>> &faces);
bool getFaceEx(Face f, vector<VertexRelative> verts, FaceEx &fe);
bool getFacesEx(vector<vector<Face>> faces, vector<vector<VertexRelative>> verts, vector<vector<FaceEx>> &facesEx);
bool combineMeshes(vector<vector<VertexRelative>> &verts, vector<vector<Face>> &faces);
bool combineMeshes(vector<vector<VertexGlobal>> &verts, vector<vector<Face>> &faces);
bool swapChirality(vector<JointRelative> &joints, vector<vector<VertexGlobal>> &verts);
bool swapChirality(vector<JointRelative> &joints, vector<vector<VertexRelative>> &verts);
bool getInverseBindMatrix(vector<JointRelative> joints, unsigned int jointIndex, vector<vector<float>> &ibm, float max = 1);
bool getPolygonCollections(vector<vector<VertexRelative>> verts, vector<vector<Face>> faces, vector<PolygonCollection> &polygonCollections);
vector<PolygonCollection> condensePolygonCollections(vector<vector<VertexRelative>> verts, vector<PolygonCollection> polygonCollections);
unsigned int countFaces(vector<PolygonCollection> polygonCollections);
bool compareFacesToPolygonCollections(vector<vector<Face>> faces, vector<PolygonCollection> polygonCollections);
VertexRelative convertJointToVert(JointRelative joint);
vector<vector<VertexRelative>> convertJointsToVerts(vector<JointRelative> joints);
bool normalizePosition(vector<vector<VertexGlobal>> vertices, vector<vector<VertexGlobal>> &vertexOutput, vector<JointRelative> joints = vector<JointRelative>(), vector<JointRelative> *jointOutput = NULL);
bool normalizePosition(vector<vector<VertexRelative>> vertices, vector<vector<VertexRelative>> &vertexOutput, vector<JointRelative> joints = vector<JointRelative>(), vector<JointRelative> *jointOutput = NULL);
bool flipUVCoordinates(vector<vector<VertexGlobal>> vertices, vector<vector<VertexGlobal>> &output);
bool flipUVCoordinates(vector<vector<VertexRelative>> vertices, vector<vector<VertexRelative>> &output);
bool updateRigging(vector<JointRelative> joints, vector<JointRelative> templateJoints, vector<vector<VertexRelative>> vertices, vector<vector<Face>> faces, string templateVertexData, string &vertexData);
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
vector<vector<Face>> sortFaces(vector<vector<Face>> faces, vector<vector<VertexRelative>> verts);
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
const unsigned int numDims = 3,
additionalVals = 2,
numLinesPerVertex = 2,
numVertsPerFace = 3,
absoluteMaxJoints = 1023,
loopTimeOut = 10000,
menvHeaderLen = 64,
mmtnHeaderLen = 16,
mobjHeaderLen = 64,
modelTableHeaderLen = 16,
texaHeaderLen = 40;

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

struct TriangleList
{
	unsigned char orientation;
	vector<unsigned int> vertexIndices;
};

struct PolygonSubCollection
{
	vector<TriangleList> triangleLists;
};

struct PolygonCollection
{
	vector<PolygonSubCollection> subcollections;
};
