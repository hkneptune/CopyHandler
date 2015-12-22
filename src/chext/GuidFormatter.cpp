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
#include "GuidFormatter.h"

std::wstring GuidFormatter::FormatGuid(const GUID& rGuid)
{
	const size_t stBufferSize = 64;
	wchar_t szData[ stBufferSize ];

	_snwprintf_s(szData, stBufferSize, L"{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
		rGuid.Data1, rGuid.Data2, rGuid.Data3,
		rGuid.Data4[ 0 ], rGuid.Data4[ 1 ], rGuid.Data4[ 2 ], rGuid.Data4[ 3 ],
		rGuid.Data4[ 4 ], rGuid.Data4[ 5 ], rGuid.Data4[ 6 ], rGuid.Data4[ 7 ]);

	return szData;
}
