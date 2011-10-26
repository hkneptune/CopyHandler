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

#include "libchcore.h"
#include "FileInfo.h"
#include <atltime.h>
#include "TDateTime.h"

BEGIN_CHCORE_NAMESPACE

class TConfig;

class LIBCHCORE_API TFileFilter
{
public:
	enum ESizeCompareType
	{
		eSizeCmp_Less = 0,
		eSizeCmp_LessOrEqual = 1,
		eSizeCmp_Equal = 2,
		eSizeCmp_GreaterOrEqual = 3,
		eSizeCmp_Greater = 4
	};

	enum EDateCompareType
	{
		eDateCmp_Less = 0,
		eDateCmp_LessOrEqual = 1,
		eDateCmp_Equal = 2,
		eDateCmp_GreaterOrEqual = 3,
		eDateCmp_Greater = 4
	};

	enum EDateType
	{
		eDateType_Created = 0,
		eDateType_Modified = 1,
		eDateType_LastAccessed = 2
	};

public:
	TFileFilter();
	TFileFilter(const TFileFilter& rFilter);
	TFileFilter& operator=(const TFileFilter& rFilter);

	bool Match(const TFileInfoPtr& spInfo) const;

	TString& GetCombinedMask(TString& pMask) const;
	void SetCombinedMask(const TString& pMask);

	TString& GetCombinedExcludeMask(TString& pMask) const;
	void SetCombinedExcludeMask(const TString& pMask);

	void StoreInConfig(TConfig& rConfig) const;
	void ReadFromConfig(const TConfig& rConfig);

	void Serialize(TReadBinarySerializer& rSerializer);
	void Serialize(TWriteBinarySerializer& rSerializer) const;

	// atrributes access
	bool GetUseMask() const { return m_bUseMask; }
	void SetUseMask(bool bUseMask) { m_bUseMask = bUseMask; }

	//   const TStringArray& GetMaskArray() const { return m_astrMask; }
	//   TStringArray& GetMaskArray() { return m_astrMask; }

	bool GetUseExcludeMask() const { return m_bUseExcludeMask; }
	void SetUseExcludeMask(bool bUseExcludeMask) { m_bUseExcludeMask = bUseExcludeMask; }

	//   const TStringArray& GetExcludeMaskArray() const { return m_astrExcludeMask; }
	//   TStringArray& GetExcludeMaskArray() { return m_astrExcludeMask; }

	bool GetUseSize1() const { return m_bUseSize1; }
	void SetUseSize1(bool bUseSize1) { m_bUseSize1 = bUseSize1; }

	ESizeCompareType GetSizeType1() const { return m_eSizeCmpType1; }
	void SetSizeType1(ESizeCompareType eSizeType1) { m_eSizeCmpType1 = eSizeType1; }

	unsigned long long GetSize1() const { return m_ullSize1; }
	void SetSize1(unsigned long long ullSize1) { m_ullSize1 = ullSize1; }

	bool GetUseSize2() const { return m_bUseSize2; }
	void SetUseSize2(bool bUseSize2) { m_bUseSize2 = bUseSize2; }

	ESizeCompareType GetSizeType2() const { return m_eSizeCmpType2; }
	void SetSizeType2(ESizeCompareType eSizeType2) { m_eSizeCmpType2 = eSizeType2; }

	unsigned long long GetSize2() const { return m_ullSize2; }
	void SetSize2(unsigned long long ullSize2) { m_ullSize2 = ullSize2; }

	// dates
	TFileFilter::EDateType GetDateType() const { return m_eDateType; }
	void SetDateType(TFileFilter::EDateType eDateType) { m_eDateType = eDateType; }

	// date 1
	bool GetUseDateTime1() const { return m_bUseDateTime1; }
	void SetUseDateTime1(bool bUseDateTime1) { m_bUseDateTime1 = bUseDateTime1; }

	TFileFilter::EDateCompareType GetDateCmpType1() const { return m_eDateCmpType1; }
	void SetDateCmpType1(TFileFilter::EDateCompareType eCmpType1) { m_eDateCmpType1 = eCmpType1; }

	bool GetUseDate1() const { return m_bUseDate1; }
	void SetUseDate1(bool tDate1) { m_bUseDate1 = tDate1; }

