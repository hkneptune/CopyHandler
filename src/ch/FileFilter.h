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
#ifndef __FILEFILTER_H__
#define __FILEFILTER_H__

#include "FileInfo.h"

// definitions for comparing sizes and dates
#define LT 0
#define LE 1
#define EQ 2
#define GE 3
#define GT 4

// date type defs
#define DATE_CREATED		0
#define DATE_MODIFIED		1
#define DATE_LASTACCESSED	2

namespace chcore { class TConfig; }

class CFileFilter
{
public:
	CFileFilter();
	CFileFilter(const CFileFilter& rFilter);
	CFileFilter& operator=(const CFileFilter& rFilter);

	bool Match(const CFileInfoPtr& spInfo) const;

	chcore::TString& GetCombinedMask(chcore::TString& pMask) const;
	void SetCombinedMask(const chcore::TString& pMask);

	chcore::TString& GetCombinedExcludeMask(chcore::TString& pMask) const;
	void SetCombinedExcludeMask(const chcore::TString& pMask);

	void StoreInConfig(chcore::TConfig& rConfig) const;
	void ReadFromConfig(const chcore::TConfig& rConfig);

	void Serialize(chcore::TReadBinarySerializer& rSerializer);
	void Serialize(chcore::TWriteBinarySerializer& rSerializer) const;

protected:
	bool MatchMask(LPCTSTR lpszMask, LPCTSTR lpszString) const;
	bool Scan(LPCTSTR& lpszMask, LPCTSTR& lpszString) const;

public:
	// files mask
	bool m_bUseMask;
	chcore::TStringArray m_astrMask;

	// files mask-
	bool m_bUseExcludeMask;
	chcore::TStringArray m_astrExcludeMask;

	// size filtering
	bool m_bUseSize;
	int m_iSizeType1;
	unsigned __int64 m_ullSize1;
	bool m_bUseSize2;
	int m_iSizeType2;
	unsigned __int64 m_ullSize2;

	// date filtering
	bool m_bUseDate;
	int m_iDateType;	// created/last modified/last accessed
	int m_iDateType1;	// before/after
	bool m_bDate1;
	CTime m_tDate1;
	bool m_bTime1;
	CTime m_tTime1;

	bool m_bUseDate2;
	int m_iDateType2;
	bool m_bDate2;
	CTime m_tDate2;
	bool m_bTime2;
	CTime m_tTime2;

	// attribute filtering
	bool m_bUseAttributes;
	int m_iArchive;
	int m_iReadOnly;
	int m_iHidden;
	int m_iSystem;
	int m_iDirectory;
};

class CFiltersArray
{
public:
	CFiltersArray() {}
	~CFiltersArray() {}

	CFiltersArray& operator=(const CFiltersArray& rSrc);
	bool Match(const CFileInfoPtr& spInfo) const;

	void StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(const chcore::TConfig& rConfig, PCTSTR pszNodeName);

	void Serialize(chcore::TReadBinarySerializer& rSerializer);
	void Serialize(chcore::TWriteBinarySerializer& rSerializer) const;

	bool IsEmpty() const;

	void Add(const CFileFilter& rFilter);
	bool SetAt(size_t stIndex, const CFileFilter& rNewFilter);
	const CFileFilter* GetAt(size_t stIndex) const;
	bool RemoveAt(size_t stIndex);
	size_t GetSize() const;

protected:
	std::vector<CFileFilter> m_vFilters;
};

CONFIG_MEMBER_SERIALIZATION(CFiltersArray)

#endif

