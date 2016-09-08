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
#include "../Common/ipcstructs.h"
#include "../libchcore/TTaskDefinition.h"
#include <boost/shared_array.hpp>
#include "ShellPathsHelpers.h"
#include "../common/TShellExtMenuConfig.h"
#include "../libchcore/TSharedMemory.h"
#include "TLogger.h"
#include "ShellExtensionVerifier.h"
#include "HResultFormatter.h"

/////////////////////////////////////////////////////////////////////////////
// CDropMenuExt

CDropMenuExt::CDropMenuExt() :
	m_piShellExtControl(nullptr)
{
	BOOST_LOG_FUNC();

	HRESULT hResult = CoCreateInstance(CLSID_CShellExtControl, nullptr, CLSCTX_ALL, IID_IShellExtControl, (void**)&m_piShellExtControl);

	TLogger& rLogger = Logger::get();
	BOOST_LOG_HRESULT(rLogger, hResult) << L"CoCreateInstance()";
}

CDropMenuExt::~CDropMenuExt()
{
	if(m_piShellExtControl)
	{
		m_piShellExtControl->Release();
		m_piShellExtControl = nullptr;
	}
}

STDMETHODIMP CDropMenuExt::Initialize(LPCITEMIDLIST pidlFolder, IDataObject* piDataObject, HKEY /*hkeyProgID*/)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

	// When called:
	// 1. R-click on a directory
	// 2. R-click on a directory background
	// 3. Pressed Ctrl+C, Ctrl+X on a specified file/directory

	if(!pidlFolder && !piDataObject)
	{
		BOOST_LOG_SEV(rLogger, error) << L"Missing both pointers.";
		return E_FAIL;
	}

	if(!pidlFolder || !piDataObject)
		BOOST_LOG_SEV(rLogger, warning) << L"Missing at least one parameter - it's unexpected.";

	if(!piDataObject)
	{
		BOOST_LOG_SEV(rLogger, error) << L"Missing piDataObject.";
		return E_FAIL;
	}

	// check options
	HWND hWnd = ShellExtensionVerifier::VerifyShellExt(m_piShellExtControl);
	if(hWnd == nullptr)
		return E_FAIL;

	HRESULT hResult = ReadShellConfig();
	if(SUCCEEDED(hResult))
		hResult = m_tShellExtData.GatherDataFromInitialize(pidlFolder, piDataObject);

	BOOST_LOG_HRESULT(rLogger, hResult) << L"";

	return hResult;
}

STDMETHODIMP CDropMenuExt::QueryContextMenu(HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT /*uFlags*/)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

	// check options
	HWND hWnd = ShellExtensionVerifier::VerifyShellExt(m_piShellExtControl);
	if(!hWnd)
		return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);

	// retrieve the default menu item; if not available, fallback to the default heuristics
	m_tShellExtData.ReadDefaultSelectionStateFromMenu(hMenu);

	// retrieve the action information to be performed
	TShellExtData::EActionSource eActionSource = m_tShellExtData.GetActionSource();

	// determine if we want to perform override based on user options and detected action source
	bool bIntercept = (m_tShellExtMenuConfig.GetInterceptDragAndDrop() && eActionSource == TShellExtData::eSrc_DropMenu ||
						m_tShellExtMenuConfig.GetInterceptKeyboardActions() && eActionSource == TShellExtData::eSrc_Keyboard ||
						m_tShellExtMenuConfig.GetInterceptCtxMenuActions() && eActionSource == TShellExtData::eSrc_CtxMenu);

	TShellMenuItemPtr spRootMenuItem = m_tShellExtMenuConfig.GetCommandRoot();
	m_tContextMenuHandler.Init(spRootMenuItem, hMenu, idCmdFirst, indexMenu, m_tShellExtData, m_tShellExtMenuConfig.GetShowShortcutIcons(), bIntercept);

	return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, m_tContextMenuHandler.GetLastCommandID() - idCmdFirst + 1);
}

STDMETHODIMP CDropMenuExt::InvokeCommand(LPCMINVOKECOMMANDINFO lpici)
{
	BOOST_LOG_FUNC();

	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"";

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

STDMETHODIMP CDropMenuExt::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax)
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

STDMETHODIMP CDropMenuExt::HandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	uMsg; wParam; lParam;
	return S_FALSE;
}

STDMETHODIMP CDropMenuExt::HandleMenuMsg2(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* /*plResult*/)
{
	uMsg; wParam; lParam;
	ATLTRACE(_T("CDropMenuExt::HandleMenuMsg2(): uMsg = %lu, wParam = %lu, lParam = %lu\n"), uMsg, wParam, lParam);
	return S_FALSE;
}

HRESULT CDropMenuExt::ReadShellConfig()
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
		if(::SendMessage(hWnd, WM_GETCONFIG, eLocation_DragAndDropMenu, ulSHMID) != TRUE)
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
