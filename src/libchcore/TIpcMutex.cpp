// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#include "stdafx.h"
#include "TIpcMutex.h"
#include "TCoreWin32Exception.h"

namespace chcore
{
	TIpcMutex::TIpcMutex()
	{
	}

	TIpcMutex::TIpcMutex(const wchar_t* pszName)
	{
		m_hMutex = ::CreateMutex(nullptr, FALSE, pszName);
		if(!m_hMutex)
			throw TCoreWin32Exception(eErr_CannotCreateMutex, GetLastError(), L"Cannot create mutex", LOCATION);
	}

	TIpcMutex::~TIpcMutex()
	{
		try
		{
			Close();
		}
		catch (const std::exception& e)
		{
		}
	}

	void TIpcMutex::CreateMutex(const wchar_t* pszName)
	{
		Close();

		m_hMutex = ::CreateMutex(nullptr, FALSE, pszName);
		if(!m_hMutex)
			throw TCoreWin32Exception(eErr_CannotCreateMutex, GetLastError(), L"Cannot create mutex", LOCATION);
	}

	void TIpcMutex::Lock(DWORD dwTimeout)
	{
		if(!m_hMutex)
			throw TCoreException(eErr_InvalidData, L"Mutex not created. Cannot lock.", LOCATION);
		if(m_bLocked)
			throw TCoreException(eErr_InvalidData, L"Mutex already locked.", LOCATION);

		DWORD dwRes = WaitForSingleObject(m_hMutex, dwTimeout);
		if(dwRes != WAIT_OBJECT_0)
		{
			ReleaseMutex(m_hMutex);
			throw TCoreWin32Exception(eErr_CannotCreateMutex, GetLastError(), L"Wait for mutex failed", LOCATION);
		}

		m_bLocked = true;
	}

	void TIpcMutex::Unlock()
	{
		if(!m_hMutex)
			throw TCoreException(eErr_InvalidData, L"Mutex not created. Cannot lock.", LOCATION);
		if(!m_bLocked)
			throw TCoreException(eErr_InvalidData, L"Mutex not locked. Cannot unlock.", LOCATION);

		ReleaseMutex(m_hMutex);
		m_bLocked = false;
	}

	bool TIpcMutex::IsLocked() const
	{
		return m_bLocked;
	}

	void TIpcMutex::Close()
	{
		if (m_bLocked)
			Unlock();

		if (m_hMutex)
			CloseHandle(m_hMutex);
	}
}
