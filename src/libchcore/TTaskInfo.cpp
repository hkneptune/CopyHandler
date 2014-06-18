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
#include "ISerializerRowData.h"
#include "ISerializerRowReader.h"

BEGIN_CHCORE_NAMESPACE

TTaskInfoEntry::TTaskInfoEntry() :
	m_iOrder(m_setModifications, 0),
	m_pathSerializeLocation(m_setModifications),
	m_stObjectID(0)
{
	m_setModifications[eMod_Added] = true;
}

TTaskInfoEntry::TTaskInfoEntry(taskid_t tTaskID, const TSmartPath& pathTask, int iOrder, const TTaskPtr& spTask) :
	m_stObjectID(tTaskID),
	m_pathSerializeLocation(m_setModifications, pathTask),
	m_iOrder(m_setModifications, iOrder),
	m_spTask(spTask)
{
	m_setModifications[eMod_Added] = true;
}

TSmartPath TTaskInfoEntry::GetTaskSerializeLocation() const
{
	return m_pathSerializeLocation;
}

void TTaskInfoEntry::SetTaskSerializeLocation(const TSmartPath& strTaskPath)
{
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
	m_iOrder = iOrder;
}

void TTaskInfoEntry::Store(const ISerializerContainerPtr& spContainer) const
{
	if(!m_setModifications.any())
		return;

	bool bAdded = m_setModifications[eMod_Added];
	ISerializerRowData& rRow = spContainer->GetRow(m_stObjectID, bAdded);

	if(bAdded || m_setModifications[eMod_TaskPath])
		rRow.SetValue(_T("path"), m_pathSerializeLocation);
	if(bAdded || m_setModifications[eMod_Order])
		rRow.SetValue(_T("task_order"), m_iOrder);

	m_setModifications.reset();
}

void TTaskInfoEntry::Load(const ISerializerRowReaderPtr& spRowReader)
{
	spRowReader->GetValue(_T("id"), m_stObjectID);
	spRowReader->GetValue(_T("path"), m_pathSerializeLocation.Modify());
	spRowReader->GetValue(_T("task_order"), m_iOrder.Modify());

	m_setModifications.reset();
}

void TTaskInfoEntry::InitColumns(IColumnsDefinition& rColumnDefs)
{
	rColumnDefs.AddColumn(_T("id"), IColumnsDefinition::eType_sizet);
	rColumnDefs.AddColumn(_T("path"), IColumnsDefinition::eType_path);
	rColumnDefs.AddColumn(_T("task_order"), IColumnsDefinition::eType_int);
}

size_t TTaskInfoEntry::GetObjectID() const
{
	return m_stObjectID;
}

void TTaskInfoEntry::ResetModifications()
{
	m_setModifications.reset();
}

///////////////////////////////////////////////////////////////////////////
TTaskInfoContainer::TTaskInfoContainer() :
	m_stLastObjectID(0)
{
}

void TTaskInfoContainer::Add(const TSmartPath& pathTask, int iOrder, const TTaskPtr& spTask)
{
	m_vTaskInfos.push_back(TTaskInfoEntry(++m_stLastObjectID, pathTask, iOrder, spTask));
}

void TTaskInfoContainer::RemoveAt(size_t stIndex)
{
	if(stIndex >= m_vTaskInfos.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	std::vector<TTaskInfoEntry>::iterator iter = m_vTaskInfos.begin() + stIndex;
	taskid_t tTaskID = (*iter).GetObjectID();
	m_vTaskInfos.erase(m_vTaskInfos.begin() + stIndex);
	m_setRemovedTasks.Add(tTaskID);
}

void TTaskInfoContainer::Clear()
{
	BOOST_FOREACH(TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		m_setRemovedTasks.Add(rEntry.GetObjectID());
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

void TTaskInfoContainer::ClearModifications()
{
	m_setRemovedTasks.Clear();

	BOOST_FOREACH(TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		// if marked as added, we don't consider it modified anymore
		rEntry.ResetModifications();
	}
}

void TTaskInfoContainer::Store(const ISerializerContainerPtr& spContainer) const
{
	InitColumns(spContainer);

	spContainer->DeleteRows(m_setRemovedTasks);
	m_setRemovedTasks.Clear();

	BOOST_FOREACH(const TTaskInfoEntry& rEntry, m_vTaskInfos)
	{
		rEntry.Store(spContainer);
	}
}

void TTaskInfoContainer::Load(const ISerializerContainerPtr& spContainer)
{
	InitColumns(spContainer);

	ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

	TTaskInfoEntry tEntry;
	while(spRowReader->Next())
	{
		tEntry.Load(spRowReader);

		m_vTaskInfos.push_back(tEntry);
		m_stLastObjectID = std::max(m_stLastObjectID, tEntry.GetObjectID());
	}
}

TTaskInfoEntry& TTaskInfoContainer::GetAtOid(size_t stObjectID)
{
	for(std::vector<TTaskInfoEntry>::iterator iter = m_vTaskInfos.begin(); iter != m_vTaskInfos.end(); ++iter)
	{
		if((*iter).GetObjectID() == stObjectID)
			return *iter;
	}

	THROW_CORE_EXCEPTION(eErr_InvalidArgument);
}

void TTaskInfoContainer::InitColumns(const ISerializerContainerPtr& spContainer) const
{
	IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
	if(rColumns.IsEmpty())
		TTaskInfoEntry::InitColumns(rColumns);
}

END_CHCORE_NAMESPACE
