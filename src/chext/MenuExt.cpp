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
#include "stdio.h"
#include "memory.h"
#include <boost/shared_array.hpp>
#include "ShellPathsHelpers.h"
#include "../common/TShellExtMenuConfig.h"
#include "../libchcore/TSharedMemory.h"
#include "../libchcore/TTaskDefinition.h"
#include <boost/numeric/conversion/cast.hpp>
#include "ShellExtensionVerifier.h"
#include "TLogger.h"

// globals
static void CutAmpersands(LPTSTR lpszString)
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
	m_piShellExtControl(nullptr)
{
	BOOST_LOG_FUNC();

	HRESULT hResult = CoCreateInstance(CLSID_CShellExtControl, nullptr, CLSCTX_ALL, IID_IShellExtControl, (void**)&m_piShellExtControl);
	TLogger& rLogger = Logger::get();
	BOOST_LOG_HRESULT(rLogger, hResult) << LOG_PARAM(m_piShellExtControl);
}

CMenuExt::~CMenuExt()
{
	if(m_piShellExtControl)
	{
		m_piShellExtControl->Release();
		m_piShellExtControl = nullptr;
	}
}

STDMETHODIMP CMenuExt::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* piDataObject, HKEY /*hkeyProgID*/)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

	if(!pidlFolder && !piDataObject)
	{
		BOOST_LOG_SEV(rLogger, error) << L"Missing both pointers.";
		return E_INVALIDARG;
	}

	// check options
	HWND hWnd = ShellExtensionVerifier::VerifyShellExt(m_piShellExtControl);
	if(!hWnd)
		return S_OK;

	HRESULT hResult = ReadShellConfig();
	if(SUCCEEDED(hResult))
		hResult = m_tShellExtData.GatherDataFromInitialize(pidlFolder, piDataObject);

	BOOST_LOG_HRESULT(rLogger, hResult) << L"";

	return hResult;
}

STDMETHODIMP CMenuExt::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT /*uFlags*/)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

	// check options
	HWND hWnd = ShellExtensionVerifier::VerifyShellExt(m_piShellExtControl);
	if(!hWnd)
		return S_OK;

	// current commands count in menu
	TCHAR szText[_MAX_PATH];
	int iCount = ::GetMenuItemCount(hMenu);

	MENUITEMINFO mii;
	mii.cbSize=sizeof(mii);
	mii.fMask=MIIM_TYPE;
	mii.dwTypeData=szText;
	mii.cch=_MAX_PATH;

	// find a place where the commands should be inserted
	for (int i=0;i<iCount;i++)
	{
		::GetMenuString(hMenu, i, szText, _MAX_PATH, MF_BYPOSITION);

		// get rid of &
		CutAmpersands(szText);
		_tcslwr(szText);

		// check for texts Wytnij/Wklej/Kopiuj/Cut/Paste/Copy
		if (_tcsstr(szText, _T("wytnij")) != nullptr || _tcsstr(szText, _T("wklej")) != nullptr ||
			_tcsstr(szText, _T("kopiuj")) != nullptr || _tcsstr(szText, _T("cut")) != nullptr ||
			_tcsstr(szText, _T("paste")) != nullptr || _tcsstr(szText, _T("copy")) != nullptr)
		{
			// found - find the nearest bar and insert above
			for (int j=i+1;j<iCount;j++)
			{
				// find bar
				::GetMenuItemInfo(hMenu, j, TRUE, &mii);

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
	TShellMenuItemPtr spRootMenuItem = m_tShellExtMenuConfig.GetCommandRoot();
	m_tContextMenuHandler.Init(spRootMenuItem, hMenu, idCmdFirst, indexMenu, m_tShellExtData, m_tShellExtMenuConfig.GetShowShortcutIcons(), false);

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, m_tContextMenuHandler.GetLastCommandID() - idCmdFirst + 1);
}

STDMETHODIMP CMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

	// textual verbs are not supported by this extension
	if(HIWORD(lpici->lpVerb) != 0)
		return E_FAIL;

	// check options
	HWND hWnd = ShellExtensionVerifier::VerifyShellExt(m_piShellExtControl);
	if(hWnd == nullptr)
		return E_FAIL;

	// find command to be executed, if not found - fail
	TShellMenuItemPtr spSelectedItem = m_tContextMenuHandler.GetCommandByMenuItemOffset(LOWORD(lpici->lpVerb));
	if(!spSelectedItem)
		return E_FAIL;

	// data retrieval and validation
	if(!m_tShellExtData.VerifyItemCanBeExecuted(spSelectedItem))
		return E_FAIL;

	chcore::TPathContainer vSourcePaths;
	chcore::TSmartPath spDestinationPath;
	chcore::EOperationType eOperationType = chcore::eOperation_None;

	if(!m_tShellExtData.GetSourcePathsByItem(spSelectedItem, vSourcePaths))
		return E_FAIL;
	if(!m_tShellExtData.GetDestinationPathByItem(spSelectedItem, spDestinationPath))
		return E_FAIL;
	if(!m_tShellExtData.GetOperationTypeByItem(spSelectedItem, eOperationType))
		return E_FAIL;

	chcore::TTaskDefinition tTaskDefinition;
	tTaskDefinition.SetSourcePaths(vSourcePaths);
	tTaskDefinition.SetDestinationPath(spDestinationPath);
	tTaskDefinition.SetOperationType(eOperationType);

	// get task data as xml
	chcore::TString wstrData;
	tTaskDefinition.StoreInString(wstrData);

	// fill struct
	COPYDATASTRUCT cds;
	cds.dwData = spSelectedItem->IsSpecialOperation() ? eCDType_TaskDefinitionContentSpecial : eCDType_TaskDefinitionContent;
	cds.lpData = (void*)wstrData.c_str();
	cds.cbData = (DWORD)((wstrData.GetLength() + 1) * sizeof(wchar_t));

	// send a message
	::SendMessage(hWnd, WM_COPYDATA, reinterpret_cast<WPARAM>(lpici->hwnd), reinterpret_cast<LPARAM>(&cds));

	return S_OK;
}

