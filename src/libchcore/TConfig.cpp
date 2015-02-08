// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
/// @file  TConfig.cpp
/// @date  2010/09/27
/// @brief Contains implementations of classes related to configuration handling.
// ============================================================================
#include "stdafx.h"
#include "TConfig.h"
#include <fstream>
#include <iostream>
#include <ios>
#include "../libicpf/exception.h"
#include "TConfigArray.h"

#pragma warning(push)
#pragma warning(disable: 4702 4512)
	#include <boost/property_tree/xml_parser.hpp>
#pragma warning(pop)

#include <boost/algorithm/string/find.hpp>
#include <deque>
#include "TConfigNotifier.h"
#include "ConfigNodeContainer.h"
#include "ErrorCodes.h"
#include "TCoreException.h"
#include "ISerializerRowData.h"

BEGIN_CHCORE_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////
// class TConfig

using namespace details;

TConfig::TConfig() :
	m_pImpl(new details::ConfigNodeContainer)
{
}

TConfig::TConfig(const TConfig& rSrc) :
	m_pImpl(new details::ConfigNodeContainer(*rSrc.m_pImpl))
{
}

TConfig& TConfig::operator=(const TConfig& rSrc)
{
	if(this != &rSrc)
		*m_pImpl = *rSrc.m_pImpl;

	return *this;
}

TConfig::~TConfig()
{
	delete m_pImpl;
}

// read/write
void TConfig::Read(PCTSTR pszFile)
{
	if(!pszFile)
		THROW(_T("Invalid argument"), 0, 0, 0);

	{
		boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
		// Note: we need to store filename for later use BEFORE trying to open a file
		//       since it might be nonexistent, but we still would like to store config to this file later
		ClearNL();
		GetImpl()->m_strFilePath = pszFile;
	}

	// convert our underlying data to a property tree (currently probably the easiest way to convert data to xml
	boost::property_tree::wiptree tPropertyTree;

	std::wifstream ifs(pszFile, std::ios_base::in);
	boost::property_tree::xml_parser::read_xml(ifs, tPropertyTree);

	boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
	GetImpl()->ImportFromPropertyTree(tPropertyTree, lock);
}

void TConfig::Write()
{
	// NOTE: locking is done inside ExportToPropertyTree()
	boost::property_tree::wiptree tPropertyTree;
	GetImpl()->ExportToPropertyTree(tPropertyTree);

	std::wofstream ofs(GetImpl()->m_strFilePath.c_str(), std::ios_base::out);
	boost::property_tree::xml_parser::write_xml(ofs, tPropertyTree);
}

void TConfig::ReadFromString(const TString& strInput)
{
	if(strInput.IsEmpty())
		THROW(_T("Invalid argument"), 0, 0, 0);

	boost::property_tree::wiptree tPropertyTree;

	std::wistringstream ifs(strInput.c_str(), std::ios_base::in);
	boost::property_tree::xml_parser::read_xml(ifs, tPropertyTree);

	boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);

	ClearNL();

	GetImpl()->ImportFromPropertyTree(tPropertyTree, lock);
}

void TConfig::WriteToString(TString& strOutput)
{
	// NOTE: locking is done inside ExportToPropertyTree()

	boost::property_tree::wiptree tPropertyTree;
	GetImpl()->ExportToPropertyTree(tPropertyTree);

	std::wostringstream ofs(std::ios_base::out);
	boost::property_tree::xml_parser::write_xml(ofs, tPropertyTree);

	strOutput = ofs.str().c_str();
}


void TConfig::Store(const ISerializerContainerPtr& spContainer) const
{
	if(!spContainer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	boost::shared_lock<boost::shared_mutex> lock(GetImpl()->m_lock);

	InitColumns(spContainer);

	spContainer->DeleteRows(m_pImpl->m_setRemovedObjects);
	m_pImpl->m_setRemovedObjects.Clear();

	BOOST_FOREACH(const ConfigNode& rNode, m_pImpl->m_mic)
	{
		bool bAdded = rNode.m_setModifications[ConfigNode::eMod_Added];
		if(rNode.m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(rNode.m_oidObjectID, bAdded);
			if(bAdded || rNode.m_setModifications[ConfigNode::eMod_NodeName])
				rRow.SetValue(_T("name"), rNode.GetNodeName());
			if(bAdded || rNode.m_setModifications[ConfigNode::eMod_Order])
				rRow.SetValue(_T("node_order"), rNode.GetOrder());
			if(bAdded || rNode.m_setModifications[ConfigNode::eMod_Value])
				rRow.SetValue(_T("value"), rNode.m_strValue.Get());

			rNode.m_setModifications.reset();
		}
	}
}

void TConfig::Load(const ISerializerContainerPtr& spContainer) const
{
	if(!spContainer)
		THROW_CORE_EXCEPTION(eErr_InvalidPointer);

	boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
	m_pImpl->m_setRemovedObjects.Clear();
	m_pImpl->m_mic.clear();

	InitColumns(spContainer);

	ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();

	while(spRowReader->Next())
	{
		TString strName;
		int iOrder = 0;
		TString strValue;

		spRowReader->GetValue(_T("name"), strName);
		spRowReader->GetValue(_T("node_order"), iOrder);
		spRowReader->GetValue(_T("value"), strValue);

		m_pImpl->AddEntry(strName.c_str(), iOrder, strValue);	// also resets modification state inside
	}
}

void TConfig::InitColumns(const ISerializerContainerPtr& spContainer) const
{
	IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
	if(rColumns.IsEmpty())
	{
		rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("name"), IColumnsDefinition::eType_string);
		rColumns.AddColumn(_T("node_order"), IColumnsDefinition::eType_int);
		rColumns.AddColumn(_T("value"), IColumnsDefinition::eType_string);
	}
}

