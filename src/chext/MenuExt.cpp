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
#include "../common/ipcstructs.h"
#include "stdio.h"
#include "memory.h"
#include <boost/shared_array.hpp>
#include "../common/TShellExtMenuConfig.h"
#include "../libchcore/TSharedMemory.h"
#include <boost/numeric/conversion/cast.hpp>
#include "ShellExtensionVerifier.h"
#include "Logger.h"
#include "../libchengine/TTaskDefinition.h"

/////////////////////////////////////////////////////////////////////////////
// CMenuExt
CMenuExt::CMenuExt() :
	m_piShellExtControl(nullptr),
	m_spLog(GetLogger(L"CMenuExt"))
{
	HRESULT hResult = CoCreateInstance(CLSID_CShellExtControl, nullptr, CLSCTX_ALL, IID_IShellExtControl, (void**)&m_piShellExtControl);
	LOG_HRESULT(m_spLog, hResult) << L"Creation of ShellExtControl " << LOG_PARAM(m_piShellExtControl);
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
	try
	{
		LOG_DEBUG(m_spLog) << L"Initializing";

		if(!pidlFolder && !piDataObject)
		{
			LOG_ERROR(m_spLog) << L"Missing both pointers.";
			return E_INVALIDARG;
		}

		// check options
		HWND hWnd = ShellExtensionVerifier::VerifyShellExt(m_piShellExtControl);
		if(!hWnd)
			return S_OK;

		HRESULT hResult = ShellExtensionVerifier::ReadShellConfig(m_piShellExtControl, m_tShellExtMenuConfig);
		LOG_HRESULT(m_spLog, hResult) << L"Read shell config";

		if(SUCCEEDED(hResult))
		{
			hResult = m_tShellExtData.GatherDataFromInitialize(pidlFolder, piDataObject);
			LOG_HRESULT(m_spLog, hResult) << L"Gather data from initialize";
		}

		return hResult;
	}
	catch(const std::exception& e)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected std exception encountered in " << __FUNCTION__ << L": " << e.what();
		return E_FAIL;
	}
	catch(...)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected other exception encountered in " << __FUNCTION__ << L".";
		return E_FAIL;
	}
}

STDMETHODIMP CMenuExt::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
	idCmdLast;
	uFlags;

	try
	{
		LOG_DEBUG(m_spLog) << L"Querying context menu";

		if(!hMenu)
		{
			LOG_ERROR(m_spLog) << L"Received null hMenu from caller";
			return E_INVALIDARG;
		}

		// check options
		HWND hWnd = ShellExtensionVerifier::VerifyShellExt(m_piShellExtControl);
		if(!hWnd)
			return S_OK;

		if (m_tContextMenuHandler.HasCHItems(hMenu))
			return S_OK;

		int iMenuInsertLocation = m_tContextMenuHandler.FindMenuInsertLocation(hMenu);
		if (iMenuInsertLocation != -1)
			indexMenu = iMenuInsertLocation;

		// main command adding
		TShellMenuItemPtr spRootMenuItem = m_tShellExtMenuConfig.GetNormalRoot();
		m_tContextMenuHandler.Init(spRootMenuItem, hMenu, idCmdFirst, indexMenu, m_tShellExtData, m_tShellExtMenuConfig.GetFormatter(),
			m_tShellExtMenuConfig.GetShowFreeSpace(), m_tShellExtMenuConfig.GetShowShortcutIcons(), false);

		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, m_tContextMenuHandler.GetLastCommandID() - idCmdFirst + 1);
	}
	catch(const std::exception& e)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected std exception encountered in " << __FUNCTION__ << L": " << e.what();
		return E_FAIL;
	}
	catch(...)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected other exception encountered in " << __FUNCTION__ << L".";
		return E_FAIL;
	}
}

