#include "stdafx.h"
#include "FeedbackRule.h"
#include "../libstring/TString.h"
#include "../libstring/TStringArray.h"

using namespace serializer;
using namespace string;

namespace chengine
{
	FeedbackRule::FeedbackRule() :
		m_bUseMask(m_setModifications, false),
		m_spaMask(m_setModifications),
		m_bUseExcludeMask(m_setModifications, false),
		m_spaExcludeMask(m_setModifications),
		m_bUseDateCompare(m_setModifications, false),
		m_cmpLastModified(m_setModifications, eCmp_Equal),
		m_bUseSizeCompare(m_setModifications, false),
		m_cmpSize(m_setModifications, eCmp_Equal),
		m_eResult(m_setModifications, eResult_Unknown)
	{
		m_setModifications[FeedbackRuleEnum::eMod_Added] = true;
	}


	FeedbackRule::FeedbackRule(const FeedbackRule& rSrc) :
		serializer::SerializableObject<FeedbackRuleEnum::eMod_Last>(rSrc),
		m_bUseMask(rSrc.m_bUseMask, m_setModifications),
		m_spaMask(rSrc.m_spaMask, m_setModifications),
		m_bUseExcludeMask(rSrc.m_bUseExcludeMask, m_setModifications),
		m_spaExcludeMask(rSrc.m_spaExcludeMask, m_setModifications),
		m_bUseDateCompare(rSrc.m_bUseDateCompare, m_setModifications),
		m_cmpLastModified(rSrc.m_cmpLastModified, m_setModifications),
		m_bUseSizeCompare(rSrc.m_bUseSizeCompare, m_setModifications),
		m_cmpSize(rSrc.m_cmpSize, m_setModifications),
		m_eResult(rSrc.m_eResult, m_setModifications)
	{
	}

	FeedbackRule& FeedbackRule::operator=(const FeedbackRule& rSrc)
	{
		if(this == &rSrc)
			return *this;

		__super::operator=(rSrc);

		SetData(rSrc);

		return *this;
	}

	bool FeedbackRule::operator==(const FeedbackRule& rSrc) const
	{
		if(m_bUseMask != rSrc.m_bUseMask)
			return false;
		if(m_spaMask != rSrc.m_spaMask)
			return false;

		if(m_bUseExcludeMask != rSrc.m_bUseExcludeMask)
			return false;

		if(m_spaExcludeMask != rSrc.m_spaExcludeMask)
			return false;

		if(m_bUseDateCompare != rSrc.m_bUseDateCompare)
			return false;
		if(m_cmpLastModified != rSrc.m_cmpLastModified)
			return false;

		if(m_bUseSizeCompare != rSrc.m_bUseSizeCompare)
			return false;
		if(m_cmpSize != rSrc.m_cmpSize)
			return false;

		if(m_eResult != rSrc.m_eResult)
			return false;

		return true;
	}

	bool FeedbackRule::operator!=(const FeedbackRule& rSrc) const
	{
		return !operator==(rSrc);
	}

	void FeedbackRule::SetData(const FeedbackRule& rSrc)
	{
		if(this == &rSrc)
			return;

		m_bUseMask = rSrc.m_bUseMask;
		m_spaMask = rSrc.m_spaMask;
		m_bUseExcludeMask = rSrc.m_bUseExcludeMask;
		m_spaExcludeMask = rSrc.m_spaExcludeMask;
		m_bUseDateCompare = rSrc.m_bUseDateCompare;
		m_cmpLastModified = rSrc.m_cmpLastModified;
		m_bUseSizeCompare = rSrc.m_bUseSizeCompare;
		m_cmpSize = rSrc.m_cmpSize;
		m_eResult = rSrc.m_eResult;
	}

