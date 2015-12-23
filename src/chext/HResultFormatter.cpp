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
#include "HResultFormatter.h"
#include <boost\lexical_cast.hpp>
#include "..\libchcore\TWin32ErrorFormatter.h"

std::wstring HResultFormatter::FormatHResult(HRESULT hResult)
{
	std::wstringstream wstr;
	wstr << L"hr=0x" << std::hex << hResult;

	switch(hResult)
	{
	case S_OK:
		wstr << L" (S_OK)";
		break;
	case S_FALSE:
		wstr << L" (S_FALSE)";
		break;
	default:
		{
			chcore::TString strDesc = chcore::TWin32ErrorFormatter::FormatWin32ErrorCodeWithFallback(hResult, nullptr, false);
			if(!strDesc.IsEmpty())
				wstr << L" (" << strDesc.c_str() << ")";
		}
	}

	return wstr.str();
}
