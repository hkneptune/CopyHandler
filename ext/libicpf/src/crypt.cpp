/***************************************************************************
 *   Copyright (C) 2004 by Józef Starosczyk                                *
 *   copyhandler@o2.pl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "crypt.h"
#include "rijndael-api-fst.h"
#include "conv.h"
#include "sha256.h"
#include "exception.h"
#include "err_codes.h"
#include <string.h>

#ifdef USE_ENCRYPTION

BEGIN_ICPF_NAMESPACE

// output buffer might be in size+16
LIBICPF_API int_t crypt_aes256(const ptr_t pIn, int_t iCnt, const char_t* pszPass, ptr_t pOut)
{
	// init the cipher
	cipherInstance ci;
	int_t iRes;
	if ((iRes=cipherInit(&ci, MODE_ECB, NULL)) < 0)
		THROW(exception::format("cipherInit() in crypt_aes256() failed with result " IFMT, iRes), EE_INIT, 0, 0);

	// make the key
	keyInstance ki;
	if ((iRes=makeKey(&ki, DIR_ENCRYPT, 256, pszPass)) < 0)
		THROW(exception::format("makeKey in crypt_aes256() failed with result " IFMT, iRes), EE_CRYPT, 0, 0);

	// now encrypt the data
	if ((iRes=padEncrypt(&ci, &ki, (uchar_t*)pIn, iCnt, (uchar_t*)pOut)) < 0)
		THROW(exception::format("padEncrypt() in crypt_aes256() failed with result " IFMT, iRes), EE_CRYPT, 0, 0);

	return iRes;
}

// output buffer has to have iCnt size
LIBICPF_API int_t decrypt_aes256(const ptr_t pIn, int_t iCnt, const char_t* pszPass, ptr_t pOut)
{
	// init the cipher
	cipherInstance ci;
	int iRes;
	if ((iRes=cipherInit(&ci, MODE_ECB, NULL)) < 0)
		THROW(exception::format("cipherInit() in decrypt_aes256() failed with result " IFMT, iRes), EE_INIT, 0, 0);

	// make the key
	keyInstance ki;
	if ((iRes=makeKey(&ki, DIR_DECRYPT, 256, pszPass)) < 0)
		THROW(exception::format("makeKey in decrypt_aes256() failed with result " IFMT, iRes), EE_DECRYPT, 0, 0);

	// decrypt the data
    if ((iRes=padDecrypt(&ci, &ki, (uchar_t*)pIn, iCnt, (uchar_t*)pOut)) <= 0)
		THROW(exception::format("padEncrypt() in decrypt_aes256() failed with result " IFMT, iRes), EE_DECRYPT, 0, 0);

	return iRes;
}

// out buffer has to be of size ((_tcslen(pszIn)+1)*sizeof(char_t)+16)*2
LIBICPF_API int_t strcrypt_aes256(const char_t* pszIn, const char_t* pszPass, char_t* pszOut)
{
	// init the cipher
	cipherInstance ci;
	int_t iRes;
	if ((iRes=cipherInit(&ci, MODE_ECB, NULL)) < 0)
		THROW(exception::format("cipherInit() in strcrypt_aes256() failed with result " IFMT, iRes), EE_INIT, 0, 0);

	// make the key
	keyInstance ki;
	if ((iRes=makeKey(&ki, DIR_ENCRYPT, 256, pszPass)) < 0)
		THROW(exception::format("cipherInit() in strcrypt_aes256() failed with result " IFMT, iRes), EE_INIT, 0, 0);

	// count of data to encrypt, buffer for encrypted output
	size_t tLen=strlen(pszIn)+1;
	char_t *pszData=new char_t[tLen+16];		// for padding

	// now encrypt the data
	if ((iRes=padEncrypt(&ci, &ki, (uchar_t*)pszIn, (int_t)(tLen*sizeof(char_t)), (uchar_t*)pszData)) < 0)
	{
		delete [] pszData;
		THROW(exception::format("padEncrypt() in strcrypt_aes256() failed with result " IFMT, iRes), EE_DECRYPT, 0, 0);
	}

	// make the data hex
	bin2hex((uchar_t*)pszData, (uint_t)iRes, pszOut);
	pszOut[iRes*2]='\0';

	// delete the old stuff
	delete [] pszData;

	return iRes*2;
}

// out buffer size must be _tcslen(pszIn)/2
LIBICPF_API int_t strdecrypt_aes256(const char_t* pszIn, const char_t* pszPass, char_t* pszOut)
{
	// count of input data - check if there is anything to process
	size_t tLen=strlen(pszIn);

	// init the cipher
	cipherInstance ci;
	int iRes;
	if ((iRes=cipherInit(&ci, MODE_ECB, NULL)) < 0)
		THROW(exception::format("cipherInit() in crypt_aes256() failed with result " IFMT, iRes), EE_INIT, 0, 0);

	// make the key
	keyInstance ki;
	if ((iRes=makeKey(&ki, DIR_DECRYPT, 256, pszPass)) < 0)
		THROW(exception::format("cipherInit() in strcrypt_aes256() failed with result " IFMT, iRes), EE_INIT, 0, 0);

	// buffer for hex data
	uchar_t *pby=new uchar_t[tLen/2];
	
	// decode hex data
	if (!hex2bin(pszIn, (uint_t)tLen, pby))
	{
		delete [] pby;
		THROW("hex2bin in strdecrypt_aes256() failed", CE_HEX2BIN, 0, 0);
	}

	// decrypt the data
    if ((iRes=padDecrypt(&ci, &ki, pby, (int)tLen/2, (uchar_t*)(pszOut))) <= 0)
	{
		delete [] pby;
		THROW(exception::format("padDecrypt() in strdecrypt_aes256() failed with result " IFMT, iRes), EE_DECRYPT, 0, 0);
	}

	// delete buffers
	delete [] pby;

	return (int_t)strlen(pszOut);
}

// pszKey of size 64+1B
LIBICPF_API void str2key256(const char_t* pszString, char_t* pszKey)
{
	// generate the SHA256 from this password
	uchar_t byData[32];
	sha256_context sc;
	sha256_starts(&sc);
	sha256_update(&sc, (uchar_t*)pszString, (ulong_t)strlen(pszString)*sizeof(char_t));
	sha256_finish(&sc, byData);

	// now the hexed hash
	bin2hex(byData, 32, pszKey);
	pszKey[64]='\0';
}

END_ICPF_NAMESPACE

#endif