HRESULT CMenuExt::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return HandleMenuMsg2(uMsg, wParam, lParam, nullptr);
}

HRESULT CMenuExt::HandleMenuMsg2(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, LRESULT* /*plResult*/)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

	switch(uMsg)
	{
	case WM_INITMENUPOPUP:
		break;
		
	case WM_DRAWITEM:
		return DrawMenuItem((LPDRAWITEMSTRUCT)lParam);
		
	case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT)lParam;
			if(!lpmis)
				return E_FAIL;

			// find command to be executed, if not found - fail
			TShellMenuItemPtr spSelectedItem = m_tContextMenuHandler.GetCommandByItemID(LOWORD(lpmis->itemID));
			if(!spSelectedItem || !spSelectedItem->SpecifiesDestinationPath())
				return E_FAIL;

			// get menu logfont
			NONCLIENTMETRICS ncm;
			ncm.cbSize = sizeof(NONCLIENTMETRICS);
			SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);

			HFONT hFont = CreateFontIndirect(&ncm.lfMenuFont);
			if(!hFont)
				return E_FAIL;

			// measure the text
			HWND hDesktop = GetDesktopWindow();
			HDC hDC = GetDC(hDesktop);

			HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

			// calc text size
			SIZE size;
			GetTextExtentPoint32(hDC, spSelectedItem->GetName().c_str(), boost::numeric_cast<int>(spSelectedItem->GetName().GetLength()), &size);

			// restore old settings
			SelectObject(hDC, hOldFont);
			ReleaseDC(hDesktop, hDC);
			DeleteObject(hFont);

			// set
			lpmis->itemWidth = size.cx + GetSystemMetrics(SM_CXMENUCHECK) + 2 * GetSystemMetrics(SM_CXSMICON);
			lpmis->itemHeight = __max(size.cy + 3, GetSystemMetrics(SM_CYMENU) + 3);

			break;
		}
	}

	return S_OK;
}

