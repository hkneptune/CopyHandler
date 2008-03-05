/************************************************************************
	Copy Handler 1.x - program for copying data	in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
#include "stdafx.h"
#include "chext.h"
#include "MenuExt.h"
#include "clipboard.h"
#include "..\common\ipcstructs.h"
#include "..\common\FileSupport.h"
#include "stdio.h"
#include "memory.h"
#include "StringHelpers.h"

extern CSharedConfigStruct* g_pscsShared;

// globals
void CutAmpersands(LPTSTR lpszString)
{
	int iOffset=0;
	int iLength=_tcslen(lpszString);
	for (int j=0;j<iLength;j++)
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
HRESULT CMenuExt::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
//	OTF("CMenuExt::HandleMenuMsg\r\n");
	return HandleMenuMsg2(uMsg, wParam, lParam, NULL);
}

HRESULT CMenuExt::HandleMenuMsg2(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, LRESULT* /*plResult*/)
{
	switch(uMsg)
	{
	case WM_INITMENUPOPUP:
		{
//			OTF("CMenuExt::HandleMenuMsg2 / Init menu popup\r\n");
			break;
		}
		
	case WM_DRAWITEM:
		{
//			OTF("CMenuExt::HandleMenuMsg2 / Drawitem\r\n");
			LPDRAWITEMSTRUCT lpdis=(LPDRAWITEMSTRUCT) lParam;
			DrawMenuItem(lpdis);
			break;
		}
		
	case WM_MEASUREITEM:
		{
//			OTF("CMenuExt::HandleMenuMsg2 / MeasureItem\r\n");
			LPMEASUREITEMSTRUCT lpmis=(LPMEASUREITEMSTRUCT)lParam;

			// establish display text
			int iShortcutIndex=(lpmis->itemID-m_uiFirstID-5)%g_pscsShared->iShortcutsCount;
			_SHORTCUT* pShortcuts=(_SHORTCUT*)(g_pscsShared->szData+g_pscsShared->iCommandCount*sizeof(_COMMAND));

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
			GetTextExtentPoint32(hDC, pShortcuts[iShortcutIndex].szName, _tcslen(pShortcuts[iShortcutIndex].szName), &size);

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
	_SHORTCUT* pShortcuts=(_SHORTCUT*)(g_pscsShared->szData+g_pscsShared->iCommandCount*sizeof(_COMMAND));

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

//	OTF("Drawing text: %s\r\n", pShortcuts[iShortcutIndex].szName);
	DrawText(lpdis->hDC, pShortcuts[iShortcutIndex].szName, -1, &rcText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);
}

STDMETHODIMP CMenuExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT /*uFlags*/)
{
	// find ch window
	HWND hWnd;
	hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if (!hWnd)
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

/*	OTF("CMenuExt::QueryContextMenu - idCmdFirst=%lu, uFlags=%lu (", idCmdFirst, uFlags);
	if (uFlags & CMF_CANRENAME)
		OTF("CMF_CANRENAME ");
	if (uFlags & CMF_DEFAULTONLY)
		OTF("CMF_DEFAULTONLY ");
	if (uFlags & CMF_EXPLORE)
		OTF("CMF_EXPLORE ");
	if (uFlags & CMF_EXTENDEDVERBS)
		OTF("CMF_EXTENDEDVERBS ");
	if (uFlags & CMF_INCLUDESTATIC)
		OTF("CMF_INCLUDESTATIC ");
	if (uFlags & CMF_NODEFAULT)
		OTF("CMF_NODEFAULT ");
	if (uFlags & CMF_NORMAL)
		OTF("CMF_NORMAL ");
	if (uFlags & CMF_NOVERBS)
		OTF("CMF_NOVERBS ");
	if (uFlags & CMF_VERBSONLY)
		OTF("CMF_VERBSONLY ");
	OTF(")\r\n");
*/
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

//	OTF("after placement\r\n");

	// main command adding
	_COMMAND* pCommand=(_COMMAND*)g_pscsShared->szData;

	// data about commands
	int iCommandCount=0;
	
	if (!m_bGroupFiles)
	{
		// paste
		if (g_pscsShared->uiFlags & EC_PASTE_FLAG)
		{
			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING | (IsClipboardFormatAvailable(CF_HDROP) ? MF_ENABLED : MF_GRAYED), 
				idCmdFirst+0, pCommand[0].szCommand);
			iCommandCount++;
		}
		
		if (g_pscsShared->uiFlags & EC_PASTESPECIAL_FLAG)
		{
			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_STRING | (IsClipboardFormatAvailable(CF_HDROP) ? MF_ENABLED : MF_GRAYED), 
				idCmdFirst+1, pCommand[1].szCommand);
			iCommandCount++;
		}
	}

