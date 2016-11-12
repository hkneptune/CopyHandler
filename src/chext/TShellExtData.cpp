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
/// @file  TShellExtData.cpp
/// @date  2011/05/29
/// @brief Implementation of class for handling data harvested with shell interfaces.
// ============================================================================
#include "stdafx.h"
#include "TShellExtData.h"
#include "ShellPathsHelpers.h"
#include "../common/TShellExtMenuConfig.h"

TShellExtData::TShellExtData() :
	m_vPathsIDataObject(),
	m_dwIDataObjectDropEffect(0),
	m_pathPidlFolder(),
	m_ulKeysState(eKey_None),
	m_vPathsClipboard(),
	m_dwClipboardDropEffect(0),
	m_bFolderBackground(false),
	m_eDefaultSystemMenuAction(eAction_None)
{
}

TShellExtData::~TShellExtData()
{
}

HRESULT TShellExtData::GatherDataFromInitialize(LPCITEMIDLIST pidlFolder, IDataObject* piDataObject)
{
	Clear();

	HRESULT hResult = S_OK;

	// read information from pidlFolder
	if(pidlFolder)
		hResult = ShellPathsHelpers::GetPathFromITEMIDLIST(pidlFolder, m_pathPidlFolder);

	// process IDataObject
	if(SUCCEEDED(hResult) && piDataObject)
	{
		hResult = ShellPathsHelpers::GetPathsFromIDataObject(piDataObject, m_vPathsIDataObject);
		if(hResult != S_OK)
			hResult = E_FAIL;
		if(SUCCEEDED(hResult))
			hResult = ReadPreferredDropEffectFromIDataObject(piDataObject);
	}

	// Read clipboard paths with preferred drop effects
	if(SUCCEEDED(hResult))
		hResult = ReadClipboard();

	if(SUCCEEDED(hResult))
	{
		ReadKeyboardState();

		// experimentally deduced condition
		m_bFolderBackground = (piDataObject == nullptr) && (pidlFolder != nullptr);
	}

	return hResult;
}

void TShellExtData::Clear()
{
	m_vPathsIDataObject.Clear();
	m_dwIDataObjectDropEffect = 0;

	m_pathPidlFolder.Clear();

	m_ulKeysState = eKey_None;

	m_vPathsClipboard.Clear();
	m_dwClipboardDropEffect = 0;

	m_bFolderBackground = false;

	m_eDefaultSystemMenuAction = eAction_None;
}

bool TShellExtData::CanServeAsSourcePaths(EDataSource eDataSource) const
{
	switch(eDataSource)
	{
	case eDS_Clipboard:
		return !m_vPathsClipboard.IsEmpty();
	case eDS_IDataObject:
		return !m_vPathsIDataObject.IsEmpty();
	case eDS_PidlFolder:
		return !m_pathPidlFolder.IsEmpty();

	default:
		BOOST_ASSERT(false);
		return false;
	}
}

bool TShellExtData::CanServeAsDestinationPath(EDataSource eDataSource) const
{
	switch(eDataSource)
	{
	case eDS_Clipboard:
		return m_vPathsClipboard.GetCount() == 1 && (::GetFileAttributes(m_vPathsClipboard.GetAt(0).ToString()) & FILE_ATTRIBUTE_DIRECTORY);
	case eDS_IDataObject:
		return m_vPathsIDataObject.GetCount() == 1 && (::GetFileAttributes(m_vPathsIDataObject.GetAt(0).ToString()) & FILE_ATTRIBUTE_DIRECTORY);
	case eDS_PidlFolder:
		return !m_pathPidlFolder.IsEmpty() && (::GetFileAttributes(m_pathPidlFolder.ToString()) & FILE_ATTRIBUTE_DIRECTORY);

	default:
		BOOST_ASSERT(false);
		return false;
	}
}

