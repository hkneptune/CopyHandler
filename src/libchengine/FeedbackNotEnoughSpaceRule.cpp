#include "stdafx.h"
#include "FeedbackNotEnoughSpaceRule.h"
#include "../libstring/TString.h"
#include "../libstring/TStringArray.h"
#include "../libchcore/TPath.h"

using namespace serializer;
using namespace string;
using namespace chcore;

namespace chengine
{
	FeedbackNotEnoughSpaceRule::FeedbackNotEnoughSpaceRule() :
		m_bUseMask(m_setModifications, false),
		m_spaMask(m_setModifications),
		m_bUseExcludeMask(m_setModifications, false),
		m_spaExcludeMask(m_setModifications),
		m_eResult(m_setModifications, eResult_Unknown)
	{
		m_setModifications[FeedbackNotEnoughSpaceRuleEnum::eMod_Added] = true;
	}


	FeedbackNotEnoughSpaceRule::FeedbackNotEnoughSpaceRule(const FeedbackNotEnoughSpaceRule& rSrc) :
		serializer::SerializableObject<FeedbackNotEnoughSpaceRuleEnum::eMod_Last, FeedbackNotEnoughSpaceRuleEnum::eMod_Added>(rSrc),
		m_bUseMask(rSrc.m_bUseMask, m_setModifications),
		m_spaMask(rSrc.m_spaMask, m_setModifications),
		m_bUseExcludeMask(rSrc.m_bUseExcludeMask, m_setModifications),
		m_spaExcludeMask(rSrc.m_spaExcludeMask, m_setModifications),
		m_eResult(rSrc.m_eResult, m_setModifications)
	{
	}

	FeedbackNotEnoughSpaceRule& FeedbackNotEnoughSpaceRule::operator=(const FeedbackNotEnoughSpaceRule& rSrc)
	{
		if(this == &rSrc)
			return *this;

		__super::operator=(rSrc);

		SetData(rSrc);

		return *this;
	}

	bool FeedbackNotEnoughSpaceRule::operator==(const FeedbackNotEnoughSpaceRule& rSrc) const
	{
		if(m_bUseMask != rSrc.m_bUseMask)
			return false;
		if(m_spaMask != rSrc.m_spaMask)
			return false;

		if(m_bUseExcludeMask != rSrc.m_bUseExcludeMask)
			return false;

		if(m_spaExcludeMask != rSrc.m_spaExcludeMask)
			return false;

		if(m_eResult != rSrc.m_eResult)
			return false;

		return true;
	}

	bool FeedbackNotEnoughSpaceRule::operator!=(const FeedbackNotEnoughSpaceRule& rSrc) const
	{
		return !operator==(rSrc);
	}

	void FeedbackNotEnoughSpaceRule::SetData(const FeedbackNotEnoughSpaceRule& rSrc)
	{
		if(this == &rSrc)
			return;

		m_bUseMask = rSrc.m_bUseMask;
		m_spaMask = rSrc.m_spaMask;
		m_bUseExcludeMask = rSrc.m_bUseExcludeMask;
		m_spaExcludeMask = rSrc.m_spaExcludeMask;
		m_eResult = rSrc.m_eResult;
	}

	bool FeedbackNotEnoughSpaceRule::Matches(const string::TString& /*strSrcPath*/, const string::TString& strDstPath, unsigned long long /*ullRequiredSize*/, EFeedbackResult& eResult) const
	{
		eResult = eResult_Unknown;

		TSmartPath path = PathFromWString(strDstPath);
		if(m_bUseMask)
		{
			if(!m_spaMask.Get().MatchesAny(path.GetFileName().ToString()))
				return false;
		}
		if(m_bUseExcludeMask)
		{
			if(m_spaExcludeMask.Get().MatchesAny(path.GetFileName().ToString()))
				return false;
		}

		eResult = m_eResult;
		return true;
	}

