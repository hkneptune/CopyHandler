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
#ifndef __TFILEFILTER_H__
#define __TFILEFILTER_H__

#include <atltime.h>
#include "TDateTime.h"
#include <bitset>
#include "../libstring/TStringPatternArray.h"
#include "../libserializer/TSharedModificationTracker.h"
#include "ECompareType.h"
#include "../libserializer/SerializableObject.h"

namespace chengine
{
	class TConfig;
	class TFileInfo;
	typedef std::shared_ptr<TFileInfo> TFileInfoPtr;

	namespace FileFilterEnum
	{
		enum EModifications
		{
			eMod_Added,
			eMod_UseMask,
			eMod_Mask,
			eMod_UseExcludeMask,
			eMod_ExcludeMask,
			eMod_UseSize1,
			eMod_SizeCmpType1,
			eMod_Size1,
			eMod_UseSize2,
			eMod_SizeCmpType2,
			eMod_Size2,
			eMod_DateType,
			eMod_UseDateTime1,
			eMod_DateCmpType1,
			eMod_UseDate1,
			eMod_UseTime1,
			eMod_DateTime1,
			eMod_UseDateTime2,
			eMod_DateCmpType2,
			eMod_UseDate2,
			eMod_UseTime2,
			eMod_DateTime2,
			eMod_UseAttributes,
			eMod_AttrArchive,
			eMod_AttrReadOnly,
			eMod_AttrHidden,
			eMod_AttrSystem,
			eMod_AttrDirectory,

			eMod_Last
		};
	};

#pragma warning(push)
#pragma warning(disable: 4251)
	class LIBCHENGINE_API TFileFilter : public serializer::SerializableObject<FileFilterEnum::eMod_Last>
	{
	public:
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

		bool operator==(const TFileFilter& rSrc) const;
		bool operator!=(const TFileFilter& rSrc) const;

		void SetData(const TFileFilter& rSrc);

		// matching
		bool Match(const TFileInfoPtr& spInfo) const;

		// serialization
		void StoreInConfig(TConfig& rConfig) const;
		void ReadFromConfig(const TConfig& rConfig);

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader);
		static void InitColumns(serializer::IColumnsDefinition& rColumns);

		// other
		serializer::object_id_t GetObjectID() const;
		void SetObjectID(serializer::object_id_t oidObjectID);
		void ResetModifications();

		// atrributes access
		bool GetUseMask() const;
		void SetUseMask(bool bUseMask);

		string::TString GetCombinedMask() const;
		void SetCombinedMask(const string::TString& pMask);

		bool GetUseExcludeMask() const;
		void SetUseExcludeMask(bool bUseExcludeMask);

		string::TString GetCombinedExcludeMask() const;
		void SetCombinedExcludeMask(const string::TString& pMask);

		bool GetUseSize1() const;
		void SetUseSize1(bool bUseSize1);

		ECompareType GetSizeType1() const;
		void SetSizeType1(ECompareType eSizeType1);

		unsigned long long GetSize1() const;
		void SetSize1(unsigned long long ullSize1);

		bool GetUseSize2() const;
		void SetUseSize2(bool bUseSize2);

		ECompareType GetSizeType2() const;
		void SetSizeType2(ECompareType eSizeType2);

		unsigned long long GetSize2() const;
		void SetSize2(unsigned long long ullSize2);

		// dates
		TFileFilter::EDateType GetDateType() const;
		void SetDateType(TFileFilter::EDateType eDateType);

		// date 1
		bool GetUseDateTime1() const;
		void SetUseDateTime1(bool bUseDateTime1);

		ECompareType GetDateCmpType1() const;
		void SetDateCmpType1(ECompareType eCmpType1);

		bool GetUseDate1() const;
		void SetUseDate1(bool tDate1);

		bool GetUseTime1() const;
		void SetUseTime1(bool tTime1);

		const TDateTime& GetDateTime1() const;
		void SetDateTime1(const TDateTime& tDateTime1);

		// date 2
		bool GetUseDateTime2() const;
		void SetUseDateTime2(bool bUseDateTime2);

