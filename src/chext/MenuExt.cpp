/***************************************************************************
*   Copyright (C) 2001-2011 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "stdafx.h"
#include "chext.h"
#include "MenuExt.h"
#include "..\common\ipcstructs.h"
#include "..\common\FileSupport.h"
#include "stdio.h"
#include "memory.h"
#include "StringHelpers.h"
#include "chext-utils.h"
#include <boost/shared_array.hpp>
#include "ShellPathsHelpers.h"
#include "../libchcore/TWStringData.h"

extern CSharedConfigStruct* g_pscsShared;

// globals
void CutAmpersands(LPTSTR lpszString)
{
	int iOffset=0;
	size_t iLength=_tcslen(lpszString);
	for (size_t j=0;j<iLength;j++)
	{
		if (lpszString[j] == _T('&'))
			iOffset++;
		else
			if (iOffset != 0)
				lpszString[j-iOffset]=lpszString[j];
	}
	lpszString[iLength-iOffset]=_T('\0');
}

/////////////////////////////////////////////////////////////////////////////
// CMenuExt
CMenuExt::CMenuExt() :
	m_piShellExtControl(NULL)
{
	CoCreateInstance(CLSID_CShellExtControl, NULL, CLSCTX_ALL, IID_IShellExtControl, (void**)&m_piShellExtControl);
}

CMenuExt::~CMenuExt()
{
	if(m_piShellExtControl)
	{
		m_piShellExtControl->Release();
		m_piShellExtControl = NULL;
	}
}

STDMETHODIMP CMenuExt::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* piDataObject, HKEY /*hkeyProgID*/)
{
	ATLTRACE(_T("[CMenuExt::Initialize] CMenuExt::Initialize(pidlFolder = 0x%p, piDataObject=0x%p)\n"), pidlFolder, piDataObject);

	if(!pidlFolder && !piDataObject)
		return E_INVALIDARG;

	// check options
	HRESULT hResult = IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
		return hResult;

	// find ch window
	HWND hWnd = ::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if(hWnd == NULL)
		return E_FAIL;

	// get cfg from ch
	::SendMessage(hWnd, WM_GETCONFIG, GC_EXPLORER, 0);

	// background or folder ?
	m_bBackground = (piDataObject == NULL) && (pidlFolder != NULL);

	// get data from IDataObject - files to copy/move
	m_bShowPasteOption = true;

	m_vPaths.Clear();

	if(piDataObject)
	{
		hResult = ShellPathsHelpers::GetPathsFromIDataObject(piDataObject, m_vPaths);
		if(hResult != S_OK)
			return E_FAIL;
	}
	else if(pidlFolder)
	{
		chcore::TSmartPath pathFromPIDL;
		hResult = ShellPathsHelpers::GetPathFromITEMIDLIST(pidlFolder, pathFromPIDL);
		if(SUCCEEDED(hResult) && !pathFromPIDL.IsEmpty())
			m_vPaths.Add(pathFromPIDL);
	}
	else
		_ASSERTE(!_T("Both pidlFolder and piDataObject specified. Report this unsupported situation."));

	// find the first non-empty entry
	m_bShowPasteOption = (m_vPaths.GetCount() == 1) && (::GetFileAttributes(m_vPaths.GetAt(0).ToString()) & FILE_ATTRIBUTE_DIRECTORY);

	return S_OK;
}

