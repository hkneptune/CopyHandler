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

#pragma warning(push)
#pragma warning(disable: 4702 4512)
#include <boost/property_tree/xml_parser.hpp>
#pragma warning(pop)

BEGIN_CHCORE_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////////
// class TConfigNotifier

TConfigNotifier::TConfigNotifier(void (*pfnCallback)(const std::set<std::wstring>&, void*), void* pParam) :
m_pfnCallback(pfnCallback),
m_pParam(pParam)
{
}

TConfigNotifier::~TConfigNotifier()
{
}

void TConfigNotifier::operator()(const std::set<std::wstring>& rsetPropNames)
{
	if(!m_pfnCallback)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	(*m_pfnCallback)(rsetPropNames, m_pParam);
}

TConfigNotifier& TConfigNotifier::operator=(const TConfigNotifier& rNotifier)
{
	if(this != &rNotifier)
	{
		m_pfnCallback = rNotifier.m_pfnCallback;
		m_pParam = rNotifier.m_pParam;
	}
	return *this;
}

bool TConfigNotifier::operator==(const TConfigNotifier& rNotifier) const
{
	return m_pfnCallback == rNotifier.m_pfnCallback/* && m_pParam == rNotifier.m_pParam*/;
}

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
		m_setDelayedNotifications.clear();
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

	std::wifstream ifs(m_strFilePath.c_str(), std::ios_base::in);
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
		std::wofstream ofs(m_strFilePath.c_str(), std::ios_base::out);

		boost::property_tree::xml_parser::write_xml(ofs, m_propTree);
		m_bModified = false;
	}
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
	m_setDelayedNotifications.clear();
	m_bDelayedEnabled = false;
	m_strFilePath.clear();
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

std::wstring TConfig::GetString(PCTSTR pszPropName, const std::wstring& strDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	std::wstring wstrData = m_propTree.get<std::wstring>(pszPropName, std::wstring(strDefault));
	return wstrData.c_str();
}

bool TConfig::GetValue(PCTSTR pszPropName, std::wstring& rstrValue) const
{
	std::wstring wstrData;
	bool bResult = InternalGetValue<std::wstring>(m_propTree, pszPropName, wstrData, m_lock);
	rstrValue = wstrData.c_str();

	return bResult;
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, const std::wstring& strValue)
{
	std::wstring wstrData = strValue;
	if(InternalSetValue(m_propTree, m_bModified, m_lock, pszPropName, wstrData))
		SendNotification(pszPropName);

	return *this;
}

bool TConfig::GetValue(PCTSTR pszPropName, std::vector<std::wstring>& rvValues) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	rvValues.clear();

	boost::optional<const boost::property_tree::wiptree&> children = m_propTree.get_child_optional(pszPropName);
	if(children.is_initialized())
	{
		BOOST_FOREACH(const boost::property_tree::wiptree::value_type& rEntry, children.get())
		{
			rvValues.push_back(rEntry.second.data().c_str());
		}

		return true;
	}
	else
		return false;
}

void TConfig::SetValue(PCTSTR pszPropName, const std::vector<std::wstring>& rvValues)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.erase(pszPropName);
		BOOST_FOREACH(const std::wstring& strValue, rvValues)
		{
			m_propTree.add(pszPropName, strValue);
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

bool TConfig::ExtractMultiSubConfigs(PCTSTR pszSubTreeName, std::vector<TConfig>& rSubConfigs) const
{
	TConfig cfg;

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	boost::optional<const boost::property_tree::wiptree&> optChildren = m_propTree.get_child_optional(pszSubTreeName);
	if(optChildren.is_initialized())
	{
		BOOST_FOREACH(const boost::property_tree::wiptree::value_type& rEntry, optChildren.get())
		{
			cfg.m_propTree = rEntry.second;
			rSubConfigs.push_back(cfg);
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

void TConfig::ConnectToNotifier(void (*pfnCallback)(const std::set<std::wstring>&, void*), void* pParam)
{
	m_notifier.connect(TConfigNotifier(pfnCallback, pParam));
}

void TConfig::DisconnectFromNotifier(void (*pfnCallback)(const std::set<std::wstring>&, void*))
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
	std::set<std::wstring> setNotifications;

	// separate scope for shared mutex (to avoid calling notifier inside critical section)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
		if(m_bDelayedEnabled)
		{
			m_bDelayedEnabled = false;
			if(!m_setDelayedNotifications.empty())
			{
				setNotifications = m_setDelayedNotifications;

				boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);
				m_setDelayedNotifications.clear();
			}
		}
	}

	// NOTE: no locking here!
	if(!setNotifications.empty())
		SendNotification(setNotifications);
}

void TConfig::SendNotification(const std::set<std::wstring>& rsetInfo)
{
	// separate scope for shared mutex (to avoid calling notifier inside critical section)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);
		if(m_bDelayedEnabled)
		{
			boost::upgrade_to_unique_lock<boost::shared_mutex> upgraded_lock(lock);

			m_setDelayedNotifications.insert(rsetInfo.begin(), rsetInfo.end());
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

			m_setDelayedNotifications.insert(pszInfo);
			return;
		}
	}

	// NOTE: we don't lock here
	std::set<std::wstring> setData;
	setData.insert(pszInfo);
	m_notifier(setData);
}

END_CHCORE_NAMESPACE
