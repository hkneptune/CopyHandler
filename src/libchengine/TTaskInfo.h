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

#include "TaskID.h"
#include <bitset>
#include "libchengine.h"
#include "../libserializer/TRemovedObjects.h"
#include "../libserializer/ISerializerContainer.h"
#include "../libserializer/TSharedModificationTracker.h"

namespace chengine
{
	class TTask;
	typedef std::shared_ptr<TTask> TTaskPtr;

	class LIBCHENGINE_API TTaskInfoEntry
	{
	public:
		enum EModifications
		{
			eMod_None = 0,
			eMod_Added,
			eMod_TaskPath,
			eMod_Order,
			eMod_LogPath,

			eMod_Last
		};

	public:
		TTaskInfoEntry();
		TTaskInfoEntry(serializer::object_id_t oidTaskID, const chcore::TSmartPath& pathTask, const chcore::TSmartPath& pathLog, int iOrder, const TTaskPtr& spTask);
		TTaskInfoEntry(const TTaskInfoEntry& rSrc);

		TTaskInfoEntry& operator=(const TTaskInfoEntry& rSrc);

		serializer::object_id_t GetObjectID() const;

		chcore::TSmartPath GetTaskSerializeLocation() const;
		void SetTaskSerializeLocation(const chcore::TSmartPath& pathTask);

		chcore::TSmartPath GetTaskLogPath() const;
		void SetTaskLogPath(const chcore::TSmartPath& pathLog);

		TTaskPtr GetTask() const;
		void SetTask(const TTaskPtr& spTask);

		int GetOrder() const;
		void SetOrder(int iOrder);

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		static void InitColumns(serializer::IColumnsDefinition& rColumnDefs);
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader);

		void ResetModifications();

	private:
#pragma warning(push)
#pragma warning(disable:4251)
		serializer::object_id_t m_oidObjectID;
		typedef std::bitset<eMod_Last> Bitset;
		mutable std::bitset<eMod_Last> m_setModifications;
		serializer::TSharedModificationTracker<chcore::TSmartPath, Bitset, eMod_TaskPath> m_pathSerializeLocation;
		serializer::TSharedModificationTracker<chcore::TSmartPath, Bitset, eMod_LogPath> m_pathLogPath;
		serializer::TSharedModificationTracker<int, Bitset, eMod_Order> m_iOrder;

		TTaskPtr m_spTask;
#pragma warning(pop)
	};

	class LIBCHENGINE_API TTaskInfoContainer
	{
	public:
		TTaskInfoContainer();
		TTaskInfoContainer(const TTaskInfoContainer&) = delete;

		TTaskInfoContainer& operator=(const TTaskInfoContainer&) = delete;

		void Add(const chcore::TSmartPath& strPath, const chcore::TSmartPath& pathLog, int iOrder, const TTaskPtr& spTask);
		void RemoveAt(size_t stIndex);

		TTaskInfoEntry& GetAt(size_t stIndex);
		const TTaskInfoEntry& GetAt(size_t stIndex) const;

		TTaskInfoEntry& GetAtOid(serializer::object_id_t oidObjectID);

		bool GetByTaskID(taskid_t tTaskID, TTaskInfoEntry& rInfo) const;

		size_t GetCount() const;
		bool IsEmpty() const;

		void Clear();

		// modifications management
		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		void Load(const serializer::ISerializerContainerPtr& spContainer);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const;

		void ClearModifications();

	private:
#pragma warning(push)
#pragma warning(disable:4251)
		std::vector<TTaskInfoEntry> m_vTaskInfos;
		mutable serializer::TRemovedObjects m_setRemovedTasks;
#pragma warning(pop)
		serializer::object_id_t m_oidLastObjectID;
	};
}

#endif
