// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TTaskInfo.h"
#include "TCoreException.h"

BEGIN_CHCORE_NAMESPACE

TTaskInfoEntry::TTaskInfoEntry() :
	m_tTaskID(0),
	m_iOrder(0),
	m_iModificationType(eMod_None)
{
}

TTaskInfoEntry::TTaskInfoEntry(taskid_t tTaskID, const TSmartPath& pathTask, int iOrder, const TTaskPtr& spTask, int iModification /*= eMod_None*/) :
	m_tTaskID(tTaskID),
	m_pathSerializeLocation(pathTask),
	m_iOrder(iOrder),
	m_spTask(spTask),
	m_iModificationType(iModification)
{
}

taskid_t TTaskInfoEntry::GetTaskID() const
{
	return m_tTaskID;
}

void TTaskInfoEntry::SetTaskID(taskid_t tTaskID)
{
	m_tTaskID = tTaskID;
}

TSmartPath TTaskInfoEntry::GetTaskSerializeLocation() const
{
	return m_pathSerializeLocation;
}

void TTaskInfoEntry::SetTaskSerializeLocation(const TSmartPath& strTaskPath)
{
	SetModification(eMod_TaskPath, eMod_TaskPath);
	m_pathSerializeLocation = strTaskPath;
}

TTaskPtr TTaskInfoEntry::GetTask() const
{
	return m_spTask;
}

void TTaskInfoEntry::SetTask(const TTaskPtr& spTask)
{
	m_spTask = spTask;
}

int TTaskInfoEntry::GetOrder() const
{
	return m_iOrder;
}

void TTaskInfoEntry::SetOrder(int iOrder)
{
	SetModification(eMod_Order, eMod_Order);
	m_iOrder = iOrder;
}

int TTaskInfoEntry::GetModifications() const
{
	return m_iModificationType;
}

void TTaskInfoEntry::SetModification(int iModification, int iMask)
{
	m_iModificationType &= ~iMask;
	m_iModificationType |= iModification;
}

void TTaskInfoEntry::ResetModifications()
{
	m_iModificationType = 0;
}

bool TTaskInfoEntry::IsAdded() const
{
	return m_iModificationType & eMod_Added;
}

bool TTaskInfoEntry::IsModified() const
{
	return (m_iModificationType & ~eMod_Added) != eMod_None;
}


TTaskInfoContainer::TTaskInfoContainer()
{
}

void TTaskInfoContainer::Add(taskid_t tTaskID, const TSmartPath& pathTask, int iOrder, const TTaskPtr& spTask)
{
	m_vTaskInfos.push_back(TTaskInfoEntry(tTaskID, pathTask, iOrder, spTask, TTaskInfoEntry::eMod_Added));
}

void TTaskInfoContainer::RemoveAt(size_t stIndex)
{
	if(stIndex >= m_vTaskInfos.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	std::vector<TTaskInfoEntry>::iterator iter = m_vTaskInfos.begin() + stIndex;
	taskid_t tTaskID = (*iter).GetTaskID();
	m_vTaskInfos.erase(m_vTaskInfos.begin() + stIndex);
	m_setRemovedTasks.insert(tTaskID);
}

void TTaskInfoContainer::Clear()
{
	BOOST_FOREACH(TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		m_setRemovedTasks.insert(rEntry.GetTaskID());
	}
	m_vTaskInfos.clear();
}

TTaskInfoEntry& TTaskInfoContainer::GetAt(size_t stIndex)
{
	if(stIndex >= m_vTaskInfos.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	return m_vTaskInfos[stIndex];
}

const TTaskInfoEntry& TTaskInfoContainer::GetAt(size_t stIndex) const
{
	if(stIndex >= m_vTaskInfos.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	return m_vTaskInfos[stIndex];
}

size_t TTaskInfoContainer::GetCount() const
{
	return m_vTaskInfos.size();
}

bool TTaskInfoContainer::GetByTaskID(taskid_t tTaskID, TTaskInfoEntry& rInfo) const
{
	for(std::vector<TTaskInfoEntry>::const_iterator iter = m_vTaskInfos.begin(); iter != m_vTaskInfos.end(); ++iter)
	{
		if((*iter).GetTaskID() == tTaskID)
		{
			rInfo = *iter;
			return true;
		}
	}

	return false;
}

bool TTaskInfoContainer::IsEmpty() const
{
	return m_vTaskInfos.empty();
}

void TTaskInfoContainer::GetDiffAndResetModifications(TTaskInfoContainer& rDiff)
{
	rDiff.Clear();
	rDiff.ClearModifications();

	rDiff.m_setRemovedTasks.insert(m_setRemovedTasks.begin(), m_setRemovedTasks.end());
	BOOST_FOREACH(TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		if(rEntry.GetModifications() != TTaskInfoEntry::eMod_None)
			rDiff.m_vTaskInfos.push_back(rEntry);
	}

	ClearModifications();
}

bool TTaskInfoContainer::HasDeletions() const
{
	return !m_setRemovedTasks.empty();
}

bool TTaskInfoContainer::HasAdditions() const
{
	BOOST_FOREACH(const TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		if(rEntry.IsAdded())
			return true;
	}

	return false;
}

bool TTaskInfoContainer::HasModifications() const
{
	BOOST_FOREACH(const TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		// if marked as added, we don't consider it modified anymore
		if(rEntry.IsModified())
			return true;
	}

	return false;
}

size_t TTaskInfoContainer::GetDeletedCount() const
{
	return m_setRemovedTasks.size();
}

chcore::taskid_t TTaskInfoContainer::GetDeletedAt(size_t stIndex) const
{
	if(stIndex >= m_setRemovedTasks.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	std::set<taskid_t>::const_iterator iter = m_setRemovedTasks.begin();
	std::advance(iter, stIndex);

	return *iter;
}

void TTaskInfoContainer::RestoreModifications(const TTaskInfoContainer& tDataDiff) throw()
{
	m_setRemovedTasks.insert(tDataDiff.m_setRemovedTasks.begin(), tDataDiff.m_setRemovedTasks.end());

	for(std::vector<TTaskInfoEntry>::const_iterator iterOther = tDataDiff.m_vTaskInfos.begin(); iterOther != tDataDiff.m_vTaskInfos.end(); ++iterOther)
	{
		bool bFound = false;

		for(std::vector<TTaskInfoEntry>::iterator iterThis = m_vTaskInfos.begin(); iterThis != m_vTaskInfos.end(); ++iterThis)
		{
			if((*iterThis).GetTaskID() == (*iterOther).GetTaskID())
			{
				(*iterThis).SetModification((*iterOther).GetModifications(), (*iterOther).GetModifications());
				bFound = true;
				break;
			}
		}

		// this method is used in catch clause, so no exception allowed here
		_ASSERTE(bFound);
	}
}

void TTaskInfoContainer::ClearModifications()
{
	m_setRemovedTasks.clear();

	BOOST_FOREACH(TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		// if marked as added, we don't consider it modified anymore
		rEntry.ResetModifications();
	}
}

END_CHCORE_NAMESPACE
