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
/// @file  TContextMenuHandler.cpp
/// @date  2011/05/28
/// @brief Contains implementation of class that handles menus.
// ============================================================================
#include "stdafx.h"
#include "TContextMenuHandler.h"
#include "../common/TShellExtMenuConfig.h"
#include "Logger.h"

TContextMenuHandler::TContextMenuHandler() :
	m_uiFirstMenuID(0),
	m_uiNextMenuID(0),
	m_bEnableOwnerDrawnPaths(false),
	m_fsLocal(GetLogFileData())
{
}

TContextMenuHandler::~TContextMenuHandler()
{
	Clear();
}

void TContextMenuHandler::Init(const TShellMenuItemPtr& spRootMenuItem, HMENU hMenu, UINT uiFirstItemID, UINT uiFirstItemPosition, const TShellExtData& rShellExtData,
	const chengine::TSizeFormatterPtr& spFormatter, bool bShowFreeSpace, bool bEnableOwnerDrawnPaths, bool bOverrideDefaultItem)
{
	Clear();

	m_uiFirstMenuID = uiFirstItemID;
	m_uiNextMenuID = uiFirstItemID;
	m_bEnableOwnerDrawnPaths = bEnableOwnerDrawnPaths;

	UpdateMenuRecursive(spRootMenuItem, hMenu, uiFirstItemPosition, rShellExtData, spFormatter, bShowFreeSpace, bOverrideDefaultItem);
}

void TContextMenuHandler::UpdateMenuRecursive(const TShellMenuItemPtr& spRootMenuItem, HMENU hMenu, UINT uiFirstItemPosition,
	const TShellExtData& rShellExtData, const chengine::TSizeFormatterPtr& spFormatter, bool bShowFreeSpace, bool bOverrideDefaultItem)
{
	for(size_t stIndex = 0; stIndex < spRootMenuItem->GetChildrenCount(); ++stIndex)
	{
		TShellMenuItemPtr spMenuItem = spRootMenuItem->GetChildAt(stIndex);
		switch(spMenuItem->GetItemType())
		{
		case TShellMenuItem::eGroupItem:
			{
				// special handling
				HMENU hSubMenu = CreatePopupMenu();
				UpdateMenuRecursive(spMenuItem, hSubMenu, 0, rShellExtData, spFormatter, bShowFreeSpace, bOverrideDefaultItem);

				MENUITEMINFO mii;
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_STRING;
				mii.fType = MFT_STRING;
				mii.fState = (spRootMenuItem->GetChildrenCount() > 0) ? MFS_ENABLED : MFS_GRAYED;
				mii.wID = m_uiNextMenuID++;
				mii.hSubMenu = hSubMenu;
				mii.dwTypeData = (PTSTR)spMenuItem->GetLocalName().c_str();
				mii.cch = 0;

				::InsertMenuItem(hMenu, uiFirstItemPosition++, TRUE, &mii);
				break;
			}
		case TShellMenuItem::eSeparatorItem:
			{
				::InsertMenu(hMenu, uiFirstItemPosition++, MF_BYPOSITION | MF_SEPARATOR, m_uiNextMenuID++, nullptr);
				break;
			}
		case TShellMenuItem::eStandardItem:
			{
				bool bEnableOwnerDrawnItem = m_bEnableOwnerDrawnPaths && spMenuItem->SpecifiesDestinationPath();
				bool bEnableItem = rShellExtData.VerifyItemCanBeExecuted(spMenuItem);

				std::wstring wstrItemName = GetDisplayText(spMenuItem, spFormatter, bShowFreeSpace);

				::InsertMenu(hMenu, uiFirstItemPosition++, MF_BYPOSITION | MF_STRING | (bEnableItem ? MF_ENABLED : MF_GRAYED) | (bEnableOwnerDrawnItem ? MF_OWNERDRAW : 0),
					m_uiNextMenuID, wstrItemName.c_str());

				if(bOverrideDefaultItem && rShellExtData.IsDefaultItem(spMenuItem))
					::SetMenuDefaultItem(hMenu, m_uiNextMenuID, FALSE);
				++m_uiNextMenuID;
				break;
			}
		default:
			BOOST_ASSERT(false);		// unhandled case
			return;
		}
		m_mapMenuItems.insert(std::make_pair(m_uiNextMenuID - 1, spMenuItem));		// (-1, because it was already incremented to point to the next free ID)
	}
}

std::wstring TContextMenuHandler::GetDisplayText(const TShellMenuItemPtr& spMenuItem, const chengine::TSizeFormatterPtr& spFormatter, bool bShowFreeSpace)
{
	std::wstring wstrItemName = spMenuItem->GetLocalName(false).c_str();

	if(wstrItemName.empty())
	{
		wstrItemName = spMenuItem->GetName().c_str();

		if(bShowFreeSpace && spMenuItem->SpecifiesDestinationPath())
		{
			try
			{
				unsigned long long ullSize = 0, ullTotal = 0;

				m_fsLocal.GetDynamicFreeSpace(spMenuItem->GetDestinationPathInfo().GetDefaultDestinationPath(), ullSize, ullTotal);

				wstrItemName += std::wstring(L" (") + spFormatter->GetSizeString(ullSize) + L")";
				spMenuItem->SetLocalName(wstrItemName.c_str());
			}
			catch(const std::exception&)
			{
			}
		}
		else
			spMenuItem->SetLocalName(wstrItemName.c_str());
	}

	return wstrItemName;
}

void TContextMenuHandler::Clear()
{
	m_mapMenuItems.clear();
	for(HMENU hMenu : m_vHandlesToFree)
	{
		DestroyMenu(hMenu);
	}
	m_vHandlesToFree.clear();
	m_uiFirstMenuID = 0;
	m_uiNextMenuID = 0;
	m_bEnableOwnerDrawnPaths = false;
}

TShellMenuItemPtr TContextMenuHandler::GetCommandByMenuItemOffset(UINT uiOffset)
{
	return GetCommandByItemID(m_uiFirstMenuID + uiOffset);
}

TShellMenuItemPtr TContextMenuHandler::GetCommandByItemID(UINT uiID)
{
	std::map<UINT, TShellMenuItemPtr>::iterator iter = m_mapMenuItems.find(uiID);
	if(iter != m_mapMenuItems.end())
		return (*iter).second;
	else
		return TShellMenuItemPtr();
}
