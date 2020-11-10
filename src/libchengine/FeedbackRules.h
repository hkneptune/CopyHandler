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

		void Clear();

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
