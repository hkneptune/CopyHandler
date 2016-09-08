// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TShellExtMenuConfig.cpp
/// @date  2011/05/06
/// @brief Contains implementation of shell menu configuration classes.
// ============================================================================
#include "stdafx.h"
#include "TShellExtMenuConfig.h"
#include "../libchcore/TConfig.h"
#include <boost/foreach.hpp>
#include <boost/smart_ptr/make_shared.hpp>
#include "../libchcore/TConfigArray.h"

// helper method for concatenating strings
PCTSTR Concat(std::wstring& wstrBuffer, PCTSTR pszFirst, PCTSTR pszSecond)
{
	if(pszFirst && pszFirst[0] != _T('\0'))
	{
		wstrBuffer = pszFirst;
		wstrBuffer += _T(".");
		wstrBuffer += pszSecond;
	}
	else
		wstrBuffer = pszSecond;

	return wstrBuffer.c_str();
}

///////////////////////////////////////////////////////////////////////////////////////////
// class TOperationTypeInfo

TOperationTypeInfo::TOperationTypeInfo() :
	m_eOperationTypeSource(eOpType_Autodetect),
	m_eDefaultOperationType(chcore::eOperation_None)
{
}

TOperationTypeInfo::TOperationTypeInfo(TOperationTypeInfo::EOperationTypeSource eType, chcore::EOperationType eDefaultOperationType) :
	m_eOperationTypeSource(eType),
	m_eDefaultOperationType(eDefaultOperationType)
{
}

void TOperationTypeInfo::SetOperationTypeInfo(TOperationTypeInfo::EOperationTypeSource eType, chcore::EOperationType eDefaultOperationType)
{
	m_eOperationTypeSource = eType;
	m_eDefaultOperationType = eDefaultOperationType;
}

TOperationTypeInfo::EOperationTypeSource TOperationTypeInfo::GetOperationTypeSource() const
{
	return m_eOperationTypeSource;
}

chcore::EOperationType TOperationTypeInfo::GetDefaultOperationType() const
{
	return m_eDefaultOperationType;
}

void TOperationTypeInfo::Clear()
{
	m_eOperationTypeSource = eOpType_Autodetect;
	m_eDefaultOperationType = chcore::eOperation_None;
}

void TOperationTypeInfo::StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const
{
	std::wstring wstrBuffer;
	SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("OperationTypeSource")), m_eOperationTypeSource);
	SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("DefaultOperationType")), m_eDefaultOperationType);
}

bool TOperationTypeInfo::ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName)
{
	std::wstring wstrBuffer;
	if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("OperationTypeSource")), *(int*)&m_eOperationTypeSource))
		return false;
	return GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("DefaultOperationType")), *(int*)&m_eDefaultOperationType);
}

///////////////////////////////////////////////////////////////////////////////////////////
// class TSourcePathsInfo

TSourcePathsInfo::TSourcePathsInfo() :
	m_eSrcPathsSource(eSrcType_InitializeAuto)
{
}

TSourcePathsInfo::TSourcePathsInfo(TSourcePathsInfo::ESrcPathsSource eSrcPathSource) :
	m_eSrcPathsSource(eSrcPathSource)
{
}

void TSourcePathsInfo::SetSourcePathsInfo(TSourcePathsInfo::ESrcPathsSource eSrcPathSource)
{
	m_eSrcPathsSource = eSrcPathSource;
}

TSourcePathsInfo::ESrcPathsSource TSourcePathsInfo::GetSrcPathsSource() const
{
	return m_eSrcPathsSource;
}

void TSourcePathsInfo::Clear()
{
	m_eSrcPathsSource = eSrcType_InitializeAuto;
}

void TSourcePathsInfo::StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const
{
	std::wstring wstrBuffer;
	SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("SrcPathsSource")), m_eSrcPathsSource);
}

bool TSourcePathsInfo::ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName)
{
	std::wstring wstrBuffer;
	return GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("SrcPathsSource")), *(int*)&m_eSrcPathsSource);
}

