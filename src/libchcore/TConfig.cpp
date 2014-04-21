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
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"
#include "TConfigArray.h"

#pragma warning(push)
#pragma warning(disable: 4702 4512)
#include <boost/property_tree/xml_parser.hpp>
#pragma warning(pop)
#include <boost/algorithm/string/find.hpp>
#include <deque>
#include "TConfigNotifier.h"

BEGIN_CHCORE_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////////
// common set/get templates
template<class T>
bool InternalGetValue(const boost::property_tree::wiptree& rTree, PCTSTR pszPropName, T& tOutValue, boost::shared_mutex& rLock)
{
	boost::shared_lock<boost::shared_mutex> lock(rLock);

	boost::optional<T> tValue = rTree.get_optional<T>(pszPropName);
	if(tValue.is_initialized())
	{
		tOutValue = tValue.get();
		return true;
	}
	else
		return false;
}

template<class T>
bool InternalSetValue(boost::property_tree::wiptree& rTree, bool& bModified, boost::shared_mutex& rLock, PCTSTR pszPropName, const T& tNewValue)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	boost::upgrade_lock<boost::shared_mutex> lock(rLock);

	boost::optional<T> tValue = rTree.get_optional<T>(pszPropName);
	if(!tValue.is_initialized() || tValue.get() != tNewValue)
	{
		boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
		rTree.put<T>(pszPropName, tNewValue);
		bModified = true;
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// class TConfig

TConfig::TConfig() :
	m_bDelayedEnabled(false),
	m_bModified(false)
{
}

TConfig::TConfig(const TConfig& rSrc) :
	m_bDelayedEnabled(false),
	m_bModified(rSrc.m_bModified)
{
	boost::shared_lock<boost::shared_mutex> lock(rSrc.m_lock);

	m_propTree = rSrc.m_propTree;
	m_strFilePath = rSrc.m_strFilePath;
}

TConfig& TConfig::operator=(const TConfig& rSrc)
{
	if(this != &rSrc)
	{
		boost::shared_lock<boost::shared_mutex> src_lock(rSrc.m_lock);
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		m_propTree = rSrc.m_propTree;
		m_bModified = rSrc.m_bModified;
		m_strFilePath = rSrc.m_strFilePath;
		m_bDelayedEnabled = false;
		m_setDelayedNotifications.Clear();
	}

	return *this;
}

TConfig::~TConfig()
{
}

// read/write
void TConfig::Read(PCTSTR pszFile)
{
	if(!pszFile)
		THROW(_T("Invalid argument"), 0, 0, 0);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	// Note: we need to store filename for later use BEFORE trying to open a file
	//       since it might be nonexistent, but we still would like to store config to this file later
	ClearNL();		// also clears m_bModified
	m_strFilePath = pszFile;

	std::wifstream ifs(m_strFilePath, std::ios_base::in);
	try
	{
		boost::property_tree::xml_parser::read_xml(ifs, m_propTree);
	}
	catch(...)
	{
		m_propTree.clear();
		throw;
	}
}

void TConfig::Write(bool bOnlyIfModified)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	if(!bOnlyIfModified || m_bModified)
	{
		std::wofstream ofs(m_strFilePath, std::ios_base::out);

		boost::property_tree::xml_parser::write_xml(ofs, m_propTree);
		m_bModified = false;
	}
}

void TConfig::SerializeLoad(TReadBinarySerializer& rSerializer)
{
	using namespace Serializers;

	boost::property_tree::wiptree propTree;

	SerializeLoadNode(rSerializer, propTree);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_propTree = propTree;
}

void TConfig::SerializeLoadNode(TReadBinarySerializer& rSerializer, boost::property_tree::wiptree& treeNode)
{
	using namespace Serializers;

	size_t stCount = 0;
	Serialize(rSerializer, stCount);

	while(stCount--)
	{
		// name of the node
		TString strNodeName;
		Serialize(rSerializer, strNodeName);

		bool bValue = false;
		Serialize(rSerializer, bValue);

		if(bValue)
		{
			// value
			TString strNodeValue;
			Serialize(rSerializer, strNodeValue);

			treeNode.add((PCTSTR)strNodeName, (PCTSTR)strNodeValue);
		}
		else
		{
			boost::property_tree::wiptree& rSubnodeTree = treeNode.add_child((PCTSTR)strNodeName, boost::property_tree::wiptree());
			SerializeLoadNode(rSerializer, rSubnodeTree);
		}
	}
}

