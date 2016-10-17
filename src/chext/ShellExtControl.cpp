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

CShellExtControl::CShellExtControl() :
	m_pShellExtData(nullptr),
	m_hMemory(nullptr),
	m_hMutex(nullptr),
	m_spLog(GetLogger(L"ShellExtControl"))
{
	LOG_DEBUG(m_spLog) << L"Constructing CShellExtControl";

	// create protection mutex
	m_hMutex = ::CreateMutex(nullptr, FALSE, _T("CHShellExtControlDataMutex"));
	if(!m_hMutex)
	{
		LOG_ERROR(m_spLog) << L"Cannot create mutex.";
		return;
	}

	DWORD dwRes = WaitForSingleObject(m_hMutex, 10000);
	if(dwRes != WAIT_OBJECT_0)
	{
		LOG_ERROR(m_spLog) << L"Timeout or fail waiting for mutex.";
		ReleaseMutex(m_hMutex);
		return;
	}
	
	// memory mapped file
	m_hMemory = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(SHELLEXT_DATA), _T("CHShellExtControlData"));    // name of map object
	DWORD dwLastError = GetLastError();	// NOTE: last error is needed also for success case (for already exists status)
	if(!m_hMemory)
	{
		LOG_HRESULT(m_spLog, dwLastError) << L"Cannot create file mapping.";
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
		return;
	}

	m_pShellExtData = (SHELLEXT_DATA*)MapViewOfFile(m_hMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if(!m_pShellExtData)
	{
		DWORD dwError = GetLastError();		// NOTE: do not overwrite dwLastError, as the value is needed later

		LOG_HRESULT(m_spLog, dwError) << L"Cannot map view of file.";
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
		CloseHandle(m_hMemory);
		m_hMemory = nullptr;
		return;
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
		m_pShellExtData->m_lID = GetTickCount();
	}

	ReleaseMutex(m_hMutex);
}

CShellExtControl::~CShellExtControl()
{
	if(m_pShellExtData)
	{
		UnmapViewOfFile((LPVOID)m_pShellExtData); 

		// Close the process's handle to the file-mapping object.
		CloseHandle(m_hMemory); 
	}

	if(m_hMutex)
		CloseHandle(m_hMutex);
}

STDMETHODIMP CShellExtControl::GetVersion(LONG* plVersion, BSTR* pbstrVersion)
{
	LOG_DEBUG(m_spLog) << "Retrieving version";

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

STDMETHODIMP CShellExtControl::SetFlags(LONG lFlags, LONG lMask)
{
	LOG_DEBUG(m_spLog) << L"Setting flags: " << LOG_PARAMS2(lFlags, lMask);

	if(!m_hMutex || !m_pShellExtData)
	{
		LOG_ERROR(m_spLog) << "Wrong internal state.";
		return E_FAIL;
	}

	DWORD dwRes = WaitForSingleObject(m_hMutex, 10000);
	if(dwRes != WAIT_OBJECT_0)
	{
		LOG_ERROR(m_spLog) << "Failed waiting for mutex.";
		return E_FAIL;
	}
	m_pShellExtData->m_lFlags = (m_pShellExtData->m_lFlags & ~lMask) | (lFlags & lMask);

	LOG_DEBUG(m_spLog) << L"Set flags: " << LOG_PARAM(m_pShellExtData->m_lFlags);

	ReleaseMutex(m_hMutex);

	return S_OK;
}

STDMETHODIMP CShellExtControl::GetFlags(LONG* plFlags)
{
	LOG_DEBUG(m_spLog) << "Retrieving flags";

	if(!m_hMutex || !m_pShellExtData)
	{
		LOG_ERROR(m_spLog) << "Wrong internal state.";
		return E_FAIL;
	}

	if(!plFlags)
	{
		LOG_ERROR(m_spLog) << "Invalid argument.";
		return E_INVALIDARG;
	}

	DWORD dwRes = WaitForSingleObject(m_hMutex, 10000);
	if(dwRes != WAIT_OBJECT_0)
	{
		LOG_ERROR(m_spLog) << "Failed waiting for mutex.";
		return E_FAIL;
	}

	(*plFlags) = m_pShellExtData->m_lFlags;

	LOG_DEBUG(m_spLog) << "Returning flags: " << LOG_PARAM(m_pShellExtData->m_lFlags);

	ReleaseMutex(m_hMutex);

	return S_OK;
}
