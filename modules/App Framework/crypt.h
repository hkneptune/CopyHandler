#ifndef __CRYPT_H__
#define __CRYPT_H__

// output buffer might be in size+16
int AES256Crypt(void* pIn, int iCnt, PCTSTR pszPass, void* pOut);

// output buffer has to have iCnt size
int AES256Decrypt(void* pIn, int iCnt, PCTSTR pszPass, void* pOut);

// out buffer has to be of size ((_tcslen(pszIn)+1)*sizeof(TCHAR)+16)*2
int AES256CipherString(PCTSTR pszIn, PCTSTR pszPass, PTSTR pszOut);

// out buffer size must be _tcslen(pszIn)/2
int AES256DecipherString(PCTSTR pszIn, PCTSTR pszPass, PTSTR pszOut);

// pszKey of size 64+1B
void StringToKey256(PCTSTR pszString, TCHAR *pszKey);

#endif