bool TShellExtData::VerifyItemCanBeExecuted(const TShellMenuItemPtr& spMenuItem) const
{
	if(!spMenuItem || spMenuItem->IsGroupItem())
		return false;

	// where do we expect source paths to get from
	switch(spMenuItem->GetSourcePathsInfo().GetSrcPathsSource())
	{
	case TSourcePathsInfo::eSrcType_Clipboard:
		{
			if(!CanServeAsSourcePaths(eDS_Clipboard))
				return false;
			break;
		}
	case TSourcePathsInfo::eSrcType_InitializePidlFolder:
		{
			if(!CanServeAsSourcePaths(eDS_PidlFolder))
				return false;
			break;
		}
	case TSourcePathsInfo::eSrcType_InitializeIDataObject:
		{
			if(!CanServeAsSourcePaths(eDS_IDataObject))
				return false;
			break;
		}
	case TSourcePathsInfo::eSrcType_InitializeAuto:
		{
			if(!CanServeAsSourcePaths(eDS_IDataObject) && !CanServeAsSourcePaths(eDS_PidlFolder))
				return false;
			break;
		}
	default:
		return false;
	}

	// destination path
	switch(spMenuItem->GetDestinationPathInfo().GetDstPathSource())
	{
	case TDestinationPathInfo::eDstType_Clipboard:
		{
			if(!CanServeAsDestinationPath(eDS_Clipboard))
				return false;
			break;
		}
	case TDestinationPathInfo::eDstType_InitializePidlFolder:
		{
			if(!CanServeAsDestinationPath(eDS_PidlFolder))
				return false;
			break;
		}
	case TDestinationPathInfo::eDstType_InitializeIDataObject:
		{
			if(!CanServeAsDestinationPath(eDS_IDataObject))
				return false;
			break;
		}
	case TDestinationPathInfo::eDstType_InitializeAuto:
		{
			if(!CanServeAsDestinationPath(eDS_IDataObject) && !CanServeAsDestinationPath(eDS_PidlFolder))
				return false;
			break;
		}
	case TDestinationPathInfo::eDstType_Specified:
		{
			// here we don't check if this is a directory or if it exists - it's assumed that user knows what is he doing
			if(spMenuItem->GetDestinationPathInfo().GetDefaultDestinationPath().IsEmpty())
				return false;
			break;
		}
	case TDestinationPathInfo::eDstType_Choose:
		break;  // returns true

	default:
		return false;
	}

	// if it passed all checks, means that we can allow this combination of source and destination path(s) to be used for command
	return true;
}