		ECompareType GetDateCmpType2() const;
		void SetDateCmpType2(ECompareType eCmpType2);

		bool GetUseDate2() const;
		void SetUseDate2(bool tDate2);

		bool GetUseTime2() const;
		void SetUseTime2(bool tTime2);

		const TDateTime& GetDateTime2() const;
		void SetDateTime2(const TDateTime& tDateTime2);

		// attributes
		bool GetUseAttributes() const;
		void SetUseAttributes(bool bUseAttributes);

		int GetArchive() const;
		void SetArchive(int iArchive);

		int GetReadOnly() const;
		void SetReadOnly(int iReadOnly);

		int GetHidden() const;
		void SetHidden(int iHidden);

		int GetSystem() const { return m_iSystem; }
		void SetSystem(int iSystem) { m_iSystem = iSystem; }

		int GetDirectory() const { return m_iDirectory; }
		void SetDirectory(int iDirectory) { m_iDirectory = iDirectory; }

	private:
		// modification management

		// files mask
		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseMask> m_bUseMask;
		serializer::TSharedModificationTracker<string::TStringPatternArray, Bitset, FileFilterEnum::eMod_Mask> m_astrMask;

		// files mask-
		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseExcludeMask> m_bUseExcludeMask;
		serializer::TSharedModificationTracker<string::TStringPatternArray, Bitset, FileFilterEnum::eMod_ExcludeMask> m_astrExcludeMask;

		// size filtering
		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseSize1> m_bUseSize1;
		serializer::TSharedModificationTracker<ECompareType, Bitset, FileFilterEnum::eMod_SizeCmpType1> m_eSizeCmpType1;
		serializer::TSharedModificationTracker<unsigned long long, Bitset, FileFilterEnum::eMod_Size1> m_ullSize1;

		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseSize2> m_bUseSize2;
		serializer::TSharedModificationTracker<ECompareType, Bitset, FileFilterEnum::eMod_SizeCmpType2> m_eSizeCmpType2;
		serializer::TSharedModificationTracker<unsigned long long, Bitset, FileFilterEnum::eMod_Size2> m_ullSize2;

		// date filtering
		serializer::TSharedModificationTracker<EDateType, Bitset, FileFilterEnum::eMod_DateType> m_eDateType;	// created/last modified/last accessed

		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseDateTime1> m_bUseDateTime1;

		serializer::TSharedModificationTracker<ECompareType, Bitset, FileFilterEnum::eMod_DateCmpType1> m_eDateCmpType1;	// before/after
		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseDate1> m_bUseDate1;
		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseTime1> m_bUseTime1;
		serializer::TSharedModificationTracker<TDateTime, Bitset, FileFilterEnum::eMod_DateTime1> m_tDateTime1;

		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseDateTime2> m_bUseDateTime2;

		serializer::TSharedModificationTracker<ECompareType, Bitset, FileFilterEnum::eMod_DateCmpType2> m_eDateCmpType2;
		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseDate2> m_bUseDate2;
		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseTime2> m_bUseTime2;
		serializer::TSharedModificationTracker<TDateTime, Bitset, FileFilterEnum::eMod_DateTime2> m_tDateTime2;

		// attribute filtering
		serializer::TSharedModificationTracker<bool, Bitset, FileFilterEnum::eMod_UseAttributes> m_bUseAttributes;
		serializer::TSharedModificationTracker<int, Bitset, FileFilterEnum::eMod_AttrArchive> m_iArchive;
		serializer::TSharedModificationTracker<int, Bitset, FileFilterEnum::eMod_AttrReadOnly> m_iReadOnly;
		serializer::TSharedModificationTracker<int, Bitset, FileFilterEnum::eMod_AttrHidden> m_iHidden;
		serializer::TSharedModificationTracker<int, Bitset, FileFilterEnum::eMod_AttrSystem> m_iSystem;
		serializer::TSharedModificationTracker<int, Bitset, FileFilterEnum::eMod_AttrDirectory> m_iDirectory;
	};
#pragma warning(pop)
}

#endif
