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
#include "FeedbackManager.h"
#include <boost/thread/locks.hpp>
#include "../libserializer/ISerializerRowData.h"
#include "TScopedRunningTimeTrackerPause.h"
#include "../libchcore/TCoreException.h"
#include "../libchcore/ErrorCodes.h"

using namespace serializer;
using namespace chcore;
using namespace string;

namespace chengine
{
	FeedbackManager::FeedbackManager(const IFeedbackHandlerPtr& spFeedbackHandler) :
		m_spFeedbackHandler(spFeedbackHandler)
	{
		if(!spFeedbackHandler)
			throw TCoreException(eErr_InvalidArgument, L"spFeedbackHandler", LOCATION);
	}

	void FeedbackManager::SetRules(const FeedbackRules& rRules)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_feedbackRules = rRules;
	}

	chengine::FeedbackRules FeedbackManager::GetRules() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		chengine::FeedbackRules rules = m_feedbackRules;
		return rules;
	}

	chengine::TFeedbackResult FeedbackManager::FileError(const string::TString& strSrcPath, const string::TString& strDstPath, EFileError eFileError, unsigned long ulError)
	{
		bool bAutomatedResponse = true;

		EFeedbackResult eResult = eResult_Unknown;

		{
			boost::shared_lock<boost::shared_mutex> lock(m_lock);
			eResult = m_feedbackRules.GetErrorRules().Matches(strSrcPath, strDstPath, eFileError, ulError);
		}

		if(eResult == eResult_Unknown)
		{
			FeedbackErrorRuleList newRules;
			{
				TScopedRunningTimeTrackerPause scopedTimePause(m_pTimeTracker);
				TScopedRunningTimeTrackerPause scopedSecondaryTimePause(m_pSecondaryTimeTracker);
				eResult = m_spFeedbackHandler->FileError(strSrcPath, strDstPath, eFileError, ulError, newRules);
			}
			if(eResult != eResult_Unknown)
			{
				bAutomatedResponse = false;
				if(!newRules.IsEmpty())
				{
					boost::unique_lock<boost::shared_mutex> lock(m_lock);
					m_feedbackRules.GetErrorRules().Merge(newRules);
				}
			}
		}

		return { eResult, bAutomatedResponse };
	}

	chengine::TFeedbackResult FeedbackManager::FileAlreadyExists(const TFileInfoPtr& spSrcFileInfo, const TFileInfo& rDstFileInfo, const TSmartPath& suggestedPath)
	{
		bool bAutomatedResponse = true;
		EFeedbackResult eResult = eResult_Unknown;

		{
			boost::shared_lock<boost::shared_mutex> lock(m_lock);
			eResult = m_feedbackRules.GetAlreadyExistsRules().Matches(*spSrcFileInfo, rDstFileInfo);
		}
		if(eResult == eResult_Unknown)
		{
			FeedbackRules modRules;
			TString strNewPath = suggestedPath.ToWString();
			{
				{
					boost::shared_lock<boost::shared_mutex> lock(m_lock);
					modRules = m_feedbackRules;
				}

				TScopedRunningTimeTrackerPause scopedTimePause(m_pTimeTracker);
				TScopedRunningTimeTrackerPause scopedSecondaryTimePause(m_pSecondaryTimeTracker);
				eResult = m_spFeedbackHandler->FileAlreadyExists(*spSrcFileInfo, rDstFileInfo, strNewPath, modRules);
			}
			if(eResult != eResult_Unknown)
			{
				bAutomatedResponse = false;

				{
					boost::unique_lock<boost::shared_mutex> lock(m_lock);
					m_feedbackRules = modRules;
				}
				if(eResult == eResult_Rename)
				{
					spSrcFileInfo->SetDstRelativePath(PathFromWString(strNewPath));
				}
			}
		}
		else if(eResult == eResult_Rename)
			spSrcFileInfo->SetDstRelativePath(suggestedPath);

		return { eResult, bAutomatedResponse };
	}

	chengine::TFeedbackResult FeedbackManager::NotEnoughSpace(const string::TString& strSrcPath, const string::TString& strDstPath, unsigned long long ullRequiredSize)
	{
		bool bAutomatedResponse = true;
		EFeedbackResult eResult = eResult_Unknown;

		{
			boost::shared_lock<boost::shared_mutex> lock(m_lock);
			eResult = m_feedbackRules.GetNotEnoughSpaceRules().Matches(strSrcPath, strDstPath, ullRequiredSize);
		}
		if(eResult == eResult_Unknown)
		{
			FeedbackNotEnoughSpaceRuleList newRules;
			{
				TScopedRunningTimeTrackerPause scopedTimePause(m_pTimeTracker);
				TScopedRunningTimeTrackerPause scopedSecondaryTimePause(m_pSecondaryTimeTracker);
				eResult = m_spFeedbackHandler->NotEnoughSpace(strSrcPath, strDstPath, ullRequiredSize, newRules);
			}
			if(eResult != eResult_Unknown)
			{
				bAutomatedResponse = false;
				if(!newRules.IsEmpty())
				{
					boost::unique_lock<boost::shared_mutex> lock(m_lock);
					m_feedbackRules.GetNotEnoughSpaceRules().Merge(newRules);
				}
			}
		}

		return { eResult, bAutomatedResponse };
	}

	chengine::TFeedbackResult FeedbackManager::OperationEvent(EOperationEvent eEvent)
	{
		bool bAutomatedResponse = true;
		EFeedbackResult eResult = eResult_Unknown;

		{
			boost::shared_lock<boost::shared_mutex> lock(m_lock);
			eResult = m_feedbackRules.GetOperationEventRules().Matches(eEvent);
		}
		if(eResult == eResult_Unknown)
		{
			FeedbackOperationEventRuleList newRules;
			{
				TScopedRunningTimeTrackerPause scopedTimePause(m_pTimeTracker);
				TScopedRunningTimeTrackerPause scopedSecondaryTimePause(m_pSecondaryTimeTracker);
				eResult = m_spFeedbackHandler->OperationEvent(eEvent, newRules);
			}
			if(eResult != eResult_Unknown)
			{
				bAutomatedResponse = false;
				if(!newRules.IsEmpty())
				{
					boost::unique_lock<boost::shared_mutex> lock(m_lock);
					m_feedbackRules.GetOperationEventRules().Merge(newRules);
				}
			}
		}

		return { eResult, bAutomatedResponse };
	}

	void FeedbackManager::Store(const ISerializerPtr& spSerializer) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		ISerializerContainerPtr spContainer = spSerializer->GetContainer(L"feedback_error");
		m_feedbackRules.GetErrorRules().Store(spContainer);

		spContainer = spSerializer->GetContainer(L"feedback_already_exists");
		m_feedbackRules.GetAlreadyExistsRules().Store(spContainer);

		spContainer = spSerializer->GetContainer(L"feedback_not_enough_space");
		m_feedbackRules.GetNotEnoughSpaceRules().Store(spContainer);

		spContainer = spSerializer->GetContainer(L"feedback_operation_event");
		m_feedbackRules.GetOperationEventRules().Store(spContainer);
	}

	void FeedbackManager::Load(const ISerializerPtr& spSerializer)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		ISerializerContainerPtr spContainer = spSerializer->GetContainer(L"feedback_error");
		m_feedbackRules.GetErrorRules().Load(spContainer);

		spContainer = spSerializer->GetContainer(L"feedback_already_exists");
		m_feedbackRules.GetAlreadyExistsRules().Load(spContainer);

		spContainer = spSerializer->GetContainer(L"feedback_not_enough_space");
		m_feedbackRules.GetNotEnoughSpaceRules().Load(spContainer);

		spContainer = spSerializer->GetContainer(L"feedback_operation_event");
		m_feedbackRules.GetOperationEventRules().Load(spContainer);
	}

	DWORD FeedbackManager::GetRetryInterval() const
	{
		return 100;
	}

	void FeedbackManager::RestoreDefaults()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_feedbackRules.GetErrorRules().Clear();
		m_feedbackRules.GetAlreadyExistsRules().Clear();
		m_feedbackRules.GetNotEnoughSpaceRules().Clear();
		m_feedbackRules.GetOperationEventRules().Clear();
	}
}
