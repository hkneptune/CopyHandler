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
#include <boost\numeric\conversion\cast.hpp>
#include "DataBuffer.h"
#include "TSubTaskStatsSnapshot.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////
// class TSubTaskProcessingGuard
TSubTaskProcessingGuard::TSubTaskProcessingGuard(TSubTaskStatsInfo& rStats) :
	m_rStats(rStats)
{
	rStats.MarkAsRunning();
	rStats.EnableTimeTracking();
}

TSubTaskProcessingGuard::~TSubTaskProcessingGuard()
{
	m_rStats.DisableTimeTracking();
	m_rStats.MarkAsNotRunning();
}

///////////////////////////////////////////////////////////////////////////////////
// class TSubTaskStatsInfo

TSubTaskStatsInfo::TSubTaskStatsInfo() :
	m_bSubTaskIsRunning(m_setModifications, false),
	m_ullTotalSize(m_setModifications, 0),
	m_ullProcessedSize(m_setModifications, 0),
	m_stTotalCount(m_setModifications, 0),
	m_stProcessedCount(m_setModifications, 0),
	m_iCurrentBufferIndex(m_setModifications, 0),
	m_strCurrentPath(m_setModifications),
	m_tSizeSpeed(m_setModifications, DefaultSpeedTrackTime, DefaultSpeedSampleTime),
	m_tCountSpeed(m_setModifications, DefaultSpeedTrackTime, DefaultSpeedSampleTime),
	m_ullCurrentItemProcessedSize(m_setModifications, 0),
	m_ullCurrentItemTotalSize(m_setModifications, 0),
	m_eSubOperationType(m_setModifications, eSubOperation_None),
	m_tTimer(m_setModifications),
	m_bIsInitialized(m_setModifications, false)
{
}

void TSubTaskStatsInfo::Clear()
{
	m_bSubTaskIsRunning = false;
	m_ullTotalSize = 0;
	m_ullProcessedSize = 0;
	m_stTotalCount = 0;
	m_stProcessedCount = 0;
	m_iCurrentBufferIndex = 0;
	m_strCurrentPath.Modify().Clear();
	m_tTimer.Modify().Reset();
	m_tSizeSpeed.Modify().Clear();
	m_tCountSpeed.Modify().Clear();
	m_ullCurrentItemProcessedSize = 0;
	m_ullCurrentItemTotalSize = 0;
	m_eSubOperationType = eSubOperation_None;
	m_bIsInitialized = false;
}

