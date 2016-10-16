// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#include "TObsoleteFiles.h"
#include "ISerializerContainer.h"

namespace chcore
{
	ObsoleteFileInfo::ObsoleteFileInfo(const TSmartPath& path, bool bAdded) :
		m_path(path),
		m_bAdded(bAdded)
	{
	}

	ObsoleteFileInfo::ObsoleteFileInfo() :
		m_bAdded(false)
	{
	}

	TObsoleteFiles::TObsoleteFiles() :
		m_oidLast(0)
	{
	}

	TObsoleteFiles::~TObsoleteFiles()
	{
	}

	void TObsoleteFiles::DeleteObsoleteFile(const TSmartPath& pathToDelete)
	{
		if (!DeleteFile(pathToDelete.ToString()) && GetLastError() != ERROR_FILE_NOT_FOUND)
			m_mapPaths.insert(std::make_pair(++m_oidLast, ObsoleteFileInfo(pathToDelete, true)));
	}

	void TObsoleteFiles::Store(const ISerializerContainerPtr& spContainer) const
	{
		InitColumns(spContainer);

		spContainer->DeleteRows(m_setRemovedObjects);
		m_setRemovedObjects.Clear();

		for (MapPaths::const_iterator iter = m_mapPaths.begin(); iter != m_mapPaths.end(); ++iter)
		{
			if (iter->second.m_bAdded)
			{
				ISerializerRowData& rRow = spContainer->GetRow(iter->first, true);
				rRow.SetValue(_T("path"), iter->second.m_path);
			}

			iter->second.m_bAdded = false;
		}
	}

	void TObsoleteFiles::Load(const ISerializerContainerPtr& spContainer)
	{
		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

		ObsoleteFileInfo tEntry;
		object_id_t oid = 0;
		while (spRowReader->Next())
		{
			spRowReader->GetValue(_T("id"), oid);
			spRowReader->GetValue(_T("path"), tEntry.m_path);
			tEntry.m_bAdded = false;

			m_mapPaths.insert(std::make_pair(oid, tEntry));
			m_oidLast = std::max(m_oidLast, oid);
		}

		m_setRemovedObjects.Clear();

		// try to delete files
		MapPaths::iterator iter = m_mapPaths.begin();
		while (iter != m_mapPaths.end())
		{
			BOOL bDeleted = DeleteFile(iter->second.m_path.ToString());
			if (bDeleted || GetLastError() == ERROR_FILE_NOT_FOUND)
			{
				m_setRemovedObjects.Add(iter->first);
				iter = m_mapPaths.erase(iter);
			}
			else
				++iter;
		}
	}

	void TObsoleteFiles::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
		{
			rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
			rColumns.AddColumn(_T("path"), IColumnsDefinition::eType_path);
		}
	}
}