STDMETHODIMP CMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	ATLTRACE(_T("CMenuExt::InvokeCommand()\n"));

	// check options
	HRESULT hResult = IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
		return E_FAIL;		// required to process other InvokeCommand handlers.

	// find window
	HWND hWnd = ::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if(hWnd == NULL)
		return E_FAIL;

	// commands
	_COMMAND* pCommand = g_pscsShared->GetCommandsPtr();

	// command type
	switch(LOWORD(lpici->lpVerb))
	{
		// paste & paste special
	case 0:
	case 1:
		{
			// paste and paste special requires a single directory path inside m_vPaths
			if((m_vPaths.GetCount() != 1) || !(::GetFileAttributes(m_vPaths.GetAt(0).ToString()) & FILE_ATTRIBUTE_DIRECTORY))
				return E_FAIL;

			// search for source paths in the clipboard
			if(IsClipboardFormatAvailable(CF_HDROP))
			{
				bool bMove = false;	// 0-copy, 1-move

				// read paths from clipboard
				OpenClipboard(lpici->hwnd);
				HANDLE hClipboardData = GetClipboardData(CF_HDROP);

				chcore::TPathContainer vPaths;
				ShellPathsHelpers::GetPathsFromHDROP(static_cast<HDROP>(hClipboardData), vPaths);

				// check if there is also a hint about operation type
				UINT nFormat = RegisterClipboardFormat(_T("Preferred DropEffect"));
				if(IsClipboardFormatAvailable(nFormat))
				{
					hClipboardData = GetClipboardData(nFormat);
					if(!hClipboardData)
						return E_FAIL;

					LPVOID pClipboardData = GlobalLock(hClipboardData);
					if(!pClipboardData)
						return E_FAIL;

					DWORD dwData = ((DWORD*)pClipboardData)[0];
					if(dwData & DROPEFFECT_MOVE)
						bMove = true;

					GlobalUnlock(hClipboardData);
				}

				CloseClipboard();

				chcore::TTaskDefinition tTaskDefinition;
				tTaskDefinition.SetSourcePaths(vPaths);
				tTaskDefinition.SetDestinationPath(m_vPaths.GetAt(0));
				tTaskDefinition.SetOperationType(bMove ? chcore::eOperation_Move : chcore::eOperation_Copy);

				// get task data as xml
				chcore::TWStringData wstrData;
				tTaskDefinition.StoreInString(wstrData);

				//::MessageBox(NULL, wstrData.GetData(), _T("MenuExt.cpp / Paste [special]"), MB_OK);		// TEMP - to be removed before commit

				// fill struct
				COPYDATASTRUCT cds;

				switch(pCommand[LOWORD(lpici->lpVerb)].uiCommandID)
				{
				case CSharedConfigStruct::EC_PASTESPECIAL_FLAG:
					cds.dwData = eCDType_TaskDefinitionContentSpecial;
					break;
				default:
					cds.dwData = eCDType_TaskDefinitionContent;
				}
				cds.lpData = (void*)wstrData.GetData();
				cds.cbData = (DWORD)wstrData.GetBytesCount();

				// send a message
				::SendMessage(hWnd, WM_COPYDATA, reinterpret_cast<WPARAM>(lpici->hwnd), reinterpret_cast<LPARAM>(&cds));
			}
		}
		break;

		//	case 2:
		//	case 3:
		//	case 4:
	default:
		{
			// out of range - may be a shortcut
			if(LOWORD(lpici->lpVerb) < g_pscsShared->iCommandCount + (m_bBackground ? 0 : 3 * g_pscsShared->iShortcutsCount))
			{
				// pClipboardData of a table with shortcuts
				_SHORTCUT* stShortcuts = g_pscsShared->GetShortcutsPtr();

				// find command for which this command is generated
				int iCommandIndex = (int)(((LOWORD(lpici->lpVerb)-5) / g_pscsShared->iShortcutsCount))+2;	// command index
				int iShortcutIndex = ((LOWORD(lpici->lpVerb)-5) % g_pscsShared->iShortcutsCount);	// shortcut index

				chcore::TTaskDefinition tTaskDefinition;

				tTaskDefinition.SetSourcePaths(m_vPaths);
				tTaskDefinition.SetDestinationPath(chcore::PathFromString(stShortcuts[iShortcutIndex].szPath));
				tTaskDefinition.SetOperationType(pCommand[iCommandIndex].eOperationType);

				// get task data as xml
				chcore::TWStringData wstrData;
				tTaskDefinition.StoreInString(wstrData);

//				::MessageBox(NULL, wstrData.GetData(), _T("MenuExt.cpp / Copy/Move to [special]"), MB_OK);		// TEMP - to be removed before commit

				// fill struct
				COPYDATASTRUCT cds;
				switch(pCommand[iCommandIndex].uiCommandID)
				{
				case CSharedConfigStruct::EC_COPYMOVETOSPECIAL_FLAG:
					cds.dwData = eCDType_TaskDefinitionContentSpecial;
					break;
				default:
					cds.dwData = eCDType_TaskDefinitionContent;
				}

				cds.dwData = pCommand[iCommandIndex].uiCommandID;
				cds.lpData = (void*)wstrData.GetData();
				cds.cbData = (DWORD)wstrData.GetBytesCount();

				// send message
				::SendMessage(hWnd, WM_COPYDATA, reinterpret_cast<WPARAM>(lpici->hwnd), reinterpret_cast<LPARAM>(&cds));
			}
			else
				return E_FAIL;
		}
		break;
	}

	return S_OK;
}

