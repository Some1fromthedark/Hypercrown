#include "PS2Textures.h"

unsigned int gsmem[1024 * 1024]; 

void writeTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	unsigned int *src = (unsigned int *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 32;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 32);

			int blockX = px / 8;
			int blockY = py / 8;
			int block  = block32[blockX + blockY * 8];

			int bx = px - blockX * 8;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord32[cx + cy * 8];

			gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw] = *src;
			src++;
		}
	}
}

void readTexPSMCT32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	unsigned int *src = (unsigned int *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 32;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 32);

			int blockX = px / 8;
			int blockY = py / 8;
			int block  = block32[blockX + blockY * 8];

			int bx = px - blockX * 8;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord32[cx + cy * 8];

			*src = gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			src++;
		}
	}
}

void writeTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	unsigned int *src = (unsigned int *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 32;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 32);

			int blockX = px / 8;
			int blockY = py / 8;
			int block  = blockZ32[blockX + blockY * 8];

			int bx = px - blockX * 8;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ32[cx + cy * 8];

			gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw] = *src;
			src++;
		}
	}
}

void readTexPSMZ32(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	unsigned int *src = (unsigned int *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 32;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 32);

			int blockX = px / 8;
			int blockY = py / 8;
			int block  = blockZ32[blockX + blockY * 8];

			int bx = px - blockX * 8;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ32[cx + cy * 8];

			*src = gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			src++;
		}
	}
}

void writeTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = block16[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord16[cx + cy * 16];
			int ch = columnHalf16[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[ch] = *src;
			src++;
		}
	}
}

void readTexPSMCT16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = block16[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord16[cx + cy * 16];
			int ch = columnHalf16[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[ch];
			src++;
		}
	}
}

void writeTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = blockZ16[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ16[cx + cy * 16];
			int ch = columnHalfZ16[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[ch] = *src;
			src++;
		}
	}
}

void readTexPSMZ16(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = blockZ16[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ16[cx + cy * 16];
			int ch = columnHalfZ16[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[ch];
			src++;
		}
	}
}

void writeTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = blockZ16S[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ16S[cx + cy * 16];
			int ch = columnHalfZ16S[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[ch] = *src;
			src++;
		}
	}
}

void readTexPSMZ16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = blockZ16S[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWordZ16S[cx + cy * 16];
			int ch = columnHalfZ16S[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[ch];
			src++;
		}
	}
}

void writeTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = block16S[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord16S[cx + cy * 16];
			int ch = columnHalf16S[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[ch] = *src;
			src++;
		}
	}
}

void readTexPSMCT16S(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	//dbw >>= 1;
	unsigned short *src = (unsigned short *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 64;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 64);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 8;
			int block  = block16S[blockX + blockY * 4];

			int bx = px - blockX * 16;
			int by = py - blockY * 8;

			int column = by / 2;

			int cx = bx;
			int cy = by - column * 2;
			int cw = columnWord16S[cx + cy * 16];
			int ch = columnHalf16S[cx + cy * 16];

			unsigned short *dst = (unsigned short *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[ch];
			src++;
		}
	}
}

void writeTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	dbw >>= 1;
	unsigned char *src = (unsigned char *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 128;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 128);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 16;
			int block  = block8[blockX + blockY * 8];

			int bx = px - (blockX * 16);
			int by = py - (blockY * 16);

			int column = by / 4;

			int cx = bx;
			int cy = by - column * 4;
			int cw = columnWord8[column & 1][cx + cy * 16];
			int cb = columnByte8[cx + cy * 16];

			unsigned char *dst = (unsigned char *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			dst[cb] = *src++;
		}
	}
}

void readTexPSMT8(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
    dbw >>= 1;
	unsigned char *src = (unsigned char *)data;
	int startBlockPos = dbp * 64;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 128;
			int pageY = y / 64;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 128);
			int py = y - (pageY * 64);

			int blockX = px / 16;
			int blockY = py / 16;
			int block  = block8[blockX + blockY * 8];

			int bx = px - blockX * 16;
			int by = py - blockY * 16;

			int column = by / 4;

			int cx = bx;
			int cy = by - column * 4;
			int cw = columnWord8[column & 1][cx + cy * 16];
			int cb = columnByte8[cx + cy * 16];

			unsigned char *dst = (unsigned char *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];
			*src = dst[cb];
			src++;
		}
	}
}

void writeTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	dbw >>= 1;
	unsigned char *src = (unsigned char *)data;
	int startBlockPos = dbp * 64;

	bool odd = false;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 128;
			int pageY = y / 128;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 128);
			int py = y - (pageY * 128);

			int blockX = px / 32;
			int blockY = py / 16;
			int block  = block4[blockX + blockY * 4];

			int bx = px - blockX * 32;
			int by = py - blockY * 16;

			int column = by / 4;

			int cx = bx;
			int cy = by - column * 4;
			int cw = columnWord4[column & 1][cx + cy * 32];
			int cb = columnByte4[cx + cy * 32];

			unsigned char *dst = (unsigned char *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];

			if(cb & 1)
			{
				if(odd)
					dst[cb >> 1] = (dst[cb >> 1] & 0x0f) | ((*src) & 0xf0);
				else
					dst[cb >> 1] = (dst[cb >> 1] & 0x0f) | (((*src) << 4) & 0xf0);
			}
			else
			{
				if(odd)
					dst[cb >> 1] = (dst[cb >> 1] & 0xf0) | (((*src) >> 4) & 0x0f);
				else
					dst[cb >> 1] = (dst[cb >> 1] & 0xf0) | ((*src) & 0x0f);
			}

			if(odd)
				src++;

			odd = !odd;
		}
	}
}

void readTexPSMT4(int dbp, int dbw, int dsax, int dsay, int rrw, int rrh, void *data)
{
	dbw >>= 1;
	unsigned char *src = (unsigned char *)data;
	int startBlockPos = dbp * 64;

	bool odd = false;

	for(int y = dsay; y < dsay + rrh; y++)
	{
		for(int x = dsax; x < dsax + rrw; x++)
		{
			int pageX = x / 128;
			int pageY = y / 128;
			int page  = pageX + pageY * dbw;

			int px = x - (pageX * 128);
			int py = y - (pageY * 128);

			int blockX = px / 32;
			int blockY = py / 16;
			int block  = block4[blockX + blockY * 4];

			int bx = px - blockX * 32;
			int by = py - blockY * 16;

			int column = by / 4;

			int cx = bx;
			int cy = by - column * 4;
			int cw = columnWord4[column & 1][cx + cy * 32];
			int cb = columnByte4[cx + cy * 32];

			unsigned char *dst = (unsigned char *)&gsmem[startBlockPos + page * 2048 + block * 64 + column * 16 + cw];

			if(cb & 1)
			{
				if(odd)
					*src = ((*src) & 0x0f) | (dst[cb >> 1] & 0xf0);
				else
					*src = ((*src) & 0xf0) | ((dst[cb >> 1] >> 4) & 0x0f);
			}
			else
			{
				if(odd)
					*src = ((*src) & 0x0f) | ((dst[cb >> 1] << 4) & 0xf0);
				else
					*src = ((*src) & 0xf0) | (dst[cb >> 1] & 0x0f);
			}

			if(odd)
				src++;

			odd = !odd;
		}
	}
}