//	OTF("After group files\r\n");

	if (!m_bBackground)
	{
		CreateShortcutsMenu(idCmdFirst+5, g_pscsShared->bShowShortcutIcons);
//		OTF("after creating shortcuts menu\r\n");
		
		// copy to >
		if (g_pscsShared->uiFlags & EC_COPYTO_FLAG)
		{
			MENUITEMINFO mii;
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.fState=(g_pscsShared->iShortcutsCount > 0) ? MFS_ENABLED : MFS_GRAYED;
			mii.wID=idCmdFirst+2;
			mii.hSubMenu=m_mMenus.hShortcuts[0];
			mii.dwTypeData=pCommand[2].szCommand;
			mii.cch=_tcslen(pCommand[2].szCommand);

			::InsertMenuItem(hmenu, indexMenu++, TRUE, &mii);
//			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_POPUP | MF_STRING | ((g_pscsShared->iShortcutsCount > 0) ? MF_ENABLED : MF_GRAYED),
//				(UINT)m_mMenus.hShortcuts[0], pCommand[2].szCommand);
			iCommandCount++;
//			OTF("added menu item\r\n");
		}
		
		// move to >
		if (g_pscsShared->uiFlags & EC_MOVETO_FLAG)
		{
			MENUITEMINFO mii;
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.fState=(g_pscsShared->iShortcutsCount > 0) ? MFS_ENABLED : MFS_GRAYED;
			mii.wID=idCmdFirst+3;
			mii.hSubMenu=m_mMenus.hShortcuts[1];
			mii.dwTypeData=pCommand[3].szCommand;
			mii.cch=_tcslen(pCommand[3].szCommand);

			::InsertMenuItem(hmenu, indexMenu++, TRUE, &mii);
//			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_POPUP | MF_STRING | ((g_pscsShared->iShortcutsCount > 0) ? MF_ENABLED : MF_GRAYED),
//				(UINT)m_mMenus.hShortcuts[1], pCommand[3].szCommand);
			iCommandCount++;
		}
		
		// copy/move to special... >
		if (g_pscsShared->uiFlags & EC_COPYMOVETOSPECIAL_FLAG)
		{
			MENUITEMINFO mii;
			mii.cbSize=sizeof(MENUITEMINFO);
			mii.fMask=MIIM_ID | MIIM_STATE | MIIM_SUBMENU | MIIM_TYPE;
			mii.fType=MFT_STRING;
			mii.fState=(g_pscsShared->iShortcutsCount > 0) ? MFS_ENABLED : MFS_GRAYED;
			mii.wID=idCmdFirst+4;
			mii.hSubMenu=m_mMenus.hShortcuts[2];
			mii.dwTypeData=pCommand[4].szCommand;
			mii.cch=_tcslen(pCommand[4].szCommand);

			::InsertMenuItem(hmenu, indexMenu++, TRUE, &mii);
//			::InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_POPUP | MF_STRING | ((g_pscsShared->iShortcutsCount > 0) ? MF_ENABLED : MF_GRAYED),
//				(UINT)m_mMenus.hShortcuts[2], pCommand[4].szCommand);
			iCommandCount++;
		}
	}

//	OTF("before return\r\n");
	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, g_pscsShared->iCommandCount+(m_bBackground ? 0 : 3*g_pscsShared->iShortcutsCount));
}

