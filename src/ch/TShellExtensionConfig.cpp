// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TShellExtensionConfig.h"
#include "ch.h"
#include "../common/TShellExtMenuConfig.h"
#include "CfgProperties.h"
#include "resource.h"
#include "shortcuts.h"

TShellExtensionConfig::TShellExtensionConfig(const logger::TLogFileDataPtr& spLogData) :
	m_spLog(logger::MakeLogger(spLogData, L"ShellExtConfig"))
{
}

void TShellExtensionConfig::PrepareConfig()
{
	try
	{
		chcore::TConfig& rConfig = GetConfig();

		TShellExtMenuConfig cfgShellExt;

		// experimental - doesn't work on all systems 
		cfgShellExt.SetShowShortcutIcons(GetPropValue<PP_SHSHOWSHELLICONS>(rConfig));

		cfgShellExt.SetInterceptDragAndDrop(GetPropValue<PP_SHINTERCEPTDRAGDROP>(rConfig));
		cfgShellExt.SetInterceptKeyboardActions(GetPropValue<PP_SHINTERCEPTKEYACTIONS>(rConfig));
		cfgShellExt.SetInterceptCtxMenuActions(GetPropValue<PP_SHINTERCEPTCTXMENUACTIONS>(rConfig));

		cfgShellExt.GetFormatter()->SetValues(
			GetResManager().LoadString(IDS_BYTE_STRING),
			GetResManager().LoadString(IDS_KBYTE_STRING),
			GetResManager().LoadString(IDS_MBYTE_STRING),
			GetResManager().LoadString(IDS_GBYTE_STRING),
			GetResManager().LoadString(IDS_TBYTE_STRING)
		);

		cfgShellExt.SetShowFreeSpace(GetPropValue<PP_SHSHOWFREESPACE>(rConfig));

		PrepareDragAndDropMenuItems(cfgShellExt);
		PrepareNormalMenuItems(cfgShellExt);

		chcore::TConfig cfgStorage;
		chcore::TString wstrData;

		cfgShellExt.StoreInConfig(cfgStorage, _T("ShellExtCfg"));
		cfgStorage.WriteToString(wstrData);

		m_shellExtProvider.SetConfigData(wstrData);
	}
	catch(const std::exception& e)
	{
		CString strMsg;
		strMsg.Format(L"Encountered problem trying to provide shell ext configuration.\nReason: %S", e.what());
		LOG_ERROR(m_spLog) << strMsg;
	}
}

void TShellExtensionConfig::PrepareDragAndDropMenuItems(TShellExtMenuConfig &cfgShellExt) const
{
	chcore::TConfig& rConfig = GetConfig();
	ictranslate::CResourceManager& rResManager = GetResManager();

	TShellMenuItemPtr spDragAndDropRootItem = cfgShellExt.GetDragAndDropRoot();
	bool bAddedAnyOption = false;
	if(GetPropValue<PP_SHSHOWCOPY>(rConfig))
	{
		spDragAndDropRootItem->AddChild(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_MENUCOPY_STRING), rResManager.LoadString(IDS_MENUTIPCOPY_STRING),
			TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Copy),
			TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeIDataObject),
			TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializePidlFolder, chcore::TSmartPath()), false, chcore::eOperation_Copy));
		bAddedAnyOption = true;
	}

	if(GetPropValue<PP_SHSHOWMOVE>(rConfig))
	{
		spDragAndDropRootItem->AddChild(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_MENUMOVE_STRING), rResManager.LoadString(IDS_MENUTIPMOVE_STRING),
			TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Move),
			TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeIDataObject),
			TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializePidlFolder, chcore::TSmartPath()), false, chcore::eOperation_Move));
		bAddedAnyOption = true;
	}

	if(GetPropValue<PP_SHSHOWCOPYMOVE>(rConfig))
	{
		spDragAndDropRootItem->AddChild(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_MENUCOPYMOVESPECIAL_STRING), rResManager.LoadString(IDS_MENUTIPCOPYMOVESPECIAL_STRING),
			TOperationTypeInfo(TOperationTypeInfo::eOpType_Autodetect, chcore::eOperation_Copy),
			TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeIDataObject),
			TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializePidlFolder, chcore::TSmartPath()), true));
		bAddedAnyOption = true;
	}

	if(bAddedAnyOption)
	{
		// insert separator as an addition to other items
		spDragAndDropRootItem->AddChild(std::make_shared<TShellMenuItem>());
	}
}