bool TShellExtData::IsDefaultItem(const TShellMenuItemPtr& spMenuItem) const
{
	if(!spMenuItem || spMenuItem->IsGroupItem() || spMenuItem->GetDefaultItemHint() == chcore::eOperation_None)
		return false;

	// check if there was a state defined by reading the current context menu
	if(m_eDefaultSystemMenuAction != eAction_None)
	{
		switch(m_eDefaultSystemMenuAction)
		{
		case eAction_Copy:
			return spMenuItem->GetDefaultItemHint() == chcore::eOperation_Copy;

		case eAction_Move:
			return spMenuItem->GetDefaultItemHint() == chcore::eOperation_Move;

		default:
			return false;
		}
	}

	// check if there is preferred drop effect associated with the source path
	switch(spMenuItem->GetSourcePathsInfo().GetSrcPathsSource())
	{
	case TSourcePathsInfo::eSrcType_Clipboard:
		{
			if(m_dwClipboardDropEffect == 0)
				break;

			if(m_dwClipboardDropEffect & DROPEFFECT_MOVE && spMenuItem->GetDefaultItemHint() == chcore::eOperation_Move ||
				m_dwClipboardDropEffect & DROPEFFECT_COPY && spMenuItem->GetDefaultItemHint() == chcore::eOperation_Copy)
				return true;

			return false;
		}

	case TSourcePathsInfo::eSrcType_InitializePidlFolder:
		{
			break;		// no associated info about drop effect
		}

	case TSourcePathsInfo::eSrcType_InitializeIDataObject:
	case TSourcePathsInfo::eSrcType_InitializeAuto:
		{
			if(m_dwIDataObjectDropEffect == 0)
				break;

			if(m_dwIDataObjectDropEffect & DROPEFFECT_MOVE && spMenuItem->GetDefaultItemHint() == chcore::eOperation_Move ||
				m_dwIDataObjectDropEffect & DROPEFFECT_COPY && spMenuItem->GetDefaultItemHint() == chcore::eOperation_Copy)
				return true;

			return false;
		}

	default:
		return false;
	}
	
	// step 3 - fallback - if there is no other info available, then assume copying, unless something else comes up from source/destination paths analysis
	// check keyboard buttons
	if(m_ulKeysState & eKey_Ctrl || m_ulKeysState & eKey_Shift)
	{
		if(m_ulKeysState & eKey_Ctrl && spMenuItem->GetDefaultItemHint() == chcore::eOperation_Copy)
			return true;
		if(m_ulKeysState & eKey_Shift && spMenuItem->GetDefaultItemHint() == chcore::eOperation_Move)
			return true;

		return false;
	}

	chcore::TSmartPath pathDestination;
	if(!GetDestinationPathByItem(spMenuItem, pathDestination))
		return false;

	bool bIsSameDriveOrServerName = false;
	switch(spMenuItem->GetSourcePathsInfo().GetSrcPathsSource())
	{
	case TSourcePathsInfo::eSrcType_Clipboard:
		{
			if(!m_vPathsClipboard.IsEmpty())
				bIsSameDriveOrServerName = IsSameDrive(pathDestination, m_vPathsClipboard.GetAt(m_vPathsClipboard.GetCount() - 1));

			break;
		}
	case TSourcePathsInfo::eSrcType_InitializePidlFolder:
		{
			bIsSameDriveOrServerName = IsSameDrive(pathDestination, m_pathPidlFolder);

			break;
		}
	case TSourcePathsInfo::eSrcType_InitializeIDataObject:
		{
			if(!m_vPathsIDataObject.IsEmpty())
				bIsSameDriveOrServerName = IsSameDrive(pathDestination, m_vPathsIDataObject.GetAt(m_vPathsIDataObject.GetCount() - 1));

			break;
		}
	case TSourcePathsInfo::eSrcType_InitializeAuto:
		{
			if(!m_vPathsIDataObject.IsEmpty())
			{
				if(!m_vPathsIDataObject.IsEmpty())
					bIsSameDriveOrServerName = IsSameDrive(pathDestination, m_vPathsIDataObject.GetAt(m_vPathsIDataObject.GetCount() - 1));
			}
			else if(!m_pathPidlFolder.IsEmpty())
				bIsSameDriveOrServerName = IsSameDrive(pathDestination, m_pathPidlFolder);
			else
				return false;
			break;
		}
	}

	// depending on bIsSameDriveOrServerName we either are operating inside single disk volume or within single server,
	// and since there is no other definition of operation to be performed, we assume either copy or move as default
	if(bIsSameDriveOrServerName)
	{
		if(spMenuItem->GetDefaultItemHint() == chcore::eOperation_Move)
			return true;
	}
	else
	{
		if(spMenuItem->GetDefaultItemHint() == chcore::eOperation_Copy)
			return true;
	}

	return false;
}

bool TShellExtData::GetSourcePathsByItem(const TShellMenuItemPtr& spMenuItem, chcore::TPathContainer& tSourcePaths) const
{
	if(!spMenuItem || spMenuItem->IsGroupItem())
		return false;

	tSourcePaths.Clear();

	// where do we expect source paths to get from
	switch(spMenuItem->GetSourcePathsInfo().GetSrcPathsSource())
	{
	case TSourcePathsInfo::eSrcType_Clipboard:
		tSourcePaths = m_vPathsClipboard;
		break;
	case TSourcePathsInfo::eSrcType_InitializePidlFolder:
		tSourcePaths.Add(m_pathPidlFolder);
		break;
	case TSourcePathsInfo::eSrcType_InitializeIDataObject:
		tSourcePaths = m_vPathsIDataObject;
		break;
	case TSourcePathsInfo::eSrcType_InitializeAuto:
		{
			if(!m_vPathsIDataObject.IsEmpty())
				tSourcePaths = m_vPathsIDataObject;
			else if(!m_pathPidlFolder.IsEmpty())
				tSourcePaths.Add(m_pathPidlFolder);
			else
				return false;
			break;
		}
	default:
		return false;
	}

	return true;
}

