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

#include "../libchengine/EOperationTypes.h"
#include "../libchcore/TPath.h"
#include <memory>
#include "../libchengine/TSizeFormatter.h"

class TShellMenuItem;

typedef std::shared_ptr<TShellMenuItem> TShellMenuItemPtr;

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
	TOperationTypeInfo(EOperationTypeSource eType, chengine::EOperationType eDefaultOperationType);

	void SetOperationTypeInfo(EOperationTypeSource eType, chengine::EOperationType eDefaultOperationType);

	EOperationTypeSource GetOperationTypeSource() const;
	chengine::EOperationType GetDefaultOperationType() const;

	void Clear();

	void StoreInConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName);

private:
	EOperationTypeSource m_eOperationTypeSource;
	chengine::EOperationType m_eDefaultOperationType;	// default operation type
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
	explicit TSourcePathsInfo(ESrcPathsSource eSrcPathSource);

	void SetSourcePathsInfo(ESrcPathsSource eSrcPathSource);
	ESrcPathsSource GetSrcPathsSource() const;

	void Clear();

	void StoreInConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName);

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

	void StoreInConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName);

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
	TShellMenuItem(const string::TString& wstrName, const string::TString& wstrItemTip, const TOperationTypeInfo& rOperationType, const TSourcePathsInfo& rSourcePaths, const TDestinationPathInfo& rDestinationPath,
		bool bSpecialOperation, chengine::EOperationType eDefaultItemHint = chengine::eOperation_None);
	TShellMenuItem(const string::TString& wstrName, const string::TString& wstrItemTip);

	~TShellMenuItem();

	// initializer for separator item
	void InitSeparatorItem();
	
	// initializer for standard item
	void InitStandardItem(const string::TString& wstrName, const string::TString& wstrItemTip, const TOperationTypeInfo& rOperationType, const TSourcePathsInfo& rSourcePaths, const TDestinationPathInfo& rDestinationPath,
		bool bSpecialOperation, chengine::EOperationType eDefaultItemHint = chengine::eOperation_None);

	// initializer for group item
	void InitGroupItem(const string::TString& wstrName, const string::TString& wstrItemTip);

	void CalculateWorkaroundSuffixes();
	
	// clears everything
	void Clear();

	// retrieving attributes - common ones
	const string::TString& GetName() const;
	const string::TString& GetItemTip() const { return m_strItemTip; }

	const string::TString& GetLocalName(bool bUseFallback = true) const;
	void SetLocalName(const string::TString& strLocalName);

	// retrieving attributes - standard items only
	const TOperationTypeInfo& GetOperationTypeInfo() const { return m_tOperationType; }
	const TDestinationPathInfo& GetDestinationPathInfo() const { return m_tDestinationPath; }
	const TSourcePathsInfo& GetSourcePathsInfo() const { return m_tSourcePaths; }

	chengine::EOperationType GetDefaultItemHint() const { return m_eDefaultItemHint; }

	bool IsGroupItem() const { return m_eItemType == eGroupItem; }
	bool IsSeparator() const { return m_eItemType == eSeparatorItem; }
	bool IsStandardItem() const { return m_eItemType == eStandardItem; }
	EItemType GetItemType() const { return m_eItemType; }

	bool IsSpecialOperation() const { return m_bSpecialOperation; }

	// helper - retrieves info if the destination path is already specified
	bool SpecifiesDestinationPath() const;
	// helper - retrieves info if this command requires some paths present in clipboard to be enabled
	bool RequiresClipboardPaths() const;

	std::wstring GetWorkaroundSuffix() const { return m_strWorkaroundSuffix; }
	void SetWorkaroundSuffix(std::wstring val) { m_strWorkaroundSuffix = val; }

	// operations on children
	size_t GetChildrenCount() const;
	TShellMenuItemPtr GetChildAt(size_t stIndex) const;

	void AddChild(const TShellMenuItemPtr& rItem);
	void SetChildAt(size_t stIndex, const TShellMenuItemPtr& rItem);
	void InsertChildAt(size_t stIndex, const TShellMenuItemPtr& rItem);

	void RemoveChildAt(size_t stIndex);
	void RemoveAllChildren();

	// serialization
	void StoreInConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName);

private:
	EItemType m_eItemType;

	string::TString m_strName;
	string::TString m_strLocalName;		// locally updated name; not serialized
	string::TString m_strItemTip;

	// where to get the operation type from? (specified here / autodetect (with fallback specified here))
	TOperationTypeInfo m_tOperationType;

	// where to get the destination path? (specified here / read from clipboard / retrieved in IShellExtInit::Initialize())
	TDestinationPathInfo m_tDestinationPath;

	// where to get source paths? (read from clipboard / retrieved in IShellExtInit::Initialize())
	TSourcePathsInfo m_tSourcePaths;

	// states if the operation allows for modifications before execution (special operation)
	bool m_bSpecialOperation;

	// hints that this item is to be made default (bold), when detected operation type is equal to this operation type
	chengine::EOperationType m_eDefaultItemHint;

	std::wstring m_strWorkaroundSuffix;

	std::vector<TShellMenuItemPtr> m_vChildItems;
};

class TShellExtMenuConfig
{
public:
	TShellExtMenuConfig();
	~TShellExtMenuConfig();

	void Clear();

	// commands support
	TShellMenuItemPtr GetDragAndDropRoot();
	TShellMenuItemPtr GetNormalRoot();

	// formatter
	chengine::TSizeFormatterPtr GetFormatter() const;

	// options
	void SetInterceptDragAndDrop(bool bEnable) { m_bInterceptDragAndDrop = bEnable; }
	bool GetInterceptDragAndDrop() const { return m_bInterceptDragAndDrop; }

	void SetInterceptKeyboardActions(bool bEnable) { m_bInterceptKeyboardActions = bEnable; }
	bool GetInterceptKeyboardActions() const { return m_bInterceptKeyboardActions; }

	void SetInterceptCtxMenuActions(bool bEnable) { m_bInterceptCtxMenuActions = bEnable; }
	bool GetInterceptCtxMenuActions() const { return m_bInterceptCtxMenuActions; }

	void SetShowShortcutIcons(bool bEnable) { m_bShowShortcutIcons = bEnable; }
	bool GetShowShortcutIcons() const { return m_bShowShortcutIcons; }

	void SetShowFreeSpace(bool bEnable) { m_bShowFreeSpace = bEnable; }
	bool GetShowFreeSpace() const { return m_bShowFreeSpace; }

	// serialize/unserialize
	void StoreInConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName) const;
	bool ReadFromConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName);

private:
	TShellMenuItemPtr m_spDragAndDropRoot;		// root under which there are commands placed
	TShellMenuItemPtr m_spNormalRoot;		// root under which there are commands placed

	chengine::TSizeFormatterPtr m_spFmtSize;

	bool m_bInterceptDragAndDrop = false;
	bool m_bInterceptKeyboardActions = false;
	bool m_bInterceptCtxMenuActions = false;
	bool m_bShowShortcutIcons = false;	// show shell icons with shortcuts ?
	bool m_bShowFreeSpace = false;
};

#endif