STDMETHODIMP CMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	try
	{
		LOG_DEBUG(m_spLog) << L"Invoking command";

		if(!lpici)
		{
			LOG_ERROR(m_spLog) << L"Parameter lpici is null";
			return E_INVALIDARG;
		}

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
		chengine::EOperationType eOperationType = chengine::eOperation_None;

		if(!m_tShellExtData.GetSourcePathsByItem(spSelectedItem, vSourcePaths))
			return E_FAIL;
		if(!m_tShellExtData.GetDestinationPathByItem(spSelectedItem, spDestinationPath))
			return E_FAIL;
		if(!m_tShellExtData.GetOperationTypeByItem(spSelectedItem, eOperationType))
			return E_FAIL;

		chengine::TTaskDefinition tTaskDefinition;
		tTaskDefinition.SetSourcePaths(vSourcePaths);
		tTaskDefinition.SetDestinationPath(spDestinationPath);
		tTaskDefinition.SetOperationType(eOperationType);

		// get task data as xml
		string::TString wstrData;
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
	catch(const std::exception& e)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected std exception encountered in " << __FUNCTION__ << L": " << e.what();
		return E_FAIL;
	}
	catch(...)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected other exception encountered in " << __FUNCTION__ << L".";
		return E_FAIL;
	}
}

HRESULT CMenuExt::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return HandleMenuMsg2(uMsg, wParam, lParam, nullptr);
}

HRESULT CMenuExt::HandleMenuMsg2(UINT uMsg, WPARAM /*wParam*/, LPARAM lParam, LRESULT* /*plResult*/)
{
	try
	{
		LOG_DEBUG(m_spLog) << L"Handle menu message (2)";

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

			SIZE size = { 0 };
			GetTextExtentPoint32(hDC, spSelectedItem->GetLocalName().c_str(), boost::numeric_cast<int>(spSelectedItem->GetLocalName().GetLength()), &size);

			// restore old settings
			SelectObject(hDC, hOldFont);
			ReleaseDC(hDesktop, hDC);
			DeleteObject(hFont);

			// set
			lpmis->itemWidth = size.cx + GetSystemMetrics(SM_CXMENUCHECK) + 2 * GetSystemMetrics(SM_CXSMICON);
			lpmis->itemHeight = std::max<int>(size.cy + 3, GetSystemMetrics(SM_CYMENU) + 3);

			break;
		}
		}

		return S_OK;
	}
	catch(const std::exception& e)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected std exception encountered in " << __FUNCTION__ << L": " << e.what();
		return E_FAIL;
	}
	catch(...)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected other exception encountered in " << __FUNCTION__ << L".";
		return E_FAIL;
	}
}

HRESULT CMenuExt::DrawMenuItem(LPDRAWITEMSTRUCT lpdis)
{
	try
	{
		LOG_DEBUG(m_spLog) << L"Drawing menu item";

		if(!lpdis)
		{
			LOG_ERROR(m_spLog) << L"Missing argument";
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

		DrawText(lpdis->hDC, spSelectedItem->GetLocalName().c_str(), -1, &rcText, DT_LEFT | DT_SINGLELINE | DT_VCENTER);

		return S_OK;
	}
	catch(const std::exception& e)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected std exception encountered in " << __FUNCTION__ << L": " << e.what();
		return E_FAIL;
	}
	catch(...)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected other exception encountered in " << __FUNCTION__ << L".";
		return E_FAIL;
	}
}

STDMETHODIMP CMenuExt::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax)
{
	try
	{
		LOG_DEBUG(m_spLog) << L"Retrieving command string for cmd " << idCmd;

		memset(pszName, 0, cchMax);

		if(uFlags != GCS_HELPTEXTW && uFlags != GCS_HELPTEXTA)
			return S_OK;

		// check options
		HRESULT hResult = ShellExtensionVerifier::IsShellExtEnabled(m_piShellExtControl);
		if(FAILED(hResult))
			return hResult;
		if(hResult == S_FALSE)
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
	catch(const std::exception& e)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected std exception encountered in " << __FUNCTION__ << L": " << e.what();
		return E_FAIL;
	}
	catch(...)
	{
		LOG_CRITICAL(m_spLog) << L"Unexpected other exception encountered in " << __FUNCTION__ << L".";
		return E_FAIL;
	}
}
