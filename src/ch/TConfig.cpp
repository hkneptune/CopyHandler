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
#pragma warning(push)
#pragma warning(disable: 4702)
#include <boost/property_tree/xml_parser.hpp>
#pragma warning(pop)

///////////////////////////////////////////////////////////////////////////////////////////////
// class TConfigNotifier

TConfigNotifier::TConfigNotifier(void (*pfnCallback)(const std::set<CString>&, void*), void* pParam) :
	m_pfnCallback(pfnCallback),
	m_pParam(pParam)
{
}

TConfigNotifier::~TConfigNotifier()
{
}

void TConfigNotifier::operator()(const std::set<CString>& rsetPropNames)
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
void TConfig::Read(const CString& strFile)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	// Note: we need to store filename for later use BEFORE trying to open a file
	//       since it might be nonexistent, but we still would like to store config to this file later
	ClearNL();		// also clears m_bModified
	m_strFilePath = strFile;

	std::wifstream ifs(m_strFilePath, ios_base::in);
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
		std::wofstream ofs(m_strFilePath, ios_base::out);

		boost::property_tree::xml_parser::write_xml(ofs, m_propTree);
		m_bModified = false;
	}
}

void TConfig::SetFilePath(const CString& strPath)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	if(m_strFilePath != strPath)
	{
		m_strFilePath = strPath;
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
	m_strFilePath.Empty();
}

// value setting/retrieval
bool TConfig::GetBool(PCTSTR pszPropName, bool bDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<bool>(pszPropName, bDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, bool& bValue, bool bDefault) const
{
	boost::optional<bool> tValue = m_propTree.get_optional<bool>(pszPropName);
	if(tValue.is_initialized())
	{
		bValue = tValue.get();
		return true;
	}
	else
	{
		bValue = bDefault;
		return false;
	}
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, bool bValue)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.put<bool>(pszPropName, bValue);
		m_bModified = true;
	}

	SendNotification(pszPropName);
	return *this;
}

int TConfig::GetInt(PCTSTR pszPropName, int iDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<int>(pszPropName, iDefault);
}


bool TConfig::GetValue(PCTSTR pszPropName, int& iValue, int iDefault) const
{
	boost::optional<int> tValue = m_propTree.get_optional<int>(pszPropName);
	if(tValue.is_initialized())
	{
		iValue = tValue.get();
		return true;
	}
	else
	{
		iValue = iDefault;
		return false;
	}
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, int iValue)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.put<int>(pszPropName, iValue);
		m_bModified = true;
	}

	SendNotification(pszPropName);
	return *this;
}

unsigned int TConfig::GetUInt(PCTSTR pszPropName, unsigned int uiDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<unsigned int>(pszPropName, uiDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, unsigned int& uiValue, unsigned int uiDefault) const
{
	boost::optional<unsigned int> tValue = m_propTree.get_optional<unsigned int>(pszPropName);
	if(tValue.is_initialized())
	{
		uiValue = tValue.get();
		return true;
	}
	else
	{
		uiValue = uiDefault;
		return false;
	}
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, unsigned int uiValue)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.put<unsigned int>(pszPropName, uiValue);
		m_bModified = true;
	}

	SendNotification(pszPropName);
	return *this;
}

long long TConfig::GetLongLong(PCTSTR pszPropName, long long llDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<long long>(pszPropName, llDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, long long& llValue, long long llDefault) const
{
	boost::optional<long long> tValue = m_propTree.get_optional<long long>(pszPropName);
	if(tValue.is_initialized())
	{
		llValue = tValue.get();
		return true;
	}
	else
	{
		llValue = llDefault;
		return false;
	}
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, long long llValue)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.put<long long>(pszPropName, llValue);
		m_bModified = true;
	}

	SendNotification(pszPropName);
	return *this;
}

