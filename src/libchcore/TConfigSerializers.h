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
/// @file  TConfigSerializers.h
/// @date  2010/10/23
/// @brief Contains declarations/implementations of additional inline serializers.
// ============================================================================
#ifndef __TCONFIGSERIALIZERS_H__
#define __TCONFIGSERIALIZERS_H__

#include "TConfig.h"
#include "TStringArray.h"

BEGIN_CHCORE_NAMESPACE

#ifdef _MFC_VER

// CString config serializer

static void StoreInConfig(const CString& strValue, TConfig& rConfig, PCTSTR pszPropName)
{
	rConfig.SetValue(pszPropName, TString((PCTSTR)strValue));
}

static bool ReadFromConfig(CString& strValue, const TConfig& rConfig, PCTSTR pszPropName)
{
	TString wstrData;
	bool bRes = rConfig.GetValue(pszPropName, wstrData);
	if(bRes)
		strValue = wstrData.c_str();
	else
		strValue.Empty();
	return bRes;
}

// vector<CString> config serializer

static void StoreInConfig(const std::vector<CString>& vValues, TConfig& rConfig, PCTSTR pszPropName)
{
	// convert to vector of wstrings (ineffective; there should be a better way to do this)
	TStringArray vToStore;
	BOOST_FOREACH(const CString& strVal, vValues)
	{
		vToStore.Add((PCTSTR)strVal);
	}
	
	rConfig.SetValue(pszPropName, vToStore);
}

static bool ReadFromConfig(std::vector<CString>& vValues, const TConfig& rConfig, PCTSTR pszPropName)
{
	vValues.clear();

	TStringArray vToConvert;

	bool bRes = rConfig.GetValue(pszPropName, vToConvert);
	if(bRes)
	{
		for(size_t stIndex = 0; stIndex < vToConvert.GetCount(); ++stIndex)
		{
			vValues.push_back(vToConvert.GetAt(stIndex).c_str());
		}
	}

	return bRes;
}

#endif

END_CHCORE_NAMESPACE

CONFIG_STANDALONE_SERIALIZATION(CString)
CONFIG_STANDALONE_SERIALIZATION(std::vector<CString>)

#endif // __TCONFIGSERIALIZERS_H__
