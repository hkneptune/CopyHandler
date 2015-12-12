/***************************************************************************
*   Copyright (C) 2001-2013 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
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
#ifndef __TSTRINGEXCEPTION_H__
#define __TSTRINGEXCEPTION_H__

#include "libchcore.h"
#include "ErrorCodes.h"
#include "TBaseException.h"

#define THROW_STRING_EXCEPTION(error_code, err_msg)\
	throw TStringException(error_code, err_msg, __FILEW__, __LINE__, __FUNCTIONW__)

namespace chcore
{
	class LIBCHCORE_API TStringException : public TBaseException
	{
	public:
		TStringException(EGeneralErrors eErrorCode, const wchar_t* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction);
		TStringException(EGeneralErrors eErrorCode, const char* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction);

	private:
		TStringException();
	};
}

#endif
