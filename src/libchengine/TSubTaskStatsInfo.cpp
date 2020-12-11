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
/// @file  TSubTaskStatsInfo.cpp
/// @date  2012/02/22
/// @brief Contains definition of class responsible for tracking stats for subtasks.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskStatsInfo.h"
#include <boost/numeric/conversion/cast.hpp>
#include "TSubTaskStatsSnapshot.h"
#include "../libchcore/TCoreException.h"
#include "../libchcore/ErrorCodes.h"
#include <boost/thread/locks.hpp>
#include "../libserializer/ISerializerRowData.h"
#include "../libchcore/MathFunctions.h"

using namespace chcore;
using namespace string;
using namespace serializer;

namespace chengine
{
	///////////////////////////////////////////////////////////////////////////////////
	// class TSubTaskStatsInfo

	TSubTaskStatsInfo::TSubTaskStatsInfo(ESubOperationType eSubTaskType, bool bIgnoreSizeSpeed) :
		m_bSubTaskIsRunning(m_setModifications, false),
		m_ullTotalSize(m_setModifications, 0),
		m_ullProcessedSize(m_setModifications, 0),
		m_tSizeSpeed(m_setModifications, DefaultSpeedTrackTime, DefaultSpeedSampleTime),
		m_fcTotalCount(m_setModifications, 0),
		m_fcProcessedCount(m_setModifications, 0),
		m_tCountSpeed(m_setModifications, DefaultSpeedTrackTime, DefaultSpeedSampleTime),
		m_fcCurrentIndex(m_setModifications, 0),
		m_ullCurrentItemProcessedSize(m_setModifications, 0),
		m_ullCurrentItemTotalSize(m_setModifications, 0),
		m_bCurrentItemSilentResume(m_setModifications, false),
		m_tTimer(m_setModifications),
		m_iCurrentBufferIndex(m_setModifications, 0),
		m_strCurrentPath(m_setModifications),
		m_bIsInitialized(m_setModifications, false),
		m_eSubOperationType(eSubTaskType),
		m_bIgnoreSizeSpeed(bIgnoreSizeSpeed)
	{
		m_setModifications[eMod_Added] = true;
	}

	void TSubTaskStatsInfo::Clear()
	{
		m_bSubTaskIsRunning = false;
		m_ullTotalSize = 0;
		m_ullProcessedSize = 0;
		m_fcTotalCount = 0UL;
		m_fcProcessedCount = 0UL;
		m_iCurrentBufferIndex = 0;
		m_strCurrentPath.Modify().Clear();
		m_tTimer.Modify().Reset();
		m_tSizeSpeed.Modify().Clear();
		m_tCountSpeed.Modify().Clear();
		m_ullCurrentItemProcessedSize = 0;
		m_ullCurrentItemTotalSize = 0;
		m_bIsInitialized = false;
		m_fcCurrentIndex = 0UL;
		m_bCurrentItemSilentResume = false;
	}

	void TSubTaskStatsInfo::GetSnapshot(TSubTaskStatsSnapshotPtr& spStatsSnapshot) const
	{
		if (!spStatsSnapshot)
			throw TCoreException(eErr_InvalidArgument, L"spStatsSnapshot", LOCATION);

		spStatsSnapshot->Clear();

		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
		if (m_bSubTaskIsRunning)
			UpdateTime(lock);

		auto totalTime = m_tTimer.Get().GetTotalTime();

		spStatsSnapshot->SetRunning(m_bSubTaskIsRunning);
		spStatsSnapshot->SetProcessedCount(m_fcProcessedCount);
		spStatsSnapshot->SetTotalCount(m_fcTotalCount);
		spStatsSnapshot->SetProcessedSize(m_ullProcessedSize);
		spStatsSnapshot->SetTotalSize(m_ullTotalSize);
		spStatsSnapshot->SetCurrentBufferIndex(m_iCurrentBufferIndex);
		spStatsSnapshot->SetCurrentPath(m_strCurrentPath);
		spStatsSnapshot->SetTimeElapsed(totalTime);

		spStatsSnapshot->SetSizeSpeed(m_tSizeSpeed.Get().GetSpeed());
		if(!m_bIgnoreSizeSpeed)
			spStatsSnapshot->SetAvgSizeSpeed(totalTime != 0 ? Math::Div64(m_ullProcessedSize, totalTime / 1000.0) : 0.0);
		else
			spStatsSnapshot->SetAvgSizeSpeed(0.0);

		spStatsSnapshot->SetCountSpeed(m_tCountSpeed.Get().GetSpeed());
		spStatsSnapshot->SetAvgCountSpeed(totalTime != 0 ? Math::Div64(m_fcProcessedCount, totalTime / 1000.0) : 0.0);

		spStatsSnapshot->SetCurrentItemProcessedSize(m_ullCurrentItemProcessedSize);
		spStatsSnapshot->SetCurrentItemTotalSize(m_ullCurrentItemTotalSize);
		spStatsSnapshot->SetSubOperationType(m_eSubOperationType);
		spStatsSnapshot->SetCurrentIndex(m_fcCurrentIndex);
	}