HRESULT CMenuExt::DrawMenuItem(LPDRAWITEMSTRUCT lpdis)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

	if(!lpdis)
	{
		BOOST_LOG_SEV(rLogger, error) << L"Missing argument";
		return E_FAIL;
	}

	// check if menu
	if(lpdis->CtlType != ODT_MENU)
		return S_OK;

	// find command to be executed, if not found - fail
	TShellMenuItemPtr spSelectedItem = m_tContextMenuHandler.GetCommandByItemID(LOWORD(lpdis->itemID));
	if(!spSelectedItem || !spSelectedItem->SpecifiesDestinationPath())
		return E_FAIL;

	// margins and other stuff
	const int iSmallIconWidth = GetSystemMetrics(SM_CXSMICON);
	const int iSmallIconHeight = GetSystemMetrics(SM_CYSMICON);
	const int iLeftMargin = GetSystemMetrics(SM_CXMENUCHECK) / 2;
	const int iRightMargin = GetSystemMetrics(SM_CXMENUCHECK) - iLeftMargin;
	
	// text color
	HBRUSH hBrush = nullptr;
	if(lpdis->itemState & ODS_SELECTED)
	{
		SetTextColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		SetBkColor(lpdis->hDC, GetSysColor(COLOR_HIGHLIGHT));
		hBrush = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
	}
	else
	{
		SetTextColor(lpdis->hDC, GetSysColor(COLOR_MENUTEXT));
		SetBkColor(lpdis->hDC, GetSysColor(COLOR_MENU));
		hBrush = CreateSolidBrush(GetSysColor(COLOR_MENU));
	}

	// draw background
	RECT rcSelect = lpdis->rcItem;
	rcSelect.top++;
	rcSelect.bottom--;

	FillRect(lpdis->hDC, &rcSelect, hBrush);
	DeleteObject(hBrush);

	// get img list
	SHFILEINFO sfi;
	HIMAGELIST hImageList = (HIMAGELIST)SHGetFileInfo(spSelectedItem->GetDestinationPathInfo().GetDefaultDestinationPath().ToString(), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_SMALLICON | SHGFI_ICON | SHGFI_SYSICONINDEX);
	ImageList_Draw(hImageList, sfi.iIcon, lpdis->hDC, lpdis->rcItem.left + iLeftMargin, lpdis->rcItem.top + (lpdis->rcItem.bottom - lpdis->rcItem.top + 1 - iSmallIconHeight) / 2, ILD_TRANSPARENT);

	RECT rcText;
	rcText.left = iLeftMargin + iSmallIconWidth + iRightMargin;
	rcText.top = lpdis->rcItem.top;
	rcText.right = lpdis->rcItem.right;
	rcText.bottom = lpdis->rcItem.bottom;

	DrawText(lpdis->hDC, spSelectedItem->GetName().c_str(), -1, &rcText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

	return S_OK;
}

STDMETHODIMP CMenuExt::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

	memset(pszName, 0, cchMax);

	if(uFlags != GCS_HELPTEXTW && uFlags != GCS_HELPTEXTA)
		return S_OK;

	// check options
	HRESULT hResult = ShellExtensionVerifier::IsShellExtEnabled(m_piShellExtControl);
	if(FAILED(hResult))
		return hResult;
	else if(hResult == S_FALSE)
		return S_OK;

	TShellMenuItemPtr spSelectedItem = m_tContextMenuHandler.GetCommandByMenuItemOffset(LOWORD(idCmd));
	if(!spSelectedItem || !spSelectedItem->SpecifiesDestinationPath())
		return E_FAIL;

	switch(uFlags)
	{
	case GCS_HELPTEXTW:
		{
			wcsncpy(reinterpret_cast<wchar_t*>(pszName), spSelectedItem->GetItemTip().c_str(), spSelectedItem->GetItemTip().GetLength() + 1);
			break;
		}
	case GCS_HELPTEXTA:
		{
			USES_CONVERSION;
			CT2A ct2a(spSelectedItem->GetItemTip().c_str());
			strncpy(reinterpret_cast<char*>(pszName), ct2a, strlen(ct2a) + 1);
			break;
		}
	}

	return S_OK;
}

HRESULT CMenuExt::ReadShellConfig()
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();

	try
	{
		HWND hWnd = ShellExtensionVerifier::VerifyShellExt(m_piShellExtControl);
		if(hWnd == nullptr)
			return E_FAIL;

		// get cfg from ch
		unsigned long ulSHMID = GetTickCount();
		if(::SendMessage(hWnd, WM_GETCONFIG, eLocation_ContextMenu, ulSHMID) != TRUE)
		{
			BOOST_LOG_SEV(rLogger, error) << L"Failed to retrieve configuration from Copy Handler";
			return E_FAIL;
		}

		std::wstring strSHMName = IPCSupport::GenerateSHMName(ulSHMID);

		chcore::TSharedMemory tSharedMemory;
		chcore::TString wstrData;
		chcore::TConfig cfgShellExtData;

		tSharedMemory.Open(strSHMName.c_str());
		tSharedMemory.Read(wstrData);

		cfgShellExtData.ReadFromString(wstrData);

		m_tShellExtMenuConfig.ReadFromConfig(cfgShellExtData, _T("ShellExtCfg"));

		return S_OK;
	}
	catch(...)
	{
		return E_FAIL;
	}
}
