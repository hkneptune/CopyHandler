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
#include "TTaskBaseData.h"
#include "ISerializerRowData.h"
#include "ISerializerContainer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

namespace chcore
{
	TTaskBaseData::TTaskBaseData() :
		m_strTaskName(m_setChanges),
		m_eCurrentState(m_setChanges),
		m_pathDestinationPath(m_setChanges)
	{
		m_setChanges[eMod_Added] = true;
	}

	TTaskBaseData::~TTaskBaseData()
	{
	}

	TString TTaskBaseData::GetTaskName() const
	{
		return m_strTaskName;
	}

	void TTaskBaseData::SetTaskName(const TString& strTaskName)
	{
		m_strTaskName = strTaskName;
	}

	ETaskCurrentState TTaskBaseData::GetCurrentState() const
	{
		return m_eCurrentState;
	}

	void TTaskBaseData::SetCurrentState(ETaskCurrentState eCurrentState)
	{
		m_eCurrentState = eCurrentState;
	}

	TSmartPath TTaskBaseData::GetDestinationPath() const
	{
		return m_pathDestinationPath;
	}

	void TTaskBaseData::SetDestinationPath(const TSmartPath& pathDst)
	{
		m_pathDestinationPath = pathDst;
	}

	void TTaskBaseData::Store(const ISerializerContainerPtr& spContainer) const
	{
		InitColumns(spContainer);

		// base data
		if (m_setChanges.any())
		{
			bool bAdded = m_setChanges[eMod_Added];

			ISerializerRowData& rRow = spContainer->GetRow(0, bAdded);

			if (bAdded || m_setChanges[eMod_TaskName])
				rRow.SetValue(_T("name"), m_strTaskName);

			if (bAdded || m_setChanges[eMod_CurrentState])
				rRow.SetValue(_T("current_state"), m_eCurrentState);

			if (bAdded || m_setChanges[eMod_DstPath])
				rRow.SetValue(_T("destination_path"), m_pathDestinationPath);

			m_setChanges.reset();
		}
	}

	void TTaskBaseData::Load(const ISerializerContainerPtr& spContainer)
	{
		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

		bool bResult = spRowReader->Next();
		if (bResult)
		{
			spRowReader->GetValue(_T("name"), m_strTaskName.Modify());
			spRowReader->GetValue(_T("current_state"), *(int*)(ETaskCurrentState*)&m_eCurrentState.Modify());
			spRowReader->GetValue(_T("destination_path"), m_pathDestinationPath.Modify());
		}
		else
			throw TCoreException(eErr_SerializeLoadError, L"Reading next row failed", LOCATION);

		m_setChanges.reset();
	}

	void TTaskBaseData::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
		{
			rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
			rColumns.AddColumn(_T("name"), IColumnsDefinition::eType_string);
			rColumns.AddColumn(_T("current_state"), IColumnsDefinition::eType_int);
			rColumns.AddColumn(_T("destination_path"), IColumnsDefinition::eType_path);
		}
	}
}