STDMETHODIMP CMenuExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT /*uFlags*/)
{
	ATLTRACE(_T("CMenuExt::QueryContextMenu()\n"));
	// check options
	HRESULT hResult = IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
		return hResult;

	// find ch window
	HWND hWnd;
	hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if(!hWnd)
		return S_OK;

	// remember ID of the first command
	m_uiFirstID=idCmdFirst;

	// current commands count in menu
	TCHAR szText[_MAX_PATH];
	int iCount=::GetMenuItemCount(hmenu);

	MENUITEMINFO mii;
	mii.cbSize=sizeof(mii);
	mii.fMask=MIIM_TYPE;
	mii.dwTypeData=szText;
	mii.cch=_MAX_PATH;

	// find a place where the commands should be inserted
	for (int i=0;i<iCount;i++)
	{
		::GetMenuString(hmenu, i, szText, _MAX_PATH, MF_BYPOSITION);

		// get rid of &
		CutAmpersands(szText);
		_tcslwr(szText);

		// check for texts Wytnij/Wklej/Kopiuj/Cut/Paste/Copy
		if (_tcsstr(szText, _T("wytnij")) != NULL || _tcsstr(szText, _T("wklej")) != NULL ||
			_tcsstr(szText, _T("kopiuj")) != NULL || _tcsstr(szText, _T("cut")) != NULL ||
			_tcsstr(szText, _T("paste")) != NULL || _tcsstr(szText, _T("copy")) != NULL)
		{
			// found - find the nearest bar and insert above
			for (int j=i+1;j<iCount;j++)
			{
				// find bar
				::GetMenuItemInfo(hmenu, j, TRUE, &mii);

				if (mii.fType == MFT_SEPARATOR)
				{
					indexMenu=j;
					j=iCount;
					i=iCount;
				}
			}
		}
	}

	// main command adding
	_COMMAND* pCommand=g_pscsShared->GetCommandsPtr();

	// data about commands
	int iCommandCount=0;

	if(m_bShowPasteOption)
	{
		// paste
		if (g_pscsShared->uiFlags & CSharedConfigStruct::EC_PASTE_FLAG)
		{
			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING | (IsClipboardFormatAvailable(CF_HDROP) ? MF_ENABLED : MF_GRAYED), 
				idCmdFirst+0, pCommand[0].szCommand);
			iCommandCount++;
		}

		if (g_pscsShared->uiFlags & CSharedConfigStruct::EC_PASTESPECIAL_FLAG)
		{
			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING | (IsClipboardFormatAvailable(CF_HDROP) ? MF_ENABLED : MF_GRAYED), 
				idCmdFirst+1, pCommand[1].szCommand);
			iCommandCount++;
		}
	}

	if (!m_bBackground)
	{
		CreateShortcutsMenu(idCmdFirst+5, g_pscsShared->bShowShortcutIcons);

		// copy to >
		if (g_pscsShared->uiFlags & CSharedConfigStruct::EC_COPYTO_FLAG)
		{
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.fState=(g_pscsShared->iShortcutsCount > 0) ? MFS_ENABLED : MFS_GRAYED;
			mii.wID=idCmdFirst+2;
			mii.hSubMenu=m_mMenus.hShortcuts[0];
			mii.dwTypeData=pCommand[2].szCommand;
			mii.cch=(UINT)_tcslen(pCommand[2].szCommand);

			::InsertMenuItem(hmenu, indexMenu++, TRUE, &mii);
			//			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_POPUP | MF_STRING | ((g_pscsShared->iShortcutsCount > 0) ? MF_ENABLED : MF_GRAYED),
			//				(UINT)m_mMenus.hShortcuts[0], pCommand[2].szCommand);
			iCommandCount++;
		}

		// move to >
		if (g_pscsShared->uiFlags & CSharedConfigStruct::EC_MOVETO_FLAG)
		{
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.fState=(g_pscsShared->iShortcutsCount > 0) ? MFS_ENABLED : MFS_GRAYED;
			mii.wID=idCmdFirst+3;
			mii.hSubMenu=m_mMenus.hShortcuts[1];
			mii.dwTypeData=pCommand[3].szCommand;
			mii.cch=(UINT)_tcslen(pCommand[3].szCommand);

			::InsertMenuItem(hmenu, indexMenu++, TRUE, &mii);
			//			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_POPUP | MF_STRING | ((g_pscsShared->iShortcutsCount > 0) ? MF_ENABLED : MF_GRAYED),
			//				(UINT)m_mMenus.hShortcuts[1], pCommand[3].szCommand);
			iCommandCount++;
		}

		// copy/move to special... >
		if (g_pscsShared->uiFlags & CSharedConfigStruct::EC_COPYMOVETOSPECIAL_FLAG)
		{
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.fState=(g_pscsShared->iShortcutsCount > 0) ? MFS_ENABLED : MFS_GRAYED;
			mii.wID=idCmdFirst+4;
			mii.hSubMenu=m_mMenus.hShortcuts[2];
			mii.dwTypeData=pCommand[4].szCommand;
			mii.cch=(UINT)_tcslen(pCommand[4].szCommand);

			::InsertMenuItem(hmenu, indexMenu++, TRUE, &mii);
			//			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_POPUP | MF_STRING | ((g_pscsShared->iShortcutsCount > 0) ? MF_ENABLED : MF_GRAYED),
			//				(UINT)m_mMenus.hShortcuts[2], pCommand[4].szCommand);
			iCommandCount++;
		}
	}

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, g_pscsShared->iCommandCount+(m_bBackground ? 0 : 3*g_pscsShared->iShortcutsCount));
}