///////////////////////////////////////////////////////////////////////////////////////////
// class TDestinationPathInfo
// 

TDestinationPathInfo::TDestinationPathInfo() :
	m_eDstPathSource(eDstType_InitializeAuto),
	m_pathDestination()
{
}

TDestinationPathInfo::TDestinationPathInfo(TDestinationPathInfo::EDstPathsSource eDstPathSource, const chcore::TSmartPath& pathDestination) :
	m_eDstPathSource(eDstPathSource),
	m_pathDestination(pathDestination)
{
}

void TDestinationPathInfo::SetDestinationPathInfo(EDstPathsSource eDstPathSource, const chcore::TSmartPath& pathDestination)
{
	m_eDstPathSource = eDstPathSource;
	m_pathDestination = pathDestination;
}

TDestinationPathInfo::EDstPathsSource TDestinationPathInfo::GetDstPathSource() const
{
	return m_eDstPathSource;
}

chcore::TSmartPath TDestinationPathInfo::GetDefaultDestinationPath() const
{
	return m_pathDestination;
}

void TDestinationPathInfo::Clear()
{
	m_eDstPathSource = eDstType_InitializeAuto;
	m_pathDestination.Clear();
}

void TDestinationPathInfo::StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const
{
	std::wstring wstrBuffer;
	SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("DstPathSource")), m_eDstPathSource);
	SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("DefaultDestinationPath")), m_pathDestination);
}

bool TDestinationPathInfo::ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName)
{
	std::wstring wstrBuffer;
	if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("DstPathSource")), *(int*)&m_eDstPathSource))
		return false;
	return GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("DefaultDestinationPath")), m_pathDestination);
}

///////////////////////////////////////////////////////////////////////////////////////////
// class TShellMenuItem

TShellMenuItem::TShellMenuItem() :
	m_tOperationType(),
	m_tSourcePaths(),
	m_tDestinationPath(),
	m_eItemType(eSeparatorItem),
	m_bSpecialOperation(false),
	m_eDefaultItemHint(chcore::eOperation_None)
{
}

TShellMenuItem::TShellMenuItem(const chcore::TString& wstrName, const chcore::TString& wstrItemTip, const TOperationTypeInfo& rOperationType, const TSourcePathsInfo& rSourcePaths, const TDestinationPathInfo& rDestinationPath, bool bSpecialOperation, chcore::EOperationType eDefaultItemHint) :
	m_strName(wstrName),
	m_strItemTip(wstrItemTip),
	m_tOperationType(rOperationType),
	m_tSourcePaths(rSourcePaths),
	m_tDestinationPath(rDestinationPath),
	m_eItemType(eStandardItem),
	m_bSpecialOperation(bSpecialOperation),
	m_eDefaultItemHint(eDefaultItemHint)
{
}

TShellMenuItem::TShellMenuItem(const chcore::TString& wstrName, const chcore::TString& wstrItemTip) :
	m_strName(wstrName),
	m_strItemTip(wstrItemTip),
	m_tOperationType(),
	m_tSourcePaths(),
	m_tDestinationPath(),
	m_eItemType(eGroupItem),
	m_bSpecialOperation(false),
	m_eDefaultItemHint(chcore::eOperation_None)
{
}

TShellMenuItem::~TShellMenuItem()
{
}

void TShellMenuItem::Clear()
{
	m_eItemType = eSeparatorItem;

	m_strName.Clear();
	m_strItemTip.Clear();

	m_tOperationType.Clear();
	m_tSourcePaths.Clear();
	m_tDestinationPath.Clear();

	m_bSpecialOperation = false;
	m_eDefaultItemHint = chcore::eOperation_None;

	m_vChildItems.clear();
}

void TShellMenuItem::InitSeparatorItem()
{
	Clear();

	m_eItemType = eSeparatorItem;
}

