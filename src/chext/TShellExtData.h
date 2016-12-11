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
/// @file  TShellExtData.h
/// @date  2011/05/28
/// @brief Contains class responsible for handling data harvested with shell interfaces.
// ============================================================================
#ifndef __TSHELLEXTDATA_H__
#define __TSHELLEXTDATA_H__

#include "../libchcore/TPath.h"
#include "../libchengine/EOperationTypes.h"
#include "../libchcore/TPathContainer.h"

class TShellMenuItem;

typedef std::shared_ptr<TShellMenuItem> TShellMenuItemPtr;

class TShellExtData
{
public:
	enum EDataSource
	{
		eDS_IDataObject,
		eDS_PidlFolder,
		eDS_Clipboard,
	};

	enum EActionSource
	{
		eSrc_None			= 0x0000,
		eSrc_CtxMenu		= 0x0100,
		eSrc_DropMenu		= 0x0200,
		eSrc_Keyboard		= 0x0400,
	};

private:
	/// State of the keyboard
	enum EStateKeys
	{
		eKey_None = 0,
		eKey_Ctrl = 1,
		eKey_Alt = 2,
		eKey_Shift = 4
	};

	/// Default operation
	enum EAction
	{
		eAction_None		= 0x0000,
		eAction_Copy		= 0x0001,
		eAction_Move		= 0x0002,
		eAction_Shortcut	= 0x0004
	};

public:
	TShellExtData();
	~TShellExtData();

	HRESULT GatherDataFromInitialize(LPCITEMIDLIST pidlFolder, IDataObject* piDataObject);
	void ReadDefaultSelectionStateFromMenu(HMENU hMenu);			///< Retrieves the state information from a menu handle; should be used only for drag&drop menu

	void Clear();

	bool CanServeAsSourcePaths(EDataSource eDataSource) const;
	bool CanServeAsDestinationPath(EDataSource eDataSource) const;

	bool VerifyItemCanBeExecuted(const TShellMenuItemPtr& spMenuItem) const;
	bool IsDefaultItem(const TShellMenuItemPtr& spMenuItem) const;

	bool GetSourcePathsByItem(const TShellMenuItemPtr& spMenuItem, chcore::TPathContainer& tSourcePaths) const;
	bool GetDestinationPathByItem(const TShellMenuItemPtr& spMenuItem, chcore::TSmartPath& tDestinationPath) const;

	bool GetOperationTypeByItem(const TShellMenuItemPtr& spMenuItem, chengine::EOperationType& eOperationType) const;

	EActionSource GetActionSource() const;

protected:
	void ReadKeyboardState();		///< Reads current keyboard state
	HRESULT ReadPreferredDropEffectFromIDataObject(IDataObject* piDataObject);
	HRESULT ReadClipboard();

	bool IsSameDrive(const chcore::TSmartPath& spPath1, const chcore::TSmartPath& spPath2) const;

private:
	// data gathered from IDataObject (Initialize())
	chcore::TPathContainer m_vPathsIDataObject;
	DWORD m_dwIDataObjectDropEffect;

	// data gathered from ITEMIDLIST (Initialize)
	chcore::TSmartPath m_pathPidlFolder;

	// Keys' state (Initialize())
	unsigned long m_ulKeysState;				///< State of the ctrl/shift/alt keys

	// data gathered from clipboard (Initialize())
	chcore::TPathContainer m_vPathsClipboard;
	DWORD m_dwClipboardDropEffect;

	// clicked in folder background
	bool m_bFolderBackground;

	// information retrieved from context menu
	EAction m_eDefaultSystemMenuAction;
};

#endif // __TSHELLEXTDATA_H__