// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TTaskConfigTracker.cpp
/// @date  2010/10/04
/// @brief Contains implementation of TTaskConfigTracker class.
// ============================================================================
#include "stdafx.h"
#include "TTaskConfigTracker.h"
#include "CfgProperties.h"

TOptionsSet& TOptionsSet::operator%(ETaskOptions eOption)
{
	m_setOptions.insert(eOption);

	return *this;
}

std::set<ETaskOptions>& TOptionsSet::Get()
{
	return m_setOptions;
}

TTaskConfigTracker::TTaskConfigTracker()
{
}

TTaskConfigTracker::~TTaskConfigTracker()
{
}

bool TTaskConfigTracker::IsModified() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return !m_setModified.empty();
}

bool TTaskConfigTracker::IsModified(ETaskOptions eOption) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_setModified.find(eOption) != m_setModified.end();
}

bool TTaskConfigTracker::IsModified(TOptionsSet setOptions) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	std::set<ETaskOptions> setCommon;
	std::set_intersection(setOptions.Get().begin(), setOptions.Get().end(), m_setModified.begin(), m_setModified.end(), std::inserter(setCommon, setCommon.begin()));

	return !setCommon.empty();
}

bool TTaskConfigTracker::IsModified(ETaskOptions eOption, bool bResetModificationState)
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	std::set<ETaskOptions>::iterator iterOption = m_setModified.find(eOption);
	bool bModified = (iterOption != m_setModified.end());
	if(bModified && bResetModificationState)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
		m_setModified.erase(iterOption);
	}

	return bModified;
}

bool TTaskConfigTracker::IsModified(TOptionsSet setOptions, bool bResetModificationState)
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	std::set<ETaskOptions> setCommon;
	std::set_intersection(setOptions.Get().begin(), setOptions.Get().end(), m_setModified.begin(), m_setModified.end(), std::inserter(setCommon, setCommon.begin()));

	bool bModified = !setCommon.empty();
	if(bModified && bResetModificationState)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
		std::set<ETaskOptions>::iterator iterOption;
		BOOST_FOREACH(ETaskOptions eOption, setCommon)
		{
			iterOption = m_setModified.find(eOption);
			if(iterOption != m_setModified.end())
				m_setModified.erase(iterOption);
		}
	}

	return bModified;
}

void TTaskConfigTracker::AddModified(const CString& strModified)
{
	ETaskOptions eOption = TTaskConfigTracker::GetOptionFromString(strModified);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_setModified.insert(eOption);
}

void TTaskConfigTracker::AddModified(ETaskOptions eModified)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_setModified.insert(eModified);
}

void TTaskConfigTracker::AddModified(TOptionsSet setOptions)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_setModified.insert(setOptions.Get().begin(), setOptions.Get().end());
}

void TTaskConfigTracker::AddModified(const std::set<CString>& setModified)
{
	BOOST_FOREACH(const CString& strVal, setModified)
	{
		AddModified(strVal);
	}
}

void TTaskConfigTracker::AddModified(const std::set<ETaskOptions>& setModified)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	m_setModified.insert(setModified.begin(), setModified.end());
}

void TTaskConfigTracker::RemoveModification(ETaskOptions eModified)
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
	std::set<ETaskOptions>::iterator iterOption = m_setModified.find(eModified);
	if(iterOption != m_setModified.end())
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
		m_setModified.erase(iterOption);
	}
}

void TTaskConfigTracker::RemoveModificationSet(TOptionsSet setOptions)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	std::set<ETaskOptions> setCommon;
	std::set_intersection(setOptions.Get().begin(), setOptions.Get().end(), m_setModified.begin(), m_setModified.end(), std::inserter(setCommon, setCommon.begin()));

	std::set<ETaskOptions>::iterator iterOption;
	BOOST_FOREACH(ETaskOptions eOption, setCommon)
	{
		iterOption = m_setModified.find(eOption);
		if(iterOption != m_setModified.end())
			m_setModified.erase(iterOption);
	}
}

