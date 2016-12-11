// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "TFeedbackHandlerWrapper.h"
#include "TScopedRunningTimeTrackerPause.h"

using namespace chcore;
using namespace string;
using namespace serializer;

namespace chengine
{
	TFeedbackHandlerWrapper::TFeedbackHandlerWrapper(const IFeedbackHandlerPtr& spFeedbackHandler, TScopedRunningTimeTracker& rTimeGuard) :
		m_spFeedbackHandler(spFeedbackHandler),
		m_rTimeGuard(rTimeGuard)
	{
	}

	TFeedbackHandlerWrapper::~TFeedbackHandlerWrapper()
	{
	}

	TFeedbackResult TFeedbackHandlerWrapper::FileError(const TString& strSrcPath, const TString& strDstPath, EFileError eFileError, unsigned long ulError)
	{
		TScopedRunningTimeTrackerPause scopedTimePause(m_rTimeGuard);

		return m_spFeedbackHandler->FileError(strSrcPath, strDstPath, eFileError, ulError);
	}

	TFeedbackResult TFeedbackHandlerWrapper::FileAlreadyExists(const TFileInfo& spSrcFileInfo, const TFileInfo& spDstFileInfo)
	{
		TScopedRunningTimeTrackerPause scopedTimePause(m_rTimeGuard);

		return m_spFeedbackHandler->FileAlreadyExists(spSrcFileInfo, spDstFileInfo);
	}

	TFeedbackResult TFeedbackHandlerWrapper::NotEnoughSpace(const TString& strSrcPath, const TString& strDstPath, unsigned long long ullRequiredSize)
	{
		TScopedRunningTimeTrackerPause scopedTimePause(m_rTimeGuard);

		return m_spFeedbackHandler->NotEnoughSpace(strSrcPath, strDstPath, ullRequiredSize);
	}

	TFeedbackResult TFeedbackHandlerWrapper::OperationFinished()
	{
		TScopedRunningTimeTrackerPause scopedTimePause(m_rTimeGuard);

		return m_spFeedbackHandler->OperationFinished();
	}

	TFeedbackResult TFeedbackHandlerWrapper::OperationError()
	{
		TScopedRunningTimeTrackerPause scopedTimePause(m_rTimeGuard);

		return m_spFeedbackHandler->OperationError();
	}

	void TFeedbackHandlerWrapper::RestoreDefaults()
	{
		return m_spFeedbackHandler->RestoreDefaults();
	}

	void TFeedbackHandlerWrapper::Store(const ISerializerContainerPtr& spContainer) const
	{
		return m_spFeedbackHandler->Store(spContainer);
	}

	void TFeedbackHandlerWrapper::Load(const ISerializerContainerPtr& spContainer)
	{
		return m_spFeedbackHandler->Load(spContainer);
	}

	DWORD TFeedbackHandlerWrapper::GetRetryInterval() const
	{
		return m_spFeedbackHandler->GetRetryInterval();
	}

}
