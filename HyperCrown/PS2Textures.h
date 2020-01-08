#pragma once

#include <stdio.h>
#include <malloc.h>
#include <memory.h>

const int block32[32] =
{
	 0,  1,  4,  5, 16, 17, 20, 21,
	 2,  3,  6,  7, 18, 19, 22, 23,
	 8,  9, 12, 13, 24, 25, 28, 29,
	10, 11, 14, 15, 26, 27, 30, 31
};


const int columnWord32[16] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15
};

const int blockZ32[32] =
{
	 24, 25, 28, 29, 8, 9, 12, 13,
	 26, 27, 30, 31,10, 11, 14, 15,
	 16, 17, 20, 21, 0, 1, 4, 5,
	 18, 19, 22, 23, 2, 3, 6, 7
};

const int columnWordZ32[16] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15
};

const int block16[32] =
{
	 0,  2,  8, 10,
	 1,  3,  9, 11,
	 4,  6, 12, 14,
	 5,  7, 13, 15,
	16, 18, 24, 26,
	17, 19, 25, 27,
	20, 22, 28, 30,
	21, 23, 29, 31
};

const int columnWord16[32] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
};

const int columnHalf16[32] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1
};

const int blockZ16[32] =
{
	 24,  26,  16, 18,
	 25,  27,  17, 19,
	28,  30, 20, 22,
	 29,  31, 21, 23,
	8, 10, 0, 2,
	9, 11, 1, 3,
	12, 14, 4, 6,
	13, 15, 5, 7
};

const int columnWordZ16[32] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
};

const int columnHalfZ16[32] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1
};

const int blockZ16S[32] =
{
	 24,  26,  8, 10,
	 25,  27,  9, 11,
	 16,  18, 0, 2,
	 17,  19, 1, 3,
	28, 30, 12, 14,
	29, 31, 13, 15,
	20, 22, 4, 6,
	21, 23, 5, 7
};

const int columnWordZ16S[32] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
};

const int columnHalfZ16S[32] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1
};

const int block16S[32] =
{
	 0,  2, 16, 18,
	 1,  3, 17, 19,
	 8, 10, 24, 26,
	 9, 11, 25, 27,
	 4,  6, 20, 22,
	 5,  7, 21, 23,
	12, 14, 28, 30,
	13, 15, 29, 31
};

const int columnWord16S[32] =
{
	 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
	 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
};

const int columnHalf16S[32] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0,  1, 1, 1, 1, 1, 1, 1, 1
};

const int block8[32] =
{
	 0,  1,  4,  5, 16, 17, 20, 21,
	 2,  3,  6,  7, 18, 19, 22, 23,
	 8,  9, 12, 13, 24, 25, 28, 29,
	10, 11, 14, 15, 26, 27, 30, 31
};

const int columnWord8[2][64] =
{
	{
		 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
		 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,

		 8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,
		10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7
	},
	{
		 8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,
		10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,

		 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
		 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
	}
};

const int columnByte8[64] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  2, 2, 2, 2, 2, 2, 2, 2,
	0, 0, 0, 0, 0, 0, 0, 0,  2, 2, 2, 2, 2, 2, 2, 2,

	1, 1, 1, 1, 1, 1, 1, 1,  3, 3, 3, 3, 3, 3, 3, 3,
	1, 1, 1, 1, 1, 1, 1, 1,  3, 3, 3, 3, 3, 3, 3, 3
};

const int block4[32] =
{
	 0,  2,  8, 10,
	 1,  3,  9, 11,
	 4,  6, 12, 14,
	 5,  7, 13, 15,
	16, 18, 24, 26,
	17, 19, 25, 27,
	20, 22, 28, 30,
	21, 23, 29, 31
};

const int columnWord4[2][128] =
{
	{
		 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
		 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,

		 8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,
		10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7
	},
	{
		 8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,   8,  9, 12, 13,  0,  1,  4,  5,
		10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,  10, 11, 14, 15,  2,  3,  6,  7,

		 0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,   0,  1,  4,  5,  8,  9, 12, 13,
		 2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15,   2,  3,  6,  7, 10, 11, 14, 15
	}
};

const int columnByte4[128] =
{
	0, 0, 0, 0, 0, 0, 0, 0,  2, 2, 2, 2, 2, 2, 2, 2,  4, 4, 4, 4, 4, 4, 4, 4,  6, 6, 6, 6, 6, 6, 6, 6,
	0, 0, 0, 0, 0, 0, 0, 0,  2, 2, 2, 2, 2, 2, 2, 2,  4, 4, 4, 4, 4, 4, 4, 4,  6, 6, 6, 6, 6, 6, 6, 6,

	1, 1, 1, 1, 1, 1, 1, 1,  3, 3, 3, 3, 3, 3, 3, 3,  5, 5, 5, 5, 5, 5, 5, 5,  7, 7, 7, 7, 7, 7, 7, 7,
	1, 1, 1, 1, 1, 1, 1, 1,  3, 3, 3, 3, 3, 3, 3, 3,  5, 5, 5, 5, 5, 5, 5, 5,  7, 7, 7, 7, 7, 7, 7, 7
};

// Display buffer modes
void writeTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void readTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void writeTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void readTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void writeTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void readTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

// Z Buffer modes
void readTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void writeTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void readTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void writeTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void readTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void writeTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

// Texture modes
void writeTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void readTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);

void writeTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
void readTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data);
