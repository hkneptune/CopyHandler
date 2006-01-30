#include "stdafx.h"
#include "crypt.h"
#include "rijndael-api-fst.h"
#include "conv.h"
#include "sha256.h"

// output buffer might be in size+16
int AES256Crypt(void* pIn, int iCnt, PCTSTR pszPass, void* pOut)
{
	// init the cipher
	cipherInstance ci;
	int iRes;
	if ((iRes=cipherInit(&ci, MODE_ECB, NULL)) < 0)
	{
		TRACE("cipherInit in ::AES256Crypt failed\n");
		return iRes;
	}

	// make the key
	keyInstance ki;
	if ((iRes=makeKey(&ki, DIR_ENCRYPT, 256, pszPass)) < 0)
	{
		TRACE("makeKey in ::AES256Crypt failed\n");
		return iRes;
	}

	// now encrypt the data
	int iEncCount;
	if ((iEncCount=padEncrypt(&ci, &ki, (BYTE*)pIn, iCnt, (BYTE*)pOut)) < 0)
	{
		TRACE("padEncrypt in ::AES256Crypt failed\n");
		return iEncCount;
	}

	return iEncCount;
}

// output buffer has to have iCnt size
int AES256Decrypt(void* pIn, int iCnt, PCTSTR pszPass, void* pOut)
{
	// init the cipher
	cipherInstance ci;
	int iRes;
	if ((iRes=cipherInit(&ci, MODE_ECB, NULL)) < 0)
	{
		TRACE("cipherInit in ::AES256Decrypt failed\n");
		return iRes;
	}

	// make the key
	keyInstance ki;
	if ((iRes=makeKey(&ki, DIR_DECRYPT, 256, pszPass)) < 0)
	{
		TRACE("makeKey in ::AES256Decrypt failed\n");
		return iRes;
	}

	// decrypt the data
    if ((iRes=padDecrypt(&ci, &ki, (BYTE*)pIn, iCnt, (BYTE*)pOut)) <= 0)
	{
		TRACE("padDecrypt in ::AES256Decrypt failed\n");
		return iRes;
	}

	return iRes;
}

// out buffer has to be of size ((_tcslen(pszIn)+1)*sizeof(TCHAR)+16)*2
int AES256CipherString(PCTSTR pszIn, PCTSTR pszPass, PTSTR pszOut)
{
	// init the cipher
	cipherInstance ci;
	int iRes;
	if ((iRes=cipherInit(&ci, MODE_ECB, NULL)) < 0)
	{
		TRACE("cipherInit in ::AES256CipherString failed\n");
		return iRes;
	}

	// make the key
	keyInstance ki;
	if ((iRes=makeKey(&ki, DIR_ENCRYPT, 256, pszPass)) < 0)
	{
		TRACE("makeKey in ::AES256CipherString failed\n");
		return iRes;
	}

	// count of data to encrypt, buffer for encrypted output
	size_t tLen=_tcslen(pszIn)+1;
	TCHAR *pszData=new TCHAR[tLen+16];		// for padding

	// now encrypt the data
	int iEncCount;
	if ((iEncCount=padEncrypt(&ci, &ki, (BYTE*)pszIn, (int)tLen*sizeof(TCHAR), (BYTE*)pszData)) < 0)
	{
		delete [] pszData;
		TRACE("padEncrypt in ::AES256CipherString failed\n");
		return iEncCount;
	}

	// make the data hex
	Bin2Hex((BYTE*)pszData, iEncCount, pszOut);
	pszOut[iEncCount*2]=_T('\0');

	// delete the old stuff
	delete [] pszData;

	return iEncCount*2;
}

// out buffer size must be _tcslen(pszIn)/2
int AES256DecipherString(PCTSTR pszIn, PCTSTR pszPass, PTSTR pszOut)
{
	// count of input data - check if there is anything to process
	size_t tLen=_tcslen(pszIn);

	// init the cipher
	cipherInstance ci;
	int iRes;
	if ((iRes=cipherInit(&ci, MODE_ECB, NULL)) < 0)
	{
		TRACE("cipherInit in ::AES256DecipherString failed\n");
		return iRes;
	}

	// make the key
	keyInstance ki;
	if ((iRes=makeKey(&ki, DIR_DECRYPT, 256, pszPass)) < 0)
	{
		TRACE("makeKey in ::AES256DecipherString failed\n");
		return iRes;
	}

	// buffer for hex data
	BYTE *pby=new BYTE[tLen/2];
	
	// decode hex data
	if (!Hex2Bin(pszIn, tLen, pby))
	{
		TRACE("Hex2Bin in ::AES256DecipherString failed\n");
		delete [] pby;
		return BAD_DATA;
	}

	// decrypt the data
    if ((iRes=padDecrypt(&ci, &ki, pby, (int)tLen/2, (BYTE*)(pszOut))) <= 0)
	{
		TRACE("padDecrypt in ::AES256DecipherString failed\n");

		delete [] pby;
		return iRes;
	}

	// delete buffers
	delete [] pby;

	return (int)_tcslen(pszOut);
}

// pszKey of size 64+1B
void StringToKey256(PCTSTR pszString, TCHAR *pszKey)
{
	// generate the SHA256 from this password
	BYTE byData[32];
	sha256_context sc;
	sha256_starts(&sc);
	sha256_update(&sc, (BYTE*)pszString, (ULONG)_tcslen(pszString)*sizeof(TCHAR));
	sha256_finish(&sc, byData);

	// now the hexed hash
	Bin2Hex(byData, 32, pszKey);
	pszKey[64]=_T('\0');
}
