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
#include <boost/smart_ptr/make_shared.hpp>
#include "../common/TShellExtMenuConfig.h"
#include <boost/foreach.hpp>

TContextMenuHandler::TContextMenuHandler() :
	m_uiNextMenuID(0),
	m_uiFirstMenuID(0),
	m_bEnableOwnerDrawnPaths(false)
{
}

TContextMenuHandler::~TContextMenuHandler()
{
	Clear();
}

void TContextMenuHandler::Init(const TShellMenuItemPtr& spRootMenuItem, HMENU hMenu, UINT uiFirstItemID, UINT uiFirstItemPosition, const TShellExtData& rShellExtData, bool bEnableOwnerDrawnPaths, bool bOverrideDefaultItem)
{
	Clear();

	m_uiFirstMenuID = uiFirstItemID;
	m_uiNextMenuID = uiFirstItemID;
	m_bEnableOwnerDrawnPaths = bEnableOwnerDrawnPaths;

	UpdateMenuRecursive(spRootMenuItem, hMenu, uiFirstItemPosition, rShellExtData, bOverrideDefaultItem);
}

void TContextMenuHandler::UpdateMenuRecursive(const TShellMenuItemPtr& spRootMenuItem, HMENU hMenu, UINT uiFirstItemPosition, const TShellExtData& rShellExtData, bool bOverrideDefaultItem)
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
				UpdateMenuRecursive(spMenuItem, hSubMenu, 0, rShellExtData, bOverrideDefaultItem);

				MENUITEMINFO mii;
				mii.cbSize = sizeof(MENUITEMINFO);
				mii.fMask = MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_STRING;
				mii.fType = MFT_STRING;
				mii.fState = (spRootMenuItem->GetChildrenCount() > 0) ? MFS_ENABLED : MFS_GRAYED;
				mii.wID = m_uiNextMenuID++;
				mii.hSubMenu = hSubMenu;
				mii.dwTypeData = (PTSTR)spMenuItem->GetName().c_str();
				mii.cch = 0;

				::InsertMenuItem(hMenu, uiFirstItemPosition++, TRUE, &mii);
				break;
			}
		case TShellMenuItem::eSeparatorItem:
			{
				::InsertMenu(hMenu, uiFirstItemPosition++, MF_BYPOSITION | MF_SEPARATOR, m_uiNextMenuID++, NULL);
				break;
			}
		case TShellMenuItem::eStandardItem:
			{
				bool bEnableOwnerDrawnItem = m_bEnableOwnerDrawnPaths && spMenuItem->SpecifiesDestinationPath();
				bool bEnableItem = rShellExtData.VerifyItemCanBeExecuted(spMenuItem);

				::InsertMenu(hMenu, uiFirstItemPosition++, MF_BYPOSITION | MF_STRING | (bEnableItem ? MF_ENABLED : MF_GRAYED) | (bEnableOwnerDrawnItem ? MF_OWNERDRAW : 0), m_uiNextMenuID, spMenuItem->GetName().c_str());

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

void TContextMenuHandler::Clear()
{
	m_mapMenuItems.clear();
	BOOST_FOREACH(HMENU hMenu, m_vHandlesToFree)
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
	std::map<UINT, TShellMenuItemPtr>::iterator iter = m_mapMenuItems.find(m_uiFirstMenuID + uiOffset);
	if(iter != m_mapMenuItems.end())
		return (*iter).second;
	else
		return TShellMenuItemPtr();
}
