#include"matmul.h"

vector<vector<float>> zeros(unsigned int n)
{
	// Construct the matrix
	vector<vector<float>> mat;
	mat.reserve(n);
	for (unsigned int i = 0; i < n; i++)
	{
		mat.push_back(vector<float>());
		mat[i].reserve(n);
		for (unsigned int j = 0; j < n; j++)
		{
			mat[i].push_back(0);
		}
	}
	return mat;
}

vector<vector<float>> zeros(unsigned int n, unsigned int m)
{
	// Construct the matrix
	vector<vector<float>> mat;
	mat.reserve(n);
	for (unsigned int i = 0; i < n; i++)
	{
		mat.push_back(vector<float>());
		mat[i].reserve(m);
		for (unsigned int j = 0; j < m; j++)
		{
			mat[i].push_back(0);
		}
	}
	return mat;
}

vector<vector<float>> eye(unsigned int n)
{
	// Construct the matrix
	vector<vector<float>> mat;
	mat.reserve(n);
	for (unsigned int i = 0; i < n; i++)
	{
		mat.push_back(vector<float>());
		mat[i].reserve(n);
		for (unsigned int j = 0; j < n; j++)
		{
			if (i == j)
			{
				mat[i].push_back(1);
			}
			else
			{
				mat[i].push_back(0);
			}
		}
	}
	return mat;
}

vector<vector<float>> eye(unsigned int n, unsigned int m)
{
	// Construct the matrix
	vector<vector<float>> mat;
	mat.reserve(n);
	for (unsigned int i = 0; i < n; i++)
	{
		mat.push_back(vector<float>());
		mat[i].reserve(m);
		for (unsigned int j = 0; j < m; j++)
		{
			if (i == j)
			{
				mat[i].push_back(1);
			}
			else
			{
				mat[i].push_back(0);
			}
		}
	}
	return mat;
}

// Performs 2D matrix multiplication
vector<vector<float>> matrixMultiply(vector<vector<float>> a, vector<vector<float>> b)
{
	// Declare variables
	vector<vector<float>> result;
	bool valid = true;
	unsigned int sizeA0 = 0, sizeA1 = 0, sizeB0 = 0, sizeB1 = 0, numThreads;
	vector<thread *> threads;
	// Check for valid input
	if (a.size() > 0 && b.size() > 0)
	{
		// Extract sizes
		sizeA0 = a.size();
		sizeA1 = a[0].size();
		sizeB0 = b.size();
		sizeB1 = b[0].size();
		// Make sure that the sizes are consistent
		for (unsigned int i = 1; i < sizeA0; i++)
		{
			if (a[i].size() != sizeA1)
			{
				valid = false;
				break;
			}
		}
		for (unsigned int i = 1; i < sizeB0; i++)
		{
			if (b[i].size() != sizeB1)
			{
				valid = false;
				break;
			}
		}
		// Make sure the dimensions line up appropriately
		if (sizeA1 != sizeB0)
		{
			valid = false;
		}
		if (valid)
		{
			// Set up the return value's dimensions
			for (unsigned int i = 0; i < sizeA0; i++)
			{
				result.push_back(vector<float>());
				for (unsigned int j = 0; j < sizeB1; j++)
				{
					result[i].push_back(float(0));
				}
			}
			// Get the number of threads
			numThreads = sizeA0 * sizeB1;
			threads.reserve(numThreads);
			// Perform the matrix multiplication
			for (unsigned int i = 0; i < sizeA0; i++)
			{
				for (unsigned int j = 0; j < sizeB1; j++)
				{
					// Spin up a thread to perform the multiplication for the cell
					//threads.push_back(new thread(subMatrixMultiply, a, b, &result, i, joints));
					for (unsigned int k = 0; k < sizeA1; k++)
					{
						result[i][j] += a[i][k] * b[k][j];
					}
				}
			}
			// Join with all of the threads
			//for (unsigned int i = 0; i < numThreads; i++)
			//{
			// Join with the thead then delete it
			//threads[i]->join();
			//delete threads[i];
			//}
		}
	}
	return result;
}

bool subMatrixMultiply(vector<vector<float>> a, vector<vector<float>> b, vector<vector<float>> *c, unsigned int i, unsigned int j)
{
	// Declare variables
	bool valid = true;
	unsigned int sizeA0 = 0, sizeA1 = 0, sizeB0 = 0, sizeB1 = 0;
	// Check for valid input
	if (a.size() > 0 && b.size() > 0)
	{
		// Extract sizes
		sizeA0 = a.size();
		sizeA1 = a[0].size();
		sizeB0 = b.size();
		sizeB1 = b[0].size();
		// Make sure that the sizes are consistent
		for (unsigned int i = 1; i < sizeA0; i++)
		{
			if (a[i].size() != sizeA1)
			{
				valid = false;
				break;
			}
		}
		for (unsigned int i = 1; i < sizeB0; i++)
		{
			if (b[i].size() != sizeB1)
			{
				valid = false;
				break;
			}
		}
		// Make sure the dimensions line up appropriately
		if (sizeA1 != sizeB0)
		{
			valid = false;
		}
		if (valid)
		{
			// Initialize the cell to 0
			(*c)[i][j] = 0;
			// Perform the cross product
			for (unsigned int k = 0; k < sizeA1; k++)
			{
				(*c)[i][j] += a[i][k] * b[k][j];
			}
		}
	}
	return valid;
}

vector<vector<float>> getScaleMatrix(float scaleFactors[], unsigned int nDims)
{
	vector<vector<float>> scaleMat = eye(nDims);
	for (unsigned int i = 0; i < nDims && i < 3; i++)
	{
		scaleMat[i][i] = scaleFactors[i];
	}
	return scaleMat;
}

