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

BEGIN_CHCORE_NAMESPACE

TWorkerThreadController::TWorkerThreadController() :
	m_hThread(NULL),
	m_hKillThread(NULL)
{
	m_hKillThread = CreateEvent(NULL, TRUE, FALSE, NULL);
	if(!m_hKillThread)
		THROW_CORE_EXCEPTION_WIN32(eErr_CannotCreateEvent, GetLastError());
}

TWorkerThreadController::~TWorkerThreadController()
{
	try
	{
		StopThread();
		CloseHandle(m_hKillThread);
	}
	catch(...)
	{
	}
}

void TWorkerThreadController::StartThread(PTHREAD_START_ROUTINE pThreadFunction, PVOID pThreadParam, int iPriority)
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	RemoveZombieData(lock);

	if(m_hThread)
		THROW_CORE_EXCEPTION(eErr_ThreadAlreadyStarted);

	// just in case reset the kill event to avoid early death of the thread to be created
	if(!::ResetEvent(m_hKillThread))
		THROW_CORE_EXCEPTION_WIN32(eErr_CannotResetEvent, GetLastError());

	boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);

	m_hThread = ::CreateThread(NULL, 0, pThreadFunction, pThreadParam, CREATE_SUSPENDED, NULL);
	if(!m_hThread)
		THROW_CORE_EXCEPTION_WIN32(eErr_CannotCreateThread, GetLastError());

	if(!::SetThreadPriority(m_hThread, iPriority))
	{
		DWORD dwLastError = GetLastError();

		CloseHandle(m_hThread);
		m_hThread = NULL;

		THROW_CORE_EXCEPTION_WIN32(eErr_CannotChangeThreadPriority, dwLastError);
	}

	if(::ResumeThread(m_hThread) == (DWORD)-1)
	{
		DWORD dwLastError = GetLastError();

		CloseHandle(m_hThread);
		m_hThread = NULL;

		THROW_CORE_EXCEPTION_WIN32(eErr_CannotResumeThread, dwLastError);
	}
}

void TWorkerThreadController::SignalThreadToStop()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	SignalThreadToStop(lock);
}

void TWorkerThreadController::WaitForThreadToExit()
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	DWORD dwRes = WaitForSingleObject(m_hThread, INFINITE);
	if(dwRes == WAIT_OBJECT_0)
	{
		if(!::ResetEvent(m_hKillThread))
			THROW_CORE_EXCEPTION_WIN32(eErr_CannotResetEvent, GetLastError());

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	else
		THROW_CORE_EXCEPTION_WIN32(eErr_WaitingFailed, GetLastError());
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

	if(!m_hThread)
		return;

	if(m_hThread != NULL)
	{
		if(::SuspendThread(m_hThread) == (DWORD)-1)
			THROW_CORE_EXCEPTION_WIN32(eErr_CannotSuspendThread, GetLastError());

		if(!::SetThreadPriority(m_hThread, iPriority))
		{
			DWORD dwLastError = GetLastError();

			// try to resume thread priority cannot be changed
			DWORD dwResult = ::ResumeThread(m_hThread);
			BOOST_ASSERT(dwResult != (DWORD)-1);

			THROW_CORE_EXCEPTION_WIN32(eErr_CannotChangeThreadPriority, dwLastError);
		}

		if(::ResumeThread(m_hThread) == (DWORD)-1)
			THROW_CORE_EXCEPTION_WIN32(eErr_CannotResumeThread, GetLastError());
	}
}

bool TWorkerThreadController::KillRequested(DWORD dwWaitForSignal)
{
	// this method does not have any mutexes, because it should be only called from within the thread
	// being controlled by this object. This implies that the thread is alive and running,
	// this class must exist because it should not be possible for the thread to exist and be active
	// when this object is out of scope, and so the m_hKillThread should be non-NULL, since it is being destroyed
	// in destructor.
	return (m_hKillThread && WaitForSingleObject(m_hKillThread, dwWaitForSignal) == WAIT_OBJECT_0);
}

void TWorkerThreadController::RemoveZombieData(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock)
{
	// if thread is already stopped, then there is nothing to do
	if(!m_hThread)
		return;

	// thread already stopped?
	if(WaitForSingleObject(m_hThread, 0) == WAIT_OBJECT_0)
	{
		if(!::ResetEvent(m_hKillThread))
			THROW_CORE_EXCEPTION_WIN32(eErr_CannotResetEvent, GetLastError());

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(rUpgradeLock);

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
}

void TWorkerThreadController::SignalThreadToStop(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock)
{
	RemoveZombieData(rUpgradeLock);
	if(!m_hThread)
		return;

	if(!::SetEvent(m_hKillThread))
		THROW_CORE_EXCEPTION_WIN32(eErr_CannotSetEvent, GetLastError());
}

void TWorkerThreadController::WaitForThreadToExit(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock)
{
	if(!m_hThread)
		return;

	DWORD dwRes = WaitForSingleObject(m_hThread, INFINITE);
	if(dwRes == WAIT_OBJECT_0)
	{
		if(!::ResetEvent(m_hKillThread))
			THROW_CORE_EXCEPTION_WIN32(eErr_CannotResetEvent, GetLastError());

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(rUpgradeLock);

		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	else
		THROW_CORE_EXCEPTION_WIN32(eErr_WaitingFailed, GetLastError());
}

END_CHCORE_NAMESPACE
