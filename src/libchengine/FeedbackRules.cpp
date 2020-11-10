#include "stdafx.h"
#include "FeedbackRules.h"
#include "../libserializer/ISerializerContainer.h"

using namespace serializer;

void chengine::FeedbackRules::Store(const serializer::ISerializerPtr& spSerializer) const
{
	ISerializerContainerPtr spContainer = spSerializer->GetContainer(L"feedback_error");
	m_feedbackErrorRules.Store(spContainer);

	spContainer = spSerializer->GetContainer(L"feedback_already_exists");
	m_feedbackAlreadyExistsRules.Store(spContainer);

	spContainer = spSerializer->GetContainer(L"feedback_not_enough_space");
	m_feedbackNotEnoughSpaceRules.Store(spContainer);

	spContainer = spSerializer->GetContainer(L"feedback_operation_event");
	m_feedbackOperationEventRules.Store(spContainer);
}

void chengine::FeedbackRules::Load(const serializer::ISerializerPtr& spSerializer)
{
	ISerializerContainerPtr spContainer = spSerializer->GetContainer(L"feedback_error");
	m_feedbackErrorRules.Load(spContainer);

	spContainer = spSerializer->GetContainer(L"feedback_already_exists");
	m_feedbackAlreadyExistsRules.Load(spContainer);

	spContainer = spSerializer->GetContainer(L"feedback_not_enough_space");
	m_feedbackNotEnoughSpaceRules.Load(spContainer);

	spContainer = spSerializer->GetContainer(L"feedback_operation_event");
	m_feedbackOperationEventRules.Load(spContainer);
}

void chengine::FeedbackRules::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
{
	std::wstring strNode = pszNodeName;
	std::wstring strNewNode = strNode + L".AlreadyExists";
	m_feedbackAlreadyExistsRules.StoreInConfig(rConfig, strNewNode.c_str());

	strNewNode = strNewNode + L".Error";
	m_feedbackErrorRules.StoreInConfig(rConfig, strNewNode.c_str());

	strNewNode = strNewNode + L".NotEnoughSpace";
	m_feedbackNotEnoughSpaceRules.StoreInConfig(rConfig, strNewNode.c_str());

	strNewNode = strNewNode + L".OperationEvent";
	m_feedbackOperationEventRules.StoreInConfig(rConfig, strNewNode.c_str());
}

bool chengine::FeedbackRules::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
{
	bool bResult = true;

	std::wstring strNode = pszNodeName;
	std::wstring strNewNode = strNode + L".AlreadyExists";
	bResult &= m_feedbackAlreadyExistsRules.ReadFromConfig(rConfig, strNewNode.c_str());

	strNewNode = strNewNode + L".Error";
	bResult &= m_feedbackErrorRules.ReadFromConfig(rConfig, strNewNode.c_str());

	strNewNode = strNewNode + L".NotEnoughSpace";
	bResult &= m_feedbackNotEnoughSpaceRules.ReadFromConfig(rConfig, strNewNode.c_str());

	strNewNode = strNewNode + L".OperationEvent";
	bResult &= m_feedbackOperationEventRules.ReadFromConfig(rConfig, strNewNode.c_str());

	return bResult;
}

void chengine::FeedbackRules::Clear()
{
	m_feedbackAlreadyExistsRules.Clear();
	m_feedbackErrorRules.Clear();
	m_feedbackNotEnoughSpaceRules.Clear();
	m_feedbackOperationEventRules.Clear();
}

const chengine::FeedbackAlreadyExistsRuleList& chengine::FeedbackRules::GetAlreadyExistsRules() const
{
	return m_feedbackAlreadyExistsRules;
}

const chengine::FeedbackErrorRuleList& chengine::FeedbackRules::GetErrorRules() const
{
	return m_feedbackErrorRules;
}

const chengine::FeedbackNotEnoughSpaceRuleList& chengine::FeedbackRules::GetNotEnoughSpaceRules() const
{
	return m_feedbackNotEnoughSpaceRules;
}

const chengine::FeedbackOperationEventRuleList& chengine::FeedbackRules::GetOperationEventRules() const
{
	return m_feedbackOperationEventRules;
}

chengine::FeedbackOperationEventRuleList& chengine::FeedbackRules::GetOperationEventRules()
{
	return m_feedbackOperationEventRules;
}

chengine::FeedbackNotEnoughSpaceRuleList& chengine::FeedbackRules::GetNotEnoughSpaceRules()
{
	return m_feedbackNotEnoughSpaceRules;
}

chengine::FeedbackErrorRuleList& chengine::FeedbackRules::GetErrorRules()
{
	return m_feedbackErrorRules;
}

chengine::FeedbackAlreadyExistsRuleList& chengine::FeedbackRules::GetAlreadyExistsRules()
{
	return m_feedbackAlreadyExistsRules;
}
