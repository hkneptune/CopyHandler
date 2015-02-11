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
	chcore::TFeedbackHandlerBase()
{
}

CFeedbackHandler::~CFeedbackHandler()
{
}

chcore::EFeedbackResult CFeedbackHandler::FileError(const TString& strSrcPath, const TString& strDstPath, EFileError eFileError, unsigned long ulError)
{
	chcore::EFeedbackResult eResult = TFeedbackHandlerBase::FileError(strSrcPath, strDstPath, eFileError, ulError);
	if (eResult == chcore::eResult_Unknown)
	{
		CFeedbackFileErrorDlg dlg(strSrcPath.c_str(), strDstPath.c_str(), ulError);
		eResult = (chcore::EFeedbackResult)dlg.DoModal();
		
		if (dlg.m_bAllItems)
			SetFileErrorPermanentResponse(eResult);
	}

	return eResult;
}

chcore::EFeedbackResult CFeedbackHandler::FileAlreadyExists(const TFileInfoPtr& spSrcFileInfo, const TFileInfoPtr& spDstFileInfo)
{
	chcore::EFeedbackResult eResult = TFeedbackHandlerBase::FileAlreadyExists(spSrcFileInfo, spDstFileInfo);
	if (eResult == chcore::eResult_Unknown)
	{
		CFeedbackReplaceDlg dlg(spSrcFileInfo, spDstFileInfo);
		eResult = (EFeedbackResult)dlg.DoModal();

		if(dlg.m_bAllItems)
			SetFileAlreadyExistsPermanentResponse(eResult);
	}

	return eResult;
}

chcore::EFeedbackResult CFeedbackHandler::NotEnoughSpace(const TString& strSrcPath, const TString& strDstPath, unsigned long long ullRequiredSize)
{
	chcore::EFeedbackResult eResult = TFeedbackHandlerBase::NotEnoughSpace(strSrcPath, strDstPath, ullRequiredSize);
	if (eResult == chcore::eResult_Unknown)
	{
		CFeedbackNotEnoughSpaceDlg dlg(ullRequiredSize, strSrcPath.c_str(), strDstPath.c_str());
		eResult = (EFeedbackResult) dlg.DoModal();

		if (dlg.m_bAllItems)
			SetNotEnoughSpacePermanentResponse(eResult);
	}

	return eResult;
}

chcore::EFeedbackResult CFeedbackHandler::OperationFinished()
{
	if (GetPropValue<PP_SNDPLAYSOUNDS>(GetConfig()))
	{
		CString strPath = GetPropValue<PP_SNDFINISHEDSOUNDPATH>(GetConfig());
		GetApp().ExpandPath(strPath.GetBufferSetLength(_MAX_PATH));
		strPath.ReleaseBuffer();

		PlaySound(strPath, NULL, SND_FILENAME | SND_ASYNC);
	}

	return eResult_Unknown;
}

chcore::EFeedbackResult CFeedbackHandler::OperationError()
{
	if (GetPropValue<PP_SNDPLAYSOUNDS>(GetConfig()))
	{
		CString strPath = GetPropValue<PP_SNDERRORSOUNDPATH>(GetConfig());
		GetApp().ExpandPath(strPath.GetBufferSetLength(_MAX_PATH));
		strPath.ReleaseBuffer();

		PlaySound(strPath, NULL, SND_FILENAME | SND_ASYNC);
	}

	return eResult_Unknown;
}
