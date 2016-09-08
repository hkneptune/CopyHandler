// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#include "TTaskManagerWrapper.h"
#include "resource.h"
#include "..\libictranslate\ResourceManager.h"
#include "..\libchcore\TTaskConfiguration.h"
#include "..\libchcore\TTaskDefinition.h"
#include "..\libchcore\TTask.h"
#include "..\libchcore\TBaseException.h"
#include "CfgProperties.h"
#include "ch.h"

TTaskManagerWrapper::TTaskManagerWrapper(const chcore::TTaskManagerPtr& spTaskManager) :
	m_spTaskManager(spTaskManager)
{
}

chcore::TTaskPtr TTaskManagerWrapper::CreateTask(chcore::TTaskDefinition& rTaskDefinition)
{
	UpdateFileNamingFormat(rTaskDefinition.GetConfiguration());

	CString strMessage;
	try
	{
		// create task with the above definition
		chcore::TTaskPtr spTask = m_spTaskManager->CreateTask(rTaskDefinition);

		// add to task list and start processing
		spTask->BeginProcessing();

		return spTask;
	}
	catch(const chcore::TBaseException& e)
	{
		const size_t stMaxError = 1024;
		wchar_t szError[ stMaxError ];
		e.GetErrorInfo(szError, stMaxError);

		strMessage = szError;
	}
	catch(const std::exception& e)
	{
		strMessage = e.what();
	}

	ictranslate::CResourceManager& rResourceManager = ictranslate::CResourceManager::Acquire();
	ictranslate::CFormat fmt;

	fmt.SetFormat(rResourceManager.LoadString(IDS_TASK_CREATE_FAILED));
	fmt.SetParam(_T("%reason"), strMessage);
	AfxMessageBox(fmt, MB_OK | MB_ICONERROR);

	return nullptr;
}

void TTaskManagerWrapper::UpdateFileNamingFormat(chcore::TConfig& rTaskConfig)
{
	ictranslate::CResourceManager& rResourceManager = ictranslate::CResourceManager::Acquire();

	CString strFirstCopyFormat;
	CString strSubsequentCopyFormat;
	bool bUseCustomNaming = GetPropValue<PP_USECUSTOMNAMING>(GetConfig());
	if(bUseCustomNaming)
	{
		strFirstCopyFormat = GetPropValue<PP_CUSTOMNAME_FIRST>(GetConfig());
		strSubsequentCopyFormat = GetPropValue<PP_CUSTOMNAME_SUBSEQUENT>(GetConfig());
	}
	else
	{
		strFirstCopyFormat = rResourceManager.LoadString(IDS_FIRSTCOPY_STRING);
		strSubsequentCopyFormat = rResourceManager.LoadString(IDS_NEXTCOPY_STRING);
	}

	// load resource strings
	chcore::SetTaskPropValue<chcore::eTO_AlternateFilenameFormatString_First>(rTaskConfig, (PCTSTR)strFirstCopyFormat);
	chcore::SetTaskPropValue<chcore::eTO_AlternateFilenameFormatString_AfterFirst>(rTaskConfig, (PCTSTR)strSubsequentCopyFormat);
}
