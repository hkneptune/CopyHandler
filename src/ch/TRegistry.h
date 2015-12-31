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
#ifndef __TREGISTRY_H__
#define __TREGISTRY_H__

class TRegistry
{
public:
	TRegistry(HKEY key, const wchar_t* pszKey);
	~TRegistry();

	bool QueryString(const wchar_t* pszValueKey, std::wstring& wstrValue);
	bool QueryDword(const wchar_t* pszValueKey, DWORD& dwOutValue);

private:
	HKEY m_hKey;
};

#endif