void TConfig::SetFilePath(PCTSTR pszPath)
{
	boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
	GetImpl()->m_strFilePath = pszPath;
}

void TConfig::Clear()
{
	boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);

	ClearNL();
}

void TConfig::ClearNL()
{
	GetImpl()->m_mic.clear();
	GetImpl()->m_setDelayedNotifications.Clear();
	GetImpl()->m_bDelayedEnabled = false;
	GetImpl()->m_strFilePath.Clear();
}

// value setting/retrieval
bool TConfig::GetBool(PCTSTR pszPropName, bool bDefault) const
{
	return GetImpl()->GetValue(pszPropName, bDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, bool& bValue) const
{
	return GetImpl()->GetValueNoDefault(pszPropName, bValue);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, bool bValue)
{
	if(GetImpl()->SetValue(pszPropName, bValue))
		SendNotification(pszPropName);

	return *this;
}

int TConfig::GetInt(PCTSTR pszPropName, int iDefault) const
{
	return GetImpl()->GetValue(pszPropName, iDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, int& iValue) const
{
	return GetImpl()->GetValueNoDefault(pszPropName, iValue);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, int iValue)
{
	if(GetImpl()->SetValue(pszPropName, iValue))
		SendNotification(pszPropName);

	return *this;
}

unsigned int TConfig::GetUInt(PCTSTR pszPropName, unsigned int uiDefault) const
{
	return GetImpl()->GetValue(pszPropName, uiDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, unsigned int& uiValue) const
{
	return GetImpl()->GetValueNoDefault(pszPropName, uiValue);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, unsigned int uiValue)
{
	if(GetImpl()->SetValue(pszPropName, uiValue))
		SendNotification(pszPropName);

	return *this;
}

long long TConfig::GetLongLong(PCTSTR pszPropName, long long llDefault) const
{
	return GetImpl()->GetValue(pszPropName, llDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, long long& llValue) const
{
	return GetImpl()->GetValueNoDefault(pszPropName, llValue);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, long long llValue)
{
	if(GetImpl()->SetValue(pszPropName, llValue))
		SendNotification(pszPropName);

	return *this;
}

unsigned long long TConfig::GetULongLong(PCTSTR pszPropName, unsigned long long ullDefault) const
{
	return GetImpl()->GetValue(pszPropName, ullDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, unsigned long long& ullValue) const
{
	return GetImpl()->GetValueNoDefault(pszPropName, ullValue);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, unsigned long long ullValue)
{
	if(GetImpl()->SetValue(pszPropName, ullValue))
		SendNotification(pszPropName);

	return *this;
}

double TConfig::GetDouble(PCTSTR pszPropName, double dDefault) const
{
	return GetImpl()->GetValue(pszPropName, dDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, double& dValue) const
{
	return GetImpl()->GetValueNoDefault(pszPropName, dValue);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, double dValue)
{
	if(GetImpl()->SetValue(pszPropName, dValue))
		SendNotification(pszPropName);

	return *this;
}

TString TConfig::GetString(PCTSTR pszPropName, const TString& strDefault) const
{
	return GetImpl()->GetValue(pszPropName, strDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, TString& rstrValue) const
{
	return GetImpl()->GetValueNoDefault(pszPropName, rstrValue);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, const TString& strValue)
{
	if(GetImpl()->SetValue(pszPropName, strValue))
		SendNotification(pszPropName);

	return *this;
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, PCTSTR pszValue)
{
	return SetValue(pszPropName, TString(pszValue));
}

bool TConfig::GetValue(PCTSTR pszPropName, TStringArray& rvValues) const
{
	return GetImpl()->GetArrayValueNoDefault(pszPropName, rvValues);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, const TStringArray& rvValues)
{
	if(GetImpl()->SetArrayValue(pszPropName, rvValues))
		SendNotification(pszPropName);

	return *this;
}

void TConfig::DeleteNode(PCTSTR pszNodeName)
{
	GetImpl()->DeleteNode(pszNodeName);
}

// extraction of subtrees
bool TConfig::ExtractSubConfig(PCTSTR pszSubTreeName, TConfig& rSubConfig) const
{
	return GetImpl()->ExtractNodes(pszSubTreeName, *rSubConfig.m_pImpl);
}

bool TConfig::ExtractMultiSubConfigs(PCTSTR pszSubTreeName, TConfigArray& rSubConfigs) const
{
	rSubConfigs.Clear();

	std::vector<ConfigNodeContainer> vNodeContainers;
	if(!GetImpl()->ExtractMultipleNodes(pszSubTreeName, vNodeContainers))
		return false;

	BOOST_FOREACH(const ConfigNodeContainer& rNode, vNodeContainers)
	{
		TConfig cfg;
		*cfg.m_pImpl = rNode;

		rSubConfigs.Add(cfg);
	}

	return true;
}

void TConfig::PutSubConfig(PCTSTR pszSubTreeName, const TConfig& rSubConfig)
{
	GetImpl()->ImportNodes(pszSubTreeName, *rSubConfig.m_pImpl);
}

void TConfig::AddSubConfig(PCTSTR pszSubTreeName, const TConfig& rSubConfig)
{
	GetImpl()->AddNodes(pszSubTreeName, *rSubConfig.m_pImpl);
}

void TConfig::ConnectToNotifier(void (*pfnCallback)(const TStringSet&, void*), void* pParam)
{
	boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
	GetImpl()->m_notifier.connect(TConfigNotifier(pfnCallback, pParam));
}

void TConfig::DisconnectFromNotifier(void (*pfnCallback)(const TStringSet&, void*))
{
	boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
	GetImpl()->m_notifier.disconnect(TConfigNotifier(pfnCallback, NULL));
}

void TConfig::DelayNotifications()
{
	boost::unique_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
	GetImpl()->m_bDelayedEnabled = true;
}

void TConfig::ResumeNotifications()
{
	TStringSet setNotifications;

	// separate scope for shared mutex (to avoid calling notifier inside critical section)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
		if(GetImpl()->m_bDelayedEnabled)
		{
			GetImpl()->m_bDelayedEnabled = false;
			if(!GetImpl()->m_setDelayedNotifications.IsEmpty())
			{
				setNotifications = GetImpl()->m_setDelayedNotifications;

				boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
				GetImpl()->m_setDelayedNotifications.Clear();
			}
		}
	}

	// NOTE: no locking here!
	if(!setNotifications.IsEmpty())
		SendNotification(setNotifications);
}

void TConfig::SendNotification(const TStringSet& rsetInfo)
{
	// separate scope for shared mutex (to avoid calling notifier inside critical section)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
		if(GetImpl()->m_bDelayedEnabled)
		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);

			GetImpl()->m_setDelayedNotifications.Insert(rsetInfo);
			return;
		}
	}

	// NOTE: we don't lock here
	GetImpl()->m_notifier(rsetInfo);
}

void TConfig::SendNotification(PCTSTR pszInfo)
{
	// separate scope for shared mutex (to avoid calling notifier inside critical section)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(GetImpl()->m_lock);
		if(GetImpl()->m_bDelayedEnabled)
		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);

			GetImpl()->m_setDelayedNotifications.Insert(pszInfo);
			return;
		}
	}

	// NOTE: we don't lock here
	TStringSet setData;
	setData.Insert(pszInfo);
	GetImpl()->m_notifier(setData);
}

details::ConfigNodeContainer* TConfig::GetImpl()
{
	return m_pImpl;
}

const details::ConfigNodeContainer* TConfig::GetImpl() const
{
	return m_pImpl;
}

TSmartPath TConfig::GetPath(PCTSTR pszPropName, const TSmartPath& pathDefault) const
{
	return PathFromWString(GetString(pszPropName, pathDefault.ToWString()));
}

bool TConfig::GetValue(PCTSTR pszPropName, TSmartPath& rpathValue) const
{
	TString strPath;
	bool bResult = GetValue(pszPropName, strPath);
	rpathValue = PathFromWString(strPath);
	return bResult;
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, const TSmartPath& pathValue)
{
	return SetValue(pszPropName, pathValue.ToWString());
}

END_CHCORE_NAMESPACE
