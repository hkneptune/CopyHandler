/***************************************************************************
*   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "stdafx.h"
#include "TFileFilter.h"
#include "TFileInfo.h"
#include "TConfig.h"
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"

BEGIN_CHCORE_NAMESPACE

////////////////////////////////////////////////////////////////////////////
bool _tcicmp(TCHAR c1, TCHAR c2)
{
	TCHAR ch1[2]={c1, 0}, ch2[2]={c2, 0};
	return (_tcsicmp(ch1, ch2) == 0);
}

TFileFilter::TFileFilter() :
	m_stObjectID(0),
	m_setModifications(),
	m_bUseMask(m_setModifications, false),
	m_astrMask(m_setModifications),
	m_bUseExcludeMask(m_setModifications, false),
	m_astrExcludeMask(m_setModifications),
	m_bUseSize1(m_setModifications, false),
	m_eSizeCmpType1(m_setModifications, eSizeCmp_Greater),
	m_ullSize1(m_setModifications, 0),
	m_bUseSize2(m_setModifications, false),
	m_eSizeCmpType2(m_setModifications, eSizeCmp_Less),
	m_ullSize2(m_setModifications, 0),
	m_bUseDateTime1(m_setModifications, false),
	m_eDateType(m_setModifications, eDateType_Created),
	m_eDateCmpType1(m_setModifications, eDateCmp_Greater),
	m_bUseDate1(m_setModifications, false),
	m_bUseTime1(m_setModifications, false),
	m_tDateTime1(m_setModifications),
	m_bUseDateTime2(m_setModifications, false),
	m_eDateCmpType2(m_setModifications, eDateCmp_Less),
	m_bUseDate2(m_setModifications, false),
	m_bUseTime2(m_setModifications, false),
	m_tDateTime2(m_setModifications),
	m_bUseAttributes(m_setModifications, false),
	m_iArchive(m_setModifications, 2),
	m_iReadOnly(m_setModifications, 2),
	m_iHidden(m_setModifications, 2),
	m_iSystem(m_setModifications, 2),
	m_iDirectory(m_setModifications, 2)
{
	m_setModifications[eMod_Added] = true;

	m_tDateTime1.Modify().SetCurrentDateTime();
	m_tDateTime2.Modify().SetCurrentDateTime();
}

TFileFilter::TFileFilter(const TFileFilter& rFilter) :
	m_stObjectID(rFilter.m_stObjectID),
	m_setModifications(rFilter.m_setModifications),
	m_bUseMask(rFilter.m_bUseMask, m_setModifications),
	m_astrMask(rFilter.m_astrMask, m_setModifications),
	m_bUseExcludeMask(rFilter.m_bUseExcludeMask, m_setModifications),
	m_astrExcludeMask(rFilter.m_astrExcludeMask, m_setModifications),
	m_bUseSize1(rFilter.m_bUseSize1, m_setModifications),
	m_eSizeCmpType1(rFilter.m_eSizeCmpType1, m_setModifications),
	m_ullSize1(rFilter.m_ullSize1, m_setModifications),
	m_bUseSize2(rFilter.m_bUseSize2, m_setModifications),
	m_eSizeCmpType2(rFilter.m_eSizeCmpType2, m_setModifications),
	m_ullSize2(rFilter.m_ullSize2, m_setModifications),
	m_bUseDateTime1(rFilter.m_bUseDateTime1, m_setModifications),
	m_eDateType(rFilter.m_eDateType, m_setModifications),
	m_eDateCmpType1(rFilter.m_eDateCmpType1, m_setModifications),
	m_bUseDate1(rFilter.m_bUseDate1, m_setModifications),
	m_bUseTime1(rFilter.m_bUseTime1, m_setModifications),
	m_tDateTime1(rFilter.m_tDateTime1, m_setModifications),
	m_bUseDateTime2(rFilter.m_bUseDateTime2, m_setModifications),
	m_eDateCmpType2(rFilter.m_eDateCmpType2, m_setModifications),
	m_bUseDate2(rFilter.m_bUseDate2, m_setModifications),
	m_bUseTime2(rFilter.m_bUseTime2, m_setModifications),
	m_tDateTime2(rFilter.m_tDateTime2, m_setModifications),
	m_bUseAttributes(rFilter.m_bUseAttributes, m_setModifications),
	m_iArchive(rFilter.m_iArchive, m_setModifications),
	m_iReadOnly(rFilter.m_iReadOnly, m_setModifications),
	m_iHidden(rFilter.m_iHidden, m_setModifications),
	m_iSystem(rFilter.m_iSystem, m_setModifications),
	m_iDirectory(rFilter.m_iDirectory, m_setModifications)
{
}

TFileFilter& TFileFilter::operator=(const TFileFilter& rFilter)
{
	if(this == &rFilter)
		return *this;

	m_stObjectID = rFilter.m_stObjectID;
	m_setModifications = rFilter.m_setModifications;

	// files mask
	m_bUseMask = rFilter.m_bUseMask;
	m_astrMask = rFilter.m_astrMask;

	m_bUseExcludeMask=rFilter.m_bUseExcludeMask;
	m_astrExcludeMask = rFilter.m_astrExcludeMask;

	// size filtering
	m_bUseSize1=rFilter.m_bUseSize1;
	m_eSizeCmpType1=rFilter.m_eSizeCmpType1;
	m_ullSize1=rFilter.m_ullSize1;
	m_bUseSize2=rFilter.m_bUseSize2;
	m_eSizeCmpType2=rFilter.m_eSizeCmpType2;
	m_ullSize2=rFilter.m_ullSize2;

	// date filtering
	m_bUseDateTime1=rFilter.m_bUseDateTime1;
	m_eDateType=rFilter.m_eDateType;
	m_eDateCmpType1=rFilter.m_eDateCmpType1;
	m_bUseDate1=rFilter.m_bUseDate1;
	m_bUseTime1=rFilter.m_bUseTime1;
	m_tDateTime1 = rFilter.m_tDateTime1;

	m_bUseDateTime2=rFilter.m_bUseDateTime2;
	m_eDateCmpType2=rFilter.m_eDateCmpType2;
	m_bUseDate2=rFilter.m_bUseDate2;
	m_bUseTime2=rFilter.m_bUseTime2;
	m_tDateTime2 = rFilter.m_tDateTime2;

	// attribute filtering
	m_bUseAttributes=rFilter.m_bUseAttributes;
	m_iArchive=rFilter.m_iArchive;
	m_iReadOnly=rFilter.m_iReadOnly;
	m_iHidden=rFilter.m_iHidden;
	m_iSystem=rFilter.m_iSystem;
	m_iDirectory=rFilter.m_iDirectory;

	return *this;
}

TString TFileFilter::GetCombinedMask() const
{
	TString strMask;
	size_t stCount = m_astrMask.Get().GetCount();
	if(stCount > 0)
	{
		strMask = m_astrMask.Get().GetAt(0);
		for(size_t stIndex = 1; stIndex < stCount; stIndex++)
		{
			strMask += _T("|") + m_astrMask.Get().GetAt(stIndex);
		}
	}

	return strMask;
}

void TFileFilter::SetCombinedMask(const TString& pMask)
{
	m_astrMask.Modify().Clear();

	pMask.Split(_T("|"), m_astrMask.Modify());
}

TString TFileFilter::GetCombinedExcludeMask() const
{
	TString strMask;
	size_t stCount = m_astrExcludeMask.Get().GetCount();
	if(stCount > 0)
	{
		strMask = m_astrExcludeMask.Get().GetAt(0);
		for(size_t stIndex = 1; stIndex < stCount; stIndex++)
		{
			strMask += _T("|") + m_astrExcludeMask.Get().GetAt(stIndex);
		}
	}

	return strMask;
}

void TFileFilter::SetCombinedExcludeMask(const TString& pMask)
{
	m_astrExcludeMask.Modify().Clear();

	pMask.Split(_T("|"), m_astrExcludeMask.Modify());
}

void TFileFilter::StoreInConfig(TConfig& rConfig) const
{
	SetConfigValue(rConfig, _T("IncludeMask.Use"), m_bUseMask.Get());
	SetConfigValue(rConfig, _T("IncludeMask.MaskList.Mask"), m_astrMask.Get());

	SetConfigValue(rConfig, _T("ExcludeMask.Use"), m_bUseExcludeMask.Get());
	SetConfigValue(rConfig, _T("ExcludeMask.MaskList.Mask"), m_astrExcludeMask.Get());

	SetConfigValue(rConfig, _T("SizeA.Use"), m_bUseSize1.Get());
	SetConfigValue(rConfig, _T("SizeA.FilteringType"), m_eSizeCmpType1.Get());
	SetConfigValue(rConfig, _T("SizeA.Value"), m_ullSize1.Get());
	SetConfigValue(rConfig, _T("SizeB.Use"), m_bUseSize2.Get());
	SetConfigValue(rConfig, _T("SizeB.FilteringType"), m_eSizeCmpType2.Get());
	SetConfigValue(rConfig, _T("SizeB.Value"), m_ullSize2.Get());

	SetConfigValue(rConfig, _T("DateA.Use"), m_bUseDateTime1.Get());
	SetConfigValue(rConfig, _T("DateA.Type"), m_eDateType.Get());	// created/last modified/last accessed
	SetConfigValue(rConfig, _T("DateA.FilteringType"), m_eDateCmpType1.Get());	// before/after
	SetConfigValue(rConfig, _T("DateA.EnableDatePart"), m_bUseDate1.Get());
	SetConfigValue(rConfig, _T("DateA.EnableTimePart"), m_bUseTime1.Get());
	SetConfigValue(rConfig, _T("DateA.DateTimeValue"), m_tDateTime1.Get());

	SetConfigValue(rConfig, _T("DateB.Type"), m_bUseDateTime2.Get());
	SetConfigValue(rConfig, _T("DateB.FilteringType"), m_eDateCmpType2.Get());
	SetConfigValue(rConfig, _T("DateB.EnableDatePart"), m_bUseDate2.Get());
	SetConfigValue(rConfig, _T("DateB.EnableTimePart"), m_bUseTime2.Get());
	SetConfigValue(rConfig, _T("DateB.DateTimeValue"), m_tDateTime2.Get());

	SetConfigValue(rConfig, _T("Attributes.Use"), m_bUseAttributes.Get());
	SetConfigValue(rConfig, _T("Attributes.Archive"), m_iArchive.Get());
	SetConfigValue(rConfig, _T("Attributes.ReadOnly"), m_iReadOnly.Get());
	SetConfigValue(rConfig, _T("Attributes.Hidden"), m_iHidden.Get());
	SetConfigValue(rConfig, _T("Attributes.System"), m_iSystem.Get());
	SetConfigValue(rConfig, _T("Attributes.Directory"), m_iDirectory.Get());
}

void TFileFilter::ReadFromConfig(const TConfig& rConfig)
{
	if(!GetConfigValue(rConfig, _T("IncludeMask.Use"), m_bUseMask.Modify()))
		m_bUseMask = false;

	m_astrMask.Modify().Clear();
	GetConfigValue(rConfig, _T("IncludeMask.MaskList.Mask"), m_astrMask.Modify());

	if(!GetConfigValue(rConfig, _T("ExcludeMask.Use"), m_bUseExcludeMask.Modify()))
		m_bUseExcludeMask = false;

	m_astrExcludeMask.Modify().Clear();
	GetConfigValue(rConfig, _T("ExcludeMask.MaskList.Mask"), m_astrExcludeMask.Modify());

	if(!GetConfigValue(rConfig, _T("SizeA.Use"), m_bUseSize1.Modify()))
		m_bUseSize1 = false;
	if(!GetConfigValue(rConfig, _T("SizeA.FilteringType"), *(int*)m_eSizeCmpType1.Modify()))
		m_eSizeCmpType1 = eSizeCmp_Equal;
	if(!GetConfigValue(rConfig, _T("SizeA.Value"), m_ullSize1.Modify()))
		m_ullSize1 = 0;
	if(!GetConfigValue(rConfig, _T("SizeB.Use"), m_bUseSize2.Modify()))
		m_bUseSize2 = false;
	if(!GetConfigValue(rConfig, _T("SizeB.FilteringType"), *(int*)m_eSizeCmpType2.Modify()))
		m_eSizeCmpType2 = eSizeCmp_Equal;
	if(!GetConfigValue(rConfig, _T("SizeB.Value"), m_ullSize2.Modify()))
		m_ullSize2 = 0;

	if(!GetConfigValue(rConfig, _T("DateA.Use"), m_bUseDateTime1.Modify()))
		m_bUseDateTime1 = false;

	if(!GetConfigValue(rConfig, _T("DateA.Type"), *(int*)m_eDateType.Modify()))	// created/last modified/last accessed
		m_eDateType = eDateType_Created;
	if(!GetConfigValue(rConfig, _T("DateA.FilteringType"), *(int*)m_eDateCmpType1.Modify()))	// before/after
		m_eDateCmpType1 = eDateCmp_Equal;
	if(!GetConfigValue(rConfig, _T("DateA.EnableDatePart"), m_bUseDate1.Modify()))
		m_bUseDate1 = false;
	if(!GetConfigValue(rConfig, _T("DateA.EnableTimePart"), m_bUseTime1.Modify()))
		m_bUseTime1 = false;

	if(!GetConfigValue(rConfig, _T("DateA.DateTimeValue"), m_tDateTime1.Modify()))
		m_tDateTime1.Modify().Clear();

	if(!GetConfigValue(rConfig, _T("DateB.Type"), m_bUseDateTime2.Modify()))
		m_bUseDateTime2 = false;
	if(!GetConfigValue(rConfig, _T("DateB.FilteringType"), *(int*)m_eDateCmpType2.Modify()))
		m_eDateCmpType2 = eDateCmp_Equal;
	if(!GetConfigValue(rConfig, _T("DateB.EnableDatePart"), m_bUseDate2.Modify()))
		m_bUseDate2 = false;

	if(!GetConfigValue(rConfig, _T("DateB.DateTimeValue"), m_tDateTime2.Modify()))
		m_tDateTime2.Modify().Clear();
	if(!GetConfigValue(rConfig, _T("DateB.EnableTimePart"), m_bUseTime2.Modify()))
		m_bUseTime2 = false;

	if(!GetConfigValue(rConfig, _T("Attributes.Use"), m_bUseAttributes.Modify()))
		m_bUseAttributes = false;
	if(!GetConfigValue(rConfig, _T("Attributes.Archive"), m_iArchive.Modify()))
		m_iArchive = 0;
	if(!GetConfigValue(rConfig, _T("Attributes.ReadOnly"), m_iReadOnly.Modify()))
		m_iReadOnly = false;
	if(!GetConfigValue(rConfig, _T("Attributes.Hidden"), m_iHidden.Modify()))
		m_iHidden = 0;
	if(!GetConfigValue(rConfig, _T("Attributes.System"), m_iSystem.Modify()))
		m_iSystem = 0;
	if(!GetConfigValue(rConfig, _T("Attributes.Directory"), m_iDirectory.Modify()))
		m_iDirectory = 0;
}

bool TFileFilter::Match(const TFileInfoPtr& spInfo) const
{
	// check by mask
	if(m_bUseMask)
	{
		bool bRes=false;
		for(TStringArray::const_iterator iterMask = m_astrMask.Get().Begin(); iterMask != m_astrMask.Get().End(); ++iterMask)
		{
			if(MatchMask(*iterMask, spInfo->GetFullFilePath().GetFileName().ToString()))
				bRes = true;
		}
		if(!bRes)
			return false;
	}

	// excluding mask
	if(m_bUseExcludeMask)
	{
		for(TStringArray::const_iterator iterExcludeMask = m_astrExcludeMask.Get().Begin(); iterExcludeMask != m_astrExcludeMask.Get().End(); ++iterExcludeMask)
		{
			if(MatchMask(*iterExcludeMask, spInfo->GetFullFilePath().GetFileName().ToString()))
				return false;
		}
	}

	// by size
	if (m_bUseSize1)
	{
		switch (m_eSizeCmpType1)
		{
		case eSizeCmp_Less:
			if (m_ullSize1 <= spInfo->GetLength64())
				return false;
			break;
		case eSizeCmp_LessOrEqual:
			if (m_ullSize1 < spInfo->GetLength64())
				return false;
			break;
		case eSizeCmp_Equal:
			if (m_ullSize1 != spInfo->GetLength64())
				return false;
			break;
		case eSizeCmp_GreaterOrEqual:
			if (m_ullSize1 > spInfo->GetLength64())
				return false;
			break;
		case eSizeCmp_Greater:
			if (m_ullSize1 >= spInfo->GetLength64())
				return false;
			break;
		}

		// second part
		if (m_bUseSize2)
		{
			switch (m_eSizeCmpType2)
			{
			case eSizeCmp_Less:
				if (m_ullSize2 <= spInfo->GetLength64())
					return false;
				break;
			case eSizeCmp_LessOrEqual:
				if (m_ullSize2 < spInfo->GetLength64())
					return false;
				break;
			case eSizeCmp_Equal:
				if (m_ullSize2 != spInfo->GetLength64())
					return false;
				break;
			case eSizeCmp_GreaterOrEqual:
				if (m_ullSize2 > spInfo->GetLength64())
					return false;
				break;
			case eSizeCmp_Greater:
				if (m_ullSize2 >= spInfo->GetLength64())
					return false;
				break;
			}
		}
	}

	// date - get the time from rInfo
	if (m_bUseDateTime1)
	{
		TDateTime tDateTime;
		switch(m_eDateType)
		{
		case eDateType_Created:
			tDateTime = spInfo->GetCreationTime().GetAsFiletime();
			break;
		case eDateType_Modified:
			tDateTime = spInfo->GetLastWriteTime().GetAsFiletime();
			break;
		case eDateType_LastAccessed:
			tDateTime = spInfo->GetLastAccessTime().GetAsFiletime();
			break;
		}

		// counting...
		time_t tDiff = m_tDateTime1.Get().Compare(tDateTime, m_bUseDate1, m_bUseTime1);

		// ... and comparing
		switch(m_eDateCmpType1)
		{
		case eDateCmp_Less:
			if(tDiff >= 0)
				return false;
			break;
		case eDateCmp_LessOrEqual:
			if(tDiff > 0)
				return false;
			break;
		case eDateCmp_Equal:
			if(tDiff != 0)
				return false;
			break;
		case eDateCmp_GreaterOrEqual:
			if(tDiff < 0)
				return false;
			break;
		case eDateCmp_Greater:
			if(tDiff <= 0)
				return false;
			break;
		}

		if (m_bUseDateTime2)
		{
			// counting...
			tDiff = m_tDateTime2.Get().Compare(tDateTime, m_bUseDate2, m_bUseTime2);

			// ... comparing
			switch (m_eDateCmpType2)
			{
			case eDateCmp_Less:
				if(tDiff >= 0)
					return false;
				break;
			case eDateCmp_LessOrEqual:
				if(tDiff > 0)
					return false;
				break;
			case eDateCmp_Equal:
				if(tDiff != 0)
					return false;
				break;
			case eDateCmp_GreaterOrEqual:
				if(tDiff < 0)
					return false;
				break;
			case eDateCmp_Greater:
				if(tDiff <= 0)
					return false;
				break;
			}
		}
	} // of m_bUseDate

	// attributes
	if (m_bUseAttributes)
	{
		if ( (m_iArchive == 1 && !spInfo->IsArchived()) || (m_iArchive == 0 && spInfo->IsArchived()))
			return false;
		if ( (m_iReadOnly == 1 && !spInfo->IsReadOnly()) || (m_iReadOnly == 0 && spInfo->IsReadOnly()))
			return false;
		if ( (m_iHidden == 1 && !spInfo->IsHidden()) || (m_iHidden == 0 && spInfo->IsHidden()))
			return false;
		if ( (m_iSystem == 1 && !spInfo->IsSystem()) || (m_iSystem == 0 && spInfo->IsSystem()))
			return false;
		if ( (m_iDirectory == 1 && !spInfo->IsDirectory()) || (m_iDirectory == 0 && spInfo->IsDirectory()))
			return false;
	}

	return true;
}

bool TFileFilter::MatchMask(LPCTSTR lpszMask, LPCTSTR lpszString) const
{
	bool bMatch = 1;

	//iterate and delete '?' and '*' one by one
	while(*lpszMask != _T('\0') && bMatch && *lpszString != _T('\0'))
	{
		if (*lpszMask == _T('?')) lpszString++;
		else if (*lpszMask == _T('*'))
		{
			bMatch = Scan(lpszMask, lpszString);
			lpszMask--;
		}
		else
		{
			bMatch = _tcicmp(*lpszMask, *lpszString);
			lpszString++;
		}
		lpszMask++;
	}
	while (*lpszMask == _T('*') && bMatch) lpszMask++;

	return bMatch && *lpszString == _T('\0') && *lpszMask == _T('\0');
}

// scan '?' and '*'
bool TFileFilter::Scan(LPCTSTR& lpszMask, LPCTSTR& lpszString) const
{
	// remove the '?' and '*'
	for(lpszMask++; *lpszString != _T('\0') && (*lpszMask == _T('?') || *lpszMask == _T('*')); lpszMask++)
		if (*lpszMask == _T('?')) lpszString++;
	while ( *lpszMask == _T('*')) lpszMask++;

	// if lpszString is empty and lpszMask has more characters or,
	// lpszMask is empty, return 
	if (*lpszString == _T('\0') && *lpszMask != _T('\0')) return false;
	if (*lpszString == _T('\0') && *lpszMask == _T('\0')) return true; 
	// else search substring
	else
	{
		LPCTSTR wdsCopy = lpszMask;
		LPCTSTR lpszStringCopy = lpszString;
		bool bMatch = true;
		do 
		{
			if (!MatchMask(lpszMask, lpszString)) lpszStringCopy++;
			lpszMask = wdsCopy;
			lpszString = lpszStringCopy;
			while (!(_tcicmp(*lpszMask, *lpszString)) && (*lpszString != '\0')) lpszString++;
			wdsCopy = lpszMask;
			lpszStringCopy = lpszString;
		}
		while ((*lpszString != _T('\0')) ? !MatchMask(lpszMask, lpszString) : (bMatch = false) != false);

		if (*lpszString == _T('\0') && *lpszMask == _T('\0')) return true;

		return bMatch;
	}
}

void TFileFilter::InitColumns(IColumnsDefinition& rColumns)
{
	rColumns.AddColumn(_T("id"), IColumnsDefinition::eType_sizet);
	rColumns.AddColumn(_T("use_mask"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("mask"), IColumnsDefinition::eType_string);
	rColumns.AddColumn(_T("use_exclude_mask"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("exclude_mask"), IColumnsDefinition::eType_string);
	rColumns.AddColumn(_T("use_size_1"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("compare_type_1"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("size_1"), IColumnsDefinition::eType_ulonglong);
	rColumns.AddColumn(_T("use_size_2"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("compare_type_2"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("size_2"), IColumnsDefinition::eType_ulonglong);
	rColumns.AddColumn(_T("date_type"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("use_date_time_1"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("date_compare_type_1"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("use_date_1"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("use_time_1"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("datetime_1"), IColumnsDefinition::eType_ulonglong);
	rColumns.AddColumn(_T("use_date_time_2"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("date_compare_type_2"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("use_date_2"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("use_time_2"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("datetime_2"), IColumnsDefinition::eType_ulonglong);
	rColumns.AddColumn(_T("use_attributes"), IColumnsDefinition::eType_bool);
	rColumns.AddColumn(_T("attr_archive"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("attr_ro"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("attr_hidden"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("attr_system"), IColumnsDefinition::eType_int);
	rColumns.AddColumn(_T("attr_directory"), IColumnsDefinition::eType_int);
}

void TFileFilter::Store(const ISerializerContainerPtr& spContainer) const
{
	ISerializerRowDataPtr spRow;

	bool bAdded = m_setModifications[eMod_Added];
	if(m_setModifications.any())
		spRow = spContainer->GetRow(m_stObjectID, bAdded);
	else
		return;

	if(bAdded || m_setModifications[eMod_UseMask])
		*spRow % TRowData(_T("use_mask"), m_bUseMask);
	if(bAdded || m_setModifications[eMod_Mask])
		*spRow % TRowData(_T("mask"), GetCombinedMask());
	if(bAdded || m_setModifications[eMod_UseExcludeMask])
		*spRow % TRowData(_T("use_exclude_mask"), m_bUseExcludeMask);
	if(bAdded || m_setModifications[eMod_ExcludeMask])
		*spRow % TRowData(_T("exclude_mask"), GetCombinedExcludeMask());
	if(bAdded || m_setModifications[eMod_UseSize1])
		*spRow % TRowData(_T("use_size_1"), m_bUseSize1);
	if(bAdded || m_setModifications[eMod_SizeCmpType1])
		*spRow % TRowData(_T("compare_type_1"), m_eSizeCmpType1);
	if(bAdded || m_setModifications[eMod_Size1])
		*spRow % TRowData(_T("size_1"), m_ullSize1);
	if(bAdded || m_setModifications[eMod_UseSize2])
		*spRow % TRowData(_T("use_size_2"), m_bUseSize2);
	if(bAdded || m_setModifications[eMod_SizeCmpType2])
		*spRow % TRowData(_T("compare_type_2"), m_eSizeCmpType2);
	if(bAdded || m_setModifications[eMod_Size2])
		*spRow % TRowData(_T("size_2"), m_ullSize2);
	if(bAdded || m_setModifications[eMod_DateType])
		*spRow % TRowData(_T("date_type"), m_eDateType);
	if(bAdded || m_setModifications[eMod_UseDateTime1])
		*spRow % TRowData(_T("use_date_time_1"), m_bUseDateTime1);
	if(bAdded || m_setModifications[eMod_DateCmpType1])
		*spRow % TRowData(_T("date_compare_type_1"), m_eDateCmpType1);
	if(bAdded || m_setModifications[eMod_UseDate1])
		*spRow % TRowData(_T("use_date_1"), m_bUseDate1);
	if(bAdded || m_setModifications[eMod_UseTime1])
		*spRow % TRowData(_T("use_time_1"), m_bUseTime1);
	if(bAdded || m_setModifications[eMod_DateTime1])
		*spRow % TRowData(_T("datetime_1"), m_tDateTime1.Get().GetAsTimeT());
	if(bAdded || m_setModifications[eMod_UseDateTime2])
		*spRow % TRowData(_T("use_date_time_2"), m_bUseDateTime2);
	if(bAdded || m_setModifications[eMod_DateCmpType2])
		*spRow % TRowData(_T("date_compare_type_2"), m_eDateCmpType2);
	if(bAdded || m_setModifications[eMod_UseDate2])
		*spRow % TRowData(_T("use_date_2"), m_bUseDate2);
	if(bAdded || m_setModifications[eMod_UseTime2])
		*spRow % TRowData(_T("use_time_2"), m_bUseTime2);
	if(bAdded || m_setModifications[eMod_DateTime2])
		*spRow % TRowData(_T("datetime_2"), m_tDateTime2.Get().GetAsTimeT());
	if(bAdded || m_setModifications[eMod_UseAttributes])
		*spRow % TRowData(_T("use_attributes"), m_bUseAttributes);
	if(bAdded || m_setModifications[eMod_AttrArchive])
		*spRow % TRowData(_T("attr_archive"), m_iArchive);
	if(bAdded || m_setModifications[eMod_AttrReadOnly])
		*spRow % TRowData(_T("attr_ro"), m_iReadOnly);
	if(bAdded || m_setModifications[eMod_AttrHidden])
		*spRow % TRowData(_T("attr_hidden"), m_iHidden);
	if(bAdded || m_setModifications[eMod_AttrSystem])
		*spRow % TRowData(_T("attr_system"), m_iSystem);
	if(bAdded || m_setModifications[eMod_AttrDirectory])
		*spRow % TRowData(_T("attr_directory"), m_iDirectory);

	m_setModifications.reset();
}

void TFileFilter::Load(const ISerializerRowReaderPtr& spRowReader)
{
	time_t tValue = 0;
	TString strMask;

	spRowReader->GetValue(_T("use_mask"), m_bUseMask.Modify());
	spRowReader->GetValue(_T("mask"), strMask);
	SetCombinedMask(strMask);
	spRowReader->GetValue(_T("use_exclude_mask"), m_bUseExcludeMask.Modify());
	spRowReader->GetValue(_T("exclude_mask"), strMask);
	SetCombinedExcludeMask(strMask);
	spRowReader->GetValue(_T("use_size_1"), m_bUseSize1.Modify());
	spRowReader->GetValue(_T("compare_type_1"), *(int*)&m_eSizeCmpType1.Modify());
	spRowReader->GetValue(_T("size_1"), m_ullSize1.Modify());
	spRowReader->GetValue(_T("use_size_2"), m_bUseSize2.Modify());
	spRowReader->GetValue(_T("compare_type_2"), *(int*)&m_eSizeCmpType2.Modify());
	spRowReader->GetValue(_T("size_2"), m_ullSize2.Modify());
	spRowReader->GetValue(_T("date_type"), *(int*)&m_eDateType.Modify());
	spRowReader->GetValue(_T("use_date_time_1"), m_bUseDateTime1.Modify());
	spRowReader->GetValue(_T("date_compare_type_1"), *(int*)&m_eDateCmpType1.Modify());
	spRowReader->GetValue(_T("use_date_1"), m_bUseDate1.Modify());
	spRowReader->GetValue(_T("use_time_1"), m_bUseTime1.Modify());
	spRowReader->GetValue(_T("datetime_1"), tValue);
	m_tDateTime1 = tValue;
	spRowReader->GetValue(_T("use_date_time_2"), m_bUseDateTime2.Modify());
	spRowReader->GetValue(_T("date_compare_type_2"), *(int*)&m_eDateCmpType2.Modify());
	spRowReader->GetValue(_T("use_date_2"), m_bUseDate2.Modify());
	spRowReader->GetValue(_T("use_time_2"), m_bUseTime2.Modify());
	spRowReader->GetValue(_T("datetime_2"), tValue);
	m_tDateTime2 = tValue;
	spRowReader->GetValue(_T("use_attributes"), m_bUseAttributes.Modify());
	spRowReader->GetValue(_T("attr_archive"), m_iArchive.Modify());
	spRowReader->GetValue(_T("attr_ro"), m_iReadOnly.Modify());
	spRowReader->GetValue(_T("attr_hidden"), m_iHidden.Modify());
	spRowReader->GetValue(_T("attr_system"), m_iSystem.Modify());
	spRowReader->GetValue(_T("attr_directory"), m_iDirectory.Modify());

	m_setModifications.reset();
}

size_t TFileFilter::GetObjectID() const
{
	return m_stObjectID;
}

void TFileFilter::SetObjectID(size_t stObjectID)
{
	m_stObjectID = stObjectID;
}

void TFileFilter::ResetModifications()
{
	m_setModifications.reset();
}

void TFileFilter::SetData(const TFileFilter& rFilter)
{
	if(this == &rFilter)
		return;

	// files mask
	m_bUseMask = rFilter.m_bUseMask;
	m_astrMask = rFilter.m_astrMask;

	m_bUseExcludeMask = rFilter.m_bUseExcludeMask;
	m_astrExcludeMask = rFilter.m_astrExcludeMask;

	// size filtering
	m_bUseSize1 = rFilter.m_bUseSize1;
	m_eSizeCmpType1 = rFilter.m_eSizeCmpType1;
	m_ullSize1 = rFilter.m_ullSize1;
	m_bUseSize2 = rFilter.m_bUseSize2;
	m_eSizeCmpType2 = rFilter.m_eSizeCmpType2;
	m_ullSize2 = rFilter.m_ullSize2;

	// date filtering
	m_bUseDateTime1 = rFilter.m_bUseDateTime1;
	m_eDateType = rFilter.m_eDateType;
	m_eDateCmpType1 = rFilter.m_eDateCmpType1;
	m_bUseDate1 = rFilter.m_bUseDate1;
	m_bUseTime1 = rFilter.m_bUseTime1;
	m_tDateTime1 = rFilter.m_tDateTime1;

	m_bUseDateTime2 = rFilter.m_bUseDateTime2;
	m_eDateCmpType2 = rFilter.m_eDateCmpType2;
	m_bUseDate2 = rFilter.m_bUseDate2;
	m_bUseTime2 = rFilter.m_bUseTime2;
	m_tDateTime2 = rFilter.m_tDateTime2;

	// attribute filtering
	m_bUseAttributes = rFilter.m_bUseAttributes;
	m_iArchive = rFilter.m_iArchive;
	m_iReadOnly = rFilter.m_iReadOnly;
	m_iHidden = rFilter.m_iHidden;
	m_iSystem = rFilter.m_iSystem;
	m_iDirectory = rFilter.m_iDirectory;
}

void TFileFilter::SetUseMask(bool bUseMask)
{
	m_bUseMask = bUseMask;
}

bool TFileFilter::GetUseMask() const
{
	return m_bUseMask;
}

bool TFileFilter::GetUseExcludeMask() const
{
	return m_bUseExcludeMask;
}

void TFileFilter::SetUseExcludeMask(bool bUseExcludeMask)
{
	m_bUseExcludeMask = bUseExcludeMask;
}

bool TFileFilter::GetUseSize1() const
{
	return m_bUseSize1;
}

void TFileFilter::SetUseSize1(bool bUseSize1)
{
	m_bUseSize1 = bUseSize1;
}

chcore::TFileFilter::ESizeCompareType TFileFilter::GetSizeType1() const
{
	return m_eSizeCmpType1;
}

void TFileFilter::SetSizeType1(ESizeCompareType eSizeType1)
{
	m_eSizeCmpType1 = eSizeType1;
}

unsigned long long TFileFilter::GetSize1() const
{
	return m_ullSize1;
}

void TFileFilter::SetSize1(unsigned long long ullSize1)
{
	m_ullSize1 = ullSize1;
}

bool TFileFilter::GetUseSize2() const
{
	return m_bUseSize2;
}

void TFileFilter::SetUseSize2(bool bUseSize2)
{
	m_bUseSize2 = bUseSize2;
}

chcore::TFileFilter::ESizeCompareType TFileFilter::GetSizeType2() const
{
	return m_eSizeCmpType2;
}

void TFileFilter::SetSizeType2(ESizeCompareType eSizeType2)
{
	m_eSizeCmpType2 = eSizeType2;
}

unsigned long long TFileFilter::GetSize2() const
{
	return m_ullSize2;
}

void TFileFilter::SetSize2(unsigned long long ullSize2)
{
	m_ullSize2 = ullSize2;
}

TFileFilter::EDateType TFileFilter::GetDateType() const
{
	return m_eDateType;
}

void TFileFilter::SetDateType(TFileFilter::EDateType eDateType)
{
	m_eDateType = eDateType;
}

bool TFileFilter::GetUseDateTime1() const
{
	return m_bUseDateTime1;
}

void TFileFilter::SetUseDateTime1(bool bUseDateTime1)
{
	m_bUseDateTime1 = bUseDateTime1;
}

TFileFilter::EDateCompareType TFileFilter::GetDateCmpType1() const
{
	return m_eDateCmpType1;
}

void TFileFilter::SetDateCmpType1(TFileFilter::EDateCompareType eCmpType1)
{
	m_eDateCmpType1 = eCmpType1;
}

bool TFileFilter::GetUseDate1() const
{
	return m_bUseDate1;
}

void TFileFilter::SetUseDate1(bool tDate1)
{
	m_bUseDate1 = tDate1;
}

bool TFileFilter::GetUseTime1() const
{
	return m_bUseTime1;
}

void TFileFilter::SetUseTime1(bool tTime1)
{
	m_bUseTime1 = tTime1;
}

const TDateTime& TFileFilter::GetDateTime1() const
{
	return m_tDateTime1;
}

void TFileFilter::SetDateTime1(const TDateTime& tDateTime1)
{
	m_tDateTime1 = tDateTime1;
}

bool TFileFilter::GetUseDateTime2() const
{
	return m_bUseDateTime2;
}

void TFileFilter::SetUseDateTime2(bool bUseDateTime2)
{
	m_bUseDateTime2 = bUseDateTime2;
}

TFileFilter::EDateCompareType TFileFilter::GetDateCmpType2() const
{
	return m_eDateCmpType2;
}

void TFileFilter::SetDateCmpType2(TFileFilter::EDateCompareType eCmpType2)
{
	m_eDateCmpType2 = eCmpType2;
}

bool TFileFilter::GetUseDate2() const
{
	return m_bUseDate2;
}

void TFileFilter::SetUseDate2(bool tDate2)
{
	m_bUseDate2 = tDate2;
}

bool TFileFilter::GetUseTime2() const
{
	return m_bUseTime2;
}

void TFileFilter::SetUseTime2(bool tTime2)
{
	m_bUseTime2 = tTime2;
}

const TDateTime& TFileFilter::GetDateTime2() const
{
	return m_tDateTime2;
}

void TFileFilter::SetDateTime2(const TDateTime& tDateTime2)
{
	m_tDateTime2 = tDateTime2;
}

bool TFileFilter::GetUseAttributes() const
{
	return m_bUseAttributes;
}

void TFileFilter::SetUseAttributes(bool bUseAttributes)
{
	m_bUseAttributes = bUseAttributes;
}

int TFileFilter::GetArchive() const
{
	return m_iArchive;
}

void TFileFilter::SetArchive(int iArchive)
{
	m_iArchive = iArchive;
}

int TFileFilter::GetReadOnly() const
{
	return m_iReadOnly;
}

void TFileFilter::SetReadOnly(int iReadOnly)
{
	m_iReadOnly = iReadOnly;
}

int TFileFilter::GetHidden() const
{
	return m_iHidden;
}

void TFileFilter::SetHidden(int iHidden)
{
	m_iHidden = iHidden;
}

END_CHCORE_NAMESPACE
