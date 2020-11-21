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
#include "TFileException.h"

namespace chengine
{
	TFileException::TFileException(chcore::EGeneralErrors eErrorCode, DWORD dwNativeErrorCode, const chcore::TSmartPath& path, const wchar_t* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction) :
		TCoreException(eErrorCode, pszMsg, pszFile, stLineNumber, pszFunction),
		m_dwNativeErrorCode(dwNativeErrorCode),
		m_path(path)
	{
	}

	void TFileException::GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
	{
		_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("%s (error code: %d, win32 error code: %lu, path: %s)"), m_strMsg.c_str(), m_eErrorCode, m_dwNativeErrorCode, m_path.ToString());
		pszBuffer[stMaxBuffer - 1] = _T('\0');
	}

	void TFileException::GetDetailedErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
	{
		_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("%s\r\nError code: %d\r\nWin32 error code: %lu\r\nFile: %s\r\nSource file: %s\r\nFunction: %s\r\nLine no: %lu"),
			m_strMsg.c_str(), m_eErrorCode, m_dwNativeErrorCode, m_path.ToString(), m_pszFile, m_pszFunction, (unsigned long)m_stLineNumber);
		pszBuffer[stMaxBuffer - 1] = _T('\0');
	}
}
