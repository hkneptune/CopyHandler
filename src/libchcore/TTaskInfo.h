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
#ifndef __TTASKINFO_H__
#define __TTASKINFO_H__

#include "libchcore.h"
#include "TString.h"
#include <boost/shared_ptr.hpp>
#include "TPath.h"
#include "TaskID.h"
#include "ISerializerContainer.h"
#include "TIntrusiveSerializableItem.h"

BEGIN_CHCORE_NAMESPACE

class TTask;
typedef boost::shared_ptr<TTask> TTaskPtr;

class LIBCHCORE_API TTaskInfoEntry : public TIntrusiveSerializableItem
{
public:
	enum ETIEntryInfo
	{
		eMod_TaskPath = TIntrusiveSerializableItem::eMod_Modified,
		eMod_Order = TIntrusiveSerializableItem::eMod_Modified << 1,
	};

public:
	TTaskInfoEntry();
	TTaskInfoEntry(taskid_t tTaskID, const TSmartPath& pathTask, int iOrder, const TTaskPtr& spTask, int iModification = eMod_None);

	TSmartPath GetTaskSerializeLocation() const;
	void SetTaskSerializeLocation(const TSmartPath& pathTask);

	TTaskPtr GetTask() const;
	void SetTask(const TTaskPtr& spTask);

	int GetOrder() const;
	void SetOrder(int iOrder);

	void Store(const ISerializerContainerPtr& spContainer);
	bool Load(const ISerializerRowReaderPtr& spRowReader);

private:
	TSmartPath m_pathSerializeLocation;
#pragma warning(push)
#pragma warning(disable:4251)
	TTaskPtr m_spTask;
#pragma warning(pop)
	int m_iOrder;
};

class LIBCHCORE_API TTaskInfoContainer
{
public:
	TTaskInfoContainer();

	void Add(taskid_t tTaskID, const TSmartPath& strPath, int iOrder, const TTaskPtr& spTask);
	void RemoveAt(size_t stIndex);

	TTaskInfoEntry& GetAt(size_t stIndex);
	const TTaskInfoEntry& GetAt(size_t stIndex) const;

	taskid_t GetLastTaskID() const;

	bool GetByTaskID(taskid_t tTaskID, TTaskInfoEntry& rInfo) const;

	size_t GetCount() const;
	bool IsEmpty() const;

	void Clear();

	size_t GetDeletedCount() const;
	taskid_t GetDeletedAt(size_t stIndex) const;

	// modifications management
	void Store(const ISerializerContainerPtr& spContainer);
	void Load(const ISerializerContainerPtr& spContainer);

	void ClearModifications();

	bool HasDeletions() const;
	bool HasAdditions() const;
	bool HasModifications() const;

private:
#pragma warning(push)
#pragma warning(disable:4251)
	std::vector<TTaskInfoEntry> m_vTaskInfos;
	std::set<taskid_t> m_setRemovedTasks;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
