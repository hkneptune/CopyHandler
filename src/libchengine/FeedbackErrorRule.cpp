#include "stdafx.h"
#include "FeedbackErrorRule.h"
#include "../libstring/TString.h"
#include "../libstring/TStringArray.h"
#include "../libchcore/TPath.h"

using namespace serializer;
using namespace string;
using namespace chcore;

namespace chengine
{
	FeedbackErrorRule::FeedbackErrorRule() :
		m_bUseMask(m_setModifications, false),
		m_spaMask(m_setModifications),
		m_bUseExcludeMask(m_setModifications, false),
		m_spaExcludeMask(m_setModifications),
		m_bUseErrorType(m_setModifications, false),
		m_eErrorType(m_setModifications, EFileError::eDeleteError),
		m_bUseSystemErrorNo(m_setModifications, false),
		m_ulSystemErrorNo(m_setModifications, 0),
		m_eResult(m_setModifications, eResult_Unknown)
	{
		m_setModifications[FeedbackErrorRuleEnum::eMod_Added] = true;
	}


	FeedbackErrorRule::FeedbackErrorRule(const FeedbackErrorRule& rSrc) :
		serializer::SerializableObject<FeedbackErrorRuleEnum::eMod_Last, FeedbackErrorRuleEnum::eMod_Added>(rSrc),
		m_bUseMask(rSrc.m_bUseMask, m_setModifications),
		m_spaMask(rSrc.m_spaMask, m_setModifications),
		m_bUseExcludeMask(rSrc.m_bUseExcludeMask, m_setModifications),
		m_spaExcludeMask(rSrc.m_spaExcludeMask, m_setModifications),
		m_bUseErrorType(rSrc.m_bUseErrorType, m_setModifications),
		m_eErrorType(rSrc.m_eErrorType, m_setModifications),
		m_bUseSystemErrorNo(rSrc.m_bUseSystemErrorNo, m_setModifications),
		m_ulSystemErrorNo(rSrc.m_ulSystemErrorNo, m_setModifications),
		m_eResult(rSrc.m_eResult, m_setModifications)
	{
	}

	FeedbackErrorRule& FeedbackErrorRule::operator=(const FeedbackErrorRule& rSrc)
	{
		if(this == &rSrc)
			return *this;

		__super::operator=(rSrc);

		SetData(rSrc);

		return *this;
	}

	bool FeedbackErrorRule::operator==(const FeedbackErrorRule& rSrc) const
	{
		if(m_bUseMask != rSrc.m_bUseMask)
			return false;
		if(m_spaMask != rSrc.m_spaMask)
			return false;

		if(m_bUseExcludeMask != rSrc.m_bUseExcludeMask)
			return false;

		if(m_spaExcludeMask != rSrc.m_spaExcludeMask)
			return false;

		if(m_bUseErrorType != rSrc.m_bUseErrorType)
			return false;
		if(m_eErrorType != rSrc.m_eErrorType)
			return false;

		if(m_bUseSystemErrorNo != rSrc.m_bUseSystemErrorNo)
			return false;
		if(m_ulSystemErrorNo != rSrc.m_ulSystemErrorNo)
			return false;

		if(m_eResult != rSrc.m_eResult)
			return false;

		return true;
	}

	bool FeedbackErrorRule::operator!=(const FeedbackErrorRule& rSrc) const
	{
		return !operator==(rSrc);
	}

	void FeedbackErrorRule::SetData(const FeedbackErrorRule& rSrc)
	{
		if(this == &rSrc)
			return;

		m_bUseMask = rSrc.m_bUseMask;
		m_spaMask = rSrc.m_spaMask;
		m_bUseExcludeMask = rSrc.m_bUseExcludeMask;
		m_spaExcludeMask = rSrc.m_spaExcludeMask;
		m_bUseErrorType = rSrc.m_bUseErrorType;
		m_eErrorType = rSrc.m_eErrorType;
		m_bUseSystemErrorNo = rSrc.m_bUseSystemErrorNo;
		m_ulSystemErrorNo = rSrc.m_ulSystemErrorNo;
		m_eResult = rSrc.m_eResult;
	}

	bool FeedbackErrorRule::Matches(const string::TString& strSrcPath, const string::TString& /*strDstPath*/, EFileError eErrorType, unsigned long ulError, EFeedbackResult& eResult) const
	{
		eResult = eResult_Unknown;

		TSmartPath path = PathFromWString(strSrcPath);
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
		if(m_bUseErrorType)
		{
			if(m_eErrorType != eErrorType)
				return false;
		}

		if(m_bUseSystemErrorNo)
		{
			if(m_ulSystemErrorNo != ulError)
				return false;
		}

		eResult = m_eResult;
		return true;
	}

