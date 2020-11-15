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
#ifndef __TFEEDBACKHANDLERBASE_H__
#define __TFEEDBACKHANDLERBASE_H__

#include "IFeedbackHandler.h"
#include <bitset>
#include "../libserializer/TSharedModificationTracker.h"
#include "../libserializer/ISerializer.h"
#include "TScopedRunningTimeTracker.h"
#include "FeedbackRules.h"

namespace chengine
{
	class LIBCHENGINE_API FeedbackManager
	{
	public:
		FeedbackManager(const IFeedbackHandlerPtr& spFeedbackHandler);
		FeedbackManager(const FeedbackManager&) = delete;

		FeedbackManager& operator=(const FeedbackManager&) = delete;

		void SetRules(const FeedbackRules& rRules);
		FeedbackRules GetRules() const;

		TFeedbackResult FileError(const string::TString& strSrcPath, const string::TString& strDstPath, EFileError eFileError, unsigned long ulError);
		TFeedbackResult FileAlreadyExists(const TFileInfoPtr& spSrcFileInfo, const TFileInfo& rDstFileInfo, const chcore::TSmartPath& suggestedPath);
		TFeedbackResult NotEnoughSpace(const string::TString& strDstPath, unsigned long long ullRequiredSize);
		TFeedbackResult OperationEvent(EOperationEvent eEvent);

		// resets the permanent status from all responses
		void RestoreDefaults();

		void SetTimeTracker(TScopedRunningTimeTracker* pTimeTracker) { m_pTimeTracker = pTimeTracker; }
		void SetSecondaryTimeTracker(TScopedRunningTimeTracker* pTimeTracker) { m_pSecondaryTimeTracker = pTimeTracker; }

		// serialization
		void Store(const serializer::ISerializerPtr& spSerializer) const;
		void Load(const serializer::ISerializerPtr& spSerializer);

		DWORD GetRetryInterval() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;

		IFeedbackHandlerPtr m_spFeedbackHandler;
		TScopedRunningTimeTracker* m_pTimeTracker = nullptr;
		TScopedRunningTimeTracker* m_pSecondaryTimeTracker = nullptr;
#pragma warning(pop)

		FeedbackRules m_feedbackRules;
	};

	using FeedbackManagerPtr = std::shared_ptr<FeedbackManager>;
}

#endif
