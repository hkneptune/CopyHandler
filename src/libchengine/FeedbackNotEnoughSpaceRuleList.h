#pragma once

#include "libchengine.h"
#include "FeedbackNotEnoughSpaceRule.h"
#include "TConfig.h"
#include "../libserializer/SerializableContainer.h"

namespace chengine
{
#pragma warning(push)
#pragma warning(disable: 4251)

	class LIBCHENGINE_API FeedbackNotEnoughSpaceRuleList : public serializer::SerializableContainer<FeedbackNotEnoughSpaceRule>
	{
	public:
		EFeedbackResult Matches(const string::TString& strSrcPath, const string::TString& strDstPath, unsigned long long ullRequiredSize) const;

		void Merge(const FeedbackNotEnoughSpaceRuleList& rSrc);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const override;

		void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

	private:
		void InsertOrUpdateRule(const FeedbackNotEnoughSpaceRule& rRule);
	};
#pragma warning(pop)
}

CONFIG_MEMBER_SERIALIZATION(FeedbackNotEnoughSpaceRuleList)