HRESULT CMenuExt::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

HRESULT CMenuExt::HandleMenuMsg2(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, LRESULT* /*plResult*/)
{
	ATLTRACE(_T("CMenuExt::HandleMenuMsg2()\n"));

	switch(uMsg)
	{
	case WM_INITMENUPOPUP:
		break;
		
	case WM_DRAWITEM:
		{
			LPDRAWITEMSTRUCT lpdis=(LPDRAWITEMSTRUCT) lParam;
			DrawMenuItem(lpdis);
			break;
		}
		
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis=(LPMEASUREITEMSTRUCT)lParam;

			// establish display text
			int iShortcutIndex=(lpmis->itemID-m_uiFirstID-5)%g_pscsShared->iShortcutsCount;
			_SHORTCUT* pShortcuts = g_pscsShared->GetShortcutsPtr();

			// measure the text
			HWND hDesktop=GetDesktopWindow();
			HDC hDC=GetDC(hDesktop);

			// get menu logfont
			NONCLIENTMETRICS ncm;
			ncm.cbSize=sizeof(NONCLIENTMETRICS);
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

			HFONT hFont=CreateFontIndirect(&ncm.lfMenuFont);
			HFONT hOldFont=(HFONT)SelectObject(hDC, hFont);

			// calc text size
			SIZE size;
			GetTextExtentPoint32(hDC, pShortcuts[iShortcutIndex].szName, (int)_tcslen(pShortcuts[iShortcutIndex].szName), &size);

			// restore old settings
			SelectObject(hDC, hOldFont);
			ReleaseDC(hDesktop, hDC);

			// set
			lpmis->itemWidth=size.cx+GetSystemMetrics(SM_CXMENUCHECK)+2*GetSystemMetrics(SM_CXSMICON);
			lpmis->itemHeight = __max(size.cy+3, GetSystemMetrics(SM_CYMENU)+3);

			break;
		}
	}

	return S_OK;
}

