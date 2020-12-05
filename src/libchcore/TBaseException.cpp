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
#include "stdafx.h"
#include "TBaseException.h"
#include <atltrace.h>

namespace chcore
{
	TBaseException::TBaseException(EGeneralErrors eErrorCode, const wchar_t* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction) :
		m_eErrorCode(eErrorCode),
		m_strMsg(pszMsg ? pszMsg : L""),
		m_pszFile(pszFile),
		m_pszFunction(pszFunction),
		m_stLineNumber(stLineNumber)
	{
		ATLTRACE(_T("*** Base Exception is being thrown:\n\tMsg: %s\n\tError code: %d\n\tFile: %s\n\tLine number: %ld\n\tFunction: %s\n"), pszMsg, eErrorCode, pszFile, stLineNumber, pszFunction);
	}

	TBaseException::~TBaseException()
	{
	}

	void TBaseException::GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
	{
		_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("%s (error code: %d)"),
			m_strMsg.c_str(), m_eErrorCode);
		pszBuffer[stMaxBuffer - 1] = _T('\0');
	}

	void TBaseException::GetDetailedErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
	{
		_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("%s\r\nError code: %d\r\nFile: %s\r\nFunction: %s\r\nLine no: %lu"),
			m_strMsg.c_str(), m_eErrorCode, m_pszFile, m_pszFunction, (unsigned long)m_stLineNumber);
		pszBuffer[stMaxBuffer - 1] = _T('\0');
	}
}
