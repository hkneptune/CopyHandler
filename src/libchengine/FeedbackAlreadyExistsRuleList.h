#pragma once

#include "libchengine.h"
#include "FeedbackAlreadyExistsRule.h"
#include "TConfig.h"
#include "../libserializer/SerializableContainer.h"

namespace chengine
{
#pragma warning(push)
#pragma warning(disable: 4251)

	class LIBCHENGINE_API FeedbackAlreadyExistsRuleList : public serializer::SerializableContainer<FeedbackAlreadyExistsRule>
	{
	public:
		EFeedbackResult Matches(const TFileInfo& rSrcFile, const TFileInfo& rDstFile) const;

		void Merge(const FeedbackAlreadyExistsRuleList& rSrc);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const override;

		void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

	private:
		void InsertOrUpdateRule(const FeedbackAlreadyExistsRule& rRule);
	};
#pragma warning(pop)
}

CONFIG_MEMBER_SERIALIZATION(FeedbackAlreadyExistsRuleList)
