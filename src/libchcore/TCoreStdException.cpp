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
#include "TCoreStdException.h"

namespace chcore
{
	// ============================================================================
	/// TCoreException::TCoreException
	/// @date 2009/11/30
	///
	/// @brief     Constructs core exception object with additional data.
	/// @param[in] eErrorCode -        error code
	/// @param[in] stdException -      standard exception info
	/// @param[in] pszFile -           source file name
	/// @param[in] stLineNumber -      source line number
	/// @param[in] pszFunction -       function name in which the problem occurred.
	// ============================================================================
	TCoreStdException::TCoreStdException(EGeneralErrors eErrorCode, std::exception& stdException, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction) :
		TCoreException(eErrorCode, stdException.what(), pszFile, stLineNumber, pszFunction)
	{
	}
}
