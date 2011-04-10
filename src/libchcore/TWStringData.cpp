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
/// @file  TWString.cpp
/// @date  2011/04/10
/// @brief Implementation of the basic wrapper over wstring.
// ============================================================================
#include "stdafx.h"
#include "TWStringData.h"

BEGIN_CHCORE_NAMESPACE

TWStringData::TWStringData()
{

}

TWStringData::TWStringData(const TWStringData& rSrc) :
	m_wstrData(rSrc.m_wstrData)
{
}

TWStringData::TWStringData(const wchar_t* pszSrc) :
	m_wstrData(pszSrc)
{
}

TWStringData::TWStringData(const std::wstring& strSrc) :
	m_wstrData(strSrc.c_str())
{
}

TWStringData::~TWStringData()
{
}

TWStringData& TWStringData::operator=(const TWStringData& rSrc)
{
	if(this != &rSrc)
		m_wstrData = rSrc.m_wstrData;

	return *this;
}

TWStringData& TWStringData::operator=(const wchar_t* pszSrc)
{
	m_wstrData = pszSrc;
	return *this;
}

TWStringData& TWStringData::operator=(const std::wstring& strSrc)
{
	m_wstrData = strSrc.c_str();
	return *this;
}

bool TWStringData::IsEmpty() const
{
	return m_wstrData.empty();
}

size_t TWStringData::GetCount() const
{
	return m_wstrData.size();
}

size_t TWStringData::GetBytesCount() const
{
	return (m_wstrData.size() + 1) * sizeof(wchar_t);
}

const wchar_t* TWStringData::GetData() const
{
	return m_wstrData.c_str();
}

END_CHCORE_NAMESPACE