void TSubTaskStatsInfo::GetSnapshot(TSubTaskStatsSnapshotPtr& spStatsSnapshot) const
{
	if(!spStatsSnapshot)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	spStatsSnapshot->Clear();

	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	if(m_bSubTaskIsRunning)
		UpdateTime(lock);

	spStatsSnapshot->SetRunning(m_bSubTaskIsRunning);
	spStatsSnapshot->SetProcessedCount(m_stProcessedCount);
	spStatsSnapshot->SetTotalCount(m_stTotalCount);
	spStatsSnapshot->SetProcessedSize(m_ullProcessedSize);
	spStatsSnapshot->SetTotalSize(m_ullTotalSize);
	spStatsSnapshot->SetCurrentBufferIndex(m_iCurrentBufferIndex);
	spStatsSnapshot->SetCurrentPath(m_strCurrentPath);
	spStatsSnapshot->SetTimeElapsed(m_tTimer.Get().GetTotalTime());
	spStatsSnapshot->SetSizeSpeed(m_tSizeSpeed.Get().GetSpeed());
	spStatsSnapshot->SetCountSpeed(m_tCountSpeed.Get().GetSpeed());
	spStatsSnapshot->SetCurrentItemProcessedSize(m_ullCurrentItemProcessedSize);
	spStatsSnapshot->SetCurrentItemTotalSize(m_ullCurrentItemTotalSize);
	spStatsSnapshot->SetSubOperationType(m_eSubOperationType);
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

void TSubTaskStatsInfo::IncreaseProcessedCount(size_t stIncreaseBy)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stProcessedCount.Modify() += stIncreaseBy;

	m_tCountSpeed.Modify().AddSample(stIncreaseBy, m_tTimer.Modify().Tick());

	_ASSERTE(m_stProcessedCount <= m_stTotalCount);
	if(m_stProcessedCount > m_stTotalCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetProcessedCount(size_t stProcessedCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_tCountSpeed.Modify().AddSample(0/*stProcessedCount - m_stProcessedCount*/, m_tTimer.Modify().Tick());

	m_stProcessedCount = stProcessedCount;

	_ASSERTE(m_stProcessedCount <= m_stTotalCount);
	if(m_stProcessedCount > m_stTotalCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetTotalCount(size_t stCount)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_stTotalCount = stCount;
	_ASSERTE(m_stProcessedCount <= m_stTotalCount);
	if(m_stProcessedCount > m_stTotalCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::IncreaseProcessedSize(unsigned long long ullIncreaseBy)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullProcessedSize.Modify() += ullIncreaseBy;

	m_tSizeSpeed.Modify().AddSample(ullIncreaseBy, m_tTimer.Modify().Tick());

	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::DecreaseProcessedSize(unsigned long long ullDecreaseBy)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullProcessedSize.Modify() -= ullDecreaseBy;

	// we didn't process anything here - hence the 0-sized sample
	m_tSizeSpeed.Modify().AddSample(0, m_tTimer.Modify().Tick());

	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetProcessedSize(unsigned long long ullProcessedSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_tSizeSpeed.Modify().AddSample(0/*ullProcessedSize - m_ullProcessedSize*/, m_tTimer.Modify().Tick());

	m_ullProcessedSize = ullProcessedSize;
	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetTotalSize(unsigned long long ullTotalSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullTotalSize = ullTotalSize;
	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

// current item
void TSubTaskStatsInfo::IncreaseCurrentItemProcessedSize(unsigned long long ullIncreaseBy)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentItemProcessedSize.Modify() += ullIncreaseBy;

	_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
	if(m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::DecreaseCurrentItemProcessedSize(unsigned long long ullDecreaseBy)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentItemProcessedSize.Modify() -= ullDecreaseBy;

	_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
	if(m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetCurrentItemProcessedSize(unsigned long long ullProcessedSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_ullCurrentItemProcessedSize = ullProcessedSize;
	_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
	if(m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
}

void TSubTaskStatsInfo::SetCurrentItemTotalSize(unsigned long long ullTotalSize)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_ullCurrentItemTotalSize = ullTotalSize;
	_ASSERTE(m_ullCurrentItemProcessedSize <= m_ullCurrentItemTotalSize);
	if(m_ullCurrentItemProcessedSize > m_ullCurrentItemTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);
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
	m_tTimer.Modify().Tick();
	m_tSizeSpeed.Modify().AddSample(0, m_tTimer.Get().GetLastTimestamp());
	m_tCountSpeed.Modify().AddSample(0, m_tTimer.Get().GetLastTimestamp());
}

void TSubTaskStatsInfo::Store(ISerializerRowData& rRowData) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	if(m_bSubTaskIsRunning.IsModified())
		rRowData.SetValue(_T("is_running"), m_bSubTaskIsRunning);
	if(m_bIsInitialized.IsModified())
		rRowData.SetValue(_T("is_initialized"), m_bIsInitialized);

	if(m_ullTotalSize.IsModified())
		rRowData.SetValue(_T("total_size"), m_ullTotalSize);

	if(m_ullProcessedSize.IsModified())
		rRowData.SetValue(_T("processed_size"), m_ullProcessedSize);
	if(m_tSizeSpeed.IsModified())
		rRowData.SetValue(_T("size_speed"), m_tSizeSpeed.Get().ToString());

	if(m_stTotalCount.IsModified())
		rRowData.SetValue(_T("total_count"), m_stTotalCount);
	if(m_ullProcessedSize.IsModified())
		rRowData.SetValue(_T("processed_count"), m_stProcessedCount);
	if(m_tSizeSpeed.IsModified())
		rRowData.SetValue(_T("count_speed"), m_tCountSpeed.Get().ToString());

	if(m_ullCurrentItemProcessedSize.IsModified())
		rRowData.SetValue(_T("ci_processed_size"), m_ullCurrentItemProcessedSize);
	if(m_ullCurrentItemTotalSize.IsModified())
		rRowData.SetValue(_T("ci_total_size"), m_ullCurrentItemTotalSize);

	if(m_tTimer.IsModified())
		rRowData.SetValue(_T("timer"), m_tTimer.Get().GetTotalTime());

	if(m_iCurrentBufferIndex.IsModified())
		rRowData.SetValue(_T("buffer_index"), m_iCurrentBufferIndex);

	if(m_strCurrentPath.IsModified())
		rRowData.SetValue(_T("current_path"), m_strCurrentPath);
	if(m_eSubOperationType.IsModified())
		rRowData.SetValue(_T("suboperation_type"), m_eSubOperationType);

	m_setModifications.reset();
}

void TSubTaskStatsInfo::InitColumns(IColumnsDefinition& rColumnDefs)
{
	rColumnDefs.AddColumn(_T("is_running"), IColumnsDefinition::eType_bool);
	rColumnDefs.AddColumn(_T("is_initialized"), IColumnsDefinition::eType_bool);
	rColumnDefs.AddColumn(_T("total_size"), IColumnsDefinition::eType_ulonglong);
	rColumnDefs.AddColumn(_T("processed_size"), IColumnsDefinition::eType_ulonglong);
	rColumnDefs.AddColumn(_T("size_speed"), IColumnsDefinition::eType_string);
	rColumnDefs.AddColumn(_T("total_count"), IColumnsDefinition::eType_ulonglong);
	rColumnDefs.AddColumn(_T("processed_count"), IColumnsDefinition::eType_ulonglong);
	rColumnDefs.AddColumn(_T("count_speed"), IColumnsDefinition::eType_string);
	rColumnDefs.AddColumn(_T("ci_processed_size"), IColumnsDefinition::eType_ulonglong);
	rColumnDefs.AddColumn(_T("ci_total_size"), IColumnsDefinition::eType_ulonglong);
	rColumnDefs.AddColumn(_T("timer"), IColumnsDefinition::eType_ulonglong);
	rColumnDefs.AddColumn(_T("buffer_index"), IColumnsDefinition::eType_int);
	rColumnDefs.AddColumn(_T("current_path"), IColumnsDefinition::eType_string);
	rColumnDefs.AddColumn(_T("suboperation_type"), IColumnsDefinition::eType_int);
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

	spRowReader->GetValue(_T("total_count"), m_stTotalCount.Modify());
	spRowReader->GetValue(_T("processed_count"), m_stProcessedCount.Modify());

	spRowReader->GetValue(_T("count_speed"), strSpeed);
	m_tCountSpeed.Modify().FromString(strSpeed);

	spRowReader->GetValue(_T("ci_processed_size"), m_ullCurrentItemProcessedSize.Modify());
	spRowReader->GetValue(_T("ci_total_size"), m_ullCurrentItemTotalSize.Modify());

	unsigned long long ullTimer = 0;
	spRowReader->GetValue(_T("timer"), ullTimer);
	m_tTimer.Modify().Init(ullTimer);

	spRowReader->GetValue(_T("buffer_index"), m_iCurrentBufferIndex.Modify());

	spRowReader->GetValue(_T("current_path"), m_strCurrentPath.Modify());
	spRowReader->GetValue(_T("suboperation_type"), *(int*)&m_eSubOperationType.Modify());

	m_setModifications.reset();
}

void TSubTaskStatsInfo::Init(int iCurrentBufferIndex, size_t stTotalCount, size_t stProcessedCount, unsigned long long ullTotalSize, unsigned long long ullProcessedSize, const TString& strCurrentPath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	if(m_bIsInitialized)
		THROW_CORE_EXCEPTION(eErr_InvalidData);

	m_iCurrentBufferIndex = iCurrentBufferIndex;

	m_stTotalCount = stTotalCount;
	m_stProcessedCount = stProcessedCount;

	_ASSERTE(m_stProcessedCount <= m_stTotalCount);
	if(m_stProcessedCount > m_stTotalCount)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	m_ullTotalSize = ullTotalSize;
	m_ullProcessedSize = ullProcessedSize;
	_ASSERTE(m_ullProcessedSize <= m_ullTotalSize);
	if(m_ullProcessedSize > m_ullTotalSize)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	m_strCurrentPath = strCurrentPath;

	m_bIsInitialized = true;
}

bool TSubTaskStatsInfo::IsInitialized() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	bool bInitialized = m_bIsInitialized;

	return bInitialized;
}

END_CHCORE_NAMESPACE