unsigned long long TConfig::GetULongLong(PCTSTR pszPropName, unsigned long long ullDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<unsigned long long>(pszPropName, ullDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, unsigned long long& ullValue, unsigned long long ullDefault) const
{
	boost::optional<unsigned long long> tValue = m_propTree.get_optional<unsigned long long>(pszPropName);
	if(tValue.is_initialized())
	{
		ullValue = tValue.get();
		return true;
	}
	else
	{
		ullValue = ullDefault;
		return false;
	}
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, unsigned long long ullValue)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.put<unsigned long long>(pszPropName, ullValue);
		m_bModified = true;
	}

	SendNotification(pszPropName);
	return *this;
}

double TConfig::GetDouble(PCTSTR pszPropName, double dDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	return m_propTree.get<double>(pszPropName, dDefault);
}

bool TConfig::GetValue(PCTSTR pszPropName, double& dValue, double dDefault) const
{
	boost::optional<double> tValue = m_propTree.get_optional<double>(pszPropName);
	if(tValue.is_initialized())
	{
		dValue = tValue.get();
		return true;
	}
	else
	{
		dValue = dDefault;
		return false;
	}
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, double dValue)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.put<double>(pszPropName, dValue);
		m_bModified = true;
	}

	SendNotification(pszPropName);
	return *this;
}

CString TConfig::GetString(PCTSTR pszPropName, const CString& strDefault) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	std::wstring wstrData = m_propTree.get<std::wstring>(pszPropName, std::wstring(strDefault));
	return wstrData.c_str();
}

bool TConfig::GetValue(PCTSTR pszPropName, CString& rstrValue, const CString& strDefault) const
{
	boost::optional<std::wstring> tValue = m_propTree.get_optional<std::wstring>(pszPropName);
	if(tValue.is_initialized())
	{
		rstrValue = tValue.get().c_str();
		return true;
	}
	else
	{
		rstrValue = strDefault;
		return false;
	}
}

TConfig& TConfig::SetValue(PCTSTR pszPropName, const CString& strValue)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.put(pszPropName, std::wstring(strValue));
		m_bModified = true;
	}

	SendNotification(pszPropName);
	return *this;
}

bool TConfig::GetValue(PCTSTR pszPropName, std::vector<CString>& rvValues) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);
	bool bResult = false;
	rvValues.clear();
	BOOST_FOREACH(const boost::property_tree::wiptree::value_type& rEntry, m_propTree.get_child(pszPropName))
	{
		rvValues.push_back(rEntry.second.data().c_str());
		bResult = true;
	}

	return bResult;
}

bool TConfig::GetValue(PCTSTR pszPropName, std::vector<CString>& rvValues, const std::vector<CString>& rvDefault) const
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
	{
		rvValues = rvDefault;
		return false;
	}
}

void TConfig::SetValue(PCTSTR pszPropName, const std::vector<CString>& rvValues)
{
	// separate scope for mutex (to avoid calling notifier inside critical section)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_propTree.erase(pszPropName);
		BOOST_FOREACH(const CString& strValue, rvValues)
		{
			m_propTree.add(pszPropName, (PCTSTR)strValue);
		}

		m_bModified = true;
	}

	SendNotification(pszPropName);
}

// extraction of subtrees
void TConfig::ExtractSubConfig(PCTSTR pszSubTreeName, TConfig& rSubConfig) const
{
	boost::unique_lock<boost::shared_mutex> dst_lock(rSubConfig.m_lock);
	rSubConfig.ClearNL();

	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	rSubConfig.m_propTree = m_propTree.get_child(pszSubTreeName);
}

void TConfig::PutSubConfig(PCTSTR pszSubTreeName, const TConfig& rSubConfig)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);
	boost::shared_lock<boost::shared_mutex> src_lock(rSubConfig.m_lock);

	m_propTree.put_child(pszSubTreeName, rSubConfig.m_propTree);

	m_bModified = true;
}

void TConfig::ConnectToNotifier(void (*pfnCallback)(const std::set<CString>&, void*), void* pParam)
{
	m_notifier.connect(TConfigNotifier(pfnCallback, pParam));
}

void TConfig::DisconnectFromNotifier(void (*pfnCallback)(const std::set<CString>&, void*))
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
	std::set<CString> setNotifications;

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

void TConfig::SendNotification(const std::set<CString>& rsetInfo)
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
	std::set<CString> setData;
	setData.insert(pszInfo);
	m_notifier(setData);
}