void CMenuExt::DrawMenuItem(LPDRAWITEMSTRUCT lpdis)
{
	// check if menu
	if (lpdis->CtlType != ODT_MENU)
		return;

	// margins and other stuff
	const int iSmallIconWidth=GetSystemMetrics(SM_CXSMICON);
	const int iSmallIconHeight=GetSystemMetrics(SM_CYSMICON);
	const int iLeftMargin=GetSystemMetrics(SM_CXMENUCHECK)/2;
	const int iRightMargin=GetSystemMetrics(SM_CXMENUCHECK)-iLeftMargin;
	
	int iShortcutIndex=(lpdis->itemID-m_uiFirstID-5)%g_pscsShared->iShortcutsCount;
	_SHORTCUT* pShortcuts=g_pscsShared->GetShortcutsPtr();

	// text color
	HBRUSH hbr;
	if (lpdis->itemState & ODS_SELECTED)
	{
		SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
		hbr=CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
	}
	else
	{
		SetTextColor(lpdis->hDC, GetSysColor(COLOR_MENUTEXT));
		SetBkColor(lpdis->hDC, GetSysColor(COLOR_MENU));
		hbr=CreateSolidBrush(GetSysColor(COLOR_MENU));
	}

	// draw background
	RECT rcSelect=lpdis->rcItem;
	rcSelect.top++;
	rcSelect.bottom--;

	FillRect(lpdis->hDC, &rcSelect, hbr);
	DeleteObject(hbr);

	// get img list
	SHFILEINFO sfi;
	HIMAGELIST himl=(HIMAGELIST)SHGetFileInfo(pShortcuts[iShortcutIndex].szPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_SMALLICON | SHGFI_ICON | SHGFI_SYSICONINDEX);
	ImageList_Draw(himl, sfi.iIcon, lpdis->hDC, lpdis->rcItem.left+iLeftMargin,
		lpdis->rcItem.top+(lpdis->rcItem.bottom-lpdis->rcItem.top+1-iSmallIconHeight)/2, ILD_TRANSPARENT);

	RECT rcText;
	rcText.left=iLeftMargin+iSmallIconWidth+iRightMargin;
	rcText.top=lpdis->rcItem.top;
	rcText.right=lpdis->rcItem.right;
	rcText.bottom=lpdis->rcItem.bottom;

	DrawText(lpdis->hDC, pShortcuts[iShortcutIndex].szName, -1, &rcText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
}

void CMenuExt::CreateShortcutsMenu(UINT uiIDBase, bool bOwnerDrawn)
{
	// create empty menus
	m_mMenus.hShortcuts[0]=CreatePopupMenu();
	m_mMenus.hShortcuts[1]=CreatePopupMenu();
	m_mMenus.hShortcuts[2]=CreatePopupMenu();
	
	// fill with shortcuts
	_SHORTCUT* pShortcuts=g_pscsShared->GetShortcutsPtr();
	TCHAR szText[256], szSize[32];
	ull_t ullFree;

	for (int i=0;i<g_pscsShared->iShortcutsCount;i++)
	{
		// modify text
		if (g_pscsShared->bShowFreeSpace && GetDynamicFreeSpace(pShortcuts[i].szPath, &ullFree, NULL))
		{
			_sntprintf(szText, 256 - 1, _T("%s (%s)"), pShortcuts[i].szName, GetSizeString(ullFree, szSize, 32));
			szText[256 - 1] = _T('\0');
			_tcsncpy(pShortcuts[i].szName, szText, 127);
			pShortcuts[i].szName[127]=_T('\0');
		}

		// add to all menus
		for (int j=0;j<3;j++)
			::InsertMenu(m_mMenus.hShortcuts[j], i, MF_BYPOSITION | MF_ENABLED | (bOwnerDrawn ? MF_OWNERDRAW : 0), uiIDBase+i+j*g_pscsShared->iShortcutsCount, (bOwnerDrawn ? NULL : pShortcuts[i].szName));
	}
}

STDMETHODIMP CMenuExt::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax)
{
	// check options
	HRESULT hResult = IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
	{
		pszName[0] = _T('\0');
		return hResult;
	}

	LONG lFlags = eShellExt_None;
	hResult = m_piShellExtControl->GetFlags(&lFlags);
	if(FAILED(hResult))
	{
		pszName[0] = _T('\0');
		return hResult;
	}
	if(!(lFlags & eShellExt_Enabled))
	{
		pszName[0] = _T('\0');
		return S_OK;
	}

	if (uFlags == GCS_HELPTEXTW)
	{
		USES_CONVERSION;
		// find window
		HWND hWnd;
		hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
		if (!hWnd)
			wcscpy(reinterpret_cast<wchar_t*>(pszName), L"");
		
		_COMMAND* pCommand=g_pscsShared->GetCommandsPtr();
	
		switch (idCmd)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			{
				CT2W ct2w(pCommand[idCmd].szDesc);
				wcsncpy(reinterpret_cast<wchar_t*>(pszName), ct2w, cchMax);
				break;
			}
		default:
			_SHORTCUT* pShortcuts = g_pscsShared->GetShortcutsPtr();
			if ((int)(idCmd-5) < g_pscsShared->iShortcutsCount*3)
			{
				CT2W ct2w(pShortcuts[(idCmd-5)%g_pscsShared->iShortcutsCount].szPath);
				wcsncpy(reinterpret_cast<wchar_t*>(pszName), ct2w, cchMax);
			}
			else
				wcsncpy(reinterpret_cast<wchar_t*>(pszName), L"", cchMax);
		}
	}
	if (uFlags == GCS_HELPTEXTA)
	{
		// find window
		HWND hWnd;
		hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
		
		if (!hWnd)
			strcpy(pszName, "");

		_COMMAND* pCommand=g_pscsShared->GetCommandsPtr();
		
		switch (idCmd)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			{
				CT2A ct2a(pCommand[idCmd].szDesc);
				strncpy(reinterpret_cast<char*>(pszName), ct2a, cchMax);
				break;
			}
		default:	// rest of commands
			_SHORTCUT* pShortcuts = g_pscsShared->GetShortcutsPtr();
			if ((int)(idCmd-5) < g_pscsShared->iShortcutsCount*3)
			{
				CT2A ct2a(pShortcuts[(idCmd-5)%g_pscsShared->iShortcutsCount].szPath);
				strncpy(pszName, ct2a, cchMax);
			}
			else
				strncpy(pszName, "", cchMax);
		}
	}

	return S_OK;
}