	// is running?
	void TSubTaskStatsInfo::MarkAsRunning()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_bSubTaskIsRunning = true;
	}

	void TSubTaskStatsInfo::MarkAsNotRunning()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_bSubTaskIsRunning = false;
	}

	void TSubTaskStatsInfo::IncreaseProcessedCount(file_count_t fcIncreaseBy)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_fcProcessedCount.Modify() += fcIncreaseBy;

		m_tCountSpeed.Modify().AddSample(fcIncreaseBy, m_tTimer.Modify().Tick());

		_ASSERTE(m_fcProcessedCount <= m_fcTotalCount);
		if (m_fcProcessedCount > m_fcTotalCount)
			throw TCoreException(eErr_InternalProblem, L"Count of processed files exceeded the total", LOCATION);
	}

	void TSubTaskStatsInfo::SetProcessedCount(file_count_t fcProcessedCount)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_tCountSpeed.Modify().AddSample(fcProcessedCount > m_fcProcessedCount ? fcProcessedCount - m_fcProcessedCount : 0, m_tTimer.Modify().Tick());

		m_fcProcessedCount = fcProcessedCount;

		_ASSERTE(m_fcProcessedCount <= m_fcTotalCount);
		if (m_fcProcessedCount > m_fcTotalCount)
			throw TCoreException(eErr_InternalProblem, L"Count of processed files exceeded total", LOCATION);
	}

	void TSubTaskStatsInfo::SetTotalCount(file_count_t fcCount)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_fcTotalCount = fcCount;
		_ASSERTE(m_fcProcessedCount <= m_fcTotalCount);
		if (m_fcProcessedCount > m_fcTotalCount)
			throw TCoreException(eErr_InternalProblem, L"Count of processed files exceeded total", LOCATION);
	}

	file_count_t TSubTaskStatsInfo::GetTotalCount() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_fcTotalCount;
	}

	void TSubTaskStatsInfo::IncreaseProcessedSize(unsigned long long ullIncreaseBy)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_ullProcessedSize.Modify() += ullIncreaseBy;

		if(!m_bIgnoreSizeSpeed)
			m_tSizeSpeed.Modify().AddSample(ullIncreaseBy, m_tTimer.Modify().Tick());

		_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
		if (m_ullProcessedSize > m_ullTotalSize)
			throw TCoreException(eErr_InternalProblem, L"Size of processed files exceeded total size", LOCATION);
	}

	void TSubTaskStatsInfo::DecreaseProcessedSize(unsigned long long ullDecreaseBy)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_ullProcessedSize.Modify() -= ullDecreaseBy;

		// we didn't process anything here - hence the 0-sized sample
		if(!m_bIgnoreSizeSpeed)
			m_tSizeSpeed.Modify().AddSample(0, m_tTimer.Modify().Tick());

		_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
		if (m_ullProcessedSize > m_ullTotalSize)
			throw TCoreException(eErr_InternalProblem, L"Size of processed files exceeded total size", LOCATION);
	}

	void TSubTaskStatsInfo::SetProcessedSize(unsigned long long ullProcessedSize)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		if(!m_bIgnoreSizeSpeed)
			m_tSizeSpeed.Modify().AddSample(ullProcessedSize > m_ullProcessedSize ? ullProcessedSize - m_ullProcessedSize : 0, m_tTimer.Modify().Tick());

		m_ullProcessedSize = ullProcessedSize;
		_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
		if (m_ullProcessedSize > m_ullTotalSize)
			throw TCoreException(eErr_InternalProblem, L"Size of processed files exceeded total size", LOCATION);
	}

	void TSubTaskStatsInfo::SetTotalSize(unsigned long long ullTotalSize)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_ullTotalSize = ullTotalSize;
		_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
		if (m_ullProcessedSize > m_ullTotalSize)
			throw TCoreException(eErr_InternalProblem, L"Size of processed files exceeded total size", LOCATION);
	}

	// current item
	void TSubTaskStatsInfo::ResetCurrentItemProcessedSize()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_ullCurrentItemProcessedSize = 0;
	}

	void TSubTaskStatsInfo::SetCurrentItemSizes(unsigned long long ullProcessedSize, unsigned long long ullTotalSize)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_ullCurrentItemProcessedSize = ullProcessedSize;
		m_ullCurrentItemTotalSize = ullTotalSize;

		_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
		if (m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
			throw TCoreException(eErr_InternalProblem, L"Current file processed size exceeded total size", LOCATION);
	}

	bool TSubTaskStatsInfo::WillAdjustProcessedSizeExceedTotalSize(file_size_t fsIncludedProcessedSize, file_size_t fsNewProcessedSize)
	{
		if (fsNewProcessedSize <= fsIncludedProcessedSize)
			return false;

		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		file_size_t fsDiff = fsNewProcessedSize - fsIncludedProcessedSize;
		if (m_ullCurrentItemProcessedSize.Get() + fsDiff > m_ullCurrentItemTotalSize.Get())
			return true;

		return false;
	}

	void TSubTaskStatsInfo::AdjustProcessedSize(file_size_t fsIncludedProcessedSize, file_size_t fsNewProcessedSize)
	{
		if (fsNewProcessedSize == fsIncludedProcessedSize)
			return;

		file_size_t fsDiff = 0;

		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		if (fsNewProcessedSize > fsIncludedProcessedSize)
		{
			fsDiff = fsNewProcessedSize - fsIncludedProcessedSize;

			m_ullCurrentItemProcessedSize.Modify() += fsDiff;
			m_ullProcessedSize.Modify() += fsDiff;

			if(!m_bIgnoreSizeSpeed)
				m_tSizeSpeed.Modify().AddSample(fsDiff, m_tTimer.Modify().Tick());
		}
		else
		{
			fsDiff = fsIncludedProcessedSize - fsNewProcessedSize;

			m_ullCurrentItemProcessedSize.Modify() -= fsDiff;
			m_ullProcessedSize.Modify() -= fsDiff;

			if(!m_bIgnoreSizeSpeed)
				m_tSizeSpeed.Modify().AddSample(0, m_tTimer.Modify().Tick());
		}

		VerifyProcessedVsTotal();
	}

	void TSubTaskStatsInfo::AdjustTotalSize(file_size_t fsIncludedSize, file_size_t fsNewSize)
	{
		if (fsIncludedSize == fsNewSize)
			return;

		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		if (fsNewSize > fsIncludedSize)
		{
			m_ullTotalSize.Modify() += fsNewSize - fsIncludedSize;
			m_ullCurrentItemTotalSize.Modify() += fsNewSize - fsIncludedSize;
		}
		else
		{
			m_ullTotalSize.Modify() -= fsIncludedSize - fsNewSize;
			m_ullCurrentItemTotalSize.Modify() -= fsIncludedSize - fsNewSize;
		}

		VerifyProcessedVsTotal();
	}

	// buffer index
	void TSubTaskStatsInfo::SetCurrentBufferIndex(int iCurrentIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_iCurrentBufferIndex = iCurrentIndex;
	}
	// current path
	void TSubTaskStatsInfo::SetCurrentPath(const TString& strPath)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_strCurrentPath = strPath;
	}

	// time
	void TSubTaskStatsInfo::EnableTimeTracking()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_tTimer.Modify().Start();
	}

	void TSubTaskStatsInfo::DisableTimeTracking()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_tTimer.Modify().Stop();
	}

	void TSubTaskStatsInfo::UpdateTime(boost::upgrade_lock<boost::shared_mutex>& lock) const
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		if (m_tTimer.Get().IsRunning())
		{
			m_tTimer.Modify().Tick();
			if(!m_bIgnoreSizeSpeed)
				m_tSizeSpeed.Modify().AddSample(0, m_tTimer.Get().GetLastTimestamp());
			m_tCountSpeed.Modify().AddSample(0, m_tTimer.Get().GetLastTimestamp());
		}
	}

	void TSubTaskStatsInfo::Store(ISerializerRowData& rRowData) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		if (m_bSubTaskIsRunning.IsModified())
			rRowData.SetValue(_T("is_running"), m_bSubTaskIsRunning);
		if (m_bIsInitialized.IsModified())
			rRowData.SetValue(_T("is_initialized"), m_bIsInitialized);

		if (m_ullTotalSize.IsModified())
			rRowData.SetValue(_T("total_size"), m_ullTotalSize);

		if (m_ullProcessedSize.IsModified())
			rRowData.SetValue(_T("processed_size"), m_ullProcessedSize);
		if (m_tSizeSpeed.IsModified())
			rRowData.SetValue(_T("size_speed"), m_tSizeSpeed.Get().ToString());

		if (m_fcTotalCount.IsModified())
			rRowData.SetValue(_T("total_count"), m_fcTotalCount);
		if (m_fcProcessedCount.IsModified())
			rRowData.SetValue(_T("processed_count"), m_fcProcessedCount);
		if (m_tCountSpeed.IsModified())
			rRowData.SetValue(_T("count_speed"), m_tCountSpeed.Get().ToString());

		if (m_ullCurrentItemProcessedSize.IsModified())
			rRowData.SetValue(_T("ci_processed_size"), m_ullCurrentItemProcessedSize);
		if (m_ullCurrentItemTotalSize.IsModified())
			rRowData.SetValue(_T("ci_total_size"), m_ullCurrentItemTotalSize);
		if (m_bCurrentItemSilentResume.IsModified())
			rRowData.SetValue(_T("ci_silent_resume"), m_bCurrentItemSilentResume);
		if (m_fcCurrentIndex.IsModified())
			rRowData.SetValue(_T("current_index"), m_fcCurrentIndex);

		if (m_tTimer.IsModified())
			rRowData.SetValue(_T("timer"), m_tTimer.Get().GetTotalTime());

		if (m_iCurrentBufferIndex.IsModified())
			rRowData.SetValue(_T("buffer_index"), m_iCurrentBufferIndex);

		if (m_strCurrentPath.IsModified())
			rRowData.SetValue(_T("current_path"), m_strCurrentPath);

		m_setModifications.reset();
	}

	void TSubTaskStatsInfo::InitColumns(IColumnsDefinition& rColumnDefs)
	{
		rColumnDefs.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumnDefs.AddColumn(_T("is_running"), IColumnsDefinition::eType_bool);
		rColumnDefs.AddColumn(_T("is_initialized"), IColumnsDefinition::eType_bool);
		rColumnDefs.AddColumn(_T("total_size"), IColumnsDefinition::eType_ulonglong);
		rColumnDefs.AddColumn(_T("processed_size"), IColumnsDefinition::eType_ulonglong);
		rColumnDefs.AddColumn(_T("size_speed"), IColumnsDefinition::eType_string);
		rColumnDefs.AddColumn(_T("total_count"), ColumnType<file_count_t>::value);
		rColumnDefs.AddColumn(_T("processed_count"), ColumnType<file_count_t>::value);
		rColumnDefs.AddColumn(_T("count_speed"), IColumnsDefinition::eType_string);
		rColumnDefs.AddColumn(_T("ci_processed_size"), IColumnsDefinition::eType_ulonglong);
		rColumnDefs.AddColumn(_T("ci_total_size"), IColumnsDefinition::eType_ulonglong);
		rColumnDefs.AddColumn(_T("ci_silent_resume"), IColumnsDefinition::eType_bool);
		rColumnDefs.AddColumn(_T("current_index"), ColumnType<file_count_t>::value);
		rColumnDefs.AddColumn(_T("timer"), IColumnsDefinition::eType_ulonglong);
		rColumnDefs.AddColumn(_T("buffer_index"), IColumnsDefinition::eType_int);
		rColumnDefs.AddColumn(_T("current_path"), IColumnsDefinition::eType_string);
	}

	void TSubTaskStatsInfo::Load(const ISerializerRowReaderPtr& spRowReader)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		spRowReader->GetValue(_T("is_running"), m_bSubTaskIsRunning.Modify());
		spRowReader->GetValue(_T("is_initialized"), m_bIsInitialized.Modify());

		spRowReader->GetValue(_T("total_size"), m_ullTotalSize.Modify());

		spRowReader->GetValue(_T("processed_size"), m_ullProcessedSize.Modify());

		TString strSpeed;
		spRowReader->GetValue(_T("size_speed"), strSpeed);
		m_tSizeSpeed.Modify().FromString(strSpeed);

		spRowReader->GetValue(_T("total_count"), m_fcTotalCount.Modify());
		spRowReader->GetValue(_T("processed_count"), m_fcProcessedCount.Modify());

		spRowReader->GetValue(_T("count_speed"), strSpeed);
		m_tCountSpeed.Modify().FromString(strSpeed);

		spRowReader->GetValue(_T("ci_processed_size"), m_ullCurrentItemProcessedSize.Modify());
		spRowReader->GetValue(_T("ci_total_size"), m_ullCurrentItemTotalSize.Modify());
		spRowReader->GetValue(_T("ci_silent_resume"), m_bCurrentItemSilentResume.Modify());
		spRowReader->GetValue(_T("current_index"), m_fcCurrentIndex.Modify());

		unsigned long long ullTimer = 0;
		spRowReader->GetValue(_T("timer"), ullTimer);
		m_tTimer.Modify().Init(ullTimer);

		spRowReader->GetValue(_T("buffer_index"), m_iCurrentBufferIndex.Modify());

		spRowReader->GetValue(_T("current_path"), m_strCurrentPath.Modify());

		m_setModifications.reset();
	}

	void TSubTaskStatsInfo::Init(int iCurrentBufferIndex, file_count_t fcTotalCount, file_count_t fcProcessedCount, unsigned long long ullTotalSize, unsigned long long ullProcessedSize, const TString& strCurrentPath)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		if (m_bIsInitialized)
			throw TCoreException(eErr_InvalidData, L"SubTask stats already initialized", LOCATION);

		m_iCurrentBufferIndex = iCurrentBufferIndex;

		m_fcTotalCount = fcTotalCount;
		m_fcProcessedCount = fcProcessedCount;

		_ASSERTE(m_fcProcessedCount <= m_fcTotalCount);
		if (m_fcProcessedCount > m_fcTotalCount)
			throw TCoreException(eErr_InternalProblem, L"Count of processed files exceeded total count", LOCATION);

		m_ullTotalSize = ullTotalSize;
		m_ullProcessedSize = ullProcessedSize;
		_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
		if (m_ullProcessedSize > m_ullTotalSize)
			throw TCoreException(eErr_InternalProblem, L"Size of processed files exceeded total size", LOCATION);

		m_strCurrentPath = strCurrentPath;

		m_bIsInitialized = true;
	}

	bool TSubTaskStatsInfo::IsInitialized() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		bool bInitialized = m_bIsInitialized;

		return bInitialized;
	}

	void TSubTaskStatsInfo::SetCurrentIndex(file_count_t fcIndex)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_fcCurrentIndex = fcIndex;
	}

	file_count_t TSubTaskStatsInfo::GetCurrentIndex() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_fcCurrentIndex.Get();
	}

	unsigned long long TSubTaskStatsInfo::GetCurrentItemProcessedSize() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_ullCurrentItemProcessedSize;
	}

	unsigned long long TSubTaskStatsInfo::GetCurrentItemTotalSize() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_ullCurrentItemTotalSize;
	}

	bool TSubTaskStatsInfo::CanCurrentItemSilentResume() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_bCurrentItemSilentResume;
	}

	void TSubTaskStatsInfo::SetCurrentItemSilentResume(bool bEnableSilentResume)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_bCurrentItemSilentResume = bEnableSilentResume;
	}

	bool TSubTaskStatsInfo::WasAdded() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_setModifications[eMod_Added];
	}

	void TSubTaskStatsInfo::IncreaseTotalSize(unsigned long long ullIncreaseBy)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_ullTotalSize = m_ullTotalSize + ullIncreaseBy;
	}

	void TSubTaskStatsInfo::DecreaseTotalSize(unsigned long long ullDecreaseBy)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_ullTotalSize = m_ullTotalSize - ullDecreaseBy;
	}

	void TSubTaskStatsInfo::VerifyProcessedVsTotal()
	{
		_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
		_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);

		if (m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
			throw TCoreException(eErr_InternalProblem, L"Current item processed size exceeded total size", LOCATION);
		if (m_ullProcessedSize > m_ullTotalSize)
			throw TCoreException(eErr_InternalProblem, L"Size of processed files exceeded total", LOCATION);
	}
}