vector<vector<float>> getRotationMatrix(unsigned int dim, float theta, unsigned int nDims)
{
	vector<vector<float>> rotMat = eye(nDims);
	float cosVal = cos(theta), sinVal = sin(theta);
	if (nDims > 2)
	{
		// Make sure a valid dimension was chosen
		if (dim < 3)
		{
			for (unsigned int i = 0; i < 3; i++)
			{
				for (unsigned int j = 0; j < 3; j++)
				{
					// Fill in the rotation matrix
					if (i == ((dim + 1) % 3) && j == ((dim + 1) % 3) || i == ((dim + 2) % 3) && j == ((dim + 2) % 3))
					{
						rotMat[i][j] = cos(theta);
					}
					else if (i == ((dim + 1) % 3) && j == ((dim + 2) % 3))
					{
						rotMat[i][j] = -sin(theta);
					}
					else if (i == ((dim + 2) % 3) && j == ((dim + 1) % 3))
					{
						rotMat[i][j] = sin(theta);
					}
				}
			}
		}
	}
	return rotMat;
}

vector<vector<float>> getTranslationMatrix(float translation[])
{
	vector<vector<float>> transMat = eye(4);
	for (unsigned int i = 0; i < 3; i++)
	{
		// Add the translation value for dim i
		transMat[i][3] = translation[i];
	}
	return transMat;
}

vector<vector<float>> getHomogeneousMatrix(unsigned int dim, float theta, float translation[])
{
	vector<vector<float>> transformMat = eye(4);
	float cosVal = cos(theta), sinVal = sin(theta);
	// Make sure a valid axis was provided for rotation
	if (dim < 3)
	{
		for (unsigned int i = 0; i < (transformMat.size() - 1); i++)
		{
			for (unsigned int j = 0; j < transformMat[i].size(); j++)
			{
				if (j == 3)
				{
					// Add the translation value
					transformMat[i][j] = translation[i];
				}
				else
				{
					// Add the rotation value
					if (i == ((dim + 1) % 3) && j == ((dim + 1) % 3) || i == ((dim + 2) % 3) && j == ((dim + 2) % 3))
					{
						transformMat[i][j] = cosVal;
					}
					else if (i == ((dim + 1) % 3) && j == ((dim + 2) % 3))
					{
						transformMat[i][j] = -sinVal;
					}
					else if (i == ((dim + 2) % 3) && j == ((dim + 1) % 3))
					{
						transformMat[i][j] = sinVal;
					}
				}
			}
		}
	}
	return transformMat;
}

vector<vector<float>> getInverseHomogeneousMatrix(unsigned int dim, float theta, float translation[])
{
	vector<vector<float>> invTransformMat = eye(4), invTranslation = zeros(3, 1), invRotationMat = getRotationMatrix(dim, theta, 3);
	float cosVal = cos(theta), sinVal = -sin(theta);
	// Get the inverse translation amount for after rotating
	for (unsigned int i = 0; i < 3; i++)
	{
		invTranslation[i][0] = translation[i];
	}
	invTranslation = matrixMultiply(invRotationMat, invTranslation);
	// Make sure a valid axis was provided for rotation
	if (dim < 3)
	{
		for (unsigned int i = 0; i < (invTransformMat.size() - 1); i++)
		{
			for (unsigned int j = 0; j < invTransformMat[i].size(); j++)
			{
				if (j == 3)
				{
					// Add the translation value
					invTransformMat[i][j] = invTranslation[i][0];
				}
				else
				{
					// Add the rotation value
					if (i == ((dim + 1) % 3) && j == ((dim + 1) % 3) || i == ((dim + 2) % 3) && j == ((dim + 2) % 3))
					{
						invTransformMat[i][j] = cosVal;
					}
					else if (i == ((dim + 1) % 3) && j == ((dim + 2) % 3))
					{
						invTransformMat[i][j] = -sinVal;
					}
					else if (i == ((dim + 2) % 3) && j == ((dim + 1) % 3))
					{
						invTransformMat[i][j] = sinVal;
					}
				}
			}
		}
	}
	return invTransformMat;
}

// Testing Matrix Multiplication
bool testMatrixMultiply(vector<vector<float>> pose, unsigned int axis, float theta, float translation[], unsigned int mode)
{
	// Declare variables
	bool valid = pose.size() == 3 || pose.size() == 4; // Check the first dim of the input matrix
	char dispDim[4] = { 'X', 'Y', 'Z', 'W' };
	// Get the rotation matrix
	vector<vector<float>> transformMat, result;
	if (mode == 0)
	{
		transformMat = getRotationMatrix(axis, theta);
	}
	else
	{
		transformMat = getTranslationMatrix(translation);
	}
	// Check the 2nd dim of the input matrix
	for (unsigned int i = 0; i < pose.size() && valid; i++)
	{
		valid = pose[i].size() == 1;
	}
	// Make sure the transform size matches the pose size
	if (valid)
	{
		valid = transformMat.size() == pose.size();
	}
	// Make sure that the input is valid
	if (valid)
	{
		// Perform multiplication
		result = matrixMultiply(transformMat, pose);
		if (result.size() > 0)
		{
			// Display the result
			cout << "Result:" << endl;
			for (unsigned int i = 0; i < result.size(); i++)
			{
				cout << dispDim[i] << ": ";
				for (unsigned int j = 0; j < result[i].size(); j++)
				{
					cout << result[i][0] << " ";
				}
				cout << endl;
			}
			cout << endl;
		}
		else
		{
			cout << "Error: Multiplication Failed" << endl;
		}
	}
	else
	{
		cout << "Error: Pose parameter must be 3x1 matrix" << endl;
	}
	return valid;
}
