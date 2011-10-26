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
#include "FileInfo.h"
#include "FileFilter.h"
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

CFileFilter::CFileFilter()
{
	// files mask
	m_bUseMask=false;
	m_astrMask.Clear();

	m_bUseExcludeMask=false;
	m_astrExcludeMask.Clear();

	// size filtering
	m_bUseSize1=false;
	m_eSizeCmpType1=eSizeCmp_Greater;
	m_ullSize1=0;
	m_bUseSize2=false;
	m_eSizeCmpType2=eSizeCmp_Less;
	m_ullSize2=0;

	// date filtering
	m_bUseDateTime1=false;
	m_eDateType = eDateType_Created;
	m_eDateCmpType1 = eDateCmp_Greater;
	m_bUseDate1 = false;
	m_bUseTime1 = false;
	m_tDateTime1.SetCurrentDateTime();

	m_bUseDateTime2=false;
	m_eDateCmpType2 = eDateCmp_Less;
	m_bUseDate2=false;
	m_bUseTime2=false;
	m_tDateTime2.SetCurrentDateTime();

	// attribute filtering
	m_bUseAttributes=false;
	m_iArchive=2;
	m_iReadOnly=2;
	m_iHidden=2;
	m_iSystem=2;
	m_iDirectory=2;
}

CFileFilter::CFileFilter(const CFileFilter& rFilter)
{
	*this=rFilter;
}

