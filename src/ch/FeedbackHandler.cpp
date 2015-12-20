/***************************************************************************
 *   Copyright (C) 2001-2008 by Józef Starosczyk                           *
 *   ixen@copyhandler.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License          *
 *   (version 2) as published by the Free Software Foundation;             *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "stdafx.h"
#include "../libchcore/TFileInfo.h"
#include "FeedbackHandler.h"
#include "FeedbackReplaceDlg.h"
#include "FeedbackFileErrorDlg.h"
#include "FeedbackNotEnoughSpaceDlg.h"
#include "ch.h"
#include "mmsystem.h"

using namespace chcore;

CFeedbackHandler::CFeedbackHandler() :
	TFeedbackHandlerBase()
{
}

CFeedbackHandler::~CFeedbackHandler()
{
}

TFeedbackResult CFeedbackHandler::FileError(const TString& strSrcPath, const TString& strDstPath, EFileError /*eFileError*/, unsigned long ulError)
{
	EFeedbackResult eResult = eResult_Unknown;
	if(HasFileErrorPermanentResponse(eResult))
		return TFeedbackResult(eResult, true);

	CFeedbackFileErrorDlg dlg(strSrcPath.c_str(), strDstPath.c_str(), ulError);
	eResult = (EFeedbackResult)dlg.DoModal();

	if (dlg.m_bAllItems)
		SetFileErrorPermanentResponse(eResult);

	return TFeedbackResult(eResult, false);
}

TFeedbackResult CFeedbackHandler::FileAlreadyExists(const TFileInfo& spSrcFileInfo, const TFileInfo& spDstFileInfo)
{
	EFeedbackResult eResult = eResult_Unknown;
	if(HasFileAlreadyExistsPermanentResponse(eResult))
		return TFeedbackResult(eResult, true);

	CFeedbackReplaceDlg dlg(spSrcFileInfo, spDstFileInfo);
	eResult = (EFeedbackResult)dlg.DoModal();

	if(dlg.m_bAllItems)
		SetFileAlreadyExistsPermanentResponse(eResult);

	return TFeedbackResult(eResult, false);
}

TFeedbackResult CFeedbackHandler::NotEnoughSpace(const TString& strSrcPath, const TString& strDstPath, unsigned long long ullRequiredSize)
{
	EFeedbackResult eResult = eResult_Unknown;
	if(HasNotEnoughSpacePermanentResponse(eResult))
		return TFeedbackResult(eResult, true);

	CFeedbackNotEnoughSpaceDlg dlg(ullRequiredSize, strSrcPath.c_str(), strDstPath.c_str());
	eResult = (EFeedbackResult) dlg.DoModal();

	if (dlg.m_bAllItems)
		SetNotEnoughSpacePermanentResponse(eResult);

	return TFeedbackResult(eResult, false);
}

TFeedbackResult CFeedbackHandler::OperationFinished()
{
	if (GetPropValue<PP_SNDPLAYSOUNDS>(GetConfig()))
	{
		CString strPath = GetPropValue<PP_SNDFINISHEDSOUNDPATH>(GetConfig());
		GetApp().ExpandPath(strPath.GetBufferSetLength(_MAX_PATH));
		strPath.ReleaseBuffer();

		PlaySound(strPath, NULL, SND_FILENAME | SND_ASYNC);
	}

	return TFeedbackResult(eResult_Unknown, true);
}

TFeedbackResult CFeedbackHandler::OperationError()
{
	if (GetPropValue<PP_SNDPLAYSOUNDS>(GetConfig()))
	{
		CString strPath = GetPropValue<PP_SNDERRORSOUNDPATH>(GetConfig());
		GetApp().ExpandPath(strPath.GetBufferSetLength(_MAX_PATH));
		strPath.ReleaseBuffer();

		PlaySound(strPath, NULL, SND_FILENAME | SND_ASYNC);
	}

	return TFeedbackResult(eResult_Unknown, true);
}
