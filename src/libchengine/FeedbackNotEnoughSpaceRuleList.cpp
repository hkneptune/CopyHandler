#include "stdafx.h"
#include "FeedbackNotEnoughSpaceRuleList.h"
#include "TConfigArray.h"
#include "../libserializer/IColumnsDefinition.h"
#include "TFileFilter.h"

using namespace string;
using namespace serializer;

namespace chengine
{
	EFeedbackResult FeedbackNotEnoughSpaceRuleList::Matches(const string::TString& strDstPath, unsigned long long ullRequiredSize) const
	{
		if(m_vEntries.empty())
			return eResult_Unknown;

		for(const FeedbackNotEnoughSpaceRule& rRule : m_vEntries)
		{
			EFeedbackResult eResult = eResult_Unknown;
			if(rRule.Matches(strDstPath, ullRequiredSize, eResult))
				return eResult;
		}

		return eResult_Unknown;
	}

	void FeedbackNotEnoughSpaceRuleList::Merge(const FeedbackNotEnoughSpaceRuleList& rSrc)
	{
		for(size_t stIndex = 0; stIndex < rSrc.GetCount(); ++stIndex)
		{
			InsertOrUpdateRule(rSrc.GetAt(stIndex));
		}
	}

	void FeedbackNotEnoughSpaceRuleList::InsertOrUpdateRule(const FeedbackNotEnoughSpaceRule& rRule)
	{
		bool bFound = false;
		for(size_t stIndex = 0; stIndex < GetCount(); ++stIndex)
		{
			FeedbackNotEnoughSpaceRule& rCurrent = GetAt(stIndex);
			if(rCurrent.HaveSameCondition(rRule))
			{
				bFound = true;
				rCurrent.SetResult(rRule.GetResult());
			}
		}

		if(!bFound)
			InsertAt(0, rRule);
	}

	void FeedbackNotEnoughSpaceRuleList::InitColumns(const serializer::ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			FeedbackNotEnoughSpaceRule::InitColumns(rColumns);
	}

	void FeedbackNotEnoughSpaceRuleList::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
	{
		rConfig.DeleteNode(pszNodeName);
		for(const FeedbackNotEnoughSpaceRule& rRule : m_vEntries)
		{
			TConfig cfgNode;
			rRule.StoreInConfig(cfgNode);

			TString strNode = TString(pszNodeName) + _T(".RuleDefinition");
			rConfig.AddSubConfig(strNode.c_str(), cfgNode);
		}
	}

	bool FeedbackNotEnoughSpaceRuleList::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
	{
		m_vEntries.clear();

		TConfigArray vConfigs;
		if(!rConfig.ExtractMultiSubConfigs(pszNodeName, vConfigs))
			return false;

		for(size_t stIndex = 0; stIndex < vConfigs.GetCount(); ++stIndex)
		{
			const TConfig& rCfg = vConfigs.GetAt(stIndex);
			FeedbackNotEnoughSpaceRule rule;
			rule.ReadFromConfig(rCfg);

			m_vEntries.push_back(rule);
		}
		return true;
	}
}
