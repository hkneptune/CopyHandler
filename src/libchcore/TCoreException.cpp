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
	m_eErrorCode(eErrorCode),
	m_pszFile(pszFile),
	m_stLineNumber(stLineNumber),
	m_pszFunction(pszFunction)
{
	BOOST_ASSERT(false);
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
	std::exception(stdException),
	m_eErrorCode(eErrorCode),
	m_pszFile(pszFile),
	m_stLineNumber(stLineNumber),
	m_pszFunction(pszFunction)
{
}

void TCoreException::GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
{
	_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("Error code: %ld\r\nFile: %s\r\nFunction: %s\r\nLine no: %lu"), m_eErrorCode, m_pszFile, m_pszFunction, m_stLineNumber);
	pszBuffer[stMaxBuffer - 1] = _T('\0');
}

END_CHCORE_NAMESPACE