	void FeedbackErrorRule::InitColumns(serializer::IColumnsDefinition& rColumns)
	{
		rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("use_mask"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("mask"), IColumnsDefinition::eType_string);
		rColumns.AddColumn(_T("use_exclude_mask"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("exclude_mask"), IColumnsDefinition::eType_string);
		rColumns.AddColumn(_T("use_error_type"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("error_type"), IColumnsDefinition::eType_int);
		rColumns.AddColumn(_T("use_system_error_no"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("system_error_no"), IColumnsDefinition::eType_uint);
		rColumns.AddColumn(_T("result"), IColumnsDefinition::eType_int);
	}

	void FeedbackErrorRule::Store(const ISerializerContainerPtr& spContainer) const
	{
		bool bAdded = m_setModifications[FeedbackErrorRuleEnum::eMod_Added];
		if(m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(m_oidObjectID, bAdded);

			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_UseMask])
				rRow.SetValue(_T("use_mask"), m_bUseMask);
			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_Mask])
				rRow.SetValue(_T("mask"), GetCombinedMask());

			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_UseExcludeMask])
				rRow.SetValue(_T("use_exclude_mask"), m_bUseExcludeMask);
			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_ExcludeMask])
				rRow.SetValue(_T("exclude_mask"), GetCombinedExcludeMask());

			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_UseErrorType])
				rRow.SetValue(_T("use_error_type"), m_bUseErrorType);
			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_ErrorType])
				rRow.SetValue(_T("error_type"), m_eErrorType);

			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_UseSystemErrorNo])
				rRow.SetValue(_T("use_system_error_no"), m_bUseSystemErrorNo);
			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_SystemErrorNo])
				rRow.SetValue(_T("system_error_no"), m_ulSystemErrorNo);

			if(bAdded || m_setModifications[FeedbackErrorRuleEnum::eMod_Result])
				rRow.SetValue(_T("result"), m_eResult);

			m_setModifications.reset();
		}
	}

	void FeedbackErrorRule::Load(const ISerializerRowReaderPtr& spRowReader)
	{
		TString strMask;

		spRowReader->GetValue(_T("id"), m_oidObjectID);

		spRowReader->GetValue(_T("use_mask"), m_bUseMask.Modify());
		spRowReader->GetValue(_T("mask"), strMask);
		SetCombinedMask(strMask);

		spRowReader->GetValue(_T("use_exclude_mask"), m_bUseExcludeMask.Modify());
		spRowReader->GetValue(_T("exclude_mask"), strMask);
		SetCombinedExcludeMask(strMask);

		spRowReader->GetValue(_T("use_error_type"), m_bUseErrorType.Modify());
		spRowReader->GetValue(_T("error_type"), *(int*)&m_eErrorType.Modify());

		spRowReader->GetValue(_T("use_system_error_no"), m_bUseSystemErrorNo.Modify());
		spRowReader->GetValue(_T("system_error_no"), m_ulSystemErrorNo.Modify());

		spRowReader->GetValue(_T("result"), *(int*)&m_eResult.Modify());

		m_setModifications.reset();
	}

	void FeedbackErrorRule::StoreInConfig(TConfig& rConfig) const
	{
		SetConfigValue(rConfig, _T("IncludeMask.Use"), m_bUseMask.Get());
		SetConfigValue(rConfig, _T("IncludeMask.MaskList.Mask"), m_spaMask.Get().ToSerializedStringArray());

		SetConfigValue(rConfig, _T("ExcludeMask.Use"), m_bUseExcludeMask.Get());
		SetConfigValue(rConfig, _T("ExcludeMask.MaskList.Mask"), m_spaExcludeMask.Get().ToSerializedStringArray());

		SetConfigValue(rConfig, _T("ErrorType.Use"), m_bUseErrorType.Get());
		SetConfigValue(rConfig, _T("ErrorType.Value"), m_eErrorType.Get());

		SetConfigValue(rConfig, _T("SystemErrorNo.Use"), m_bUseSystemErrorNo.Get());
		SetConfigValue(rConfig, _T("SystemErrorNo.Value"), m_ulSystemErrorNo.Get());

		SetConfigValue(rConfig, _T("Result"), m_eResult.Get());
	}

	void FeedbackErrorRule::ReadFromConfig(const TConfig& rConfig)
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

		if(!GetConfigValue(rConfig, _T("ErrorType.Use"), m_bUseErrorType.Modify()))
			m_bUseErrorType = false;
		if(!GetConfigValue(rConfig, _T("ErrorType.Value"), *(int*)m_eErrorType.Modify()))
			m_eErrorType = EFileError::eDeleteError;

		if(!GetConfigValue(rConfig, _T("SystemErrorNo.Use"), m_bUseSystemErrorNo.Modify()))
			m_bUseSystemErrorNo = false;
		if(!GetConfigValue(rConfig, _T("SystemErrorNo.Value"), m_ulSystemErrorNo.Modify()))
			m_ulSystemErrorNo = 0UL;

		if(!GetConfigValue(rConfig, _T("Result"), *(int*)m_eResult.Modify()))
			m_eResult = eResult_Unknown;
	}

	bool FeedbackErrorRule::HaveSameCondition(const FeedbackErrorRule& rSrc) const
	{
		if(m_bUseMask != rSrc.m_bUseMask)
			return false;
		else if(m_bUseMask == true && m_spaMask != rSrc.m_spaMask)
			return false;

		if(m_bUseExcludeMask != rSrc.m_bUseExcludeMask)
			return false;
		else if(m_bUseExcludeMask == true && m_spaExcludeMask != rSrc.m_spaExcludeMask)
			return false;

		if(m_bUseErrorType != rSrc.m_bUseErrorType)
			return false;
		else if(m_bUseErrorType == true && m_eErrorType != rSrc.m_eErrorType)
			return false;

		if(m_bUseSystemErrorNo != rSrc.m_bUseSystemErrorNo)
			return false;
		else if(m_bUseSystemErrorNo == true && m_ulSystemErrorNo != rSrc.m_ulSystemErrorNo)
			return false;

		return true;
	}

	void FeedbackErrorRule::SetUseMask(bool bUseMask)
	{
		m_bUseMask = bUseMask;
	}

	bool FeedbackErrorRule::GetUseMask() const
	{
		return m_bUseMask;
	}

	bool FeedbackErrorRule::GetUseExcludeMask() const
	{
		return m_bUseExcludeMask;
	}

	void FeedbackErrorRule::SetUseExcludeMask(bool bUseExcludeMask)
	{
		m_bUseExcludeMask = bUseExcludeMask;
	}

	bool FeedbackErrorRule::GetUseErrorType() const
	{
		return m_bUseErrorType;
	}

	void FeedbackErrorRule::SetUseErrorType(bool bUseErrorType)
	{
		m_bUseErrorType = bUseErrorType;
	}

	EFileError FeedbackErrorRule::GetErrorType() const
	{
		return m_eErrorType;
	}

	void FeedbackErrorRule::SetErrorType(EFileError eErrorType)
	{
		m_eErrorType = eErrorType;
	}

	bool FeedbackErrorRule::GetUseSystemErrorNo() const
	{
		return m_bUseSystemErrorNo;
	}

	void FeedbackErrorRule::SetUseSystemErrorNo(bool bUseSystemErrorNo)
	{
		m_bUseSystemErrorNo = bUseSystemErrorNo;
	}

	unsigned int FeedbackErrorRule::GetSystemErrorNo() const
	{
		return m_ulSystemErrorNo;
	}

	void FeedbackErrorRule::SetSystemErrorNo(unsigned int ulErrorNo)
	{
		m_ulSystemErrorNo = ulErrorNo;
	}

	TString FeedbackErrorRule::GetCombinedMask() const
	{
		return m_spaMask.Get().ToString();
	}

	void FeedbackErrorRule::SetCombinedMask(const TString& strMask)
	{
		TStringPatternArray& rPatterns = m_spaMask.Modify();
		rPatterns.Clear();
		rPatterns.FromString(strMask);
	}

	TString FeedbackErrorRule::GetCombinedExcludeMask() const
	{
		return m_spaExcludeMask.Get().ToString();
	}

	void FeedbackErrorRule::SetCombinedExcludeMask(const TString& strMask)
	{
		TStringPatternArray& rPatterns = m_spaExcludeMask.Modify();
		rPatterns.Clear();
		rPatterns.FromString(strMask);
	}

	chengine::EFeedbackResult FeedbackErrorRule::GetResult() const
	{
		return m_eResult;
	}

	void FeedbackErrorRule::SetResult(EFeedbackResult eResult)
	{
		m_eResult = eResult;
	}
}
