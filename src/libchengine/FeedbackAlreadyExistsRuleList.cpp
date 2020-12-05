#include "stdafx.h"
#include "FeedbackAlreadyExistsRuleList.h"
#include "TConfigArray.h"
#include "../libserializer/IColumnsDefinition.h"
#include "TFileFilter.h"

using namespace string;
using namespace serializer;

namespace chengine
{
	EFeedbackResult FeedbackAlreadyExistsRuleList::Matches(const TFileInfo& rSrcFile, const TFileInfo& rDstFile) const
	{
		if(m_vEntries.empty())
			return eResult_Unknown;

		for(const FeedbackAlreadyExistsRule& rRule : m_vEntries)
		{
			EFeedbackResult eResult = eResult_Unknown;
			if(rRule.Matches(rSrcFile, rDstFile, eResult))
				return eResult;
		}

		return eResult_Unknown;
	}

	void FeedbackAlreadyExistsRuleList::Merge(const FeedbackAlreadyExistsRuleList& rSrc)
	{
		for(size_t stIndex = 0; stIndex < rSrc.GetCount(); ++stIndex)
		{
			InsertOrUpdateRule(rSrc.GetAt(stIndex));
		}
	}

	void FeedbackAlreadyExistsRuleList::InsertOrUpdateRule(const FeedbackAlreadyExistsRule& rRule)
	{
		bool bFound = false;
		for(size_t stIndex = 0; stIndex < GetCount(); ++stIndex)
		{
			FeedbackAlreadyExistsRule& rCurrent = GetAt(stIndex);
			if(rCurrent.HaveSameCondition(rRule))
			{
				bFound = true;
				rCurrent.SetResult(rRule.GetResult());
			}
		}

		if(!bFound)
			InsertAt(0, rRule);
	}

	void FeedbackAlreadyExistsRuleList::InitColumns(const serializer::ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			FeedbackAlreadyExistsRule::InitColumns(rColumns);
	}

	void FeedbackAlreadyExistsRuleList::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
	{
		rConfig.DeleteNode(pszNodeName);
		for(const FeedbackAlreadyExistsRule& rRule : m_vEntries)
		{
			TConfig cfgNode;
			rRule.StoreInConfig(cfgNode);

			TString strNode = TString(pszNodeName) + _T(".RuleDefinition");
			rConfig.AddSubConfig(strNode.c_str(), cfgNode);
		}
	}

	bool FeedbackAlreadyExistsRuleList::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
	{
		m_vEntries.clear();

		TConfigArray vConfigs;
		if(!rConfig.ExtractMultiSubConfigs(pszNodeName, vConfigs))
			return false;

		for(size_t stIndex = 0; stIndex < vConfigs.GetCount(); ++stIndex)
		{
			const TConfig& rCfg = vConfigs.GetAt(stIndex);
			FeedbackAlreadyExistsRule rule;
			rule.ReadFromConfig(rCfg);

			m_vEntries.push_back(rule);
		}
		return true;
	}
}