	void FeedbackNotEnoughSpaceRule::InitColumns(serializer::IColumnsDefinition& rColumns)
	{
		rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("use_mask"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("mask"), IColumnsDefinition::eType_string);
		rColumns.AddColumn(_T("use_exclude_mask"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("exclude_mask"), IColumnsDefinition::eType_string);
		rColumns.AddColumn(_T("result"), IColumnsDefinition::eType_int);
	}

	void FeedbackNotEnoughSpaceRule::Store(const ISerializerContainerPtr& spContainer) const
	{
		bool bAdded = m_setModifications[FeedbackNotEnoughSpaceRuleEnum::eMod_Added];
		if(m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(m_oidObjectID, bAdded);

			if(bAdded || m_setModifications[FeedbackNotEnoughSpaceRuleEnum::eMod_UseMask])
				rRow.SetValue(_T("use_mask"), m_bUseMask);
			if(bAdded || m_setModifications[FeedbackNotEnoughSpaceRuleEnum::eMod_Mask])
				rRow.SetValue(_T("mask"), GetCombinedMask());

			if(bAdded || m_setModifications[FeedbackNotEnoughSpaceRuleEnum::eMod_UseExcludeMask])
				rRow.SetValue(_T("use_exclude_mask"), m_bUseExcludeMask);
			if(bAdded || m_setModifications[FeedbackNotEnoughSpaceRuleEnum::eMod_ExcludeMask])
				rRow.SetValue(_T("exclude_mask"), GetCombinedExcludeMask());

			if(bAdded || m_setModifications[FeedbackNotEnoughSpaceRuleEnum::eMod_Result])
				rRow.SetValue(_T("result"), m_eResult);

			m_setModifications.reset();
		}
	}

	void FeedbackNotEnoughSpaceRule::Load(const ISerializerRowReaderPtr& spRowReader)
	{
		TString strMask;

		spRowReader->GetValue(_T("id"), m_oidObjectID);

		spRowReader->GetValue(_T("use_mask"), m_bUseMask.Modify());
		spRowReader->GetValue(_T("mask"), strMask);
		SetCombinedMask(strMask);

		spRowReader->GetValue(_T("use_exclude_mask"), m_bUseExcludeMask.Modify());
		spRowReader->GetValue(_T("exclude_mask"), strMask);
		SetCombinedExcludeMask(strMask);

		spRowReader->GetValue(_T("result"), *(int*)&m_eResult.Modify());

		m_setModifications.reset();
	}

	void FeedbackNotEnoughSpaceRule::StoreInConfig(TConfig& rConfig) const
	{
		SetConfigValue(rConfig, _T("IncludeMask.Use"), m_bUseMask.Get());
		SetConfigValue(rConfig, _T("IncludeMask.MaskList.Mask"), m_spaMask.Get().ToSerializedStringArray());

		SetConfigValue(rConfig, _T("ExcludeMask.Use"), m_bUseExcludeMask.Get());
		SetConfigValue(rConfig, _T("ExcludeMask.MaskList.Mask"), m_spaExcludeMask.Get().ToSerializedStringArray());

		SetConfigValue(rConfig, _T("Result"), m_eResult.Get());
	}

	void FeedbackNotEnoughSpaceRule::ReadFromConfig(const TConfig& rConfig)
	{
		if(!GetConfigValue(rConfig, _T("IncludeMask.Use"), m_bUseMask.Modify()))
			m_bUseMask = false;

		TStringArray arrMask;
		m_spaMask.Modify().Clear();
		GetConfigValue(rConfig, _T("IncludeMask.MaskList.Mask"), arrMask);
		m_spaMask.Modify().FromSerializedStringArray(arrMask);

		if(!GetConfigValue(rConfig, _T("ExcludeMask.Use"), m_bUseExcludeMask.Modify()))
			m_bUseExcludeMask = false;

		m_spaExcludeMask.Modify().Clear();
		GetConfigValue(rConfig, _T("ExcludeMask.MaskList.Mask"), arrMask);
		m_spaExcludeMask.Modify().FromSerializedStringArray(arrMask);

		if(!GetConfigValue(rConfig, _T("Result"), *(int*)m_eResult.Modify()))
			m_eResult = eResult_Unknown;
	}

	bool FeedbackNotEnoughSpaceRule::HaveSameCondition(const FeedbackNotEnoughSpaceRule& rSrc) const
	{
		if(m_bUseMask != rSrc.m_bUseMask)
			return false;
		else if(m_bUseMask == true && m_spaMask != rSrc.m_spaMask)
			return false;

		if(m_bUseExcludeMask != rSrc.m_bUseExcludeMask)
			return false;
		else if(m_bUseExcludeMask == true && m_spaExcludeMask != rSrc.m_spaExcludeMask)
			return false;

		return true;
	}

	void FeedbackNotEnoughSpaceRule::SetUseMask(bool bUseMask)
	{
		m_bUseMask = bUseMask;
	}

	bool FeedbackNotEnoughSpaceRule::GetUseMask() const
	{
		return m_bUseMask;
	}

	bool FeedbackNotEnoughSpaceRule::GetUseExcludeMask() const
	{
		return m_bUseExcludeMask;
	}

	void FeedbackNotEnoughSpaceRule::SetUseExcludeMask(bool bUseExcludeMask)
	{
		m_bUseExcludeMask = bUseExcludeMask;
	}

	TString FeedbackNotEnoughSpaceRule::GetCombinedMask() const
	{
		return m_spaMask.Get().ToString();
	}

	void FeedbackNotEnoughSpaceRule::SetCombinedMask(const TString& strMask)
	{
		TStringPatternArray& rPatterns = m_spaMask.Modify();
		rPatterns.Clear();
		rPatterns.FromString(strMask);
	}

	TString FeedbackNotEnoughSpaceRule::GetCombinedExcludeMask() const
	{
		return m_spaExcludeMask.Get().ToString();
	}

	void FeedbackNotEnoughSpaceRule::SetCombinedExcludeMask(const TString& strMask)
	{
		TStringPatternArray& rPatterns = m_spaExcludeMask.Modify();
		rPatterns.Clear();
		rPatterns.FromString(strMask);
	}

	chengine::EFeedbackResult FeedbackNotEnoughSpaceRule::GetResult() const
	{
		return m_eResult;
	}

	void FeedbackNotEnoughSpaceRule::SetResult(EFeedbackResult eResult)
	{
		m_eResult = eResult;
	}
}
