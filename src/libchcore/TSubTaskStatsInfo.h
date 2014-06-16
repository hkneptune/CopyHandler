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

#include "libchcore.h"
#include "TString.h"
#include "TSimpleTimer.h"
#include "TSpeedTracker.h"
#include "ESubTaskTypes.h"
#include "TSubTaskStatsSnapshot.h"
#include "ISerializerRowData.h"
#include "ISerializerRowReader.h"
#include "TSharedModificationTracker.h"
#include <bitset>

BEGIN_CHCORE_NAMESPACE

class TSubTaskStatsInfo;
class TSubTaskStatsSnapshot;

// class used to guard scope of the subtask processing (
class TSubTaskProcessingGuard
{
public:
	TSubTaskProcessingGuard(TSubTaskStatsInfo& rStats);
	~TSubTaskProcessingGuard();

private:
	TSubTaskProcessingGuard(const TSubTaskProcessingGuard&);
	TSubTaskProcessingGuard& operator=(const TSubTaskProcessingGuard&);

private:
	TSubTaskStatsInfo& m_rStats;
};

class TSubTaskStatsInfo
{
private:
	static const unsigned long long DefaultSpeedTrackTime = 1000;	// in miliseconds
	static const unsigned long long DefaultSpeedSampleTime = 100;	// in miliseconds

public:
	TSubTaskStatsInfo();

	void Init(int iCurrentBufferIndex, size_t stTotalCount, size_t stProcessedCount, unsigned long long ullTotalSize, unsigned long long ullProcessedSize, const TString& strCurrentPath);
	void Clear();

	bool IsInitialized() const;

	void GetSnapshot(TSubTaskStatsSnapshotPtr& spStatsSnapshot) const;

	void IncreaseProcessedCount(size_t stIncreaseBy);
	void SetProcessedCount(size_t stIndex);

	void SetTotalCount(size_t stCount);

	// size stats
	void IncreaseProcessedSize(unsigned long long ullIncreaseBy);
	void DecreaseProcessedSize(unsigned long long ullDecreaseBy);
	void SetProcessedSize(unsigned long long ullProcessedSize);

	void SetTotalSize(unsigned long long ullTotalSize);

	// current item
	void IncreaseCurrentItemProcessedSize(unsigned long long ullIncreaseBy);
	void DecreaseCurrentItemProcessedSize(unsigned long long ullDecreaseBy);
	void SetCurrentItemProcessedSize(unsigned long long ullProcessedSize);

	void SetCurrentItemTotalSize(unsigned long long ullTotalSize);

	// buffer index
	void SetCurrentBufferIndex(int iCurrentIndex);

	// current path
	void SetCurrentPath(const TString& strPath);

	ESubOperationType GetSubOperationType() const { return m_eSubOperationType; }
	void SetSubOperationType(ESubOperationType val) { m_eSubOperationType = val; }

	// serialization
	void Store(const ISerializerRowDataPtr& spRowData) const;
	static void InitLoader(IColumnsDefinition& rColumnDefs);
	void Load(const ISerializerRowReaderPtr& spRowReader);

private:
	TSubTaskStatsInfo(const TSubTaskStatsInfo&);
	TSubTaskStatsInfo& operator=(const TSubTaskStatsInfo&);

	// is running?
	void MarkAsRunning();
	void MarkAsNotRunning();

	// time tracking
	void EnableTimeTracking();
	void DisableTimeTracking();
	void UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const;

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
		eMod_SubOperationType,
		eMod_IsInitialized,

		// last item
		eMod_Last
	};

	typedef std::bitset<eMod_Last> Bitset;
	mutable Bitset m_setModifications;

	TSharedModificationTracker<bool, Bitset, eMod_IsRunning> m_bSubTaskIsRunning;

	TSharedModificationTracker<unsigned long long, Bitset, eMod_TotalSize> m_ullTotalSize;
	TSharedModificationTracker<unsigned long long, Bitset, eMod_ProcessedSize> m_ullProcessedSize;
	mutable TSharedModificationTracker<TSpeedTracker, Bitset, eMod_SizeSpeed> m_tSizeSpeed;

	TSharedModificationTracker<size_t, Bitset, eMod_TotalCount> m_stTotalCount;
	TSharedModificationTracker<size_t, Bitset, eMod_ProcessedCount> m_stProcessedCount;
	mutable TSharedModificationTracker<TSpeedTracker, Bitset, eMod_CountSpeed> m_tCountSpeed;

	TSharedModificationTracker<unsigned long long, Bitset, eMod_CurrentItemProcessedSize> m_ullCurrentItemProcessedSize;
	TSharedModificationTracker<unsigned long long, Bitset, eMod_CurrentItemTotalSize> m_ullCurrentItemTotalSize;

	mutable TSharedModificationTracker<TSimpleTimer, Bitset, eMod_Timer> m_tTimer;

	TSharedModificationTracker<int, Bitset, eMod_CurrentBufferIndex> m_iCurrentBufferIndex;

	TSharedModificationTracker<TString, Bitset, eMod_CurrentPath> m_strCurrentPath;		// currently processed path

	TSharedModificationTracker<ESubOperationType, Bitset, eMod_SubOperationType> m_eSubOperationType;

	TSharedModificationTracker<bool, Bitset, eMod_IsInitialized> m_bIsInitialized;

#pragma warning(push)
#pragma warning(disable: 4251)
	mutable boost::shared_mutex m_lock;
#pragma warning(pop)

	friend class TSubTaskProcessingGuard;
};

END_CHCORE_NAMESPACE

#endif