void TShellMenuItem::InitStandardItem(const chcore::TString& wstrName, const chcore::TString& wstrItemTip, const TOperationTypeInfo& rOperationType, const TSourcePathsInfo& rSourcePaths, const TDestinationPathInfo& rDestinationPath, bool bSpecialOperation, chcore::EOperationType eDefaultItemHint)
{
	Clear();

	m_eItemType = eStandardItem;

	m_strName = wstrName;
	m_strItemTip = wstrItemTip;
	m_tOperationType = rOperationType;
	m_tSourcePaths = rSourcePaths;
	m_tDestinationPath = rDestinationPath;

	m_bSpecialOperation = bSpecialOperation;
	m_eDefaultItemHint = eDefaultItemHint;
}

void TShellMenuItem::InitGroupItem(const chcore::TString& wstrName, const chcore::TString& wstrItemTip)
{
	Clear();

	m_eItemType = eGroupItem;
	m_strName = wstrName;
	m_strItemTip = wstrItemTip;
}

bool TShellMenuItem::SpecifiesDestinationPath() const
{
	return !IsGroupItem() && (m_tDestinationPath.GetDstPathSource() == TDestinationPathInfo::eDstType_Specified);
}

bool TShellMenuItem::RequiresClipboardPaths() const
{
	return !IsGroupItem() && (m_tDestinationPath.GetDstPathSource() == TDestinationPathInfo::eDstType_Clipboard || m_tSourcePaths.GetSrcPathsSource() == TSourcePathsInfo::eSrcType_Clipboard);
}

size_t TShellMenuItem::GetChildrenCount() const
{
	return m_vChildItems.size();
}

TShellMenuItemPtr TShellMenuItem::GetChildAt(size_t stIndex) const
{
	return m_vChildItems[stIndex];
}

void TShellMenuItem::AddChild(const TShellMenuItemPtr& rItem)
{
	m_vChildItems.push_back(rItem);
}

void TShellMenuItem::SetChildAt(size_t stIndex, const TShellMenuItemPtr& rItem)
{
	m_vChildItems[stIndex] = rItem;
}

void TShellMenuItem::InsertChildAt(size_t stIndex, const TShellMenuItemPtr& rItem)
{
	m_vChildItems.insert(m_vChildItems.begin() + stIndex, rItem);
}

void TShellMenuItem::RemoveChildAt(size_t stIndex)
{
	m_vChildItems.erase(m_vChildItems.begin() + stIndex);
}

void TShellMenuItem::RemoveAllChildren()
{
	m_vChildItems.clear();
}

void TShellMenuItem::StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const
{
	std::wstring wstrBuffer;

	SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemType")), m_eItemType);
	switch(m_eItemType)
	{
	case eSeparatorItem:
		break;
	case eGroupItem:
		{
			SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemName")), m_strName);
			SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemDescription")), m_strItemTip);

			BOOST_FOREACH(const TShellMenuItemPtr& rItem, m_vChildItems)
			{
				chcore::TConfig cfgItem;
				rItem->StoreInConfig(cfgItem, _T(""));
				rConfig.AddSubConfig(Concat(wstrBuffer, pszNodeName, _T("Subitems.Subitem")), cfgItem);
			}

			break;
		}
	case eStandardItem:
		{
			SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemName")), m_strName);
			SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemDescription")), m_strItemTip);
			SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("SpecialOperation")), m_bSpecialOperation);
			SetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("DefaultItemHint")), m_eDefaultItemHint);

			m_tOperationType.StoreInConfig(rConfig, Concat(wstrBuffer, pszNodeName, _T("OperationType")));
			m_tSourcePaths.StoreInConfig(rConfig, Concat(wstrBuffer, pszNodeName, _T("SourcePaths")));
			m_tDestinationPath.StoreInConfig(rConfig, Concat(wstrBuffer, pszNodeName, _T("DestinationPath")));

			break;
		}
	default:
		BOOST_ASSERT(false);	// unhandled case
	}
}

