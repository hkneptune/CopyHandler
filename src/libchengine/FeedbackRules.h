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
#pragma once

#include "libchengine.h"
#include "FeedbackAlreadyExistsRuleList.h"
#include "FeedbackErrorRuleList.h"
#include "FeedbackNotEnoughSpaceRuleList.h"
#include "FeedbackOperationEventRuleList.h"
#include "../libserializer/ISerializer.h"
#include "TConfig.h"

namespace chengine
{
	class LIBCHENGINE_API FeedbackRules
	{
	public:
		// serialization
		void Store(const serializer::ISerializerPtr& spSerializer) const;
		void Load(const serializer::ISerializerPtr& spSerializer);

		void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

		bool IsEmpty() const;
		void Clear();

		void ResetModifications();
		bool IsModified() const;

		const FeedbackAlreadyExistsRuleList& GetAlreadyExistsRules() const;
		FeedbackAlreadyExistsRuleList& GetAlreadyExistsRules();

		const FeedbackErrorRuleList& GetErrorRules() const;
		FeedbackErrorRuleList& GetErrorRules();

		const FeedbackNotEnoughSpaceRuleList& GetNotEnoughSpaceRules() const;
		FeedbackNotEnoughSpaceRuleList& GetNotEnoughSpaceRules();

		const FeedbackOperationEventRuleList& GetOperationEventRules() const;
		FeedbackOperationEventRuleList& GetOperationEventRules();

	private:
		FeedbackAlreadyExistsRuleList m_feedbackAlreadyExistsRules;
		FeedbackErrorRuleList m_feedbackErrorRules;
		FeedbackNotEnoughSpaceRuleList m_feedbackNotEnoughSpaceRules;
		FeedbackOperationEventRuleList m_feedbackOperationEventRules;
	};
}

CONFIG_MEMBER_SERIALIZATION(FeedbackRules)
