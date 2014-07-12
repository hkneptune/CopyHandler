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
/// @file  TShellExtMenuConfig.h
/// @date  2011/05/06
/// @brief Contains class responsible for maintaining shell ext configuration (via shared mem).
// ============================================================================
#ifndef __TSHELLEXTMENUCONFIG_H__
#define __TSHELLEXTMENUCONFIG_H__

#include "../libchcore/EOperationTypes.h"
#include "../libchcore/TPath.h"

namespace chcore { class TConfig; }

class TShellMenuItem;

typedef boost::shared_ptr<TShellMenuItem> TShellMenuItemPtr;

// specifies information about operation type
class TOperationTypeInfo
{
public:
	enum EOperationTypeSource
	{
		eOpType_Specified,
		eOpType_Autodetect,
	};

public:
	TOperationTypeInfo();
	TOperationTypeInfo(EOperationTypeSource eType, chcore::EOperationType eDefaultOperationType);

	void SetOperationTypeInfo(EOperationTypeSource eType, chcore::EOperationType eDefaultOperationType);

	EOperationTypeSource GetOperationTypeSource() const;
	chcore::EOperationType GetDefaultOperationType() const;

	void Clear();

	void StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName);

private:
	EOperationTypeSource m_eOperationTypeSource;
	chcore::EOperationType m_eDefaultOperationType;	// default operation type
};

// specifies information about source paths
class TSourcePathsInfo
{
public:
	enum ESrcPathsSource
	{
		eSrcType_Clipboard,
		eSrcType_InitializePidlFolder,
		eSrcType_InitializeIDataObject,
		eSrcType_InitializeAuto,
	};

public:
	TSourcePathsInfo();
	TSourcePathsInfo(ESrcPathsSource eSrcPathSource);

	void SetSourcePathsInfo(ESrcPathsSource eSrcPathSource);
	ESrcPathsSource GetSrcPathsSource() const;

	void Clear();

	void StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName);

private:
	ESrcPathsSource m_eSrcPathsSource;
};

// specifies information about destination path
class TDestinationPathInfo
{
public:
	enum EDstPathsSource
	{
		eDstType_Specified,
		eDstType_Clipboard,
		eDstType_InitializePidlFolder,
		eDstType_InitializeIDataObject,
		eDstType_InitializeAuto,
		eDstType_Choose,
	};

public:
	TDestinationPathInfo();
	TDestinationPathInfo(EDstPathsSource eDstPathSource, const chcore::TSmartPath& pathDestination);

	void SetDestinationPathInfo(EDstPathsSource eDstPathSource, const chcore::TSmartPath& m_pathDestination);
	EDstPathsSource GetDstPathSource() const;
	chcore::TSmartPath GetDefaultDestinationPath() const;

	void Clear();

	void StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName);

private:
	EDstPathsSource m_eDstPathSource;
	chcore::TSmartPath m_pathDestination;
};

// class is not to be exported from DLL (because we're using wstrings here!)
class TShellMenuItem
{
public:
	enum EItemType
	{
		eStandardItem,
		eGroupItem,
		eSeparatorItem
	};

public:
	TShellMenuItem();
	TShellMenuItem(const chcore::TString& wstrName, const chcore::TString& wstrItemTip, const TOperationTypeInfo& rOperationType, const TSourcePathsInfo& rSourcePaths, const TDestinationPathInfo& rDestinationPath,
		bool bSpecialOperation, chcore::EOperationType eDefaultItemHint = chcore::eOperation_None);
	TShellMenuItem(const chcore::TString& wstrName, const chcore::TString& wstrItemTip);

	~TShellMenuItem();

	// initializer for separator item
	void InitSeparatorItem();
	
	// initializer for standard item
	void InitStandardItem(const chcore::TString& wstrName, const chcore::TString& wstrItemTip, const TOperationTypeInfo& rOperationType, const TSourcePathsInfo& rSourcePaths, const TDestinationPathInfo& rDestinationPath,
		bool bSpecialOperation, chcore::EOperationType eDefaultItemHint = chcore::eOperation_None);

	// initializer for group item
	void InitGroupItem(const chcore::TString& wstrName, const chcore::TString& wstrItemTip);

	// clears everything
	void Clear();

	// retrieving attributes - common ones
	const chcore::TString& GetName() const { return m_strName; }
	const chcore::TString& GetItemTip() const { return m_strItemTip; }

