// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TWString.h
/// @date  2011/04/10
/// @brief Basic wrapper over std::wstring.
// ============================================================================
#ifndef __TWSTRINGDATA_H__
#define __TWSTRINGDATA_H__

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TWStringData
{
public:
	TWStringData();
	TWStringData(const TWStringData& rSrc);
	TWStringData(const wchar_t* pszSrc);
	TWStringData(const std::wstring& strSrc);
	
	~TWStringData();

	TWStringData& operator=(const TWStringData& rSrc);
	TWStringData& operator=(const wchar_t* pszSrc);
	TWStringData& operator=(const std::wstring& strSrc);

	bool IsEmpty() const;
	size_t GetCount() const;
	size_t GetBytesCount() const;
	const wchar_t* GetData() const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::wstring m_wstrData;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
