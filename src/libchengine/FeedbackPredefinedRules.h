#pragma once

#include "libchengine.h"
#include "EFeedbackResult.h"
#include "FeedbackAlreadyExistsRuleList.h"
#include "FeedbackErrorRuleList.h"

namespace chengine
{
	enum class EAlreadyExistsPredefinedRuleCondition
	{
		eCondition_ApplyToAll,

		eCondition_WhenDifferentDateOrSize,
		eCondition_WhenSameDateAndSize,

		eCondition_WhenNewerThanDst,
		eCondition_WhenOlderThanDst,

		eCondition_WhenSmallerThanDst,
		eCondition_WhenBiggerThanDst
	};

	enum class EErrorPredefinedRuleCondition
	{
		eCondition_WhenSameError
	};

	class LIBCHENGINE_API FeedbackPredefinedRules
	{
	public:
		static FeedbackAlreadyExistsRuleList CreateAlreadyExistsRule(EAlreadyExistsPredefinedRuleCondition eCondition, EFeedbackResult eResult);
		static FeedbackErrorRuleList CreateErrorRule(EErrorPredefinedRuleCondition eCondition, unsigned int uiSystemError, EFeedbackResult eResult);
	};
}
