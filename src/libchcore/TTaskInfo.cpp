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
#include "TRowData.h"
#include "ISerializerRowData.h"
#include "ISerializerRowReader.h"

BEGIN_CHCORE_NAMESPACE

TTaskInfoEntry::TTaskInfoEntry() :
	TIntrusiveSerializableItem(),
	m_iOrder(0)
{
}

TTaskInfoEntry::TTaskInfoEntry(taskid_t tTaskID, const TSmartPath& pathTask, int iOrder, const TTaskPtr& spTask, int iModification) :
	TIntrusiveSerializableItem(tTaskID, iModification),
	m_pathSerializeLocation(pathTask),
	m_iOrder(iOrder),
	m_spTask(spTask)
{
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

void TTaskInfoEntry::Store(const ISerializerContainerPtr& spContainer)
{
	if(!IsModified())
		return;

	if(IsAdded())
	{
		ISerializerRowDataPtr spRow = spContainer->AddRow(GetObjectID());

		*spRow % TRowData(_T("path"), m_pathSerializeLocation)
				% TRowData(_T("task_order"), m_iOrder);
	}
	else
	{
		ISerializerRowDataPtr spRow = spContainer->GetRow(GetObjectID());
		if(GetModifications() & eMod_TaskPath)
			*spRow % TRowData(_T("path"), m_pathSerializeLocation);
		else if(GetModifications() & eMod_Order)
			*spRow % TRowData(_T("task_order"), m_iOrder);
	}

	ResetModifications();
}

bool TTaskInfoEntry::Load(const ISerializerRowReaderPtr& spRowReader)
{
	IColumnsDefinitionPtr spColumns = spRowReader->GetColumnsDefinitions();
	if(spColumns->IsEmpty())
		*spColumns % _T("id") % _T("path") % _T("task_order");

	bool bResult = spRowReader->Next();
	if(bResult)
	{
		spRowReader->GetValue(_T("id"), m_stObjectID);
		spRowReader->GetValue(_T("path"), m_pathSerializeLocation);
		spRowReader->GetValue(_T("task_order"), m_iOrder);
	}

	return bResult;
}

///////////////////////////////////////////////////////////////////////////
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
	taskid_t tTaskID = (*iter).GetObjectID();
	m_vTaskInfos.erase(m_vTaskInfos.begin() + stIndex);
	m_setRemovedTasks.insert(tTaskID);
}

void TTaskInfoContainer::Clear()
{
	BOOST_FOREACH(TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		m_setRemovedTasks.insert(rEntry.GetObjectID());
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
		if((*iter).GetObjectID() == tTaskID)
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

void TTaskInfoContainer::ClearModifications()
{
	m_setRemovedTasks.clear();

	BOOST_FOREACH(TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		// if marked as added, we don't consider it modified anymore
		rEntry.ResetModifications();
	}
}

void TTaskInfoContainer::Store(const ISerializerContainerPtr& spContainer)
{
	BOOST_FOREACH(taskid_t stObjectID, m_setRemovedTasks)
	{
		spContainer->DeleteRow(stObjectID);
	}

	BOOST_FOREACH(TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		if(rEntry.GetModifications() != TTaskInfoEntry::eMod_None)
			rEntry.Store(spContainer);
	}

	ClearModifications();
}

void TTaskInfoContainer::Load(const ISerializerContainerPtr& spContainer)
{
	ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

	TTaskInfoEntry tEntry;
	while(tEntry.Load(spRowReader))
	{
		m_vTaskInfos.push_back(tEntry);
	}
}

taskid_t TTaskInfoContainer::GetLastTaskID() const
{
	taskid_t tLastTaskID = NoTaskID;

	BOOST_FOREACH(const TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		tLastTaskID = std::max(rEntry.GetObjectID(), tLastTaskID);
	}

	if(!m_setRemovedTasks.empty())
		tLastTaskID = std::max(*m_setRemovedTasks.rbegin(), tLastTaskID);

	return tLastTaskID;
}

END_CHCORE_NAMESPACE