void TTaskConfigTracker::RemoveModification(const CString& strModified)
{
	ETaskOptions eOption = TTaskConfigTracker::GetOptionFromString(strModified);
	RemoveModification(eOption);
}

void TTaskConfigTracker::Clear()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_setModified.clear();
}

void TTaskConfigTracker::NotificationProc(const std::set<CString>& setModifications, void* pParam)
{
	if(!pParam)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	TTaskConfigTracker* pTracker = (TTaskConfigTracker*)pParam;
	pTracker->AddModified(setModifications);
}

ETaskOptions TTaskConfigTracker::GetOptionFromString(const CString& strOption)
{
	if(strOption == TaskPropData<eTO_UseOnlyDefaultBuffer>::GetPropertyName())
		return eTO_UseOnlyDefaultBuffer;
	else if(strOption == TaskPropData<eTO_DefaultBufferSize>::GetPropertyName())
		return eTO_DefaultBufferSize;
	else if(strOption == TaskPropData<eTO_OneDiskBufferSize>::GetPropertyName())
		return eTO_OneDiskBufferSize;
	else if(strOption == TaskPropData<eTO_TwoDisksBufferSize>::GetPropertyName())
		return eTO_TwoDisksBufferSize;
	else if(strOption == TaskPropData<eTO_CDBufferSize>::GetPropertyName())
		return eTO_CDBufferSize;
	else if(strOption == TaskPropData<eTO_LANBufferSize>::GetPropertyName())
		return eTO_LANBufferSize;
	else if(strOption == TaskPropData<eTO_DisableBuffering>::GetPropertyName())
		return eTO_DisableBuffering;
	else if(strOption == TaskPropData<eTO_DisableBufferingMinSize>::GetPropertyName())
		return eTO_DisableBufferingMinSize;

	else if(strOption == TaskPropData<eTO_SetDestinationAttributes>::GetPropertyName())
		return eTO_SetDestinationAttributes;
	else if(strOption == TaskPropData<eTO_SetDestinationDateTime>::GetPropertyName())
		return eTO_SetDestinationDateTime;
	else if(strOption == TaskPropData<eTO_ProtectReadOnlyFiles>::GetPropertyName())
		return eTO_ProtectReadOnlyFiles;
	else if(strOption == TaskPropData<eTO_ScanDirectoriesBeforeBlocking>::GetPropertyName())
		return eTO_ScanDirectoriesBeforeBlocking;
	else if(strOption == TaskPropData<eTO_ThreadPriority>::GetPropertyName())
		return eTO_ThreadPriority;
	else if(strOption == TaskPropData<eTO_DisablePriorityBoost>::GetPropertyName())
		return eTO_DisablePriorityBoost;
	else if(strOption == TaskPropData<eTO_DeleteInSeparateSubTask>::GetPropertyName())
		return eTO_DeleteInSeparateSubTask;

	else if(strOption == TaskPropData<eTO_CreateEmptyFiles>::GetPropertyName())
		return eTO_CreateEmptyFiles;
	else if(strOption == TaskPropData<eTO_CreateDirectoriesRelativeToRoot>::GetPropertyName())
		return eTO_CreateDirectoriesRelativeToRoot;
	else if(strOption == TaskPropData<eTO_IgnoreDirectories>::GetPropertyName())
		return eTO_IgnoreDirectories;
	else if(strOption == TaskPropData<eTO_AlternateFilenameFormatString_First>::GetPropertyName())
		return eTO_AlternateFilenameFormatString_AfterFirst;
	else if(strOption == TaskPropData<eTO_AlternateFilenameFormatString_AfterFirst>::GetPropertyName())
		return eTO_AlternateFilenameFormatString_First;
	else
	{
		BOOST_ASSERT(false);		// unhandled case
		THROW(_T("Unhandled case"), 0, 0, 0);
	}

	// add new elements before this one
	BOOST_STATIC_ASSERT(eTO_Last == eTO_AlternateFilenameFormatString_AfterFirst + 1);
}
