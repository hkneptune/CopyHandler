#ifndef __CONV_H__
#define __CONV_H__

void Bin2Hex(const BYTE* pbyIn, size_t tInCount, TCHAR *pszOut);
bool Hex2Bin(PCTSTR pszIn, size_t tInCount, BYTE* pbyOut);

#endif