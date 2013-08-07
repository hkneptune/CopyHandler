// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#ifndef __TSERIALIZEREXCEPTION_H__
#define __TSERIALIZEREXCEPTION_H__

#include "libchcore.h"
#include "TBaseException.h"

#define THROW_SERIALIZER_EXCEPTION(error_code, err_msg)\
	throw TSerializerException(error_code, err_msg, __FILEW__, __LINE__, __FUNCTIONW__)

BEGIN_CHCORE_NAMESPACE

class TSerializerException : public TBaseException
{
public:
	TSerializerException(EGeneralErrors eErrorCode, const wchar_t* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction);
	TSerializerException(EGeneralErrors eErrorCode, const char* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction);
};

END_CHCORE_NAMESPACE

#endif
