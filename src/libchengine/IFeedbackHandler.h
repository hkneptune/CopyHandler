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
#ifndef __FEEDBACKHANDLERBASE_H__
#define __FEEDBACKHANDLERBASE_H__

#include "EFileError.h"
#include "TFeedbackResult.h"
#include "../libstring/TString.h"
#include "../libserializer/ISerializerContainer.h"
#include "EOperationEvent.h"
#include "FeedbackErrorRuleList.h"
#include "FeedbackAlreadyExistsRuleList.h"
#include "FeedbackNotEnoughSpaceRuleList.h"
#include "FeedbackOperationEventRuleList.h"

namespace chengine
{
	class TFileInfo;

	class LIBCHENGINE_API IFeedbackHandler
	{
	public:
		virtual ~IFeedbackHandler();

		virtual EFeedbackResult FileError(const string::TString& strSrcPath, const string::TString& strDstPath, EFileError eFileError, unsigned long ulError, FeedbackErrorRuleList& rNewRules) = 0;
		virtual EFeedbackResult FileAlreadyExists(const TFileInfo& spSrcFileInfo, const TFileInfo& spDstFileInfo, FeedbackAlreadyExistsRuleList& rNewRules) = 0;
		virtual EFeedbackResult NotEnoughSpace(const string::TString& strSrcPath, const string::TString& strDstPath, unsigned long long ullRequiredSize, FeedbackNotEnoughSpaceRuleList& rNewRules) = 0;
		virtual EFeedbackResult OperationEvent(EOperationEvent eEvent, FeedbackOperationEventRuleList& rNewRules) = 0;
	};

	typedef std::shared_ptr<IFeedbackHandler> IFeedbackHandlerPtr;
}

#endif
