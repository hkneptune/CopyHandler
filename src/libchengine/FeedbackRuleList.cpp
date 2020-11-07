#include "stdafx.h"
#include "FeedbackRuleList.h"
#include "TConfigArray.h"
#include "../libserializer/IColumnsDefinition.h"
#include "TFileFilter.h"

using namespace string;
using namespace serializer;

namespace chengine
{
	EFeedbackResult FeedbackRuleList::Matches(const TFileInfoPtr& spSrcFile, const TFileInfoPtr& spDstFile) const
	{
		if(m_vEntries.empty())
			return eResult_Unknown;

		for(const FeedbackRule& rRule : m_vEntries)
		{
			EFeedbackResult eResult = eResult_Unknown;
			if(rRule.Matches(spSrcFile, spDstFile, eResult))
				return eResult;
		}

		return eResult_Unknown;
	}

	void FeedbackRuleList::InitColumns(const serializer::ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if(rColumns.IsEmpty())
			TFileFilter::InitColumns(rColumns);
	}

	void FeedbackRuleList::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
	{
		rConfig.DeleteNode(pszNodeName);
		for(const FeedbackRule& rRule : m_vEntries)
		{
			TConfig cfgNode;
			rRule.StoreInConfig(cfgNode);

			TString strNode = TString(pszNodeName) + _T(".RuleDefinition");
			rConfig.AddSubConfig(strNode.c_str(), cfgNode);
		}
	}

	bool FeedbackRuleList::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
	{
		m_vEntries.clear();

		TConfigArray vConfigs;
		if(!rConfig.ExtractMultiSubConfigs(pszNodeName, vConfigs))
			return false;

		for(size_t stIndex = 0; stIndex < vConfigs.GetCount(); ++stIndex)
		{
			const TConfig& rCfg = vConfigs.GetAt(stIndex);
			FeedbackRule rule;
			rule.ReadFromConfig(rCfg);

			m_vEntries.push_back(rule);
		}
		return true;
	}
}
