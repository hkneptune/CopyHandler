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

	CString& GetCombinedMask(CString& pMask) const;
	void SetCombinedMask(const CString& pMask);

	CString& GetCombinedExcludeMask(CString& pMask) const;
	void SetCombinedExcludeMask(const CString& pMask);

	void StoreInConfig(chcore::TConfig& rConfig) const;
	void ReadFromConfig(const chcore::TConfig& rConfig);

	template<class Archive>
	void serialize(Archive& ar, unsigned int /*uiVersion*/)
	{
		ar & m_bUseMask;
		ar & m_astrMask;

		ar & m_bUseExcludeMask;
		ar & m_astrExcludeMask;

		ar & m_bUseSize;
		ar & m_iSizeType1;
		ar & m_ullSize1;
		ar & m_bUseSize2;
		ar & m_iSizeType2;
		ar & m_ullSize2;

		ar & m_bUseDate;
		ar & m_iDateType;	// created/last modified/last accessed
		ar & m_iDateType1;	// before/after
		ar & m_bDate1;
		ar & m_tDate1;
		ar & m_bTime1;
		ar & m_tTime1;

		ar & m_bUseDate2;
		ar & m_iDateType2;
		ar & m_bDate2;
		ar & m_tDate2;
		ar & m_bTime2;
		ar & m_tTime2;

		ar & m_bUseAttributes;
		ar & m_iArchive;
		ar & m_iReadOnly;
		ar & m_iHidden;
		ar & m_iSystem;
		ar & m_iDirectory;
	}

protected:
	bool MatchMask(LPCTSTR lpszMask, LPCTSTR lpszString) const;
	bool Scan(LPCTSTR& lpszMask, LPCTSTR& lpszString) const;

public:
	// files mask
	bool m_bUseMask;
	std::vector<CString> m_astrMask;

	// files mask-
	bool m_bUseExcludeMask;
	std::vector<CString> m_astrExcludeMask;

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

	template<class Archive>
	void serialize(Archive& ar, unsigned int /*uiVersion*/)
	{
		ar & m_vFilters;
	}

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

