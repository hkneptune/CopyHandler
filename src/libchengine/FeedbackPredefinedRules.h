#pragma once

#include "libchengine.h"
#include "EFeedbackResult.h"
#include "FeedbackAlreadyExistsRuleList.h"

namespace chengine
{
	enum class EPredefinedRuleCondition
	{
		eCondition_ApplyToAll,

		eCondition_WhenDifferentDateOrSize,
		eCondition_WhenSameDateAndSize,

		eCondition_WhenNewerThanDst,
		eCondition_WhenOlderThanDst,

		eCondition_WhenSmallerThanDst,
		eCondition_WhenBiggerThanDst
	};

	class LIBCHENGINE_API FeedbackPredefinedRules
	{
	public:
		static FeedbackAlreadyExistsRuleList CreateAlreadyExistsRule(EPredefinedRuleCondition eCondition, EFeedbackResult eResult);
	};
}