void TShellExtensionConfig::PrepareNormalMenuItems(TShellExtMenuConfig &cfgShellExt) const
{
	chcore::TConfig& rConfig = GetConfig();
	ictranslate::CResourceManager& rResManager = GetResManager();

	TShellMenuItemPtr spNormalRootItem = cfgShellExt.GetNormalRoot();

	if(GetPropValue<PP_SHSHOWPASTE>(rConfig))
	{
		spNormalRootItem->AddChild(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_MENUPASTE_STRING), rResManager.LoadString(IDS_MENUTIPPASTE_STRING),
			TOperationTypeInfo(TOperationTypeInfo::eOpType_Autodetect, chcore::eOperation_Copy),
			TSourcePathsInfo(TSourcePathsInfo::eSrcType_Clipboard),
			TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializeAuto, chcore::TSmartPath()), false));
	}

	if(GetPropValue<PP_SHSHOWPASTESPECIAL>(rConfig))
	{
		spNormalRootItem->AddChild(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_MENUPASTESPECIAL_STRING), rResManager.LoadString(IDS_MENUTIPPASTESPECIAL_STRING),
			TOperationTypeInfo(TOperationTypeInfo::eOpType_Autodetect, chcore::eOperation_Copy),
			TSourcePathsInfo(TSourcePathsInfo::eSrcType_Clipboard),
			TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializeAuto, chcore::TSmartPath()), true));
	}

	if(GetPropValue<PP_SHSHOWCOPYTO>(rConfig) || GetPropValue<PP_SHSHOWMOVETO>(rConfig) || GetPropValue<PP_SHSHOWCOPYMOVETO>(rConfig))
	{
		// prepare shortcuts for all menu options
		std::vector<CString> vShortcutStrings;
		GetPropValue<PP_SHORTCUTS>(rConfig, vShortcutStrings);

		std::vector<CShortcut> vShortcuts;

		for(const CString& strShortcutString : vShortcutStrings)
		{
			CShortcut tShortcut;
			if(tShortcut.FromString(strShortcutString))
				vShortcuts.push_back(tShortcut);
			else
				BOOST_ASSERT(false);	// non-critical, but not very nice
		}

		if(GetPropValue<PP_SHSHOWCOPYTO>(rConfig))
		{
			std::shared_ptr<TShellMenuItem> menuItem(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_MENUCOPYTO_STRING), rResManager.LoadString(IDS_MENUTIPCOPYTO_STRING)));
			for(const CShortcut& tShortcut : vShortcuts)
			{
				menuItem->AddChild(std::make_shared<TShellMenuItem>((PCTSTR)tShortcut.m_strName, (PCTSTR)tShortcut.m_strPath,
					TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Copy),
					TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
					TDestinationPathInfo(TDestinationPathInfo::eDstType_Specified, chcore::PathFromString((PCTSTR)tShortcut.m_strPath)), false));
			}

			spNormalRootItem->AddChild(menuItem);

			// optionally separator
			if(!vShortcuts.empty())
				menuItem->AddChild(std::make_shared<TShellMenuItem>());

			// "Choose" menu option
			menuItem->AddChild(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_SHELLEXT_CHOOSE_DIR_STRING), rResManager.LoadString(IDS_SHELLEXT_CHOOSE_DIR_TOOLTIP_STRING),
				TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Copy),
				TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
				TDestinationPathInfo(TDestinationPathInfo::eDstType_Choose, chcore::TSmartPath()), false));
		}

		if(GetPropValue<PP_SHSHOWMOVETO>(rConfig))
		{
			std::shared_ptr<TShellMenuItem> menuItem(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_MENUMOVETO_STRING), rResManager.LoadString(IDS_MENUTIPMOVETO_STRING)));
			for(const CShortcut& tShortcut : vShortcuts)
			{
				menuItem->AddChild(std::make_shared<TShellMenuItem>((PCTSTR)tShortcut.m_strName, (PCTSTR)tShortcut.m_strPath,
					TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Move),
					TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
					TDestinationPathInfo(TDestinationPathInfo::eDstType_Specified, chcore::PathFromString((PCTSTR)tShortcut.m_strPath)), false));
			}

			spNormalRootItem->AddChild(menuItem);

			// optionally separator
			if(!vShortcuts.empty())
				menuItem->AddChild(std::make_shared<TShellMenuItem>());

			// "Choose" menu option
			menuItem->AddChild(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_SHELLEXT_CHOOSE_DIR_STRING), rResManager.LoadString(IDS_SHELLEXT_CHOOSE_DIR_TOOLTIP_STRING),
				TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Move),
				TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
				TDestinationPathInfo(TDestinationPathInfo::eDstType_Choose, chcore::TSmartPath()), false));
		}

		if(GetPropValue<PP_SHSHOWCOPYMOVETO>(rConfig))
		{
			std::shared_ptr<TShellMenuItem> menuItem(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_MENUCOPYMOVETOSPECIAL_STRING), rResManager.LoadString(IDS_MENUTIPCOPYMOVETOSPECIAL_STRING)));
			for(const CShortcut& tShortcut : vShortcuts)
			{
				menuItem->AddChild(std::make_shared<TShellMenuItem>((PCTSTR)tShortcut.m_strName, (PCTSTR)tShortcut.m_strPath,
					TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Copy),
					TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
					TDestinationPathInfo(TDestinationPathInfo::eDstType_Specified, chcore::PathFromString((PCTSTR)tShortcut.m_strPath)), true));
			}

			spNormalRootItem->AddChild(menuItem);

			// optionally separator
			if(!vShortcuts.empty())
				menuItem->AddChild(std::make_shared<TShellMenuItem>());

			// "Choose" menu option
			menuItem->AddChild(std::make_shared<TShellMenuItem>(rResManager.LoadString(IDS_SHELLEXT_CHOOSE_DIR_STRING), rResManager.LoadString(IDS_SHELLEXT_CHOOSE_DIR_TOOLTIP_STRING),
				TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Copy),
				TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
				TDestinationPathInfo(TDestinationPathInfo::eDstType_Choose, chcore::TSmartPath()), true));
		}
	}
}