	bool GetUseTime1() const { return m_bUseTime1; }
	void SetUseTime1(bool tTime1) { m_bUseTime1 = tTime1; }

	const TDateTime& GetDateTime1() const { return m_tDateTime1; }
	void SetDateTime1(const TDateTime& tDateTime1) { m_tDateTime1 = tDateTime1; }

	// date 2
	bool GetUseDateTime2() const { return m_bUseDateTime2; }
	void SetUseDateTime2(bool bUseDateTime2) { m_bUseDateTime2 = bUseDateTime2; }

	TFileFilter::EDateCompareType GetDateCmpType2() const { return m_eDateCmpType2; }
	void SetDateCmpType2(TFileFilter::EDateCompareType eCmpType2) { m_eDateCmpType2 = eCmpType2; }

	bool GetUseDate2() const { return m_bUseDate2; }
	void SetUseDate2(bool tDate2) { m_bUseDate2 = tDate2; }

	bool GetUseTime2() const { return m_bUseTime2; }
	void SetUseTime2(bool tTime2) { m_bUseTime2 = tTime2; }

	const TDateTime& GetDateTime2() const { return m_tDateTime2; }
	void SetDateTime2(const TDateTime& tDateTime2) { m_tDateTime2 = tDateTime2; }

	// attributes
	bool GetUseAttributes() const { return m_bUseAttributes; }
	void SetUseAttributes(bool bUseAttributes) { m_bUseAttributes = bUseAttributes; }

	int GetArchive() const { return m_iArchive; }
	void SetArchive(int iArchive) { m_iArchive = iArchive; }

	int GetReadOnly() const { return m_iReadOnly; }
	void SetReadOnly(int iReadOnly) { m_iReadOnly = iReadOnly; }

	int GetHidden() const { return m_iHidden; }
	void SetHidden(int iHidden) { m_iHidden = iHidden; }

	int GetSystem() const { return m_iSystem; }
	void SetSystem(int iSystem) { m_iSystem = iSystem; }

	int GetDirectory() const { return m_iDirectory; }
	void SetDirectory(int iDirectory) { m_iDirectory = iDirectory; }

protected:
	bool MatchMask(LPCTSTR lpszMask, LPCTSTR lpszString) const;
	bool Scan(LPCTSTR& lpszMask, LPCTSTR& lpszString) const;

private:
	// files mask
	bool m_bUseMask;
	TStringArray m_astrMask;

	// files mask-
	bool m_bUseExcludeMask;
	TStringArray m_astrExcludeMask;

	// size filtering
	bool m_bUseSize1;
	ESizeCompareType m_eSizeCmpType1;
	unsigned long long m_ullSize1;

	bool m_bUseSize2;
	ESizeCompareType m_eSizeCmpType2;
	unsigned long long m_ullSize2;

	// date filtering
	EDateType m_eDateType;	// created/last modified/last accessed

	bool m_bUseDateTime1;
#pragma warning(push)
#pragma warning(disable: 4251)
	EDateCompareType m_eDateCmpType1;	// before/after
	bool m_bUseDate1;
	bool m_bUseTime1;
	TDateTime m_tDateTime1;

	bool m_bUseDateTime2;

	EDateCompareType m_eDateCmpType2;
	bool m_bUseDate2;
	bool m_bUseTime2;
	TDateTime m_tDateTime2;
#pragma warning(pop)

	// attribute filtering
	bool m_bUseAttributes;
	int m_iArchive;
	int m_iReadOnly;
	int m_iHidden;
	int m_iSystem;
	int m_iDirectory;
};

class LIBCHCORE_API TFiltersArray
{
public:
	TFiltersArray() {}
	~TFiltersArray() {}

	TFiltersArray& operator=(const TFiltersArray& rSrc);
	bool Match(const TFileInfoPtr& spInfo) const;

	void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

	void Serialize(TReadBinarySerializer& rSerializer);
	void Serialize(TWriteBinarySerializer& rSerializer) const;

	bool IsEmpty() const;

	void Add(const TFileFilter& rFilter);
	bool SetAt(size_t stIndex, const TFileFilter& rNewFilter);
	const TFileFilter* GetAt(size_t stIndex) const;
	bool RemoveAt(size_t stIndex);
	size_t GetSize() const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<TFileFilter> m_vFilters;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

CONFIG_MEMBER_SERIALIZATION(TFiltersArray)

#endif
