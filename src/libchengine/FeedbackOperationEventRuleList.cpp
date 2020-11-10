#include "stdafx.h"
#include "FeedbackOperationEventRuleList.h"
#include "TConfigArray.h"
#include "../libserializer/IColumnsDefinition.h"
#include "TFileFilter.h"

using namespace string;
using namespace serializer;

namespace chengine
{
	EFeedbackResult FeedbackOperationEventRuleList::Matches(EOperationEvent eEvent) const
	{
		if(m_vEntries.empty())
			return eResult_Unknown;

		for(const FeedbackOperationEventRule& rRule : m_vEntries)
		{
			EFeedbackResult eResult = eResult_Unknown;
			if(rRule.Matches(eEvent, eResult))
				return eResult;
		}

		return eResult_Unknown;
	}

	void FeedbackOperationEventRuleList::Merge(const FeedbackOperationEventRuleList& rSrc)
	{
		for(size_t stIndex = 0; stIndex < rSrc.GetCount(); ++stIndex)
		{
			InsertOrUpdateRule(rSrc.GetAt(stIndex));
		}
	}

	void FeedbackOperationEventRuleList::InsertOrUpdateRule(const FeedbackOperationEventRule& rRule)
	{
		bool bFound = false;
		for(size_t stIndex = 0; stIndex < GetCount(); ++stIndex)
		{
			FeedbackOperationEventRule& rCurrent = GetAt(stIndex);
			if(rCurrent.HaveSameCondition(rRule))
			{
				bFound = true;
				rCurrent.SetResult(rRule.GetResult());
			}
		}

		if(!bFound)
			InsertAt(0, rRule);
	}

	void FeedbackOperationEventRuleList::InitColumns(const serializer::ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			TFileFilter::InitColumns(rColumns);
	}

	void FeedbackOperationEventRuleList::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
	{
		rConfig.DeleteNode(pszNodeName);
		for(const FeedbackOperationEventRule& rRule : m_vEntries)
		{
			TConfig cfgNode;
			rRule.StoreInConfig(cfgNode);

			TString strNode = TString(pszNodeName) + _T(".RuleDefinition");
			rConfig.AddSubConfig(strNode.c_str(), cfgNode);
		}
	}

	bool FeedbackOperationEventRuleList::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
	{
		m_vEntries.clear();

		TConfigArray vConfigs;
		if(!rConfig.ExtractMultiSubConfigs(pszNodeName, vConfigs))
			return false;

		for(size_t stIndex = 0; stIndex < vConfigs.GetCount(); ++stIndex)
		{
			const TConfig& rCfg = vConfigs.GetAt(stIndex);
			FeedbackOperationEventRule rule;
			rule.ReadFromConfig(rCfg);

			m_vEntries.push_back(rule);
		}
		return true;
	}
}
