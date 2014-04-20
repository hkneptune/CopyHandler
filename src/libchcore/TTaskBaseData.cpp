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

BEGIN_CHCORE_NAMESPACE

TTaskBaseData::TTaskBaseData() :
	m_strTaskName(m_setChanges),
	m_eCurrentState(m_setChanges),
	m_pathLog(m_setChanges),
	m_pathDestinationPath(m_setChanges)
{
	m_setChanges[eMod_Added] = true;
}

TTaskBaseData::~TTaskBaseData()
{
}

chcore::TString TTaskBaseData::GetTaskName() const
{
	return m_strTaskName;
}

void TTaskBaseData::SetTaskName(const TString& strTaskName)
{
	m_strTaskName = strTaskName;
}

chcore::ETaskCurrentState TTaskBaseData::GetCurrentState() const
{
	return m_eCurrentState;
}

void TTaskBaseData::SetCurrentState(ETaskCurrentState eCurrentState)
{
	m_eCurrentState = eCurrentState;
}

chcore::TSmartPath TTaskBaseData::GetLogPath() const
{
	return m_pathLog;
}

void TTaskBaseData::SetLogPath(const TSmartPath& pathLog)
{
	m_pathLog = pathLog;
}

chcore::TSmartPath TTaskBaseData::GetDestinationPath() const
{
	return m_pathDestinationPath;
}

void TTaskBaseData::SetDestinationPath(const TSmartPath& pathDst)
{
	m_pathDestinationPath = pathDst;
}

void TTaskBaseData::Store(const ISerializerContainerPtr& spContainer) const
{
	ISerializerRowDataPtr spRow;

	// base data
	if(m_setChanges.any())
	{
		bool bAdded = m_setChanges[eMod_Added];

		if(bAdded)
			spRow = spContainer->AddRow(0);
		else
			spRow = spContainer->GetRow(0);

		if(bAdded || m_setChanges[eMod_TaskName])
			*spRow % TRowData(_T("name"), m_strTaskName);

		if(bAdded || m_setChanges[eMod_LogPath])
			*spRow % TRowData(_T("log_path"), m_pathLog);

		if(bAdded || m_setChanges[eMod_CurrentState])
			*spRow % TRowData(_T("current_state"), m_eCurrentState);

		if(bAdded || m_setChanges[eMod_DstPath])
			*spRow % TRowData(_T("destination_path"), m_pathDestinationPath);

		m_setChanges.reset();
	}
}

void TTaskBaseData::Load(const ISerializerContainerPtr& spContainer)
{
	ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

	IColumnsDefinitionPtr spColumns = spRowReader->GetColumnsDefinitions();
	if(spColumns->IsEmpty())
		*spColumns % _T("name") % _T("log_path") % _T("current_state") % _T("destination_path");

	bool bResult = spRowReader->Next();
	if(bResult)
	{
		spRowReader->GetValue(_T("name"), m_strTaskName.Modify());
		spRowReader->GetValue(_T("log_path"), m_pathLog.Modify());
		spRowReader->GetValue(_T("current_state"), *(int*)(ETaskCurrentState*)&m_eCurrentState.Modify());
		spRowReader->GetValue(_T("destination_path"), m_pathDestinationPath.Modify());
	}
	else
		THROW_CORE_EXCEPTION(eErr_SerializeLoadError);

	m_setChanges.reset();
}

END_CHCORE_NAMESPACE
