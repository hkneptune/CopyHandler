#pragma once

#include "libchengine.h"
#include "FeedbackRule.h"
#include "TConfig.h"
#include "../libserializer/SerializableContainer.h"

namespace chengine
{
#pragma warning(push)
#pragma warning(disable: 4251)

	class LIBCHENGINE_API FeedbackRuleList : public serializer::SerializableContainer<FeedbackRule>
	{
	public:
		EFeedbackResult Matches(const TFileInfoPtr& spSrcFile, const TFileInfoPtr& spDstFile) const;

		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const override;

		void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);
	};
#pragma warning(pop)
}

CONFIG_MEMBER_SERIALIZATION(FeedbackRuleList)