void CMenuExt::CreateShortcutsMenu(UINT uiIDBase, bool bOwnerDrawn)
{
//	OTF("CreateShortcutsMenu\r\n");

	// twórz puste menu
	m_mMenus.hShortcuts[0]=CreatePopupMenu();
	m_mMenus.hShortcuts[1]=CreatePopupMenu();
	m_mMenus.hShortcuts[2]=CreatePopupMenu();
	
	// fill with shortcuts
	_SHORTCUT* pShortcuts=(_SHORTCUT*)(g_pscsShared->szData+g_pscsShared->iCommandCount*sizeof(_COMMAND));
	TCHAR szText[256], szSize[32];
	__int64 iiFree;

	for (int i=0;i<g_pscsShared->iShortcutsCount;i++)
	{
		// modify text
		if (g_pscsShared->bShowFreeSpace && GetDynamicFreeSpace(pShortcuts[i].szPath, &iiFree, NULL))
		{
			_sntprintf(szText, 256, _T("%s (%s)"), pShortcuts[i].szName, GetSizeString(iiFree, szSize, 32));
			_tcsncpy(pShortcuts[i].szName, szText, 127);
//			OTF("Text to display=%s\r\n", pShortcuts[i].szName);
			pShortcuts[i].szName[127]=_T('\0');
		}

		// add to all menus
		for (int j=0;j<3;j++)
			::InsertMenu(m_mMenus.hShortcuts[j], i, MF_BYPOSITION | MF_ENABLED | (bOwnerDrawn ? MF_OWNERDRAW : 0), uiIDBase+i+j*g_pscsShared->iShortcutsCount, (bOwnerDrawn ? NULL : pShortcuts[i].szName));
	}
}

STDMETHODIMP CMenuExt::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax)
{
	if (uFlags == GCS_HELPTEXTW)
	{
		USES_CONVERSION;
		// find window
		HWND hWnd;
		hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
		if (!hWnd)
			wcscpy(reinterpret_cast<wchar_t*>(pszName), L"");
		
		_COMMAND* pCommand=(_COMMAND*)g_pscsShared->szData;
	
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
			_SHORTCUT* pShortcuts=(_SHORTCUT*)(g_pscsShared->szData+g_pscsShared->iCommandCount*sizeof(_COMMAND));
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

		_COMMAND* pCommand=(_COMMAND*)g_pscsShared->szData;
		
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
			_SHORTCUT* pShortcuts=(_SHORTCUT*)(g_pscsShared->szData+g_pscsShared->iCommandCount*sizeof(_COMMAND));
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


STDMETHODIMP CMenuExt::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY /*hkeyProgID*/)
{
	// find ch window
	HWND hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if (hWnd == NULL)
		return E_FAIL;

	// get cfg from ch
	::SendMessage(hWnd, WM_GETCONFIG, GC_EXPLORER, 0);

	// read dest folder
	m_szDstPath[0]=_T('\0');

	// TEMP
//	OTF("****************************************************************\r\n");
//	OTF("CMenuExt::Initialize: pidlFolder=%lu, lpdobj=%lu\r\n", pidlFolder, lpdobj);

	// get data from IDataObject - files to copy/move
	bool bPathFound=false;
	m_bGroupFiles=false;
	if (lpdobj) 
	{
		STGMEDIUM medium;
		FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
			
		HRESULT hr = lpdobj->GetData(&fe, &medium);
		if (FAILED(hr))
			return E_FAIL;

		// copy all filenames to a table
		GetDataFromClipboard(static_cast<HDROP>(medium.hGlobal), NULL, &m_bBuffer.m_pszFiles, &m_bBuffer.m_iDataSize);

		// find the first non-empty entry
		UINT fileCount = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);
		TCHAR szPath[_MAX_PATH];
		UINT uiRes;
		for (UINT i=0;i<fileCount;i++)
		{
			uiRes=DragQueryFile((HDROP)medium.hGlobal, i++, szPath, _MAX_PATH);
			if (!bPathFound && uiRes != 0)
			{
				_tcscpy(m_szDstPath, szPath);
				bPathFound=true;
			}

			// check if there are files
			if (!(GetFileAttributes(szPath) & FILE_ATTRIBUTE_DIRECTORY))
				m_bGroupFiles=true;

			if (bPathFound && m_bGroupFiles)
				break;
		}
		
		ReleaseStgMedium(&medium);
	}

	// if all paths are empty - check pidlfolder
	if (!bPathFound)
	{
		if (!SHGetPathFromIDList(pidlFolder, m_szDstPath))
			return E_FAIL;

		// empty path - error
		if (_tcslen(m_szDstPath) == 0)
			return E_FAIL;
	}

	// background or folder ?
	m_bBackground=(lpdobj == NULL) && (pidlFolder != NULL);
		
	return S_OK;
}

