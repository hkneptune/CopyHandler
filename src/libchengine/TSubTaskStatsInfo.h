// ============================================================================
//  Copyright (C) 2001-2012 by Jozef Starosczyk
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
/// @file  TSubTaskStatsInfo.h
/// @date  2012/02/22
/// @brief Contains declaration of class responsible for tracking stats for subtasks.
// ============================================================================
#ifndef __TSUBTASKSTATSINFO_H__
#define __TSUBTASKSTATSINFO_H__

#include "../libstring/TString.h"
#include "../libchcore/TSimpleTimer.h"
#include "TSpeedTracker.h"
#include "ESubTaskTypes.h"
#include "TSubTaskStatsSnapshot.h"
#include <bitset>
#include "CommonDataTypes.h"
#include "IRunningTimeControl.h"
#include "../libserializer/ISerializer.h"
#include "../libserializer/TSharedModificationTracker.h"

namespace serializer {
	class IColumnsDefinition;
	class ISerializerRowData;
}

namespace boost
{
	template<class T> class upgrade_lock;
}

namespace chengine
{
	class TSubTaskStatsInfo;
	class TSubTaskStatsSnapshot;

	class TSubTaskStatsInfo : public IRunningTimeControl
	{
	private:
		static const unsigned long long DefaultSpeedTrackTime = 1000;	// in miliseconds
		static const unsigned long long DefaultSpeedSampleTime = 100;	// in miliseconds

	public:
		explicit TSubTaskStatsInfo(ESubOperationType eSubTaskType, bool bIgnoreSizeSpeed = false);
		TSubTaskStatsInfo(const TSubTaskStatsInfo&) = delete;

		TSubTaskStatsInfo& operator=(const TSubTaskStatsInfo&) = delete;

		void Init(int iCurrentBufferIndex, file_count_t fcTotalCount, file_count_t fcProcessedCount, unsigned long long ullTotalSize, unsigned long long ullProcessedSize, const string::TString& strCurrentPath);
		void Clear();
		bool WasAdded() const;

		bool IsInitialized() const;

		void GetSnapshot(TSubTaskStatsSnapshotPtr& spStatsSnapshot) const;

		void IncreaseProcessedCount(file_count_t fcIncreaseBy);
		void SetProcessedCount(file_count_t fcIndex);

		void SetTotalCount(file_count_t fcCount);
		file_count_t GetTotalCount() const;

		// size stats
		void IncreaseProcessedSize(unsigned long long ullIncreaseBy);
		void DecreaseProcessedSize(unsigned long long ullDecreaseBy);
		void SetProcessedSize(unsigned long long ullProcessedSize);

		void IncreaseTotalSize(unsigned long long ullIncreaseBy);
		void DecreaseTotalSize(unsigned long long ullDecreaseBy);
		void SetTotalSize(unsigned long long ullTotalSize);

		// current item
		void SetCurrentItemSizes(unsigned long long ullProcessedSize, unsigned long long ullTotalSize);
		void ResetCurrentItemProcessedSize();
		unsigned long long GetCurrentItemProcessedSize() const;
		unsigned long long GetCurrentItemTotalSize() const;

		// current item processed and overall processed
		bool WillAdjustProcessedSizeExceedTotalSize(file_size_t fsIncludedProcessedSize, file_size_t fsNewProcessedSize);
		void AdjustProcessedSize(file_size_t fsIncludedProcessedSize, file_size_t fsNewProcessedSize);
		void AdjustTotalSize(file_size_t fsIncludedSize, file_size_t fsNewSize);

		bool CanCurrentItemSilentResume() const;
		void SetCurrentItemSilentResume(bool bEnableSilentResume);

		// current index
		void SetCurrentIndex(file_count_t fcIndex);
		file_count_t GetCurrentIndex() const;

		// buffer index
		void SetCurrentBufferIndex(int iCurrentIndex);

		// current path
		void SetCurrentPath(const string::TString& strPath);

		ESubOperationType GetSubOperationType() const { return m_eSubOperationType; }

		// serialization
		void Store(serializer::ISerializerRowData& rRowData) const;
		static void InitColumns(serializer::IColumnsDefinition& rColumnDefs);
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader);

	private:
		// is running?
		void MarkAsRunning() override;
		void MarkAsNotRunning() override;

		// time tracking
		void EnableTimeTracking() override;
		void DisableTimeTracking() override;

		void UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const;

		void VerifyProcessedVsTotal();

	private:
		enum EModifications
		{
			eMod_Added = 0,
			eMod_IsRunning,
			eMod_TotalSize,
			eMod_ProcessedSize,
			eMod_SizeSpeed,
			eMod_TotalCount,
			eMod_ProcessedCount,
			eMod_CountSpeed,
			eMod_CurrentItemProcessedSize,
			eMod_CurrentItemTotalSize,
			eMod_Timer,
			eMod_CurrentBufferIndex,
			eMod_CurrentPath,
			eMod_IsInitialized,
			eMod_CurrentItemIndex,
			eMod_CurrentItemCanResumeSilently,

			// last item
			eMod_Last
		};

		typedef std::bitset<eMod_Last> Bitset;
		mutable Bitset m_setModifications;

		serializer::TSharedModificationTracker<bool, Bitset, eMod_IsRunning> m_bSubTaskIsRunning;

		serializer::TSharedModificationTracker<unsigned long long, Bitset, eMod_TotalSize> m_ullTotalSize;
		serializer::TSharedModificationTracker<unsigned long long, Bitset, eMod_ProcessedSize> m_ullProcessedSize;
		mutable serializer::TSharedModificationTracker<TSpeedTracker, Bitset, eMod_SizeSpeed> m_tSizeSpeed;

		serializer::TSharedModificationTracker<file_count_t, Bitset, eMod_TotalCount> m_fcTotalCount;
		serializer::TSharedModificationTracker<file_count_t, Bitset, eMod_ProcessedCount> m_fcProcessedCount;
		mutable serializer::TSharedModificationTracker<TSpeedTracker, Bitset, eMod_CountSpeed> m_tCountSpeed;

		serializer::TSharedModificationTracker<file_count_t, Bitset, eMod_CurrentItemIndex> m_fcCurrentIndex;	//??

		serializer::TSharedModificationTracker<unsigned long long, Bitset, eMod_CurrentItemProcessedSize> m_ullCurrentItemProcessedSize;	//??
		serializer::TSharedModificationTracker<unsigned long long, Bitset, eMod_CurrentItemTotalSize> m_ullCurrentItemTotalSize;			//??
		serializer::TSharedModificationTracker<bool, Bitset, eMod_CurrentItemCanResumeSilently> m_bCurrentItemSilentResume;					//??

		mutable serializer::TSharedModificationTracker<chcore::TSimpleTimer, Bitset, eMod_Timer> m_tTimer;

		serializer::TSharedModificationTracker<int, Bitset, eMod_CurrentBufferIndex> m_iCurrentBufferIndex;	//??
		serializer::TSharedModificationTracker<string::TString, Bitset, eMod_CurrentPath> m_strCurrentPath;		//??

		serializer::TSharedModificationTracker<bool, Bitset, eMod_IsInitialized> m_bIsInitialized;

		const ESubOperationType m_eSubOperationType;
		bool m_bIgnoreSizeSpeed = false;

#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)

		friend class TSubTaskProcessingGuard;
	};

	using TSubTaskStatsInfoPtr = std::shared_ptr<TSubTaskStatsInfo>;
}

#endif
