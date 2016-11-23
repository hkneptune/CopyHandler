// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TWorkerThreadController.cpp
/// @date  2010/09/04
/// @brief Contains implementation of class TWorkerThreadController.
// ============================================================================
#include "stdafx.h"
#include "TWorkerThreadController.h"
#include "TCoreWin32Exception.h"
#include "ErrorCodes.h"
#include <boost/thread/locks.hpp>

namespace chcore
{
	TWorkerThreadController::TWorkerThreadController() :
		m_hThread(nullptr),
		m_hKillThread(nullptr)
	{
		m_hKillThread = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (!m_hKillThread)
			throw TCoreWin32Exception(eErr_CannotCreateEvent, GetLastError(), L"Failed to create event", LOCATION);
	}

	TWorkerThreadController::~TWorkerThreadController()
	{
		try
		{
			StopThread();
			CloseHandle(m_hKillThread);
		}
		catch (...)
		{
		}
	}

	void TWorkerThreadController::StartThread(PTHREAD_START_ROUTINE pThreadFunction, PVOID pThreadParam, int iPriority)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

		RemoveZombieData(lock);

		if (m_hThread)
			throw TCoreException(eErr_ThreadAlreadyStarted, L"Thread was already started", LOCATION);

		// just in case reset the kill event to avoid early death of the thread to be created
		if (!::ResetEvent(m_hKillThread))
			throw TCoreWin32Exception(eErr_CannotResetEvent, GetLastError(), L"Failed to reset event", LOCATION);

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);

		m_hThread = ::CreateThread(nullptr, 0, pThreadFunction, pThreadParam, CREATE_SUSPENDED, nullptr);
		if (!m_hThread)
			throw TCoreWin32Exception(eErr_CannotCreateThread, GetLastError(), L"Failed to create thread", LOCATION);

		if (!::SetThreadPriority(m_hThread, iPriority))
		{
			DWORD dwLastError = GetLastError();

			CloseHandle(m_hThread);
			m_hThread = nullptr;

			throw TCoreWin32Exception(eErr_CannotChangeThreadPriority, dwLastError, L"Failed to set thread priority", LOCATION);
		}

		if (::ResumeThread(m_hThread) == (DWORD)-1)
		{
			DWORD dwLastError = GetLastError();

			CloseHandle(m_hThread);
			m_hThread = nullptr;

			throw TCoreWin32Exception(eErr_CannotResumeThread, dwLastError, L"Failed to resume thread", LOCATION);
		}
	}

	void TWorkerThreadController::SignalThreadToStop()
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

		SignalThreadToStop(lock);
	}

	void TWorkerThreadController::WaitForThreadToExit(DWORD dwMiliseconds)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

		DWORD dwRes = WaitForSingleObject(m_hThread, dwMiliseconds);
		if (dwRes == WAIT_OBJECT_0)
		{
			if (!::ResetEvent(m_hKillThread))
				throw TCoreWin32Exception(eErr_CannotResetEvent, GetLastError(), L"Failed to reset event", LOCATION);

			boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);

			CloseHandle(m_hThread);
			m_hThread = nullptr;
		}
		else
			throw TCoreWin32Exception(eErr_WaitingFailed, GetLastError(), L"Waiting failed", LOCATION);
	}

	void TWorkerThreadController::StopThread()
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

		SignalThreadToStop(lock);

		WaitForThreadToExit(lock);
	}

	void TWorkerThreadController::ChangePriority(int iPriority)
	{
		boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

		RemoveZombieData(lock);

		if (!m_hThread)
			return;

		if (m_hThread != nullptr)
		{
			if (::SuspendThread(m_hThread) == (DWORD)-1)
				throw TCoreWin32Exception(eErr_CannotSuspendThread, GetLastError(), L"Failed to suspend thread", LOCATION);

			if (!::SetThreadPriority(m_hThread, iPriority))
			{
				DWORD dwLastError = GetLastError();

				// try to resume thread priority cannot be changed
				DWORD dwResult = ::ResumeThread(m_hThread);
				dwResult;	// to avoid warnings in release builds
				BOOST_ASSERT(dwResult != (DWORD)-1);

				throw TCoreWin32Exception(eErr_CannotChangeThreadPriority, dwLastError, L"Failed to set thread priority", LOCATION);
			}

			if (::ResumeThread(m_hThread) == (DWORD)-1)
				throw TCoreWin32Exception(eErr_CannotResumeThread, GetLastError(), L"Failed to resume thread", LOCATION);
		}
	}

	bool TWorkerThreadController::KillRequested(DWORD dwWaitForSignal)
	{
		// this method does not have any mutexes, because it should be only called from within the thread
		// being controlled by this object. This implies that the thread is alive and running,
		// this class must exist because it should not be possible for the thread to exist and be active
		// when this object is out of scope, and so the m_hKillThread should be non-nullptr, since it is being destroyed
		// in destructor.
		return (m_hKillThread && WaitForSingleObject(m_hKillThread, dwWaitForSignal) == WAIT_OBJECT_0);
	}

	HANDLE TWorkerThreadController::GetKillThreadHandle() const
	{
		return m_hKillThread;
	}

	void TWorkerThreadController::RemoveZombieData(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock)
	{
		// if thread is already stopped, then there is nothing to do
		if (!m_hThread)
			return;

		// thread already stopped?
		if (WaitForSingleObject(m_hThread, 0) == WAIT_OBJECT_0)
		{
			if (!::ResetEvent(m_hKillThread))
				throw TCoreWin32Exception(eErr_CannotResetEvent, GetLastError(), L"Failed to reset event", LOCATION);

			boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(rUpgradeLock);

			CloseHandle(m_hThread);
			m_hThread = nullptr;
		}
	}

	void TWorkerThreadController::SignalThreadToStop(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock)
	{
		RemoveZombieData(rUpgradeLock);
		if (!m_hThread)
			return;

		if (!::SetEvent(m_hKillThread))
			throw TCoreWin32Exception(eErr_CannotSetEvent, GetLastError(), L"Failed to set event", LOCATION);
	}

	void TWorkerThreadController::WaitForThreadToExit(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock, DWORD dwMiliseconds)
	{
		if (!m_hThread)
			return;

		DWORD dwRes = WaitForSingleObject(m_hThread, dwMiliseconds);
		if (dwRes == WAIT_OBJECT_0)
		{
			if (!::ResetEvent(m_hKillThread))
				throw TCoreWin32Exception(eErr_CannotResetEvent, GetLastError(), L"Failed to reset event", LOCATION);

			boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(rUpgradeLock);

			CloseHandle(m_hThread);
			m_hThread = nullptr;
		}
		else
			throw TCoreWin32Exception(eErr_WaitingFailed, GetLastError(), L"Failed to wait for object", LOCATION);
	}
}
