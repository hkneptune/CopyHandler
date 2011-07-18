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

BEGIN_CHCORE_NAMESPACE

// throws core exception object
#define THROW_CORE_EXCEPTION(error_code)\
	throw TCoreException(error_code, __FILEW__, __LINE__, __FUNCTIONW__)

#define THROW_CORE_EXCEPTION_STD(error_code, std_exception)\
	throw TCoreException(error_code, std_exception, __FILEW__, __LINE__, __FUNCTIONW__)

#define THROW_CORE_EXCEPTION_WIN32(error_code, win32_error_code)\
	throw TCoreWin32Exception(error_code, win32_error_code, __FILEW__, __LINE__, __FUNCTIONW__)

class LIBCHCORE_API TCoreException : public virtual std::exception
{
public:
	TCoreException(EGeneralErrors eErrorCode, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction);
	TCoreException(EGeneralErrors eErrorCode, std::exception& stdException, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction);

	// error information
	EGeneralErrors GetErrorCode() const { return m_eErrorCode; }

	// location info
	const wchar_t* GetSourceFile() const { return m_pszFile; }
	size_t GetSourceLineNumber() const { return m_stLineNumber; }
	const wchar_t* GetFunctionName() const { return m_pszFunction; }

	void GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const;

private:
	TCoreException();

protected:
	// what happened?
	EGeneralErrors m_eErrorCode;

	// where it happened?
	const wchar_t* m_pszFile;
	const wchar_t* m_pszFunction;
	size_t m_stLineNumber;
};

class LIBCHCORE_API TCoreWin32Exception : public TCoreException
{
public:
	TCoreWin32Exception(EGeneralErrors eErrorCode, DWORD dwWin32Exception, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction);

	DWORD GetWin32ErrorCode() const { return m_dwWin32ErrorCode; }

	void GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const;

private:
	TCoreWin32Exception();

protected:
	// what happened?
	EGeneralErrors m_eErrorCode;
	DWORD m_dwWin32ErrorCode;

	// where it happened?
	const wchar_t* m_pszFile;
	const wchar_t* m_pszFunction;
	size_t m_stLineNumber;
};

END_CHCORE_NAMESPACE

#endif
