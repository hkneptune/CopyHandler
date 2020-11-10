#pragma once

#include "libchengine.h"
#include "FeedbackErrorRule.h"
#include "TConfig.h"
#include "../libserializer/SerializableContainer.h"

namespace chengine
{
#pragma warning(push)
#pragma warning(disable: 4251)

	class LIBCHENGINE_API FeedbackErrorRuleList : public serializer::SerializableContainer<FeedbackErrorRule>
	{
	public:
		EFeedbackResult Matches(const string::TString& strSrcPath, const string::TString& /*strDstPath*/, EFileError eErrorType, unsigned long ulError) const;

		void Merge(const FeedbackErrorRuleList& rSrc);

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const override;

		void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

	private:
		void InsertOrUpdateRule(const FeedbackErrorRule& rRule);
	};
#pragma warning(pop)
}

CONFIG_MEMBER_SERIALIZATION(FeedbackErrorRuleList)