CFileFilter& CFileFilter::operator=(const CFileFilter& rFilter)
{
	// files mask
	m_bUseMask=rFilter.m_bUseMask;
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

chcore::TString& CFileFilter::GetCombinedMask(chcore::TString& strMask) const
{
	strMask.Clear();
	size_t stCount = m_astrMask.GetCount();
	if(stCount > 0)
	{
		strMask = m_astrMask.GetAt(0);
		for(size_t stIndex = 1; stIndex < stCount; stIndex++)
		{
			strMask += _T("|") + m_astrMask.GetAt(stIndex);
		}
	}

	return strMask;
}

void CFileFilter::SetCombinedMask(const chcore::TString& pMask)
{
	m_astrMask.Clear();

	pMask.Split(_T("|"), m_astrMask);
}

chcore::TString& CFileFilter::GetCombinedExcludeMask(chcore::TString& strMask) const
{
	strMask.Clear();
	size_t stCount = m_astrExcludeMask.GetCount();
	if(stCount > 0)
	{
		strMask = m_astrExcludeMask.GetAt(0);
		for(size_t stIndex = 1; stIndex < stCount; stIndex++)
		{
			strMask += _T("|") + m_astrExcludeMask.GetAt(stIndex);
		}
	}

	return strMask;
}

void CFileFilter::SetCombinedExcludeMask(const chcore::TString& pMask)
{
	m_astrExcludeMask.Clear();

	pMask.Split(_T("|"), m_astrExcludeMask);
}

void CFileFilter::StoreInConfig(chcore::TConfig& rConfig) const
{
	SetConfigValue(rConfig, _T("IncludeMask.Use"), m_bUseMask);
	SetConfigValue(rConfig, _T("IncludeMask.MaskList.Mask"), m_astrMask);

	SetConfigValue(rConfig, _T("ExcludeMask.Use"), m_bUseExcludeMask);
	SetConfigValue(rConfig, _T("ExcludeMask.MaskList.Mask"), m_astrExcludeMask);

	SetConfigValue(rConfig, _T("SizeA.Use"), m_bUseSize1);
	SetConfigValue(rConfig, _T("SizeA.FilteringType"), m_eSizeCmpType1);
	SetConfigValue(rConfig, _T("SizeA.Value"), m_ullSize1);
	SetConfigValue(rConfig, _T("SizeB.Use"), m_bUseSize2);
	SetConfigValue(rConfig, _T("SizeB.FilteringType"), m_eSizeCmpType2);
	SetConfigValue(rConfig, _T("SizeB.Value"), m_ullSize2);

	SetConfigValue(rConfig, _T("DateA.Use"), m_bUseDateTime1);
	SetConfigValue(rConfig, _T("DateA.Type"), m_eDateType);	// created/last modified/last accessed
	SetConfigValue(rConfig, _T("DateA.FilteringType"), m_eDateCmpType1);	// before/after
	SetConfigValue(rConfig, _T("DateA.EnableDatePart"), m_bUseDate1);
	SetConfigValue(rConfig, _T("DateA.EnableTimePart"), m_bUseTime1);
	SetConfigValue(rConfig, _T("DateA.DateTimeValue"), m_tDateTime1);

	SetConfigValue(rConfig, _T("DateB.Type"), m_bUseDateTime2);
	SetConfigValue(rConfig, _T("DateB.FilteringType"), m_eDateCmpType2);
	SetConfigValue(rConfig, _T("DateB.EnableDatePart"), m_bUseDate2);
	SetConfigValue(rConfig, _T("DateB.EnableTimePart"), m_bUseTime2);
	SetConfigValue(rConfig, _T("DateB.DateTimeValue"), m_tDateTime2);

	SetConfigValue(rConfig, _T("Attributes.Use"), m_bUseAttributes);
	SetConfigValue(rConfig, _T("Attributes.Archive"), m_iArchive);
	SetConfigValue(rConfig, _T("Attributes.ReadOnly"), m_iReadOnly);
	SetConfigValue(rConfig, _T("Attributes.Hidden"), m_iHidden);
	SetConfigValue(rConfig, _T("Attributes.System"), m_iSystem);
	SetConfigValue(rConfig, _T("Attributes.Directory"), m_iDirectory);
}

void CFileFilter::ReadFromConfig(const chcore::TConfig& rConfig)
{
	if(!GetConfigValue(rConfig, _T("IncludeMask.Use"), m_bUseMask))
		m_bUseMask = false;

	m_astrMask.Clear();
	GetConfigValue(rConfig, _T("IncludeMask.MaskList.Mask"), m_astrMask);

	if(!GetConfigValue(rConfig, _T("ExcludeMask.Use"), m_bUseExcludeMask))
		m_bUseExcludeMask = false;

	m_astrExcludeMask.Clear();
	GetConfigValue(rConfig, _T("ExcludeMask.MaskList.Mask"), m_astrExcludeMask);

	if(!GetConfigValue(rConfig, _T("SizeA.Use"), m_bUseSize1))
		m_bUseSize1 = false;
	if(!GetConfigValue(rConfig, _T("SizeA.FilteringType"), *(int*)m_eSizeCmpType1))
		m_eSizeCmpType1 = eSizeCmp_Equal;
	if(!GetConfigValue(rConfig, _T("SizeA.Value"), m_ullSize1))
		m_ullSize1 = 0;
	if(!GetConfigValue(rConfig, _T("SizeB.Use"), m_bUseSize2))
		m_bUseSize2 = false;
	if(!GetConfigValue(rConfig, _T("SizeB.FilteringType"), *(int*)m_eSizeCmpType2))
		m_eSizeCmpType2 = eSizeCmp_Equal;
	if(!GetConfigValue(rConfig, _T("SizeB.Value"), m_ullSize2))
		m_ullSize2 = 0;

	if(!GetConfigValue(rConfig, _T("DateA.Use"), m_bUseDateTime1))
		m_bUseDateTime1 = false;

	if(!GetConfigValue(rConfig, _T("DateA.Type"), *(int*)m_eDateType))	// created/last modified/last accessed
		m_eDateType = eDateType_Created;
	if(!GetConfigValue(rConfig, _T("DateA.FilteringType"), *(int*)m_eDateCmpType1))	// before/after
		m_eDateCmpType1 = eDateCmp_Equal;
	if(!GetConfigValue(rConfig, _T("DateA.EnableDatePart"), m_bUseDate1))
		m_bUseDate1 = false;
	if(!GetConfigValue(rConfig, _T("DateA.EnableTimePart"), m_bUseTime1))
		m_bUseTime1 = false;

	if(!GetConfigValue(rConfig, _T("DateA.DateTimeValue"), m_tDateTime1))
		m_tDateTime1.Clear();

	if(!GetConfigValue(rConfig, _T("DateB.Type"), m_bUseDateTime2))
		m_bUseDateTime2 = false;
	if(!GetConfigValue(rConfig, _T("DateB.FilteringType"), *(int*)m_eDateCmpType2))
		m_eDateCmpType2 = eDateCmp_Equal;
	if(!GetConfigValue(rConfig, _T("DateB.EnableDatePart"), m_bUseDate2))
		m_bUseDate2 = false;

	if(!GetConfigValue(rConfig, _T("DateB.DateTimeValue"), m_tDateTime2))
		m_tDateTime2.Clear();
	if(!GetConfigValue(rConfig, _T("DateB.EnableTimePart"), m_bUseTime2))
		m_bUseTime2 = false;

	if(!GetConfigValue(rConfig, _T("Attributes.Use"), m_bUseAttributes))
		m_bUseAttributes = false;
	if(!GetConfigValue(rConfig, _T("Attributes.Archive"), m_iArchive))
		m_iArchive = 0;
	if(!GetConfigValue(rConfig, _T("Attributes.ReadOnly"), m_iReadOnly))
		m_iReadOnly = false;
	if(!GetConfigValue(rConfig, _T("Attributes.Hidden"), m_iHidden))
		m_iHidden = 0;
	if(!GetConfigValue(rConfig, _T("Attributes.System"), m_iSystem))
		m_iSystem = 0;
	if(!GetConfigValue(rConfig, _T("Attributes.Directory"), m_iDirectory))
		m_iDirectory = 0;
}

void CFileFilter::Serialize(chcore::TReadBinarySerializer& rSerializer)
{
	using chcore::Serializers::Serialize;

	Serialize(rSerializer, m_bUseMask);
	Serialize(rSerializer, m_astrMask);

	Serialize(rSerializer, m_bUseExcludeMask);
	Serialize(rSerializer, m_astrExcludeMask);

	Serialize(rSerializer, m_bUseSize1);
	Serialize(rSerializer, m_eSizeCmpType1);
	Serialize(rSerializer, m_ullSize1);
	Serialize(rSerializer, m_bUseSize2);
	Serialize(rSerializer, m_eSizeCmpType2);
	Serialize(rSerializer, m_ullSize2);

	Serialize(rSerializer, m_bUseDateTime1);
	Serialize(rSerializer, m_eDateType);	// created/last modified/last accessed
	Serialize(rSerializer, m_eDateCmpType1);	// before/after
	Serialize(rSerializer, m_bUseDate1);
	Serialize(rSerializer, m_bUseTime1);
	Serialize(rSerializer, m_tDateTime1);

	Serialize(rSerializer, m_bUseDateTime2);
	Serialize(rSerializer, m_eDateCmpType2);
	Serialize(rSerializer, m_bUseDate2);
	Serialize(rSerializer, m_bUseTime2);
	Serialize(rSerializer, m_tDateTime2);

	Serialize(rSerializer, m_bUseAttributes);
	Serialize(rSerializer, m_iArchive);
	Serialize(rSerializer, m_iReadOnly);
	Serialize(rSerializer, m_iHidden);
	Serialize(rSerializer, m_iSystem);
	Serialize(rSerializer, m_iDirectory);
}

void CFileFilter::Serialize(chcore::TWriteBinarySerializer& rSerializer) const
{
	using chcore::Serializers::Serialize;

	Serialize(rSerializer, m_bUseMask);
	Serialize(rSerializer, m_astrMask);

	Serialize(rSerializer, m_bUseExcludeMask);
	Serialize(rSerializer, m_astrExcludeMask);

	Serialize(rSerializer, m_bUseSize1);
	Serialize(rSerializer, m_eSizeCmpType1);
	Serialize(rSerializer, m_ullSize1);
	Serialize(rSerializer, m_bUseSize2);
	Serialize(rSerializer, m_eSizeCmpType2);
	Serialize(rSerializer, m_ullSize2);

	Serialize(rSerializer, m_bUseDateTime1);
	Serialize(rSerializer, m_eDateType);	// created/last modified/last accessed
	Serialize(rSerializer, m_eDateCmpType1);	// before/after
	Serialize(rSerializer, m_bUseDate1);
	Serialize(rSerializer, m_bUseTime1);
	Serialize(rSerializer, m_tDateTime1);

	Serialize(rSerializer, m_bUseDateTime2);
	Serialize(rSerializer, m_eDateCmpType2);
	Serialize(rSerializer, m_bUseDate2);
	Serialize(rSerializer, m_bUseTime2);
	Serialize(rSerializer, m_tDateTime2);

	Serialize(rSerializer, m_bUseAttributes);
	Serialize(rSerializer, m_iArchive);
	Serialize(rSerializer, m_iReadOnly);
	Serialize(rSerializer, m_iHidden);
	Serialize(rSerializer, m_iSystem);
	Serialize(rSerializer, m_iDirectory);
}

bool CFileFilter::Match(const CFileInfoPtr& spInfo) const
{
	// check by mask
	if(m_bUseMask)
	{
		bool bRes=false;
		for(chcore::TStringArray::const_iterator iterMask = m_astrMask.Begin(); iterMask != m_astrMask.End(); ++iterMask)
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
		for(chcore::TStringArray::const_iterator iterExcludeMask = m_astrExcludeMask.Begin(); iterExcludeMask != m_astrExcludeMask.End(); ++iterExcludeMask)
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
			tDateTime = spInfo->GetCreationTime();
			break;
		case eDateType_Modified:
			tDateTime = spInfo->GetLastWriteTime();
			break;
		case eDateType_LastAccessed:
			tDateTime = spInfo->GetLastAccessTime();
			break;
		}

		// counting...
		time_t tDiff = m_tDateTime1.Compare(tDateTime, m_bUseDate1, m_bUseTime1);

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
			tDiff = m_tDateTime2.Compare(tDateTime, m_bUseDate2, m_bUseTime2);

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

bool CFileFilter::MatchMask(LPCTSTR lpszMask, LPCTSTR lpszString) const
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
bool CFileFilter::Scan(LPCTSTR& lpszMask, LPCTSTR& lpszString) const
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

CFiltersArray& CFiltersArray::operator=(const CFiltersArray& rSrc)
{
	if(this != &rSrc)
	{
		m_vFilters = rSrc.m_vFilters;
	}

	return *this;
}

bool CFiltersArray::Match(const CFileInfoPtr& spInfo) const
{
	if(m_vFilters.empty())
		return true;

	// if only one of the filters matches - return true
	for(std::vector<CFileFilter>::const_iterator iterFilter = m_vFilters.begin(); iterFilter != m_vFilters.end(); iterFilter++)
	{
		if((*iterFilter).Match(spInfo))
			return true;
	}

	return false;
}

void CFiltersArray::StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const
{
	rConfig.DeleteNode(pszNodeName);
	BOOST_FOREACH(const CFileFilter& rFilter, m_vFilters)
	{
		chcore::TConfig cfgNode;
		rFilter.StoreInConfig(cfgNode);
		rConfig.AddSubConfig(chcore::TString(pszNodeName) + _T(".FilterDefinition"), cfgNode);
	}
}

bool CFiltersArray::ReadFromConfig(const chcore::TConfig& rConfig, PCTSTR pszNodeName)
{
	m_vFilters.clear();

	chcore::TConfigArray vConfigs;
	if(!rConfig.ExtractMultiSubConfigs(pszNodeName, vConfigs))
		return false;

	for(size_t stIndex = 0; stIndex < vConfigs.GetCount(); ++stIndex)
	{
		const chcore::TConfig& rCfg = vConfigs.GetAt(stIndex);
		CFileFilter tFilter;
		tFilter.ReadFromConfig(rCfg);

		m_vFilters.push_back(tFilter);
	}
	return true;
}

void CFiltersArray::Serialize(chcore::TReadBinarySerializer& rSerializer)
{
	using chcore::Serializers::Serialize;
	Serialize(rSerializer, m_vFilters);
}

void CFiltersArray::Serialize(chcore::TWriteBinarySerializer& rSerializer) const
{
	using chcore::Serializers::Serialize;
	Serialize(rSerializer, m_vFilters);
}

bool CFiltersArray::IsEmpty() const
{
	return m_vFilters.empty();
}

void CFiltersArray::Add(const CFileFilter& rFilter)
{
	m_vFilters.push_back(rFilter);
}

bool CFiltersArray::SetAt(size_t stIndex, const CFileFilter& rNewFilter)
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
	{
		CFileFilter& rFilter = m_vFilters.at(stIndex);
		rFilter = rNewFilter;
		return true;
	}
	else
		return false;
}

const CFileFilter* CFiltersArray::GetAt(size_t stIndex) const
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
		return &m_vFilters.at(stIndex);
	else
		return NULL;
}

bool CFiltersArray::RemoveAt(size_t stIndex)
{
	BOOST_ASSERT(stIndex < m_vFilters.size());
	if(stIndex < m_vFilters.size())
	{
		m_vFilters.erase(m_vFilters.begin() + stIndex);
		return true;
	}
	else
		return false;
}

size_t CFiltersArray::GetSize() const
{
	return m_vFilters.size();
}

END_CHCORE_NAMESPACE