bool TShellExtData::GetDestinationPathByItem(const TShellMenuItemPtr& spMenuItem, chcore::TSmartPath& tDestinationPath) const
{
	if(!spMenuItem || spMenuItem->IsGroupItem())
		return false;

	tDestinationPath.Clear();

	// destination path
	switch(spMenuItem->GetDestinationPathInfo().GetDstPathSource())
	{
	case TDestinationPathInfo::eDstType_Clipboard:
		{
			if(m_vPathsClipboard.GetCount() != 1)
				return false;
			tDestinationPath = m_vPathsClipboard.GetAt(0);
			break;
		}
	case TDestinationPathInfo::eDstType_InitializePidlFolder:
		tDestinationPath = m_pathPidlFolder;
		break;
	case TDestinationPathInfo::eDstType_InitializeIDataObject:
		{
			if(m_vPathsIDataObject.GetCount() != 1)
				return false;
			tDestinationPath = m_vPathsIDataObject.GetAt(0);
			break;
		}
	case TDestinationPathInfo::eDstType_InitializeAuto:
		{
			if(!m_vPathsIDataObject.IsEmpty())
			{
				if(m_vPathsIDataObject.GetCount() != 1)
					return false;
				tDestinationPath = m_vPathsIDataObject.GetAt(0);
			}
			else if(!m_pathPidlFolder.IsEmpty())
				tDestinationPath = m_pathPidlFolder;
			else
				return false;

			break;
		}
	case TDestinationPathInfo::eDstType_Specified:
		{
			// here we don't check if this is a directory or if it exists - it's assumed that user knows what is he doing
			if(spMenuItem->GetDestinationPathInfo().GetDefaultDestinationPath().IsEmpty())
				return false;
			tDestinationPath = spMenuItem->GetDestinationPathInfo().GetDefaultDestinationPath();
			break;
		}
	case TDestinationPathInfo::eDstType_Choose:
		// tDestinationPath is already clear; returning true
		break;

	default:
		return false;
	}

	return true;
}

bool TShellExtData::GetOperationTypeByItem(const TShellMenuItemPtr& spMenuItem, chcore::EOperationType& eOperationType) const
{
	if(!spMenuItem || spMenuItem->IsGroupItem())
		return false;

	eOperationType = chcore::eOperation_None;

	switch(spMenuItem->GetOperationTypeInfo().GetOperationTypeSource())
	{
	case TOperationTypeInfo::eOpType_Autodetect:
		{
			// depending on the source and destination...
			switch(spMenuItem->GetSourcePathsInfo().GetSrcPathsSource())
			{
			case TSourcePathsInfo::eSrcType_Clipboard:
				eOperationType = (m_dwClipboardDropEffect & DROPEFFECT_MOVE) ? chcore::eOperation_Move : chcore::eOperation_Copy;
				break;
			case TSourcePathsInfo::eSrcType_InitializeIDataObject:
			case TSourcePathsInfo::eSrcType_InitializeAuto:
				eOperationType = (m_dwIDataObjectDropEffect & DROPEFFECT_MOVE) ? chcore::eOperation_Move : chcore::eOperation_Copy;
				break;
			case TSourcePathsInfo::eSrcType_InitializePidlFolder:
				return false;

			default:
				return false;
			}
			break;
		}
	case TOperationTypeInfo::eOpType_Specified:
		eOperationType = spMenuItem->GetOperationTypeInfo().GetDefaultOperationType();
		break;
	default:
		return false;
	}

	return true;
}

void TShellExtData::ReadKeyboardState()
{
	m_ulKeysState = ((GetKeyState(VK_SHIFT) & 0x80) ? eKey_Shift : 0) |
		((GetKeyState(VK_CONTROL) & 0x80) ? eKey_Ctrl : 0) |
		((GetKeyState(VK_MENU) & 0x80) ? eKey_Alt : 0);
}