	// retrieving attributes - standard items only
	const TOperationTypeInfo& GetOperationTypeInfo() const { return m_tOperationType; }
	const TDestinationPathInfo& GetDestinationPathInfo() const { return m_tDestinationPath; }
	const TSourcePathsInfo& GetSourcePathsInfo() const { return m_tSourcePaths; }

	chcore::EOperationType GetDefaultItemHint() const { return m_eDefaultItemHint; }

	bool IsGroupItem() const { return m_eItemType == eGroupItem; }
	bool IsSeparator() const { return m_eItemType == eSeparatorItem; }
	bool IsStandardItem() const { return m_eItemType == eStandardItem; }
	EItemType GetItemType() const { return m_eItemType; }

	bool IsSpecialOperation() const { return m_bSpecialOperation; }

	// helper - retrieves info if the destination path is already specified
	bool SpecifiesDestinationPath() const { return !IsGroupItem() && !m_tDestinationPath.GetDstPathSource() == TDestinationPathInfo::eDstType_Specified; }
	// helper - retrieves info if this command requires some paths present in clipboard to be enabled
	bool RequiresClipboardPaths() const { return !IsGroupItem() && (m_tDestinationPath.GetDstPathSource() == TDestinationPathInfo::eDstType_Clipboard || m_tSourcePaths.GetSrcPathsSource() == TSourcePathsInfo::eSrcType_Clipboard); }

	// operations on children
	size_t GetChildrenCount() const;
	TShellMenuItemPtr GetChildAt(size_t stIndex) const;

	void AddChild(const TShellMenuItemPtr& rItem);
	void SetChildAt(size_t stIndex, const TShellMenuItemPtr& rItem);
	void InsertChildAt(size_t stIndex, const TShellMenuItemPtr& rItem);

	void RemoveChildAt(size_t stIndex);
	void RemoveAllChildren();

	// serialization
	void StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName);

private:
	EItemType m_eItemType;

	chcore::TString m_strName;
	chcore::TString m_strItemTip;

	// where to get the operation type from? (specified here / autodetect (with fallback specified here))
	TOperationTypeInfo m_tOperationType;

	// where to get the destination path? (specified here / read from clipboard / retrieved in IShellExtInit::Initialize())
	TDestinationPathInfo m_tDestinationPath;

	// where to get source paths? (read from clipboard / retrieved in IShellExtInit::Initialize())
	TSourcePathsInfo m_tSourcePaths;

	// states if the operation allows for modifications before execution (special operation)
	bool m_bSpecialOperation;

	// hints that this item is to be made default (bold), when detected operation type is equal to this operation type
	chcore::EOperationType m_eDefaultItemHint;

	std::vector<TShellMenuItemPtr> m_vChildItems;
};

class TShellExtMenuConfig
{
public:
	TShellExtMenuConfig();
	~TShellExtMenuConfig();

	void Clear();

	// commands support
	TShellMenuItemPtr GetCommandRoot();

	// options
	void SetInterceptDragAndDrop(bool bEnable) { m_bInterceptDragAndDrop = bEnable; }
	bool GetInterceptDragAndDrop() const { return m_bInterceptDragAndDrop; }

	void SetInterceptKeyboardActions(bool bEnable) { m_bInterceptKeyboardActions = bEnable; }
	bool GetInterceptKeyboardActions() const { return m_bInterceptKeyboardActions; }

	void SetInterceptCtxMenuActions(bool bEnable) { m_bInterceptCtxMenuActions = bEnable; }
	bool GetInterceptCtxMenuActions() const { return m_bInterceptCtxMenuActions; }

	void SetShowShortcutIcons(bool bEnable) { m_bShowShortcutIcons = bEnable; }
	bool GetShowShortcutIcons() const { return m_bShowShortcutIcons; }

	// serialize/unserialize
	void StoreInConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chcore::TConfig& rConfig, PCTSTR pszNodeName);

private:
	TShellMenuItemPtr m_spCommandsRoot;		// root under which there are commands placed

	bool m_bInterceptDragAndDrop;
	bool m_bInterceptKeyboardActions;
	bool m_bInterceptCtxMenuActions;
	bool m_bShowShortcutIcons;	// show shell icons with shortcuts ?
};

#endif
