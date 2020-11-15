// ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
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
#include "FeedbackHandler.h"
#include "FeedbackReplaceDlg.h"
#include "FeedbackFileErrorDlg.h"
#include "FeedbackNotEnoughSpaceDlg.h"
#include "ch.h"
#include "mmsystem.h"
#include "CfgProperties.h"
#include "../libchengine/TConfigSerializers.h"

using namespace chcore;
using namespace chengine;
using namespace string;

chengine::EFeedbackResult CFeedbackHandler::FileError(const TString& strSrcPath, const TString& strDstPath, EFileError eOperationType, unsigned long ulError, FeedbackRules& rNewRules)
{
	CFeedbackFileErrorDlg dlg(rNewRules, strSrcPath.c_str(), strDstPath.c_str(), eOperationType, ulError);
	EFeedbackResult eResult = (EFeedbackResult)dlg.DoModal();

	if(dlg.GetRules().IsModified())
		rNewRules = dlg.GetRules();

	return eResult;
}

chengine::EFeedbackResult CFeedbackHandler::FileAlreadyExists(const TFileInfo& spSrcFileInfo, const TFileInfo& spDstFileInfo, TString& strRenameName, FeedbackRules& rNewRules)
{
	rNewRules.ResetModifications();

	CFeedbackReplaceDlg dlg(rNewRules, spSrcFileInfo, spDstFileInfo, strRenameName);
	EFeedbackResult eResult = (EFeedbackResult)dlg.DoModal();

	if(eResult == eResult_Rename)
		strRenameName = dlg.GetNewName();

	if(dlg.GetRules().IsModified())
		rNewRules = dlg.GetRules();

	return eResult;
}

chengine::EFeedbackResult CFeedbackHandler::NotEnoughSpace(const TString& strDstPath, unsigned long long ullRequiredSize, FeedbackRules& rNewRules)
{
	CFeedbackNotEnoughSpaceDlg dlg(rNewRules, ullRequiredSize, strDstPath.c_str());
	EFeedbackResult eResult = (EFeedbackResult) dlg.DoModal();

	if(dlg.GetRules().IsModified())
		rNewRules = dlg.GetRules();

	return eResult;
}

chengine::EFeedbackResult CFeedbackHandler::OperationEvent(EOperationEvent eEvent, FeedbackRules&)
{
	switch(eEvent)
	{
	case eOperationEvent_Finished:
	{
		if(GetPropValue<PP_SNDPLAYSOUNDS>(GetConfig()))
		{
			CString strPath = GetPropValue<PP_SNDFINISHEDSOUNDPATH>(GetConfig());
			PlaySound(GetApp().ExpandPath(strPath), nullptr, SND_FILENAME | SND_ASYNC);
		}
		break;
	}
	case eOperationEvent_Error:
	{
		if(GetPropValue<PP_SNDPLAYSOUNDS>(GetConfig()))
		{
			CString strPath = GetPropValue<PP_SNDERRORSOUNDPATH>(GetConfig());
			PlaySound(GetApp().ExpandPath(strPath), nullptr, SND_FILENAME | SND_ASYNC);
		}

		break;
	}
	}

	return eResult_Unknown;
}
