#include "stdafx.h"
#include "FeedbackErrorRuleList.h"
#include "TConfigArray.h"
#include "../libserializer/IColumnsDefinition.h"
#include "TFileFilter.h"

using namespace string;
using namespace serializer;

namespace chengine
{
	EFeedbackResult FeedbackErrorRuleList::Matches(const string::TString& strSrcPath, const string::TString& strDstPath, EFileError eErrorType, unsigned long ulError) const
	{
		if(m_vEntries.empty())
			return eResult_Unknown;

		for(const FeedbackErrorRule& rRule : m_vEntries)
		{
			EFeedbackResult eResult = eResult_Unknown;
			if(rRule.Matches(strSrcPath, strDstPath, eErrorType, ulError, eResult))
				return eResult;
		}

		return eResult_Unknown;
	}

	void FeedbackErrorRuleList::Merge(const FeedbackErrorRuleList& rSrc)
	{
		for(size_t stIndex = 0; stIndex < rSrc.GetCount(); ++stIndex)
		{
			InsertOrUpdateRule(rSrc.GetAt(stIndex));
		}
	}

	void FeedbackErrorRuleList::InsertOrUpdateRule(const FeedbackErrorRule& rRule)
	{
		bool bFound = false;
		for(size_t stIndex = 0; stIndex < GetCount(); ++stIndex)
		{
			FeedbackErrorRule& rCurrent = GetAt(stIndex);
			if(rCurrent.HaveSameCondition(rRule))
			{
				bFound = true;
				rCurrent.SetResult(rRule.GetResult());
			}
		}

		if(!bFound)
			InsertAt(0, rRule);
	}

	void FeedbackErrorRuleList::InitColumns(const serializer::ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			TFileFilter::InitColumns(rColumns);
	}

	void FeedbackErrorRuleList::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
	{
		rConfig.DeleteNode(pszNodeName);
		for(const FeedbackErrorRule& rRule : m_vEntries)
		{
			TConfig cfgNode;
			rRule.StoreInConfig(cfgNode);

			TString strNode = TString(pszNodeName) + _T(".RuleDefinition");
			rConfig.AddSubConfig(strNode.c_str(), cfgNode);
		}
	}

	bool FeedbackErrorRuleList::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
	{
		m_vEntries.clear();

		TConfigArray vConfigs;
		if(!rConfig.ExtractMultiSubConfigs(pszNodeName, vConfigs))
			return false;

		for(size_t stIndex = 0; stIndex < vConfigs.GetCount(); ++stIndex)
		{
			const TConfig& rCfg = vConfigs.GetAt(stIndex);
			FeedbackErrorRule rule;
			rule.ReadFromConfig(rCfg);

			m_vEntries.push_back(rule);
		}
		return true;
	}
}
