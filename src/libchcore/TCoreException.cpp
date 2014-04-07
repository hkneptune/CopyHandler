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
#include "stdafx.h"
#include "TCoreException.h"
#include <atltrace.h>

BEGIN_CHCORE_NAMESPACE

// ============================================================================
/// chcore::TCoreException::TCoreException
/// @date 2009/11/30
///
/// @brief     Constructs core exception object with additional data.
/// @param[in] eErrorCode -	       error code
/// @param[in] pszFile -           source file name
/// @param[in] stLineNumber -      source line number
/// @param[in] pszFunction -       function name in which the problem occured.
// ============================================================================
TCoreException::TCoreException(EGeneralErrors eErrorCode, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction) :
	TBaseException(eErrorCode, _T(""), pszFile, stLineNumber, pszFunction)
{
}

// ============================================================================
/// chcore::TCoreException::TCoreException
/// @date 2009/11/30
///
/// @brief     Constructs core exception object with additional data.
/// @param[in] eErrorCode -        error code
/// @param[in] stdException -      standard exception info
/// @param[in] pszFile -           source file name
/// @param[in] stLineNumber -      source line number
/// @param[in] pszFunction -       function name in which the problem occured.
// ============================================================================
TCoreException::TCoreException(EGeneralErrors eErrorCode, std::exception& stdException, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction) :
	TBaseException(eErrorCode, stdException.what(), pszFile, stLineNumber, pszFunction)
{
}

// ============================================================================
/// chcore::TCoreWin32Exception::TCoreWin32Exception
/// @date 2011/07/18
///
/// @brief     Constructs core win32 exception.
/// @param[in] eErrorCode - core error code
/// @param[in] dwWin32Exception - win32 error code
/// @param[in] pszFile -source file where the exception was thrown
/// @param[in] stLineNumber - source code line number where the exception was thrown
/// @param[in] pszFunction - function throwing the exception
// ============================================================================
TCoreWin32Exception::TCoreWin32Exception(EGeneralErrors eErrorCode, DWORD dwWin32Exception, const tchar_t* pszFile, size_t stLineNumber, const tchar_t* pszFunction) :
	TBaseException(eErrorCode, _T(""), pszFile, stLineNumber, pszFunction),
	m_dwWin32ErrorCode(dwWin32Exception)
{
}

// ============================================================================
/// chcore::TCoreWin32Exception::GetErrorInfo
/// @date 2011/07/18
///
/// @brief     Retrieves formatted exception information.
/// @param[in] pszBuffer - buffer for formatted string
/// @param[in] stMaxBuffer - max size of buffer
// ============================================================================
void TCoreWin32Exception::GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
{
	_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("Error code: %ld (win32 error code: %lu)"), m_eErrorCode, m_dwWin32ErrorCode);
	pszBuffer[stMaxBuffer - 1] = _T('\0');
}

void TCoreWin32Exception::GetDetailedErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
{
	_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("Error code: %ld\r\nWin32 error code: %lu\r\nFile: %s\r\nFunction: %s\r\nLine no: %lu"), m_eErrorCode, m_dwWin32ErrorCode, m_pszFile, m_pszFunction, (unsigned long)m_stLineNumber);
	pszBuffer[stMaxBuffer - 1] = _T('\0');
}

END_CHCORE_NAMESPACE