void TConfig::SerializeStore(TWriteBinarySerializer& rSerializer)
{
	using namespace Serializers;

	boost::property_tree::wiptree propTree;

	// make a copy of internal tree (with locking) to avoid locking within serialization operation
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		propTree = m_propTree;
	}

	// serialize the copy
	SerializeStoreNode(rSerializer, propTree);
}

void TConfig::SerializeStoreNode(TWriteBinarySerializer& rSerializer, boost::property_tree::wiptree& treeNode)
{
	using namespace Serializers;
	Serialize(rSerializer, treeNode.size());

	for(boost::property_tree::wiptree::iterator iterNode = treeNode.begin(); iterNode != treeNode.end(); ++iterNode)
	{
		// is this node the leaf one?
		boost::property_tree::wiptree& rSubNode = (*iterNode).second;

		// name of the node
		Serialize(rSerializer, (*iterNode).first.c_str());

		bool bValue = rSubNode.empty();
		Serialize(rSerializer, bValue);

		if(bValue)
		{
			// value
			Serialize(rSerializer, rSubNode.data().c_str());
		}
		else
		{
			// store children
			SerializeStoreNode(rSerializer, rSubNode);
		}
	}
}

void TConfig::ReadFromString(const TString& strInput)
{
	if(strInput.IsEmpty())
		THROW(_T("Invalid argument"), 0, 0, 0);

	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	ClearNL();		// also clears m_bModified

	std::wistringstream ifs((const wchar_t*)strInput, std::ios_base::in);
	try
	{
		boost::property_tree::xml_parser::read_xml(ifs, m_propTree);
	}
	catch(...)
	{
		m_propTree.clear();
		throw;
	}
}

void TConfig::WriteToString(TString& strOutput)
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	std::wostringstream ofs(std::ios_base::out);

	boost::property_tree::xml_parser::write_xml(ofs, m_propTree);

	strOutput = ofs.str().c_str();
}

void TConfig::SetFilePath(PCTSTR pszPath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(m_strFilePath != pszPath)
	{
		m_strFilePath = pszPath;
		m_bModified = true;			// since the path is modified, we can only assume that stuff needs to be written into it regardless of the current modification state
	}
}

bool TConfig::IsModified() const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_bModified;
}

void TConfig::MarkAsModified()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bModified = true;
}

void TConfig::MarkAsNotModified()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bModified = false;
}

void TConfig::Clear()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	ClearNL();
}

void TConfig::ClearNL()
{
	m_propTree.clear();
	m_bModified = false;
	m_setDelayedNotifications.Clear();
	m_bDelayedEnabled = false;
	m_strFilePath.Clear();
}

// value setting/retrieval
bool TConfig::GetBool(PCTSTR pszPropName, bool bDefault) const
{
	bool bResult = bDefault;
	if(!InternalGetValue<bool>(m_propTree, pszPropName, bResult, m_lock))
		bResult = bDefault;
	return bResult;
}

bool TConfig::GetValue(PCTSTR pszPropName, bool& bValue) const
{
	return InternalGetValue<bool>(m_propTree, pszPropName, bValue, m_lock);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, bool bValue)
{
	if(InternalSetValue(m_propTree, m_bModified, m_lock, pszPropName, bValue))
		SendNotification(pszPropName);

	return *this;
}

int TConfig::GetInt(PCTSTR pszPropName, int iDefault) const
{
	int iResult = 0;
	if(!InternalGetValue<int>(m_propTree, pszPropName, iResult, m_lock))
		iResult = iDefault;
	return iResult;
}

bool TConfig::GetValue(PCTSTR pszPropName, int& iValue) const
{
	return InternalGetValue<int>(m_propTree, pszPropName, iValue, m_lock);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, int iValue)
{
	if(InternalSetValue(m_propTree, m_bModified, m_lock, pszPropName, iValue))
		SendNotification(pszPropName);

	return *this;
}

