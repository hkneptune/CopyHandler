#include "stdafx.h"
#include "FeedbackPredefinedRules.h"
#include "FeedbackAlreadyExistsRuleList.h"

namespace chengine
{
	FeedbackAlreadyExistsRuleList FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition eCondition, EFeedbackResult eResult)
	{
		FeedbackAlreadyExistsRuleList ruleList;

		switch(eCondition)
		{
		case EPredefinedRuleCondition::eCondition_ApplyToAll:
		{
			FeedbackAlreadyExistsRule rule;
			rule.SetResult(eResult);
			ruleList.Add(rule);
			break;
		}
		case EPredefinedRuleCondition::eCondition_WhenDifferentDateOrSize:
		{
			FeedbackAlreadyExistsRule rule;
			rule.SetResult(eResult);
			rule.SetUseDateCompare(true);
			rule.SetDateCompareType(eCmp_NotEqual);
			ruleList.Add(rule);

			FeedbackAlreadyExistsRule rule2;
			rule2.SetResult(eResult);
			rule2.SetUseSizeCompare(true);
			rule2.SetSizeCompareType(eCmp_NotEqual);
			ruleList.Add(rule2);

			break;
		}
		case EPredefinedRuleCondition::eCondition_WhenSameDateAndSize:
		{
			FeedbackAlreadyExistsRule rule;
			rule.SetResult(eResult);
			rule.SetUseDateCompare(true);
			rule.SetDateCompareType(eCmp_Equal);
			rule.SetUseSizeCompare(true);
			rule.SetSizeCompareType(eCmp_Equal);
			ruleList.Add(rule);
			break;
		}
		case EPredefinedRuleCondition::eCondition_WhenNewerThanDst:
		{
			FeedbackAlreadyExistsRule rule;
			rule.SetResult(eResult);
			rule.SetUseDateCompare(true);
			rule.SetDateCompareType(eCmp_Greater);
			ruleList.Add(rule);
			break;
		}
		case EPredefinedRuleCondition::eCondition_WhenOlderThanDst:
		{
			FeedbackAlreadyExistsRule rule;
			rule.SetResult(eResult);
			rule.SetUseDateCompare(true);
			rule.SetDateCompareType(eCmp_Less);
			ruleList.Add(rule);
			break;
		}
		case EPredefinedRuleCondition::eCondition_WhenSmallerThanDst:
		{
			FeedbackAlreadyExistsRule rule;
			rule.SetResult(eResult);
			rule.SetUseSizeCompare(true);
			rule.SetSizeCompareType(eCmp_Less);
			ruleList.Add(rule);
			break;
		}
		case EPredefinedRuleCondition::eCondition_WhenBiggerThanDst:
		{
			FeedbackAlreadyExistsRule rule;
			rule.SetResult(eResult);
			rule.SetUseSizeCompare(true);
			rule.SetSizeCompareType(eCmp_Greater);
			ruleList.Add(rule);
			break;
		}
		default:
			throw std::runtime_error("Unhandled predefined rule condition");
		}

		return ruleList;
	}
}