bool TShellMenuItem::ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName)
{
	Clear();

	std::wstring wstrBuffer;
	if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemType")), *(int*)&m_eItemType))
		return false;
	switch(m_eItemType)
	{
	case eSeparatorItem:
		break;
	case eGroupItem:
		{
			if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemName")), m_strName))
				return false;
			if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemDescription")), m_strItemTip))
				return false;

			chcore::TConfigArray vCfgs;
			if(rConfig.ExtractMultiSubConfigs(Concat(wstrBuffer, pszNodeName, _T("Subitems.Subitem")), vCfgs))
			{
				for(size_t stIndex = 0; stIndex < vCfgs.GetCount(); ++stIndex)
				{
					chcore::TConfig& rCfg = vCfgs.GetAt(stIndex);

					TShellMenuItemPtr spItem(std::make_shared<TShellMenuItem>());
					spItem->ReadFromConfig(rCfg, nullptr);
					m_vChildItems.push_back(spItem);
				}
			}

			break;
		}
	case eStandardItem:
		{
			if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemName")), m_strName))
				return false;
			if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("ItemDescription")), m_strItemTip))
				return false;

			if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("SpecialOperation")), m_bSpecialOperation))
				return false;
			if(!GetConfigValue(rConfig, Concat(wstrBuffer, pszNodeName, _T("DefaultItemHint")), *(int*)&m_eDefaultItemHint))
				return false;

			if(!m_tOperationType.ReadFromConfig(rConfig, Concat(wstrBuffer, pszNodeName, _T("OperationType"))))
				return false;

			if(!m_tSourcePaths.ReadFromConfig(rConfig, Concat(wstrBuffer, pszNodeName, _T("SourcePaths"))))
				return false;

			if(!m_tDestinationPath.ReadFromConfig(rConfig, Concat(wstrBuffer, pszNodeName, _T("DestinationPath"))))
				return false;

			break;
		}
	default:
		BOOST_ASSERT(false);	// unhandled case
		return false;
	}

	return true;
}

TShellExtMenuConfig::TShellExtMenuConfig() :
	m_bInterceptDragAndDrop(false),
	m_bInterceptKeyboardActions(false),
	m_bInterceptCtxMenuActions(false),
	m_bShowShortcutIcons(false),
	m_spCommandsRoot(std::make_shared<TShellMenuItem>(_T(""), _T("")))
{
}

TShellExtMenuConfig::~TShellExtMenuConfig()
{
}

void TShellExtMenuConfig::Clear()
{
	m_spCommandsRoot->Clear();

	m_bInterceptDragAndDrop = false;
	m_bInterceptKeyboardActions = false;
	m_bInterceptCtxMenuActions = false;
	m_bShowShortcutIcons = false;
}

// commands support
TShellMenuItemPtr TShellExtMenuConfig::GetCommandRoot()
{
	return m_spCommandsRoot;
}

void TShellExtMenuConfig::StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const
{
	std::wstring strBuffer;
	SetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("InterceptDragAndDrop")), m_bInterceptDragAndDrop);
	SetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("InterceptKeyboardActions")), m_bInterceptKeyboardActions);
	SetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("InterceptCtxMenuActions")), m_bInterceptCtxMenuActions);
	SetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("ShowShortcutIcons")), m_bShowShortcutIcons);

	m_spCommandsRoot->StoreInConfig(rConfig, Concat(strBuffer, pszNodeName, _T("RootItem")));
}

bool TShellExtMenuConfig::ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName)
{
	Clear();

	std::wstring strBuffer;
	if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("InterceptDragAndDrop")), m_bInterceptDragAndDrop))
		return false;
	if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("InterceptKeyboardActions")), m_bInterceptKeyboardActions))
		return false;
	if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("InterceptCtxMenuActions")), m_bInterceptCtxMenuActions))
		return false;
	if(!GetConfigValue(rConfig, Concat(strBuffer, pszNodeName, _T("ShowShortcutIcons")), m_bShowShortcutIcons))
		return false;

	if(!m_spCommandsRoot->ReadFromConfig(rConfig, Concat(strBuffer, pszNodeName, _T("RootItem"))))
		return false;

	return true;
}
