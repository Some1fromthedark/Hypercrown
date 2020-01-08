#include"endian-utils.h"

unsigned short swapEndianness(unsigned short s)
{
	unsigned short d;
	char *sp = (char *)&s, *dp = (char *)&d;
	for (unsigned int i = 0; i < sizeof(unsigned short); i++)
	{
		dp[sizeof(unsigned short) - i - 1] = sp[i];
	}
	return d;
}

unsigned int swapEndianness(unsigned int s)
{
	unsigned int d;
	char *sp = (char *)&s, *dp = (char *)&d;
	for (unsigned int i = 0; i < sizeof(unsigned int); i++)
	{
		dp[sizeof(unsigned int) - i - 1] = sp[i];
	}
	return d;
}

unsigned long long swapEndianness(unsigned long long s)
{
	unsigned long long d;
	char *sp = (char *)&s, *dp = (char *)&d;
	for (unsigned int i = 0; i < sizeof(unsigned long long); i++)
	{
		dp[sizeof(unsigned long long) - i - 1] = sp[i];
	}
	return d;
}
