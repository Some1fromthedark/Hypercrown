#pragma once

#define _USE_MATH_DEFINES // Enable math defines

#include<iostream>
#include<math.h>
#include<thread>
#include<vector>

using namespace std;

vector<vector<float>> zeros(unsigned int n);
vector<vector<float>> zeros(unsigned int n, unsigned int m);
vector<vector<float>> eye(unsigned int n);
vector<vector<float>> eye(unsigned int n, unsigned int m);
vector<vector<float>> matrixMultiply(vector<vector<float>> a, vector<vector<float>> b);
bool subMatrixMultiply(vector<vector<float>> a, vector<vector<float>> b, vector<vector<float>> *c, unsigned int i, unsigned int j);
vector<vector<float>> getScaleMatrix(float scaleFactors[], unsigned int nDims = 3);
vector<vector<float>> getRotationMatrix(unsigned int dim, float theta, unsigned int nDims=3);
vector<vector<float>> getTranslationMatrix(float translation[]);
vector<vector<float>> getHomogeneousMatrix(unsigned int dim, float theta, float translation[]);
vector<vector<float>> getInverseHomogeneousMatrix(unsigned int dim, float theta, float translation[]);
bool testMatrixMultiply(vector<vector<float>> pose, unsigned int axis, float theta, float translation[], unsigned int mode);
