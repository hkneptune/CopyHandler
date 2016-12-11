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

#include "../libserializer/ISerializerContainer.h"
#include "../libstring/TStringSet.h"
#include "../libchcore/TPathContainer.h"
#include "libchengine.h"

namespace chengine
{
	class TConfigArray;

	namespace details
	{
		struct ConfigNodeContainer;
	}

	// class for handling configuration settings
	class LIBCHENGINE_API TConfig
	{
	public:
		TConfig();
		TConfig(const TConfig& rSrc);
		~TConfig();

		TConfig& operator=(const TConfig& rSrc);

		void Clear();

		// read/write
		void Read(PCTSTR pszFile);
		void Write();
		void SetFilePath(PCTSTR pszPath);

		void ReadFromString(const string::TString& strInput);
		void WriteToString(string::TString& strOutput);

		void Store(const serializer::ISerializerContainerPtr& spContainer) const;
		void Load(const serializer::ISerializerContainerPtr& spContainer) const;
		void InitColumns(const serializer::ISerializerContainerPtr& spContainer) const;

		// value setting/retrieval
		bool GetBool(PCTSTR pszPropName, bool bDefault = false) const;
		bool GetValue(PCTSTR pszPropName, bool& bValue) const;
		TConfig& SetValue(PCTSTR pszPropName, bool bValue);

		int GetInt(PCTSTR pszPropName, int iDefault = 0) const;
		bool GetValue(PCTSTR pszPropName, int& iValue) const;
		TConfig& SetValue(PCTSTR pszPropName, int iValue);

		unsigned int GetUInt(PCTSTR pszPropName, unsigned int uiDefault = 0) const;
		bool GetValue(PCTSTR pszPropName, unsigned int& uiValue) const;
		TConfig& SetValue(PCTSTR pszPropName, unsigned int uiValue);

		long long GetLongLong(PCTSTR pszPropName, long long llDefault = 0) const;
		bool GetValue(PCTSTR pszPropName, long long& llValue) const;
		TConfig& SetValue(PCTSTR pszPropName, long long llValue);

		unsigned long long GetULongLong(PCTSTR pszPropName, unsigned long long ullDefault = 0) const;
		bool GetValue(PCTSTR pszPropName, unsigned long long& ullValue) const;
		TConfig& SetValue(PCTSTR pszPropName, unsigned long long ullValue);

		double GetDouble(PCTSTR pszPropName, double dDefault = 0.0) const;
		bool GetValue(PCTSTR pszPropName, double& dValue) const;
		TConfig& SetValue(PCTSTR pszPropName, double dValue);

		string::TString GetString(PCTSTR pszPropName, const string::TString& strDefault = string::TString()) const;
		bool GetValue(PCTSTR pszPropName, string::TString& rstrValue) const;
		TConfig& SetValue(PCTSTR pszPropName, const string::TString& strValue);
		TConfig& SetValue(PCTSTR pszPropName, PCTSTR pszValue);

		chcore::TSmartPath GetPath(PCTSTR pszPropName, const chcore::TSmartPath& pathDefault = chcore::TSmartPath()) const;
		bool GetValue(PCTSTR pszPropName, chcore::TSmartPath& rpathValue) const;
		TConfig& SetValue(PCTSTR pszPropName, const chcore::TSmartPath& pathValue);

		bool GetValue(PCTSTR pszPropName, string::TStringArray& rvValues) const;
		TConfig& SetValue(PCTSTR pszPropName, const string::TStringArray& rvValues);

		bool GetValue(PCTSTR pszPropName, chcore::TPathContainer& rvValues) const;
		TConfig& SetValue(PCTSTR pszPropName, const chcore::TPathContainer& rvValues);

		void DeleteNode(PCTSTR pszNodeName);

		// extraction of subtrees
		bool ExtractSubConfig(PCTSTR pszSubTreeName, TConfig& rSubConfig) const;
		bool ExtractMultiSubConfigs(PCTSTR pszSubTreeName, TConfigArray& rSubConfigs) const;
		void PutSubConfig(PCTSTR pszSubTreeName, const TConfig& rSubConfig);
		void AddSubConfig(PCTSTR pszSubTreeName, const TConfig& rSubConfig);

		// property change notification
		void ConnectToNotifier(void(*pfnCallback)(const string::TStringSet&, void*), void* pParam);
		void DisconnectFromNotifier(void(*pfnCallback)(const string::TStringSet&, void*));

		void DelayNotifications();
		void ResumeNotifications();

#ifdef _DEBUG
		void Dump();
#endif

	protected:
		void SendNotification(const string::TStringSet& rsetInfo);
		void SendNotification(PCTSTR pszInfo);

		void ClearNL();

	private:
		details::ConfigNodeContainer* GetImpl();
		const details::ConfigNodeContainer* GetImpl() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		details::ConfigNodeContainer* m_pImpl;
#pragma warning(pop)
	};

	template<class Type>
	void SetConfigValue(TConfig& rConfig, PCTSTR pszPropName, const Type& rValue)
	{
		rConfig.SetValue(pszPropName, rValue);
	}

	template<class Type>
	Type GetConfigValueDef(const TConfig& rConfig, PCTSTR pszPropName, const Type& rDefault)
	{
		Type tValue;
		if (!rConfig.GetValue(pszPropName, tValue))
			tValue = rDefault;
		return tValue;
	}

	template<class Type>
	bool GetConfigValue(const TConfig& rConfig, PCTSTR pszPropName, Type& rValue)
	{
		return rConfig.GetValue(pszPropName, rValue);
	}

#define CONFIG_MEMBER_SERIALIZATION(cls)\
	namespace chengine {\
	template<>\
	inline void SetConfigValue<cls>(TConfig& rConfig, PCTSTR pszPropName, const cls& rValue)\
	{\
		rValue.StoreInConfig(rConfig, pszPropName);\
	}\
\
	template<>\
	inline cls GetConfigValueDef<cls>(const TConfig& rConfig, PCTSTR pszPropName, const cls& rDefault)\
	{\
		cls tValue;\
		if(!tValue.ReadFromConfig(rConfig, pszPropName))\
			tValue = rDefault;\
		return tValue;\
	}\
\
	template<>\
	inline bool GetConfigValue<cls>(const TConfig& rConfig, PCTSTR pszPropName, cls& rValue)\
	{\
		return rValue.ReadFromConfig(rConfig, pszPropName);\
	}\
	}

#define CONFIG_STANDALONE_SERIALIZATION(cls)\
	namespace chengine {\
	template<>\
	inline void SetConfigValue<cls>(TConfig& rConfig, PCTSTR pszPropName, const cls& rValue)\
	{\
		StoreInConfig(rValue, rConfig, pszPropName);\
	}\
\
	template<>\
	inline cls GetConfigValueDef<cls>(const TConfig& rConfig, PCTSTR pszPropName, const cls& rDefault)\
	{\
		cls tValue;\
		if(!ReadFromConfig(tValue, rConfig, pszPropName))\
			tValue = rDefault;\
		return tValue;\
	}\
\
	template<>\
	inline bool GetConfigValue<cls>(const TConfig& rConfig, PCTSTR pszPropName, cls& rValue)\
	{\
		return ReadFromConfig(rValue, rConfig, pszPropName);\
	}\
	}
}

#endif
