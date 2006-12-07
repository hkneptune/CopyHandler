/***************************************************************************
 *   Copyright (C) 2004-2006 by Józef Starosczyk                           *
 *   ixen@copyhandler.com                                                  *
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
#ifndef __CRYPT_H__
#define __CRYPT_H__

#include "libicpf.h"
#include "gen_types.h"

#ifdef USE_ENCRYPTION

BEGIN_ICPF_NAMESPACE

/// output buffer might be in size+16
LIBICPF_API int_t crypt_aes256(const ptr_t pIn, int_t iCnt, const char_t* pszPass, ptr_t pOut);

// output buffer has to have iCnt size
LIBICPF_API int_t decrypt_aes256(const ptr_t pIn, int_t iCnt, const char_t* pszPass, ptr_t pOut);

// out buffer has to be of size ((strlen(pszIn)+1)*sizeof(char_t)+16)*2
LIBICPF_API int_t strcrypt_aes256(const char_t* pszIn, const char_t* pszPass, char_t* pszOut);

// out buffer size must be strlen(pszIn)/2
LIBICPF_API int_t strdecrypt_aes256(const char_t* pszIn, const char_t* pszPass, char_t* pszOut);

// pszKey of size 64+1B
LIBICPF_API void str2key256(const char_t* pszString, char_t* pszKey);

END_ICPF_NAMESPACE

#endif

#endif
