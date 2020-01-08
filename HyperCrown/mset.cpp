#include"mset.h"

using namespace std;

vector<string> getMsetAnims(string raw)
{
	vector<string> anims;
	unsigned int rawLen = raw.size();
	if (rawLen >= 20)
	{
		unsigned int readOffset = 0, numSections = *(unsigned int *)&(raw[0]);
		if (numSections == 3)
		{
			unsigned int sectionLen, sectionOffsets[3], sectionLengths[3];
			readOffset = 4;
			for (unsigned int i = 0; i < 3; i++)
			{
				sectionOffsets[i] = *(unsigned int *)&(raw[readOffset + i * 4]);
				sectionLengths[i] = *(unsigned int *)&(raw[readOffset + i * 4 + 4]);
				if (sectionLengths[i] != sectionOffsets[i] && sectionLengths[i] > rawLen)
				{
					sectionLengths[i] = rawLen;
				}
				sectionLengths[i] -= sectionOffsets[i];
			}
			readOffset = sectionOffsets[0];
			sectionLen = sectionLengths[0];
			if (rawLen >= readOffset + sectionLen && sectionLen >= 56)
			{
				unsigned int subsectionLen, subsectionOffsets[6], subsectionLengths[6];
				for (unsigned int i = 0; i < 6; i++)
				{
					subsectionOffsets[i] = raw[readOffset + i * 8 + 8];
					subsectionLengths[i] = raw[readOffset + i * 8 + 12];
				}
				readOffset += subsectionOffsets[5];
				subsectionLen = subsectionLengths[5];
				if (rawLen >= readOffset + subsectionLen)
				{
					vector<unsigned int> animOffsets;
					unsigned int readOffset2 = 0, animOffset = 0, numAnims;
					do
					{
						if ((readOffset2 + 4) <= subsectionLen)
						{
							animOffset = *(unsigned int *)&(raw[readOffset + readOffset2]);
							if (animOffset != 0xFFFFFFFF)
							{
								animOffsets.push_back(animOffset);
							}
						}
						else
						{
							animOffset = 0xFFFFFFFF;
						}
					} while (animOffset != 0xFFFFFFFF);
					numAnims = animOffsets.size();
					for (unsigned int i = 0; i < numAnims; i++)
					{
						unsigned int animLen;
						animOffset = animOffsets[i];
						if (i + 1 < numAnims)
						{
							animLen = animOffsets[i + 1] - animOffset;
						}
						else
						{
							animLen = subsectionLen - animOffset;
						}
						anims.push_back(raw.substr(readOffset + animOffset, animLen));
					}
				}
				else
				{
					cout << "Error: Invalid Animation Section Offset" << endl;
				}
			}
			else
			{
				cout << "Error: Invalid Section Offset for KNA" << endl;
			}
		}
		else
		{
			cout << "Error: Invalid MSET Header" << endl;
		}
	}
	else
	{
		cout << "Error: Missing MSET Header" << endl;
	}
	return anims;
}