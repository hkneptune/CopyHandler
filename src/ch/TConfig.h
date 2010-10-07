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
/// @file  TConfig.h
/// @date  2010/09/27
/// @brief Contains declaration of classes related to configuration handling.
// ============================================================================
#ifndef __TCONFIG_H__
#define __TCONFIG_H__

#include "../libchcore/TPath.h"

#pragma warning(push)
#pragma warning(disable: 4100 4702)
#include <boost/property_tree/ptree.hpp>
#include <boost/signals2.hpp>
#pragma warning(pop)

// class defines configuration change notification record; not to be used outside
class TConfigNotifier
{
public:
	TConfigNotifier(void (*pfnCallback)(const std::set<CString>&, void*), void* pParam);
	~TConfigNotifier();

	void operator()(const std::set<CString>& rsetPropNames);

	TConfigNotifier& operator=(const TConfigNotifier& rNotifier);

	bool operator==(const TConfigNotifier& rNotifier) const;

private:
	void (*m_pfnCallback)(const std::set<CString>&, void*);
	void* m_pParam;
};

// class for handling configuration settings
class TConfig
{
public:
	TConfig();
	TConfig(const TConfig& rSrc);
	~TConfig();

	TConfig& operator=(const TConfig& rSrc);

	void Clear();

	// read/write
	void Read(const CString& strFile);
	void Write(bool bOnlyIfModified = false);

	void SetFilePath(const CString& strPath);

	// Modifications management
	bool IsModified() const;
	void MarkAsModified();
	void MarkAsNotModified();

	// value setting/retrieval
	bool GetBool(PCTSTR pszPropName, bool bDefault = false) const;
	bool GetValue(PCTSTR pszPropName, bool& bValue) const;
	TConfig& SetValue(PCTSTR pszPropName, bool bValue);

	int GetInt(PCTSTR pszPropName, int iDefault = 0) const;
	bool GetValue(PCTSTR pszPropName, int& iValue) const;
	TConfig& SetValue(PCTSTR pszPropName, int iValue);

	unsigned int GetUInt(PCTSTR pszPropName, unsigned int uiDefault) const;
	bool GetValue(PCTSTR pszPropName, unsigned int& uiValue) const;
	TConfig& SetValue(PCTSTR pszPropName, unsigned int uiValue);

	long long GetLongLong(PCTSTR pszPropName, long long llDefault) const;
	bool GetValue(PCTSTR pszPropName, long long& llValue) const;
	TConfig& SetValue(PCTSTR pszPropName, long long llValue);

	unsigned long long GetULongLong(PCTSTR pszPropName, unsigned long long ullDefault) const;
	bool GetValue(PCTSTR pszPropName, unsigned long long& ullValue) const;
	TConfig& SetValue(PCTSTR pszPropName, unsigned long long ullValue);

	double GetDouble(PCTSTR pszPropName, double dDefault) const;
	bool GetValue(PCTSTR pszPropName, double& dValue) const;
	TConfig& SetValue(PCTSTR pszPropName, double dValue);

	CString GetString(PCTSTR pszPropName, const CString& strDefault) const;
	bool GetValue(PCTSTR pszPropName, CString& rstrValue) const;
	TConfig& SetValue(PCTSTR pszPropName, const CString& strValue);

	chcore::TSmartPath GetPath(PCTSTR pszPropName, const chcore::TSmartPath& pathDefault) const;
	bool GetValue(PCTSTR pszPropName, chcore::TSmartPath& rpathValue) const;
	TConfig& SetValue(PCTSTR pszPropName, const chcore::TSmartPath& strValue);

	bool GetValue(PCTSTR pszPropName, std::vector<CString>& rvValues) const;
	void SetValue(PCTSTR pszPropName, const std::vector<CString>& rvValues);

	// extraction of subtrees
	void ExtractSubConfig(PCTSTR pszSubTreeName, TConfig& rSubConfig) const;
	void PutSubConfig(PCTSTR pszSubTreeName, const TConfig& rSubConfig);

	// property change notification
	void ConnectToNotifier(void (*pfnCallback)(const std::set<CString>&, void*), void* pParam);
	void DisconnectFromNotifier(void (*pfnCallback)(const std::set<CString>&, void*));

	void DelayNotifications();
	void ResumeNotifications();

	// template access
	template<class PropInfo>
	typename PropInfo::value_type GetPropValue() const
	{
		typename PropInfo::value_type tValue;
		if(!GetValue(PropInfo::GetPropertyName(), tValue))
			tValue = PropInfo::GetDefaultValue();

		PropInfo::ValidateRange(tValue);
		return tValue;
	}

	template<class PropInfo>
	bool GetPropValue(typename PropInfo::value_type& rValue) const
	{
		bool bResult = GetValue(PropInfo::GetPropertyName(), rValue, PropInfo::GetDefaultValue());
		if(bResult)
			PropInfo::ValidateRange(rValue);
		return bResult;
	}

	template<class PropInfo>
	void SetPropValue(const typename PropInfo::value_type& rValue)
	{
		typename PropInfo::value_type tValue(rValue);
		PropInfo::ValidateRange(tValue);

		SetValue(PropInfo::GetPropertyName(), tValue);
	}


protected:
	void SendNotification(const std::set<CString>& rsetInfo);
	void SendNotification(PCTSTR pszInfo);

	void ClearNL();

private:
	boost::property_tree::wiptree m_propTree;
	CString m_strFilePath;

	boost::signals2::signal<void (const std::set<CString>&)> m_notifier;
	std::set<CString> m_setDelayedNotifications;
	bool m_bDelayedEnabled;

	bool m_bModified;		///< Modification state - cleared when saving

	mutable boost::shared_mutex m_lock;
};

template<class DataType>
struct PropDataBase
{
	typedef DataType value_type;

	static void ValidateRange(value_type&) {}	// does nothing
};

template<class DataType>
struct PropDataMinMaxBase
{
	typedef DataType value_type;
};

#endif
