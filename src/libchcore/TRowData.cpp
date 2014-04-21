// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#include "TRowData.h"

BEGIN_CHCORE_NAMESPACE

TRowData::TRowData(const TString& strColName, bool bValue) :
	m_strColName(strColName),
	m_varValue(bValue)
{
}

TRowData::TRowData(const TString& strColName, short iValue) :
	m_strColName(strColName),
	m_varValue(iValue)
{
}

TRowData::TRowData(const TString& strColName, unsigned short uiValue) :
	m_strColName(strColName),
	m_varValue(uiValue)
{
}

TRowData::TRowData(const TString& strColName, int iValue) :
	m_strColName(strColName),
	m_varValue(iValue)
{
}

TRowData::TRowData(const TString& strColName, unsigned int uiValue) :
	m_strColName(strColName),
	m_varValue(uiValue)
{
}

TRowData::TRowData(const TString& strColName, long long llValue) :
	m_strColName(strColName),
	m_varValue(llValue)
{
}

TRowData::TRowData(const TString& strColName, unsigned long long ullValue) :
	m_strColName(strColName),
	m_varValue(ullValue)
{
}

TRowData::TRowData(const TString& strColName, double dValue) :
	m_strColName(strColName),
	m_varValue(dValue)
{
}

TRowData::TRowData(const TString& strColName, const TString& strValue) :
	m_strColName(strColName),
	m_varValue(strValue)
{
}

TRowData::TRowData(const TString& strColName, const TSmartPath& pathValue) :
	m_strColName(strColName),
	m_varValue(pathValue)
{
}

TRowData::TRowData(const TString& strColName, long lValue) :
	m_strColName(strColName),
	m_varValue(lValue)
{
}

TRowData::TRowData(const TString& strColName, unsigned long ulValue) :
	m_strColName(strColName),
	m_varValue(ulValue)
{
}

TRowData::~TRowData()
{
}

END_CHCORE_NAMESPACE
