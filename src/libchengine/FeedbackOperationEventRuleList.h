#pragma once

#include "libchengine.h"
#include "FeedbackOperationEventRule.h"
#include "TConfig.h"
#include "../libserializer/SerializableContainer.h"

namespace chengine
{
#pragma warning(push)
#pragma warning(disable: 4251)

	class LIBCHENGINE_API FeedbackOperationEventRuleList : public serializer::SerializableContainer<FeedbackOperationEventRule>
	{
	public:
		EFeedbackResult Matches(EOperationEvent eEvent) const;

		void Merge(const FeedbackOperationEventRuleList& rSrc);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const override;

		void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

	private:
		void InsertOrUpdateRule(const FeedbackOperationEventRule& rRule);
	};
#pragma warning(pop)
}

CONFIG_MEMBER_SERIALIZATION(FeedbackOperationEventRuleList)
