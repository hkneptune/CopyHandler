#include "stdafx.h"
#include "conv.h"

TCHAR __hex[16] = { _T('0'), _T('1'), _T('2'), _T('3'), _T('4'), _T('5'), _T('6'), _T('7'), _T('8'), _T('9'), _T('a'), _T('b'), _T('c'), _T('d'), _T('e'), _T('f'), };

void Bin2Hex(const BYTE* pbyIn, size_t tInCount, TCHAR *pszOut)
{
	for (size_t i=0;i<tInCount;i++)
	{
		*pszOut++=__hex[(pbyIn[i] >> 4) & 0x0f];
		*pszOut++=__hex[pbyIn[i] & 0x0f];
	}
}

bool Hex2Bin(PCTSTR pszIn, size_t tInCount, BYTE* pbyOut)
{
	// we can pass -1 as in size - count it then
	if (tInCount == -1)
		tInCount=_tcslen(pszIn);

	// make sure the tInCount is even
	tInCount &= ~((size_t)1);
	BYTE by;
	for (size_t i=0;i<tInCount;i+=2)
	{
		// msb 4 bits
		if (*pszIn >= '0' && *pszIn <= '9')
			by=(*pszIn - '0') << 4;
		else if (*pszIn >= 'a' && *pszIn <= 'f')
			by=(*pszIn - 'a' + 10) << 4;
		else if (*pszIn >= 'A' && *pszIn <= 'F')
			by=(*pszIn - 'A' + 10) << 4;
		else
			return false;

		// lsb 4bits
		*pszIn++;
		if (*pszIn >= '0' && *pszIn <= '9')
			by|=(*pszIn - '0');
		else if (*pszIn >= 'a' && *pszIn <= 'f')
			by|=(*pszIn - 'a' + 10);
		else if (*pszIn >= 'A' && *pszIn <= 'F')
			by|=(*pszIn - 'A' + 10);
		else
			return false;

		*pszIn++;
		*pbyOut++=by;
	}

	return true;
}
