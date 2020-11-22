#include "stdafx.h"
#include "FeedbackOperationEventRule.h"
#include "../libstring/TString.h"
#include "../libstring/TStringArray.h"
#include "../libchcore/TPath.h"
#include "EOperationEventMapper.h"
#include "EFeedbackResultMapper.h"

using namespace serializer;
using namespace string;
using namespace chcore;

namespace chengine
{
	FeedbackOperationEventRule::FeedbackOperationEventRule() :
		m_bUseOperationEvent(m_setModifications, false),
		m_eOperationEvent(m_setModifications, eOperationEvent_Finished),
		m_eResult(m_setModifications, eResult_Unknown)
	{
		m_setModifications[FeedbackOperationEventRuleEnum::eMod_Added] = true;
	}

	FeedbackOperationEventRule::FeedbackOperationEventRule(const FeedbackOperationEventRule& rSrc) :
		serializer::SerializableObject<FeedbackOperationEventRuleEnum::eMod_Last, FeedbackOperationEventRuleEnum::eMod_Added>(rSrc),
		m_bUseOperationEvent(rSrc.m_bUseOperationEvent, m_setModifications),
		m_eOperationEvent(rSrc.m_eOperationEvent, m_setModifications),
		m_eResult(rSrc.m_eResult, m_setModifications)
	{
	}

	FeedbackOperationEventRule& FeedbackOperationEventRule::operator=(const FeedbackOperationEventRule& rSrc)
	{
		if(this == &rSrc)
			return *this;

		__super::operator=(rSrc);

		SetData(rSrc);

		return *this;
	}

	bool FeedbackOperationEventRule::operator==(const FeedbackOperationEventRule& rSrc) const
	{
		if(m_bUseOperationEvent != rSrc.m_bUseOperationEvent)
			return false;
		if(m_eOperationEvent != rSrc.m_eOperationEvent)
			return false;

		if(m_eResult != rSrc.m_eResult)
			return false;

		return true;
	}

	bool FeedbackOperationEventRule::operator!=(const FeedbackOperationEventRule& rSrc) const
	{
		return !operator==(rSrc);
	}

	void FeedbackOperationEventRule::SetData(const FeedbackOperationEventRule& rSrc)
	{
		if(this == &rSrc)
			return;

		m_bUseOperationEvent = rSrc.m_bUseOperationEvent;
		m_eOperationEvent = rSrc.m_eOperationEvent;
		m_eResult = rSrc.m_eResult;
	}

	bool FeedbackOperationEventRule::Matches(EOperationEvent eEvent, EFeedbackResult& eResult) const
	{
		eResult = eResult_Unknown;

		if(m_bUseOperationEvent)
		{
			if(m_eOperationEvent != eEvent)
				return false;
		}

		eResult = m_eResult;
		return true;
	}

	void FeedbackOperationEventRule::InitColumns(serializer::IColumnsDefinition& rColumns)
	{
		rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("use_operation_event"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("operation_event"), IColumnsDefinition::eType_int);
		rColumns.AddColumn(_T("result"), IColumnsDefinition::eType_int);
	}

	void FeedbackOperationEventRule::Store(const ISerializerContainerPtr& spContainer) const
	{
		bool bAdded = m_setModifications[FeedbackOperationEventRuleEnum::eMod_Added];
		if(m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(m_oidObjectID, bAdded);

			if(bAdded || m_setModifications[FeedbackOperationEventRuleEnum::eMod_UseOperationEvent])
				rRow.SetValue(_T("use_operation_event"), m_bUseOperationEvent);
			if(bAdded || m_setModifications[FeedbackOperationEventRuleEnum::eMod_OperationEvent])
				rRow.SetValue(_T("operation_event"), m_eOperationEvent);

			if(bAdded || m_setModifications[FeedbackOperationEventRuleEnum::eMod_Result])
				rRow.SetValue(_T("result"), m_eResult);

			m_setModifications.reset();
		}
	}

	void FeedbackOperationEventRule::Load(const ISerializerRowReaderPtr& spRowReader)
	{
		TString strMask;

		spRowReader->GetValue(_T("id"), m_oidObjectID);

		spRowReader->GetValue(_T("use_operation_event"), m_bUseOperationEvent.Modify());
		spRowReader->GetValue(_T("operation_event"), *(int*)&m_eOperationEvent.Modify());

		spRowReader->GetValue(_T("result"), *(int*)&m_eResult.Modify());

		m_setModifications.reset();
	}

	void FeedbackOperationEventRule::StoreInConfig(TConfig& rConfig) const
	{
		SetConfigValue(rConfig, _T("OperationEvent.Use"), m_bUseOperationEvent.Get());
		SetConfigValue(rConfig, _T("OperationEvent.Value"), MapEnum(m_eOperationEvent.Get()));

		SetConfigValue(rConfig, _T("Result"), m_eResult.Get());
	}

	void FeedbackOperationEventRule::ReadFromConfig(const TConfig& rConfig)
	{
		m_bUseOperationEvent = GetConfigValueDef(rConfig, _T("OperationEvent.Use"), false);
		m_eOperationEvent = UnmapEnum<EOperationEvent>(GetConfigValueDef(rConfig, _T("OperationEvent.Value"), TString(L"finished")));
		m_eResult = UnmapEnum<EFeedbackResult>(GetConfigValueDef(rConfig, _T("Result"), TString(L"unknown")));
	}

	bool FeedbackOperationEventRule::HaveSameCondition(const FeedbackOperationEventRule& rSrc) const
	{
		if(m_bUseOperationEvent != rSrc.m_bUseOperationEvent)
			return false;
		else if(m_bUseOperationEvent == true && m_eOperationEvent != rSrc.m_eOperationEvent)
			return false;

		return true;
	}

	bool FeedbackOperationEventRule::GetUseOperationEvent() const
	{
		return m_bUseOperationEvent;
	}

	void FeedbackOperationEventRule::SetUseOperationEventType(bool bUseOperationEvent)
	{
		m_bUseOperationEvent = bUseOperationEvent;
	}

	EOperationEvent FeedbackOperationEventRule::GetOperationEvent() const
	{
		return m_eOperationEvent;
	}

	void FeedbackOperationEventRule::SetOperationEvent(EOperationEvent eOperationEvent)
	{
		m_eOperationEvent = eOperationEvent;
	}

	chengine::EFeedbackResult FeedbackOperationEventRule::GetResult() const
	{
		return m_eResult;
	}

	void FeedbackOperationEventRule::SetResult(EFeedbackResult eResult)
	{
		m_eResult = eResult;
	}
}
