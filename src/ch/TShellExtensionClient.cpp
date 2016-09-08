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
#include "TShellExtensionClient.h"
#include "objbase.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

TShellExtensionClient::TShellExtensionClient() :
	m_piShellExtControl(nullptr),
	m_bInitialized(false)
{
}

TShellExtensionClient::~TShellExtensionClient()
{
	FreeControlInterface();
	UninitializeCOM();
}

HRESULT TShellExtensionClient::InitializeCOM()
{
	if(m_bInitialized)
		return S_FALSE;

	HRESULT hResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if(SUCCEEDED(hResult))
		m_bInitialized = true;

	return hResult;
}

void TShellExtensionClient::UninitializeCOM()
{
	if(m_bInitialized)
	{
		CoUninitialize();
		m_bInitialized = false;
	}
}

HRESULT TShellExtensionClient::RegisterShellExtDll(const CString& strPath, long lClientVersion, long& rlExtensionVersion, CString& rstrExtensionStringVersion)
{
	if(strPath.IsEmpty())
		return E_INVALIDARG;

	HRESULT hResult = S_OK;

	if(SUCCEEDED(hResult))
		hResult = InitializeCOM();

	// get rid of the interface, so we can at least try to re-register
	if(SUCCEEDED(hResult))
		FreeControlInterface();

	// first try - load dll and register it manually.
	// if failed - try by loading extension manually (would fail on vista when running as user)
	if(SUCCEEDED(hResult))
	{
		HRESULT (STDAPICALLTYPE *pfn)(void) = nullptr;
		HINSTANCE hMod = LoadLibrary(strPath);	// load the dll
		if(hMod == nullptr)
			hResult = HRESULT_FROM_WIN32(GetLastError());
		if(SUCCEEDED(hResult) && !hMod)
			hResult = E_FAIL;
		if(SUCCEEDED(hResult))
		{
			(FARPROC&)pfn = GetProcAddress(hMod, "DllRegisterServer");
			if(pfn == nullptr)
				hResult = E_FAIL;
			if(SUCCEEDED(hResult))
				hResult = (*pfn)();

			FreeLibrary(hMod);
		}
	}

	// if previous operation failed (ie. vista system) - try running regsvr32 with elevated privileges
	if(SCODE_CODE(hResult) == ERROR_ACCESS_DENIED)
	{
		// try with regsvr32
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(sei));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_UNICODE;
		sei.lpVerb = _T("runas");
		sei.lpFile = _T("regsvr32.exe");
		CString strParams = CString(_T("/s \"")) + strPath + CString(_T("\""));
		sei.lpParameters = strParams;
		sei.nShow = SW_SHOW;

		if(!ShellExecuteEx(&sei))
			hResult = E_FAIL;
		else
			hResult = S_OK;
	}

	if(SUCCEEDED(hResult))
	{
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

		// NOTE: we are re-trying to enable the shell extension through our notification interface
		// in case of class-not-registered error because (it seems) system needs some time to process
		// DLL's self registration and usually the first call fails.
		int iTries = 3;
		do
		{
			hResult = EnableExtensionIfCompatible(lClientVersion, rlExtensionVersion, rstrExtensionStringVersion);
			if(hResult == REGDB_E_CLASSNOTREG)
			{
				ATLTRACE(_T("Class CLSID_CShellExtControl still not registered...\r\n"));
				Sleep(500);
			}
		}
		while(--iTries && hResult == REGDB_E_CLASSNOTREG);
	}

	return hResult;
}

HRESULT TShellExtensionClient::UnRegisterShellExtDll(const CString& strPath)
{
	if(strPath.IsEmpty())
		return E_INVALIDARG;

	HRESULT hResult = S_OK;

	if(SUCCEEDED(hResult))
		hResult = InitializeCOM();

	// get rid of the interface if unregistering
	if(SUCCEEDED(hResult))
		FreeControlInterface();

	// first try - load dll and register it manually.
	// if failed - try by loading extension manually (would fail on vista when running as user)
	if(SUCCEEDED(hResult))
	{
		HRESULT (STDAPICALLTYPE *pfn)(void) = nullptr;
		HINSTANCE hMod = LoadLibrary(strPath);	// load the dll
		if(hMod == nullptr)
			hResult = HRESULT_FROM_WIN32(GetLastError());
		if(SUCCEEDED(hResult) && !hMod)
			hResult = E_FAIL;
		if(SUCCEEDED(hResult))
		{
			(FARPROC&)pfn = GetProcAddress(hMod, "DllUnregisterServer");
			if(pfn == nullptr)
				hResult = E_FAIL;
			if(SUCCEEDED(hResult))
				hResult = (*pfn)();

			FreeLibrary(hMod);
		}
	}

	// if previous operation failed (ie. vista system) - try running regsvr32 with elevated privileges
	if(SCODE_CODE(hResult) == ERROR_ACCESS_DENIED)
	{
		// try with regsvr32
		SHELLEXECUTEINFO sei;
		memset(&sei, 0, sizeof(sei));
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_UNICODE;
		sei.lpVerb = _T("runas");
		sei.lpFile = _T("regsvr32.exe");
		CString strParams = CString(_T("/u /s \"")) + strPath + CString(_T("\""));
		sei.lpParameters = strParams;
		sei.nShow = SW_SHOW;

		if(!ShellExecuteEx(&sei))
			hResult = E_FAIL;
		else
			hResult = S_OK;
	}

	if(SUCCEEDED(hResult))
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);

	return hResult;
}

HRESULT TShellExtensionClient::EnableExtensionIfCompatible(long lClientVersion, long& rlExtensionVersion, CString& rstrExtensionStringVersion)
{
	rlExtensionVersion = 0;
	rstrExtensionStringVersion.Empty();

	BSTR bstrVersion = nullptr;

	HRESULT hResult = RetrieveControlInterface();
	if(SUCCEEDED(hResult) && !m_piShellExtControl)
		hResult = E_FAIL;
	if(SUCCEEDED(hResult))
		hResult = m_piShellExtControl->GetVersion(&rlExtensionVersion, &bstrVersion);
	if(SUCCEEDED(hResult))
	{
		// enable or disable extension - currently we only support extension from strictly the same version as CH
		bool bVersionMatches = (lClientVersion == rlExtensionVersion);
		hResult = m_piShellExtControl->SetFlags(bVersionMatches ? eShellExt_Enabled : 0, eShellExt_Enabled);
		if(SUCCEEDED(hResult))
			hResult = bVersionMatches ? S_OK : S_FALSE;
	}

	// do not overwrite S_OK/S_FALSE status after this line - it needs to be propagated upwards
	if(bstrVersion)
	{
		rstrExtensionStringVersion = bstrVersion;
		::SysFreeString(bstrVersion);
	}

	return hResult;
}

void TShellExtensionClient::Close()
{
	FreeControlInterface();
}

HRESULT TShellExtensionClient::RetrieveControlInterface()
{
	HRESULT hResult = InitializeCOM();
	if(SUCCEEDED(hResult))
		hResult = CoCreateInstance(CLSID_CShellExtControl, nullptr, CLSCTX_ALL, IID_IShellExtControl, (void**)&m_piShellExtControl);
	if(SUCCEEDED(hResult) && !m_piShellExtControl)
		hResult = E_FAIL;

	return hResult;
}

void TShellExtensionClient::FreeControlInterface()
{
	if(m_piShellExtControl)
	{
		m_piShellExtControl->Release();
		m_piShellExtControl = nullptr;
	}
}