HRESULT TShellExtData::ReadPreferredDropEffectFromIDataObject(IDataObject* piDataObject)
{
	if(!piDataObject)
		return E_INVALIDARG;

	// try to retrieve the preferred drop effect from the input data object
	UINT uiPreferredDropEffect = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	if(!uiPreferredDropEffect)
		return E_FAIL;

	FORMATETC fe = { (CLIPFORMAT)uiPreferredDropEffect, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	// if the drop effect does not exist - just report it
	m_dwIDataObjectDropEffect = 0;
	HRESULT hResult = piDataObject->QueryGetData(&fe);
	if(hResult == S_OK)
	{
		STGMEDIUM medium;
		hResult = piDataObject->GetData(&fe, &medium);
		if(SUCCEEDED(hResult) && !medium.hGlobal)
			hResult = E_FAIL;
		if(SUCCEEDED(hResult))
		{
			// specify operation
			DWORD* pdwData = (DWORD*)GlobalLock(medium.hGlobal);
			if(pdwData)
			{
				m_dwIDataObjectDropEffect = *pdwData;
				GlobalUnlock(medium.hGlobal);
			}
			else
				hResult = E_FAIL;
		}

		ReleaseStgMedium(&medium);
	}

	return hResult;
}

HRESULT TShellExtData::ReadClipboard()
{
	HRESULT hResult = S_FALSE;

	if(IsClipboardFormatAvailable(CF_HDROP))
	{
		// read paths from clipboard
		if(!OpenClipboard(nullptr))
			return E_FAIL;

		HANDLE hClipboardData = GetClipboardData(CF_HDROP);
		if(!hClipboardData)
			hResult = E_FAIL;

		if(SUCCEEDED(hResult))
		{
			ShellPathsHelpers::GetPathsFromHDROP(static_cast<HDROP>(hClipboardData), m_vPathsClipboard);

			// check if there is also a hint about operation type
			UINT nFormat = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
			if(IsClipboardFormatAvailable(nFormat))
			{
				hClipboardData = GetClipboardData(nFormat);
				if(!hClipboardData)
					hResult = E_FAIL;
				else
				{
					LPVOID pClipboardData = GlobalLock(hClipboardData);
					if(!pClipboardData)
						hResult = E_FAIL;
					else
						m_dwClipboardDropEffect = *((DWORD*)pClipboardData);

					GlobalUnlock(hClipboardData);
				}
			}
		}

		CloseClipboard();

		return hResult;
	}
	else
		return S_FALSE;
}

bool TShellExtData::IsSameDrive(const chcore::TSmartPath& spPath1, const chcore::TSmartPath& spPath2) const
{
	// NOTE: see operator== in TSmartPath for comments on path case sensitivity and possible issues with it
	return (spPath1.HasDrive() && spPath2.HasDrive() && spPath1.GetDrive() == spPath2.GetDrive()) 
			|| (spPath1.IsNetworkPath() && spPath2.IsNetworkPath() && spPath1.GetServerName() == spPath2.GetServerName());
}

void TShellExtData::ReadDefaultSelectionStateFromMenu(HMENU hMenu)
{
	// it's none by default
	m_eDefaultSystemMenuAction = eAction_None;

	BOOST_ASSERT(hMenu != nullptr);
	if(hMenu)
	{
		MENUITEMINFO mii;
		mii.cbSize = sizeof(MENUITEMINFO);
		mii.fMask = MIIM_STATE;

		if(::GetMenuItemInfo(hMenu, 1, FALSE, &mii) && mii.fState & MFS_DEFAULT)
			m_eDefaultSystemMenuAction = eAction_Copy;
		if(::GetMenuItemInfo(hMenu, 2, FALSE, &mii) && mii.fState & MFS_DEFAULT)
			m_eDefaultSystemMenuAction = eAction_Move;
		if(::GetMenuItemInfo(hMenu, 3, FALSE, &mii) && mii.fState & MFS_DEFAULT)
			m_eDefaultSystemMenuAction = eAction_Shortcut;
	}
}

TShellExtData::EActionSource TShellExtData::GetActionSource() const
{
	return (m_dwIDataObjectDropEffect != 0) ? (m_ulKeysState != eKey_None ? eSrc_Keyboard : eSrc_CtxMenu) : eSrc_DropMenu;
}