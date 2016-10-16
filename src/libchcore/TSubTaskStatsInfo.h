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

#include "TString.h"
#include "TSimpleTimer.h"
#include "TSpeedTracker.h"
#include "ESubTaskTypes.h"
#include "TSubTaskStatsSnapshot.h"
#include "ISerializerRowData.h"
#include "ISerializerRowReader.h"
#include "TSharedModificationTracker.h"
#include <bitset>
#include "CommonDataTypes.h"
#include "IRunningTimeControl.h"

namespace boost
{
	template<class T> class upgrade_lock;
}

namespace chcore
{
	class TSubTaskStatsInfo;
	class TSubTaskStatsSnapshot;

	class TSubTaskStatsInfo : public IRunningTimeControl
	{
	private:
		static const unsigned long long DefaultSpeedTrackTime = 1000;	// in miliseconds
		static const unsigned long long DefaultSpeedSampleTime = 100;	// in miliseconds

	public:
		explicit TSubTaskStatsInfo(ESubOperationType eSubTaskType);

		void Init(int iCurrentBufferIndex, file_count_t fcTotalCount, file_count_t fcProcessedCount, unsigned long long ullTotalSize, unsigned long long ullProcessedSize, const TString& strCurrentPath);
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
		void SetCurrentPath(const TString& strPath);

		ESubOperationType GetSubOperationType() const { return m_eSubOperationType; }

		// serialization
		void Store(ISerializerRowData& rRowData) const;
		static void InitColumns(IColumnsDefinition& rColumnDefs);
		void Load(const ISerializerRowReaderPtr& spRowReader);

	private:
		TSubTaskStatsInfo(const TSubTaskStatsInfo&) = delete;
		TSubTaskStatsInfo& operator=(const TSubTaskStatsInfo&) = delete;

		// is running?
		virtual void MarkAsRunning() override;
		virtual void MarkAsNotRunning() override;

		// time tracking
		virtual void EnableTimeTracking() override;
		virtual void DisableTimeTracking() override;

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

		TSharedModificationTracker<bool, Bitset, eMod_IsRunning> m_bSubTaskIsRunning;

		TSharedModificationTracker<unsigned long long, Bitset, eMod_TotalSize> m_ullTotalSize;
		TSharedModificationTracker<unsigned long long, Bitset, eMod_ProcessedSize> m_ullProcessedSize;
		mutable TSharedModificationTracker<TSpeedTracker, Bitset, eMod_SizeSpeed> m_tSizeSpeed;

		TSharedModificationTracker<file_count_t, Bitset, eMod_TotalCount> m_fcTotalCount;
		TSharedModificationTracker<file_count_t, Bitset, eMod_ProcessedCount> m_fcProcessedCount;
		mutable TSharedModificationTracker<TSpeedTracker, Bitset, eMod_CountSpeed> m_tCountSpeed;

		TSharedModificationTracker<file_count_t, Bitset, eMod_CurrentItemIndex> m_fcCurrentIndex;

		TSharedModificationTracker<unsigned long long, Bitset, eMod_CurrentItemProcessedSize> m_ullCurrentItemProcessedSize;
		TSharedModificationTracker<unsigned long long, Bitset, eMod_CurrentItemTotalSize> m_ullCurrentItemTotalSize;
		TSharedModificationTracker<bool, Bitset, eMod_CurrentItemCanResumeSilently> m_bCurrentItemSilentResume;

		mutable TSharedModificationTracker<TSimpleTimer, Bitset, eMod_Timer> m_tTimer;

		TSharedModificationTracker<int, Bitset, eMod_CurrentBufferIndex> m_iCurrentBufferIndex;

		TSharedModificationTracker<TString, Bitset, eMod_CurrentPath> m_strCurrentPath;		// currently processed path

		TSharedModificationTracker<bool, Bitset, eMod_IsInitialized> m_bIsInitialized;

		const ESubOperationType m_eSubOperationType;

#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)

		friend class TSubTaskProcessingGuard;
	};
}

#endif
