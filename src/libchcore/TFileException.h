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
#ifndef __TFILEWIN32EXCEPTION_H__
#define __TFILEWIN32EXCEPTION_H__

#include "ErrorCodes.h"
#include "TPath.h"
#include "TCoreException.h"

namespace chcore
{
	class LIBCHCORE_API TFileException : public TCoreException
	{
	public:
		TFileException(EGeneralErrors eErrorCode, DWORD dwNativeErrorCode, const TSmartPath& path, wchar_t* pszMsg,
			const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction);

		void GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const override;
		void GetDetailedErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const override;

		const TSmartPath& GetPath() const { return m_path; }
		DWORD GetNativeError() const { return m_dwNativeErrorCode; }

	private:
		DWORD m_dwNativeErrorCode;
		TSmartPath m_path;
	};
}

#endif
