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
/// @file  TContextMenuHandler.h
/// @date  2011/05/28
/// @brief Contains class which handles context menu.
// ============================================================================
#ifndef __TCONTEXTMENUHANDLER_H__
#define __TCONTEXTMENUHANDLER_H__

#include "TShellExtData.h"

class TShellMenuItem;

typedef boost::shared_ptr<TShellMenuItem> TShellMenuItemPtr;

class TContextMenuHandler
{
public:
	TContextMenuHandler();
	~TContextMenuHandler();

	void Init(const TShellMenuItemPtr& spRootMenuItem, HMENU hMenu, UINT uiFirstItemID, UINT uiFirstItemPosition, const TShellExtData& rShellExtData, bool bEnableOwnerDrawnPaths, bool bOverrideDefaultItem);
	void Clear();

	UINT GetLastCommandID() const { return m_uiNextMenuID; }
	TShellMenuItemPtr GetCommandByMenuItemOffset(UINT uiOffset);

protected:
	void UpdateMenuRecursive(const TShellMenuItemPtr& spRootMenuItem, HMENU hMenu, UINT uiFirstItemPosition, const TShellExtData& rShellExtData, bool bOverrideDefaultItem);

private:
	std::map<UINT, TShellMenuItemPtr> m_mapMenuItems;
	std::vector<HMENU> m_vHandlesToFree;

	UINT m_uiFirstMenuID;		// menu ID from which the numbering started
	UINT m_uiNextMenuID;		// next menu ID to be used

	bool m_bEnableOwnerDrawnPaths;
};

#endif