	bool FeedbackRule::Matches(const TFileInfoPtr& spSrcFile, const TFileInfoPtr& spDstFile, EFeedbackResult& eResult) const
	{
		eResult = eResult_Unknown;

		if(m_bUseMask)
		{
			if(!m_spaMask.Get().MatchesAny(spDstFile->GetFullFilePath().GetFileName().ToString()))
				return false;
		}
		if(m_bUseExcludeMask)
		{
			if(m_spaExcludeMask.Get().MatchesAny(spDstFile->GetFullFilePath().GetFileName().ToString()))
				return false;
		}
		if(m_bUseDateCompare)
		{
			if(!CompareByType(spSrcFile->GetLastWriteTime(), spDstFile->GetLastWriteTime(), m_cmpLastModified))
				return false;
		}

		if(m_bUseSizeCompare)
		{
			if(!CompareByType(spSrcFile->GetLength64(), spDstFile->GetLength64(), m_cmpSize))
				return false;
		}

		eResult = m_eResult;
		return true;
	}

	void FeedbackRule::InitColumns(serializer::IColumnsDefinition& rColumns)
	{
		rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("use_mask"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("mask"), IColumnsDefinition::eType_string);
		rColumns.AddColumn(_T("use_exclude_mask"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("exclude_mask"), IColumnsDefinition::eType_string);
		rColumns.AddColumn(_T("use_date_compare"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("date_compare_type"), IColumnsDefinition::eType_int);
		rColumns.AddColumn(_T("use_size_compare"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("size_compare_type"), IColumnsDefinition::eType_int);
		rColumns.AddColumn(_T("result"), IColumnsDefinition::eType_int);
	}

	void FeedbackRule::Store(const ISerializerContainerPtr& spContainer) const
	{
		bool bAdded = m_setModifications[FeedbackRuleEnum::eMod_Added];
		if(m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(m_oidObjectID, bAdded);

			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_UseMask])
				rRow.SetValue(_T("use_mask"), m_bUseMask);
			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_Mask])
				rRow.SetValue(_T("mask"), GetCombinedMask());

			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_UseExcludeMask])
				rRow.SetValue(_T("use_exclude_mask"), m_bUseExcludeMask);
			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_ExcludeMask])
				rRow.SetValue(_T("exclude_mask"), GetCombinedExcludeMask());

			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_UseDateCompare])
				rRow.SetValue(_T("use_date_compare"), m_bUseDateCompare);
			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_DateCompare])
				rRow.SetValue(_T("date_compare_type"), m_cmpLastModified);

			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_UseSizeCompare])
				rRow.SetValue(_T("use_size_compare"), m_bUseSizeCompare);
			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_SizeCompare])
				rRow.SetValue(_T("size_compare_type"), m_cmpSize);

			if(bAdded || m_setModifications[FeedbackRuleEnum::eMod_Result])
				rRow.SetValue(_T("result"), m_eResult);

			m_setModifications.reset();
		}
	}

	void FeedbackRule::Load(const ISerializerRowReaderPtr& spRowReader)
	{
		TString strMask;

		spRowReader->GetValue(_T("use_mask"), m_bUseMask.Modify());
		spRowReader->GetValue(_T("mask"), strMask);
		SetCombinedMask(strMask);

		spRowReader->GetValue(_T("use_exclude_mask"), m_bUseExcludeMask.Modify());
		spRowReader->GetValue(_T("exclude_mask"), strMask);
		SetCombinedExcludeMask(strMask);

		spRowReader->GetValue(_T("use_date_compare"), m_bUseDateCompare.Modify());
		spRowReader->GetValue(_T("date_compare_type"), *(int*)&m_cmpLastModified.Modify());

		spRowReader->GetValue(_T("use_size_compare"), m_bUseSizeCompare.Modify());
		spRowReader->GetValue(_T("size_compare_type"), *(int*)&m_cmpSize.Modify());

		spRowReader->GetValue(_T("result"), *(int*)&m_eResult.Modify());

		m_setModifications.reset();
	}

	void FeedbackRule::StoreInConfig(TConfig& rConfig) const
	{
		SetConfigValue(rConfig, _T("IncludeMask.Use"), m_bUseMask.Get());
		SetConfigValue(rConfig, _T("IncludeMask.MaskList.Mask"), m_spaMask.Get().ToSerializedStringArray());

		SetConfigValue(rConfig, _T("ExcludeMask.Use"), m_bUseExcludeMask.Get());
		SetConfigValue(rConfig, _T("ExcludeMask.MaskList.Mask"), m_spaExcludeMask.Get().ToSerializedStringArray());

		SetConfigValue(rConfig, _T("DateCompare.Use"), m_bUseDateCompare.Get());
		SetConfigValue(rConfig, _T("DateCompare.CompareType"), m_cmpLastModified.Get());

		SetConfigValue(rConfig, _T("Result"), m_eResult.Get());
	}

	void FeedbackRule::ReadFromConfig(const TConfig& rConfig)
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

		if(!GetConfigValue(rConfig, _T("DateCompare.Use"), m_bUseDateCompare.Modify()))
			m_bUseDateCompare = false;
		if(!GetConfigValue(rConfig, _T("DateCompare.CompareType"), *(int*)m_cmpLastModified.Modify()))
			m_cmpLastModified = eCmp_Equal;

		if(!GetConfigValue(rConfig, _T("SizeCompare.Use"), m_bUseSizeCompare.Modify()))
			m_bUseSizeCompare = false;
		if(!GetConfigValue(rConfig, _T("SizeCompare.CompareType"), *(int*)m_cmpSize.Modify()))
			m_cmpSize = eCmp_Equal;

		if(!GetConfigValue(rConfig, _T("Result"), *(int*)m_eResult.Modify()))
			m_eResult = eResult_Unknown;
	}

	void FeedbackRule::SetUseMask(bool bUseMask)
	{
		m_bUseMask = bUseMask;
	}

	bool FeedbackRule::GetUseMask() const
	{
		return m_bUseMask;
	}

	bool FeedbackRule::GetUseExcludeMask() const
	{
		return m_bUseExcludeMask;
	}

	void FeedbackRule::SetUseExcludeMask(bool bUseExcludeMask)
	{
		m_bUseExcludeMask = bUseExcludeMask;
	}

	bool FeedbackRule::GetUseDateCompare() const
	{
		return m_bUseDateCompare;
	}

	void FeedbackRule::SetUseDateCompare(bool bUseDateCompare)
	{
		m_bUseDateCompare = bUseDateCompare;
	}

	ECompareType FeedbackRule::GetDateCompareType() const
	{
		return m_cmpLastModified;
	}

	void FeedbackRule::SetDateCompareType(ECompareType eCmpType)
	{
		m_cmpLastModified = eCmpType;
	}

	bool FeedbackRule::GetUseSizeCompare() const
	{
		return m_bUseSizeCompare;
	}

	void FeedbackRule::SetUseSizeCompare(bool bUseSizeCompare)
	{
		m_bUseSizeCompare = bUseSizeCompare;
	}

	ECompareType FeedbackRule::GetSizeCompareType() const
	{
		return m_cmpSize;
	}

	void FeedbackRule::SetSizeCompareType(ECompareType eCmpType)
	{
		m_cmpSize = eCmpType;
	}

	TString FeedbackRule::GetCombinedMask() const
	{
		return m_spaMask.Get().ToString();
	}

	void FeedbackRule::SetCombinedMask(const TString& strMask)
	{
		TStringPatternArray& rPatterns = m_spaMask.Modify();
		rPatterns.Clear();
		rPatterns.FromString(strMask);
	}

	TString FeedbackRule::GetCombinedExcludeMask() const
	{
		return m_spaExcludeMask.Get().ToString();
	}

	void FeedbackRule::SetCombinedExcludeMask(const TString& strMask)
	{
		TStringPatternArray& rPatterns = m_spaExcludeMask.Modify();
		rPatterns.Clear();
		rPatterns.FromString(strMask);
	}
}
