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
#include "TLogger.h"

CShellExtControl::CShellExtControl() :
	m_pShellExtData(NULL),
	m_hMemory(NULL),
	m_hMutex(NULL)
{
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << L"CShellExtControl::CShellExtControl()";

	// create protection mutex
	m_hMutex = ::CreateMutex(NULL, FALSE, _T("CHShellExtControlDataMutex"));
	if(!m_hMutex)
	{
		BOOST_LOG_SEV(rLogger, error) << L"Cannot create mutex.";
		return;
	}

	DWORD dwRes = WaitForSingleObject(m_hMutex, 10000);
	if(dwRes != WAIT_OBJECT_0)
	{
		BOOST_LOG_SEV(rLogger, error) << L"Timeout or fail waiting for mutex.";
		ReleaseMutex(m_hMutex);
		return;
	}
	
	// memory mapped file
	DWORD dwLastError = ERROR_SUCCESS;

	m_hMemory = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(SHELLEXT_DATA), _T("CHShellExtControlData"));    // name of map object
	if(!m_hMemory)
	{
		dwLastError = GetLastError();
		BOOST_LOG_SEV(rLogger, error) << L"Cannot create file mapping. Error code=" << dwLastError;
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
		return;
	}

	m_pShellExtData = (SHELLEXT_DATA*)MapViewOfFile(m_hMemory, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	if(!m_pShellExtData)
	{
		BOOST_LOG_SEV(rLogger, error) << L"Cannot map view of file.";
		ReleaseMutex(m_hMutex);
		CloseHandle(m_hMutex);
		CloseHandle(m_hMemory);
		m_hMemory = NULL;
		return;
	}

	if(dwLastError != ERROR_ALREADY_EXISTS)
	{
		BOOST_LOG_SEV(rLogger, debug) << L"Copy Handler is not running. Disabling shell extension.";
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
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << "CShellExtControl::GetVersion()";

	if(!plVersion || !pbstrVersion || (*pbstrVersion))
	{
		BOOST_LOG_SEV(rLogger, error) << "CShellExtControl::GetVersion(): Invalid arguments.";
		return E_INVALIDARG;
	}

	(*plVersion) = PRODUCT_VERSION1 << 24 | PRODUCT_VERSION2 << 16 | PRODUCT_VERSION3 << 8 | PRODUCT_VERSION4;
	_bstr_t strVer(SHELLEXT_PRODUCT_FULL_VERSION);
	*pbstrVersion = strVer.Detach();

	BOOST_LOG_SEV(rLogger, debug) << "CShellExtControl::GetVersion(): *plVersion=" << *plVersion << ", pbstrVersion=" << *pbstrVersion;

	return S_OK;
}

STDMETHODIMP CShellExtControl::SetFlags(LONG lFlags, LONG lMask)
{
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << "CShellExtControl::SetFlags(lFlags=" << lFlags << ", lMask=" << lMask << ")";

	if(!m_hMutex || !m_pShellExtData)
	{
		BOOST_LOG_SEV(rLogger, error) << "CShellExtControl::SetFlags(): Wrong internal state.";
		return E_FAIL;
	}

	DWORD dwRes = WaitForSingleObject(m_hMutex, 10000);
	if(dwRes != WAIT_OBJECT_0)
	{
		BOOST_LOG_SEV(rLogger, error) << "CShellExtControl::SetFlags(): Failed waiting for mutex.";
		return E_FAIL;
	}
	m_pShellExtData->m_lFlags = (m_pShellExtData->m_lFlags & ~lMask) | (lFlags & lMask);

	BOOST_LOG_SEV(rLogger, debug) << "CShellExtControl::SetFlags(): New flags=" << m_pShellExtData->m_lFlags;

	ReleaseMutex(m_hMutex);

	return S_OK;
}

STDMETHODIMP CShellExtControl::GetFlags(LONG* plFlags)
{
	TLogger& rLogger = Logger::get();
	BOOST_LOG_SEV(rLogger, debug) << "CShellExtControl::GetFlags()";

	if(!m_hMutex || !m_pShellExtData)
	{
		BOOST_LOG_SEV(rLogger, error) << "CShellExtControl::GetFlags(): wrong internal state.";
		return E_FAIL;
	}

	if(!plFlags)
	{
		BOOST_LOG_SEV(rLogger, error) << "CShellExtControl::GetFlags(): invalid argument.";
		return E_INVALIDARG;
	}

	DWORD dwRes = WaitForSingleObject(m_hMutex, 10000);
	if(dwRes != WAIT_OBJECT_0)
	{
		BOOST_LOG_SEV(rLogger, error) << "CShellExtControl::GetFlags(): failed waiting for mutex.";
		return E_FAIL;
	}

	(*plFlags) = m_pShellExtData->m_lFlags;

	BOOST_LOG_SEV(rLogger, debug) << "CShellExtControl::GetFlags(): returning flags=" << m_pShellExtData->m_lFlags;

	ReleaseMutex(m_hMutex);

	return S_OK;
}