STDMETHODIMP CMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	// find window
	HWND hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if (hWnd == NULL)
		return E_FAIL;

	// commands
	_COMMAND* pCommand=(_COMMAND*)g_pscsShared->szData;

//	OTF("Invoke Command\r\n");
	// command type
	switch (LOWORD(lpici->lpVerb))
	{
		// paste & paste special
	case 0:
	case 1:
		{
			// search for data in a clipboard
			if (IsClipboardFormatAvailable(CF_HDROP))
			{
				bool bMove=false;	// 0-copy, 1-move
				
				// get data
				OpenClipboard(lpici->hwnd);
				HANDLE handle=GetClipboardData(CF_HDROP);
				TCHAR *pchBuffer=NULL;
				UINT uiSize;
				
				GetDataFromClipboard(static_cast<HDROP>(handle), m_szDstPath, &pchBuffer, &uiSize);
				
				// register clipboard format nad if exists in it
				UINT nFormat=RegisterClipboardFormat(_T("Preferred DropEffect"));
				if (IsClipboardFormatAvailable(nFormat))
				{
					HANDLE handle=GetClipboardData(nFormat);
					LPVOID addr=GlobalLock(handle);
					
					DWORD dwData=((DWORD*)addr)[0];
					if (dwData & DROPEFFECT_MOVE)
						bMove=true;
					
					GlobalUnlock(handle);
				}
				
				CloseClipboard();
				
				// fill struct
				COPYDATASTRUCT cds;
				cds.dwData=(((DWORD)bMove) << 31) | pCommand[LOWORD(lpici->lpVerb)].uiCommandID;
				cds.lpData=pchBuffer;
				cds.cbData=uiSize * sizeof(TCHAR);
				
				// send a message
				::SendMessage(hWnd, WM_COPYDATA, reinterpret_cast<WPARAM>(lpici->hwnd), reinterpret_cast<LPARAM>(&cds));
				
				// delete buffer
				delete [] pchBuffer;
			}
		}
		break;
		
//	case 2:
//	case 3:
//	case 4:
	default:
		{
			// out of range - may be a shortcut
			if (LOWORD(lpici->lpVerb) < g_pscsShared->iCommandCount+(m_bBackground ? 0 : 3*g_pscsShared->iShortcutsCount))
			{
				// addr of a table with shortcuts
				_SHORTCUT* stShortcuts=(_SHORTCUT*)(g_pscsShared->szData+g_pscsShared->iCommandCount*sizeof(_COMMAND));
				
				// find command for which this command is generated
				int iCommandIndex=(int)(((LOWORD(lpici->lpVerb)-5) / g_pscsShared->iShortcutsCount))+2;	// command index
				int iShortcutIndex=((LOWORD(lpici->lpVerb)-5) % g_pscsShared->iShortcutsCount);	// shortcut index
				
				// buffer for data
				UINT uiSize=_tcslen(stShortcuts[iShortcutIndex].szPath)+1+m_bBuffer.m_iDataSize;
				TCHAR *pszBuffer=new TCHAR[uiSize];
				_tcscpy(pszBuffer, stShortcuts[iShortcutIndex].szPath);	// œcie¿ka docelowa
				
				// buffer with files
				memcpy(pszBuffer+_tcslen(stShortcuts[iShortcutIndex].szPath)+1, m_bBuffer.m_pszFiles, m_bBuffer.m_iDataSize*sizeof(TCHAR));
				
				// fill struct
				COPYDATASTRUCT cds;
				cds.dwData=pCommand[iCommandIndex].uiCommandID;
				cds.lpData=pszBuffer;
				cds.cbData=uiSize * sizeof(TCHAR);
				
				// send message
				::SendMessage(hWnd, WM_COPYDATA, reinterpret_cast<WPARAM>(lpici->hwnd), reinterpret_cast<LPARAM>(&cds));
				
				// delete buffer
				delete [] pszBuffer;
				m_bBuffer.Destroy();
			}
			else
				return E_FAIL;
		}
		break;
	}

	return S_OK;
}