unsigned int TConfig::GetUInt(PCTSTR pszPropName, unsigned int uiDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<unsigned int>(pszPropName, uiDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, unsigned int& uiValue) const
{
	return InternalGetValue<unsigned int>(m_propTree, pszPropName, uiValue, m_lock);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, unsigned int uiValue)
{
	if(InternalSetValue(m_propTree, m_bModified, m_lock, pszPropName, uiValue))
		SendNotification(pszPropName);

	return *this;
}

long long TConfig::GetLongLong(PCTSTR pszPropName, long long llDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<long long>(pszPropName, llDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, long long& llValue) const
{
	return InternalGetValue<long long>(m_propTree, pszPropName, llValue, m_lock);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, long long llValue)
{
	if(InternalSetValue(m_propTree, m_bModified, m_lock, pszPropName, llValue))
		SendNotification(pszPropName);

	return *this;
}

unsigned long long TConfig::GetULongLong(PCTSTR pszPropName, unsigned long long ullDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<unsigned long long>(pszPropName, ullDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, unsigned long long& ullValue) const
{
	return InternalGetValue<unsigned long long>(m_propTree, pszPropName, ullValue, m_lock);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, unsigned long long ullValue)
{
	if(InternalSetValue(m_propTree, m_bModified, m_lock, pszPropName, ullValue))
		SendNotification(pszPropName);

	return *this;
}

double TConfig::GetDouble(PCTSTR pszPropName, double dDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<double>(pszPropName, dDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, double& dValue) const
{
	return InternalGetValue<double>(m_propTree, pszPropName, dValue, m_lock);
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, double dValue)
{
	if(InternalSetValue(m_propTree, m_bModified, m_lock, pszPropName, dValue))
		SendNotification(pszPropName);

	return *this;
}

TString TConfig::GetString(PCTSTR pszPropName, const TString& strDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	std::wstring wstrData = m_propTree.get<std::wstring>(pszPropName, std::wstring(strDefault));
	return wstrData.c_str();
}

bool TConfig::GetValue(PCTSTR pszPropName, TString& rstrValue) const
{
	std::wstring wstrData;
	bool bResult = InternalGetValue<std::wstring>(m_propTree, pszPropName, wstrData, m_lock);
	rstrValue = wstrData.c_str();

	return bResult;
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, const TString& strValue)
{
	std::wstring wstrData = strValue;
	if(InternalSetValue(m_propTree, m_bModified, m_lock, pszPropName, wstrData))
		SendNotification(pszPropName);

	return *this;
}

bool TConfig::GetValue(PCTSTR pszPropName, TStringArray& rvValues) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	rvValues.Clear();

	// get rid of the last part of the property to get to the parent holding the real entries
	std::wstring wstrPropertyName = pszPropName;
	boost::iterator_range<std::wstring::iterator> iterFnd = boost::find_last(wstrPropertyName, _T("."));
	if(iterFnd.begin() == wstrPropertyName.end())
		return false;

	std::wstring wstrNewPropName;
	wstrNewPropName.insert(wstrNewPropName.end(), wstrPropertyName.begin(), iterFnd.begin());

	boost::optional<const boost::property_tree::wiptree&> children = m_propTree.get_child_optional(wstrNewPropName);
	if(children.is_initialized())
	{
		BOOST_FOREACH(const boost::property_tree::wiptree::value_type& rEntry, children.get())
		{
			rvValues.Add(rEntry.second.data().c_str());
		}

		return true;
	}
	else
		return false;
}

void TConfig::SetValue(PCTSTR pszPropName, const TStringArray& rvValues)
{
	// get rid of the last part of the property to get to the parent holding the real entries
	std::wstring wstrNewPropName;
	std::wstring wstrPropertyName = pszPropName;
	boost::iterator_range<std::wstring::iterator> iterFnd = boost::find_last(wstrPropertyName, _T("."));
	if(iterFnd.begin() != wstrPropertyName.end())
		wstrNewPropName.insert(wstrNewPropName.end(), wstrPropertyName.begin(), iterFnd.begin());
	else
		wstrNewPropName = pszPropName;

	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);

		boost::optional<boost::property_tree::wiptree&> children = m_propTree.get_child_optional(wstrNewPropName);
		if(children.is_initialized())
			children.get().clear();
		for(size_t stIndex = 0; stIndex < rvValues.GetCount(); ++stIndex)
		{
			m_propTree.add(pszPropName, (const wchar_t*)rvValues.GetAt(stIndex));
		}

		m_bModified = true;
	}

	SendNotification(pszPropName);
}

// extraction of subtrees
bool TConfig::ExtractSubConfig(PCTSTR pszSubTreeName, TConfig& rSubConfig) const
{
	boost::unique_lock<boost::shared_mutex> dst_lock(rSubConfig.m_lock);
	rSubConfig.ClearNL();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	boost::optional<const boost::property_tree::wiptree&> optChildren = m_propTree.get_child_optional(pszSubTreeName);
	if(optChildren.is_initialized())
	{
		rSubConfig.m_propTree = optChildren.get();
		return true;
	}
	else
		return false;
}

bool TConfig::ExtractMultiSubConfigs(PCTSTR pszSubTreeName, TConfigArray& rSubConfigs) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	std::wstring wstrPropertyName = pszSubTreeName;
	boost::iterator_range<std::wstring::iterator> iterFnd = boost::find_last(wstrPropertyName, _T("."));
	if(iterFnd.begin() == wstrPropertyName.end())
		return false;

	std::wstring wstrNewPropName;
	wstrNewPropName.insert(wstrNewPropName.end(), wstrPropertyName.begin(), iterFnd.begin());

	boost::optional<const boost::property_tree::wiptree&> optChildren = m_propTree.get_child_optional(wstrNewPropName);
	if(optChildren.is_initialized())
	{
		BOOST_FOREACH(const boost::property_tree::wiptree::value_type& rEntry, optChildren.get())
		{
			std::wstring strData = rEntry.first.c_str();

			TConfig cfg;
			cfg.m_propTree = rEntry.second;
			rSubConfigs.Add(cfg);
		}

		return true;
	}
	else
		return false;
}

