// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TSharedMemory.cpp
/// @date  2011/05/03
/// @brief Contains implementation of shared memory handling classes.
// ============================================================================
#include "stdafx.h"
#include "TSharedMemory.h"
#include <boost/cast.hpp>
#include "ErrorCodes.h"
#include "TCoreException.h"

namespace chcore
{
#define MUTEX_SUFFIX _T("_Mutex")

	class TMutexLock
	{
	public:
		TMutexLock(HANDLE hMutex) :
			m_hMutex(hMutex)
		{
			if (m_hMutex)
			{
				DWORD dwRes = WaitForSingleObject(hMutex, 10000);
				if (dwRes != WAIT_OBJECT_0)
					THROW_CORE_EXCEPTION(eErr_MutexTimedOut);
			}
		}

		~TMutexLock()
		{
			if (m_hMutex)
				ReleaseMutex(m_hMutex);
		}

	private:
		HANDLE m_hMutex;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	// TSharedMemory class

	TSharedMemory::TSharedMemory() :
		m_hFileMapping(NULL),
		m_pMappedMemory(NULL),
		m_hMutex(NULL),
		m_stSize(0)
	{
	}

	TSharedMemory::~TSharedMemory()
	{
		Close();
	}

	void TSharedMemory::Create(const wchar_t* pszName, shm_size_t stSize)
	{
		if (!pszName || pszName[0] == _T('\0') || stSize == 0)
			THROW_CORE_EXCEPTION(eErr_InvalidArgument);

		Close();

		try
		{
			std::wstring wstrMutexName = pszName;
			wstrMutexName += MUTEX_SUFFIX;

			SECURITY_DESCRIPTOR secDesc;
			if (!InitializeSecurityDescriptor(&secDesc, SECURITY_DESCRIPTOR_REVISION))
				THROW_CORE_EXCEPTION(eErr_CannotOpenSharedMemory);

			SECURITY_ATTRIBUTES secAttr;
			secAttr.nLength = sizeof(secAttr);
			secAttr.bInheritHandle = FALSE;
			secAttr.lpSecurityDescriptor = &secDesc;

			if (!SetSecurityDescriptorDacl(secAttr.lpSecurityDescriptor, TRUE, 0, FALSE))
				THROW_CORE_EXCEPTION(eErr_CannotOpenSharedMemory);

			m_hMutex = ::CreateMutex(&secAttr, FALSE, wstrMutexName.c_str());
			if (!m_hMutex)
				THROW_CORE_EXCEPTION(eErr_CannotOpenSharedMemory);

			m_hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, &secAttr, PAGE_READWRITE, 0, boost::numeric_cast<DWORD>(stSize + sizeof(size_t)), pszName);
			if (!m_hFileMapping)
				THROW_CORE_EXCEPTION(eErr_CannotOpenSharedMemory);
			else if (GetLastError() == ERROR_ALREADY_EXISTS)
				THROW_CORE_EXCEPTION(eErr_SharedMemoryAlreadyExists);		// shared memory already exists - cannot guarantee that the size is correct

			// Get a pointer to the file-mapped shared memory.
			m_pMappedMemory = (BYTE*)MapViewOfFile(m_hFileMapping, FILE_MAP_WRITE, 0, 0, 0);
			if (!m_pMappedMemory)
				THROW_CORE_EXCEPTION(eErr_CannotOpenSharedMemory);
		}
		catch (...)
		{
			Close();
			throw;
		}

		TMutexLock lock(m_hMutex);

		m_stSize = stSize + sizeof(shm_size_t);
		*(shm_size_t*)m_pMappedMemory = sizeof(shm_size_t);  // no data inside (set just in case)
	}

	void TSharedMemory::Create(const wchar_t* pszName, const TString& wstrData)
	{
		Create(pszName, (const BYTE*)wstrData.c_str(), boost::numeric_cast<shm_size_t>((wstrData.GetLength() + 1) * sizeof(wchar_t)));
	}

	void TSharedMemory::Create(const wchar_t* pszName, const BYTE* pbyData, shm_size_t stSize)
	{
		Create(pszName, stSize);

		TMutexLock lock(m_hMutex);

		*(shm_size_t*)m_pMappedMemory = stSize;
		memcpy(m_pMappedMemory + sizeof(shm_size_t), pbyData, stSize);
	}

