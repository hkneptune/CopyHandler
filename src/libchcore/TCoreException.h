/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
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
#ifndef __TEXCEPTION_H__
#define __TEXCEPTION_H__

#include "libchcore.h"
#include "ErrorCodes.h"
#include "TBaseException.h"

namespace chcore
{
	// throws core exception object
#define THROW_CORE_EXCEPTION(error_code)\
	throw TCoreException(error_code, __FILEW__, __LINE__, __FUNCTIONW__)

#define THROW_CORE_EXCEPTION_STD(error_code, std_exception)\
	throw TCoreException(error_code, std_exception, __FILEW__, __LINE__, __FUNCTIONW__)

#define THROW_CORE_EXCEPTION_WIN32(error_code, win32_error_code)\
	throw TCoreWin32Exception(error_code, win32_error_code, __FILEW__, __LINE__, __FUNCTIONW__)

	class LIBCHCORE_API TCoreException : public TBaseException
	{
	public:
		TCoreException(EGeneralErrors eErrorCode, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction);
		TCoreException(EGeneralErrors eErrorCode, std::exception& stdException, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction);

	private:
		TCoreException();
	};

	class LIBCHCORE_API TCoreWin32Exception : public TBaseException
	{
	public:
		TCoreWin32Exception(EGeneralErrors eErrorCode, DWORD dwWin32Exception, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction);

		DWORD GetWin32ErrorCode() const { return m_dwWin32ErrorCode; }

		virtual void GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const;
		virtual void GetDetailedErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const;

	private:
		TCoreWin32Exception();

	protected:
		// what happened?
		DWORD m_dwWin32ErrorCode;
	};
}

#endif