void TConfig::PutSubConfig(PCTSTR pszSubTreeName, const TConfig& rSubConfig)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	boost::shared_lock<boost::shared_mutex> src_lock(rSubConfig.m_lock);

	m_propTree.put_child(pszSubTreeName, rSubConfig.m_propTree);

	m_bModified = true;
}

void TConfig::AddSubConfig(PCTSTR pszSubTreeName, const TConfig& rSubConfig)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	boost::shared_lock<boost::shared_mutex> src_lock(rSubConfig.m_lock);

	m_propTree.add_child(pszSubTreeName, rSubConfig.m_propTree);

	m_bModified = true;
}

void TConfig::DeleteNode(PCTSTR pszNodeName)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_propTree.erase(pszNodeName);
}

void TConfig::ConnectToNotifier(void (*pfnCallback)(const TStringSet&, void*), void* pParam)
{
	m_notifier.connect(TConfigNotifier(pfnCallback, pParam));
}

void TConfig::DisconnectFromNotifier(void (*pfnCallback)(const TStringSet&, void*))
{
	m_notifier.disconnect(TConfigNotifier(pfnCallback, NULL));
}

void TConfig::DelayNotifications()
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	m_bDelayedEnabled = true;
}

void TConfig::ResumeNotifications()
{
	TStringSet setNotifications;

	// separate scope for shared mutex (to avoid calling notifier inside critical section)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
		if(m_bDelayedEnabled)
		{
			m_bDelayedEnabled = false;
			if(!m_setDelayedNotifications.IsEmpty())
			{
				setNotifications = m_setDelayedNotifications;

				boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
				m_setDelayedNotifications.Clear();
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
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
		if(m_bDelayedEnabled)
		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);

			m_setDelayedNotifications.Insert(rsetInfo);
			return;
		}
	}

	// NOTE: we don't lock here
	m_notifier(rsetInfo);
}

void TConfig::SendNotification(PCTSTR pszInfo)
{
	// separate scope for shared mutex (to avoid calling notifier inside critical section)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
		if(m_bDelayedEnabled)
		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);

			m_setDelayedNotifications.Insert(pszInfo);
			return;
		}
	}

	// NOTE: we don't lock here
	TStringSet setData;
	setData.Insert(pszInfo);
	m_notifier(setData);
}

END_CHCORE_NAMESPACE
