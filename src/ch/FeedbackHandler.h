/***************************************************************************
 *   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
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
#ifndef __FEEDBACKHANDLER_H__
#define __FEEDBACKHANDLER_H__

#include "../libchengine/IFeedbackHandler.h"

namespace string
{
	class TString;
}

class CFeedbackHandler : public chengine::IFeedbackHandler
{
public:
	chengine::EFeedbackResult FileError(const string::TString& strSrcPath, const string::TString& strDstPath, chengine::EFileError eFileError, unsigned long ulError, chengine::FeedbackErrorRuleList& rNewRules) override;
	chengine::EFeedbackResult FileAlreadyExists(const chengine::TFileInfo& spSrcFileInfo, const chengine::TFileInfo& spDstFileInfo, chengine::FeedbackAlreadyExistsRuleList& rNewRules) override;
	chengine::EFeedbackResult NotEnoughSpace(const string::TString& strSrcPath, const string::TString& strDstPath, unsigned long long ullRequiredSize, chengine::FeedbackNotEnoughSpaceRuleList& rNewRules) override;
	chengine::EFeedbackResult OperationEvent(chengine::EOperationEvent eEvent, chengine::FeedbackOperationEventRuleList& rNewRules) override;

protected:
	friend class CFeedbackHandlerFactory;
};

typedef std::shared_ptr<CFeedbackHandler> CFeedbackHandlerPtr;

#endif