	void TSharedMemory::Open(const wchar_t* pszName)
	{
		Close();

		try
		{
			m_hFileMapping = OpenFileMapping(FILE_MAP_READ, FALSE, pszName);
			if (!m_hFileMapping)
				THROW_CORE_EXCEPTION(eErr_CannotOpenSharedMemory);

			// Get a pointer to the file-mapped shared memory.
			m_pMappedMemory = (BYTE*)MapViewOfFile(m_hFileMapping, FILE_MAP_READ, 0, 0, 0);
			if (!m_pMappedMemory)
				THROW_CORE_EXCEPTION(eErr_CannotOpenSharedMemory);
		}
		catch (...)
		{
			Close();
			throw;
		}

		TMutexLock lock(m_hMutex);

		m_stSize = *(shm_size_t*)m_pMappedMemory + sizeof(shm_size_t);
	}

	void TSharedMemory::Close() throw()
	{
		try
		{
			if (m_hMutex)
			{
				CloseHandle(m_hMutex);
				m_hMutex = NULL;
			}

			if (m_pMappedMemory)
			{
				UnmapViewOfFile(m_pMappedMemory);
				m_pMappedMemory = NULL;
			}

			// Close the process's handle to the file-mapping object.
			if (m_hFileMapping)
			{
				CloseHandle(m_hFileMapping);
				m_hFileMapping = NULL;
			}
		}
		catch (...)
		{
		}
	}

	void TSharedMemory::Read(TString& wstrData) const
	{
		if (!m_hFileMapping || !m_pMappedMemory || m_stSize <= sizeof(shm_size_t))
			THROW_CORE_EXCEPTION(eErr_SharedMemoryNotOpen);

		TMutexLock lock(m_hMutex);

		shm_size_t stByteSize = *(shm_size_t*)m_pMappedMemory;
		if ((stByteSize % 2) != 0)
			THROW_CORE_EXCEPTION(eErr_SharedMemoryInvalidFormat);

		const wchar_t* pszRealData = (const wchar_t*)(m_pMappedMemory + sizeof(shm_size_t));
		shm_size_t stCharCount = stByteSize / 2;

		if (pszRealData[stCharCount - 1] != _T('\0'))
			THROW_CORE_EXCEPTION(eErr_SharedMemoryInvalidFormat);

		wstrData = pszRealData;
	}

	void TSharedMemory::Write(const TString& wstrData)
	{
		Write((const BYTE*)wstrData.c_str(), boost::numeric_cast<shm_size_t>((wstrData.GetLength() + 1) * sizeof(wchar_t)));
	}

	void TSharedMemory::Write(const BYTE* pbyData, shm_size_t stSize)
	{
		if (stSize + sizeof(shm_size_t) > m_stSize)
			THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

		TMutexLock lock(m_hMutex);

		*(shm_size_t*)m_pMappedMemory = stSize;
		memcpy(m_pMappedMemory + sizeof(shm_size_t), pbyData, stSize);
	}

	const BYTE* TSharedMemory::GetData() const
	{
		if (!m_hFileMapping || !m_pMappedMemory || m_stSize <= sizeof(shm_size_t))
			return NULL;

		return (BYTE*)m_pMappedMemory + sizeof(shm_size_t);
	}

	BYTE* TSharedMemory::GetData()
	{
		if (!m_hFileMapping || !m_pMappedMemory || m_stSize <= sizeof(shm_size_t))
			return NULL;

		return (BYTE*)m_pMappedMemory + sizeof(shm_size_t);
	}

	TSharedMemory::shm_size_t TSharedMemory::GetSharedMemorySize() const
	{
		if (!m_hFileMapping || !m_pMappedMemory)
			return 0;
		return m_stSize;
	}

	TSharedMemory::shm_size_t TSharedMemory::GetDataSize() const
	{
		if (!m_hFileMapping || !m_pMappedMemory || m_stSize <= sizeof(shm_size_t))
			return 0;

		TMutexLock lock(m_hMutex);

		return *(shm_size_t*)m_pMappedMemory;
	}
}
