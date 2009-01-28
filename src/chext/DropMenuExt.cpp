/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
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
#include "DropMenuExt.h"
#include "clipboard.h"
#include "chext-utils.h"

/////////////////////////////////////////////////////////////////////////////
// CDropMenuExt

extern CSharedConfigStruct* g_pscsShared;

#define DE_COPY		0
#define DE_MOVE		1
#define DE_SPECIAL	2
#define DE_AUTO		3

CDropMenuExt::CDropMenuExt() :
	m_piShellExtControl(NULL)
{
	CoCreateInstance(CLSID_CShellExtControl, NULL, CLSCTX_ALL, IID_IShellExtControl, (void**)&m_piShellExtControl);
}

CDropMenuExt::~CDropMenuExt()
{
	if(m_piShellExtControl)
	{
		m_piShellExtControl->Release();
		m_piShellExtControl = NULL;
	}
}

STDMETHODIMP CDropMenuExt::QueryContextMenu(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT /*uFlags*/)
{
	// check options
	HRESULT hResult = IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
		return hResult;

	// find CH's window
	HWND hWnd;
	hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if (hWnd)
	{
		// get state of keys
		bool bShift=(GetKeyState(VK_SHIFT) & 0x80) != 0;
		bool bCtrl=(GetKeyState(VK_CONTROL) & 0x80) != 0;
		bool bAlt=(GetKeyState(VK_MENU) & 0x80) != 0;

/*		OTF2("CDropMenuExt::QueryContextMenu - uFlags=%lu (", uFlags);
		if (uFlags & CMF_CANRENAME)
			OTF2("CMF_CANRENAME ");
		if (uFlags & CMF_DEFAULTONLY)
			OTF2("CMF_DEFAULTONLY ");
		if (uFlags & CMF_EXPLORE)
			OTF2("CMF_EXPLORE ");
		if (uFlags & CMF_EXTENDEDVERBS)
			OTF2("CMF_EXTENDEDVERBS ");
		if (uFlags & CMF_INCLUDESTATIC)
			OTF2("CMF_INCLUDESTATIC ");
		if (uFlags & CMF_NODEFAULT)
			OTF2("CMF_NODEFAULT ");
		if (uFlags & CMF_NORMAL)
			OTF2("CMF_NORMAL ");
		if (uFlags & CMF_NOVERBS)
			OTF2("CMF_NOVERBS ");
		if (uFlags & CMF_VERBSONLY)
			OTF2("CMF_VERBSONLY ");
		OTF2(")\r\n");
		OTF2("Keys State: Shift:%u, ctrl:%u, alt:%u\r\n", bShift, bCtrl, bAlt);
*/
		// got a config
		_COMMAND* pCommand = g_pscsShared->GetCommandsPtr();
		int iCommandCount=0;

		if (g_pscsShared->uiFlags & DD_COPY_FLAG)
		{
			::InsertMenu(hmenu, indexMenu+iCommandCount, MF_BYPOSITION | MF_STRING, idCmdFirst+0, pCommand[0].szCommand);
			if (g_pscsShared->bOverrideDefault)
				::SetMenuDefaultItem(hmenu, idCmdFirst+0, FALSE);
			iCommandCount++;
		}

		if (g_pscsShared->uiFlags & DD_MOVE_FLAG)
		{
			::InsertMenu(hmenu, indexMenu+iCommandCount, MF_BYPOSITION | MF_STRING, idCmdFirst+1, pCommand[1].szCommand);
			if (g_pscsShared->bOverrideDefault && (bShift || (m_uiDropEffect == DE_MOVE && (m_bExplorer || !bCtrl))) )
				::SetMenuDefaultItem(hmenu, idCmdFirst+1, FALSE);
			iCommandCount++;
		}

		if (g_pscsShared->uiFlags & DD_COPYMOVESPECIAL_FLAG)
		{
			::InsertMenu(hmenu, indexMenu+iCommandCount, MF_BYPOSITION | MF_STRING, idCmdFirst+2, pCommand[2].szCommand);
			if (g_pscsShared->bOverrideDefault && (bAlt || (bCtrl && bShift) || m_uiDropEffect == DE_SPECIAL) && !(m_uiDropEffect == DE_MOVE))
				::SetMenuDefaultItem(hmenu, idCmdFirst+2, FALSE);
			iCommandCount++;
		}
	
		if (iCommandCount)
		{
			::InsertMenu(hmenu, indexMenu+iCommandCount, MF_BYPOSITION | MF_SEPARATOR, idCmdFirst+3, NULL);
			iCommandCount++;
		}

		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 4);
	}
	else
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
}

STDMETHODIMP CDropMenuExt::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax)
{
	// check options
	HRESULT hResult = IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
	{
		pszName[0] = _T('\0');
		return hResult;
	}

	if (uFlags == GCS_HELPTEXTW)
	{
		USES_CONVERSION;

		// find CH's window
		HWND hWnd;
		hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
		if (hWnd)
		{
			_COMMAND* pCommand = g_pscsShared->GetCommandsPtr();
			
			switch (idCmd)
			{
			case 0:
			case 1:
			case 2:
				{
					CT2W ct2w(pCommand[idCmd].szDesc);
					wcsncpy(reinterpret_cast<wchar_t*>(pszName), ct2w, cchMax);
					break;
				}
			default:
				wcsncpy(reinterpret_cast<wchar_t*>(pszName), L"", cchMax);
				break;
			}
		}
		else
			wcsncpy(reinterpret_cast<wchar_t*>(pszName), L"", cchMax);
	}
	if (uFlags == GCS_HELPTEXTA)
	{
		// find CH's window
		HWND hWnd;
		hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
		
		if (hWnd)
		{
			_COMMAND* pCommand = g_pscsShared->GetCommandsPtr();

			switch (idCmd)
			{
			case 0:
			case 1:
			case 2:
				{
					CT2A ct2a(pCommand[idCmd].szDesc);
					strncpy(pszName, ct2a, cchMax);
					break;
				}
			default:
				strncpy(pszName, "", cchMax);
				break;
			}
		}
		else
			strncpy(pszName, "", cchMax);
	}

	return S_OK;
}

