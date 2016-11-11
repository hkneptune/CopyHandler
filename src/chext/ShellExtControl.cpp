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
#include <comutil.h>
#include "ShellExtControl.h"
#include "../common/version.h"
#include "Logger.h"
#include "../libchcore/TIpcMutexLock.h"

CShellExtControl::CShellExtControl() :
	m_pShellExtData(nullptr),
	m_hMemory(nullptr),
	m_mutex(L"CHShellExtControlDataMutex"),
	m_spLog(GetLogger(L"ShellExtControl"))
{
	LOG_DEBUG(m_spLog) << L"Constructing CShellExtControl";
}

CShellExtControl::~CShellExtControl()
{
	if(m_pShellExtData)
	{
		UnmapViewOfFile((LPVOID)m_pShellExtData); 

		// Close the process's handle to the file-mapping object.
		CloseHandle(m_hMemory); 
	}
}

STDMETHODIMP CShellExtControl::GetVersion(LONG* plVersion, BSTR* pbstrVersion)
{
	try
	{
		LOG_DEBUG(m_spLog) << "Retrieving version";

		HRESULT hResult = Initialize();
		if(FAILED(hResult))
		{
			LOG_ERROR(m_spLog) << L"CShellExtControl initialization failed";
			return hResult;
		}

		if(!plVersion || !pbstrVersion || (*pbstrVersion))
		{
			LOG_ERROR(m_spLog) << "Invalid arguments.";
			return E_INVALIDARG;
		}

		(*plVersion) = PRODUCT_VERSION1 << 24 | PRODUCT_VERSION2 << 16 | PRODUCT_VERSION3 << 8 | PRODUCT_VERSION4;
		_bstr_t strVer(SHELLEXT_PRODUCT_FULL_VERSION);
		*pbstrVersion = strVer.Detach();

		LOG_DEBUG(m_spLog) << LOG_PARAMS2(*plVersion, *pbstrVersion);

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

STDMETHODIMP CShellExtControl::SetFlags(LONG lFlags, LONG lMask)
{
	try
	{
		LOG_DEBUG(m_spLog) << L"Setting flags: " << LOG_PARAMS2(lFlags, lMask);

		HRESULT hResult = Initialize();
		if(FAILED(hResult))
		{
			LOG_ERROR(m_spLog) << L"CShellExtControl initialization failed";
			return hResult;
		}

		if(!m_pShellExtData)
		{
			LOG_ERROR(m_spLog) << "Wrong internal state.";
			return E_FAIL;
		}

		chcore::TIpcMutexLock lock(m_mutex);
		m_pShellExtData->m_lFlags = (m_pShellExtData->m_lFlags & ~lMask) | (lFlags & lMask);

		LOG_DEBUG(m_spLog) << L"Set flags: " << LOG_PARAM(m_pShellExtData->m_lFlags);

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

STDMETHODIMP CShellExtControl::GetFlags(LONG* plFlags)
{
	try
	{
		LOG_DEBUG(m_spLog) << "Retrieving flags";

		HRESULT hResult = Initialize();
		if(FAILED(hResult))
		{
			LOG_ERROR(m_spLog) << L"CShellExtControl initialization failed";
			return hResult;
		}

		if(!m_pShellExtData)
		{
			LOG_ERROR(m_spLog) << "Wrong internal state.";
			return E_FAIL;
		}

		if(!plFlags)
		{
			LOG_ERROR(m_spLog) << "Invalid argument.";
			return E_INVALIDARG;
		}

		chcore::TIpcMutexLock lock(m_mutex);

		(*plFlags) = m_pShellExtData->m_lFlags;

		LOG_DEBUG(m_spLog) << "Returning flags: " << LOG_PARAM(m_pShellExtData->m_lFlags);

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

HRESULT CShellExtControl::Initialize()
{
	if(m_bInitialized)
		return S_OK;

	// create protection mutex
	chcore::TIpcMutexLock lock(m_mutex);

	// memory mapped file
	m_hMemory = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(SHELLEXT_DATA), _T("CHShellExtControlData"));    // name of map object
	DWORD dwLastError = GetLastError();	// NOTE: last error is needed also for success case (for already exists status)
	if(!m_hMemory)
	{
		LOG_HRESULT(m_spLog, dwLastError) << L"Cannot create file mapping.";
		return E_FAIL;
	}

	m_pShellExtData = (SHELLEXT_DATA*)MapViewOfFile(m_hMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if(!m_pShellExtData)
	{
		DWORD dwError = GetLastError();		// NOTE: do not overwrite dwLastError, as the value is needed later

		LOG_HRESULT(m_spLog, dwError) << L"Cannot map view of file.";
		CloseHandle(m_hMemory);
		m_hMemory = nullptr;
		return E_FAIL;
	}

	if(dwLastError != ERROR_ALREADY_EXISTS)
	{
		if(dwLastError == ERROR_SUCCESS)
		{
			LOG_DEBUG(m_spLog) << L"Copy Handler is not running. Disabling shell extension.";
		}
		else
		{
			LOG_HRESULT(m_spLog, dwLastError) << L"Copy Handler is not running. Disabling shell extension.";
		}
		m_pShellExtData->m_lFlags = 0;
	}

	m_bInitialized = true;
	return S_OK;
}
