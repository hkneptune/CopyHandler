// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#ifndef __TCOREWIN32EXCEPTION_H__
#define __TCOREWIN32EXCEPTION_H__

#include "TCoreException.h"

#define THROW_CORE_EXCEPTION_WIN32(error_code, win32_error_code)\
	throw chcore::TCoreWin32Exception(error_code, win32_error_code, L"", __FILEW__, __LINE__, __FUNCTIONW__)

#define THROW_CORE_EXCEPTION_WIN32_MSG(error_code, win32_error_code, msg)\
	throw chcore::TCoreWin32Exception(error_code, win32_error_code, msg, __FILEW__, __LINE__, __FUNCTIONW__)

namespace chcore
{
	class LIBCHCORE_API TCoreWin32Exception : public TCoreException
	{
	public:
		TCoreWin32Exception(EGeneralErrors eErrorCode, DWORD dwWin32Exception, const wchar_t* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction);

		DWORD GetWin32ErrorCode() const { return m_dwWin32ErrorCode; }

		virtual void GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const;
		virtual void GetDetailedErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const;

	protected:
		DWORD m_dwWin32ErrorCode;
	};
}

#endif