STDMETHODIMP CDropMenuExt::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY /*hkeyProgID*/)
{
//	OTF2("Initialize cdropmenuext\r\n");
	HRESULT hResult = IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
		return hResult;

	// find window
	HWND hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if (hWnd == NULL)
		return E_FAIL;

	// gets the config from CH
	::SendMessage(hWnd, WM_GETCONFIG, GC_DRAGDROP, 0);

//	OTF2("========================================================\r\n");
//	OTF2("Initialize drag&drop context menu handler pidlFolder=%lu, lpdobj=%lu\r\n", pidlFolder, lpdobj);

	// get dest folder
	m_szDstPath[0]=_T('\0');
	if (!SHGetPathFromIDList(pidlFolder, m_szDstPath))
		return E_FAIL;

	// get data from IDataObject - files to copy/move
	if (lpdobj) 
	{
//		ReportAvailableFormats(lpdobj);
		// file list
		STGMEDIUM medium;
		FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

		HRESULT hr = lpdobj->GetData(&fe, &medium);
		if (FAILED(hr))
			return E_FAIL;

		GetDataFromClipboard(static_cast<HDROP>(medium.hGlobal), m_szDstPath, &m_bBuffer.m_pszFiles, &m_bBuffer.m_iDataSize);

		// set std text
		switch (g_pscsShared->uiDefaultAction)
		{
		case 1:
			// move action
			m_uiDropEffect=DE_MOVE;
			break;
		case 2:
			// special operation
			m_uiDropEffect=DE_SPECIAL;
			break;
		case 3:
			{
				// autodetecting - copy or move - check the last path
				UINT uiCount=DragQueryFile((HDROP)medium.hGlobal, 0xffffffff, NULL, 0);
				TCHAR szPath[_MAX_PATH];
				if (DragQueryFile((HDROP)medium.hGlobal, uiCount-1, szPath, _MAX_PATH))
				{
					if (_tcsncmp(szPath, _T("\\\\"), 2) == 0)
					{
						TCHAR* pFnd=_tcsstr(szPath+2, _T("\\"));

						if (pFnd)
						{
							int iCount;
							// find another
							TCHAR *pSecond=_tcsstr(pFnd+1, _T("\\"));
							if (pSecond)
							{
								iCount=pSecond-szPath;
//								OTF2("Counted: %lu\r\n", iCount);
							}
							else
								iCount=_tcslen(szPath);

							// found - compare
//							OTF2("Compare %s and %s\r\n", szPath, m_szDstPath);
							if (_tcsnicmp(szPath, m_szDstPath, iCount) == 0)
							{
//								OTF2("OP: MOVE\r\n");
								m_uiDropEffect=DE_MOVE;
							}
							else
							{
//								OTF2("OP: COPY\r\n");
								m_uiDropEffect=DE_COPY;
							}
						}
						else
							m_uiDropEffect=DE_COPY;
						
					}
					else
					{
						// local path - check drive letter
						if (m_szDstPath[0] == szPath[0])
							m_uiDropEffect=DE_MOVE;
						else
							m_uiDropEffect=DE_COPY;
					}
				}
				else
					m_uiDropEffect=DE_COPY;
			}
			break;

		default:
			m_uiDropEffect=DE_COPY;		// std copying
			break;
		}

		ReleaseStgMedium(&medium);

		// get operation type
		UINT cf=RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
		fe.cfFormat=(unsigned short)cf;

		// if explorer knows better - change effect
		m_bExplorer=false;
		hr=lpdobj->GetData(&fe, &medium);
		if (SUCCEEDED(hr))
		{
			// specify operation
			LPVOID lpv=GlobalLock(medium.hGlobal);
			if (lpv)
			{
				UINT uiDrop=*((DWORD*)lpv);
				if (uiDrop & DROPEFFECT_MOVE)
					m_uiDropEffect=DE_MOVE;
				else
					m_uiDropEffect=DE_COPY;
				m_bExplorer=true;

//				OTF2("Detected operation %lu\r\n", m_uiDropEffect);
				GlobalUnlock(medium.hGlobal);
			}

			ReleaseStgMedium(&medium);
		}
	}

	return S_OK;
}

STDMETHODIMP CDropMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	HRESULT hResult = IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult) || hResult == S_FALSE)
		return E_FAIL;		// required to process other InvokeCommand handlers.

	// find window
	HWND hWnd=::FindWindow(_T("Copy Handler Wnd Class"), _T("Copy handler"));
	if (hWnd == NULL)
		return E_FAIL;

	// commands
	_COMMAND* pCommand = g_pscsShared->GetCommandsPtr();

	// IPC struct
	COPYDATASTRUCT cds;
	cds.dwData=pCommand[LOWORD(lpici->lpVerb)].uiCommandID;	// based on command's number (0-copy, 1-move, 2-special (copy), 3-special (move))
	cds.cbData=m_bBuffer.m_iDataSize * sizeof(TCHAR);
	cds.lpData=m_bBuffer.m_pszFiles;

	// send a message to ch
	::SendMessage(hWnd, WM_COPYDATA, reinterpret_cast<WPARAM>(lpici->hwnd), reinterpret_cast<LPARAM>(&cds));

	m_bBuffer.Destroy();

	return S_OK;
}