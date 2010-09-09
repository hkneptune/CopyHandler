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

TWorkerThreadController::TWorkerThreadController() :
	m_hThread(NULL),
	m_hKillThread(NULL)
{
	m_hKillThread = CreateEvent(NULL, FALSE, FALSE, NULL);
	if(!m_hKillThread)
		THROW(_T(""), 0, GetLastError(), 0);
}

TWorkerThreadController::~TWorkerThreadController()
{
	StopThread();
	VERIFY(CloseHandle(m_hKillThread));
}

void TWorkerThreadController::StartThread(PTHREAD_START_ROUTINE pThreadFunction, PVOID pThreadParam, int iPriority)
{
	boost::upgrade_lock<boost::shared_mutex> lock(m_lock);

	RemoveZombieData(lock);

	if(m_hThread)
		THROW(_T("Thread already started"), 0, 0, 0);

	// just in case reset the kill event to avoid early death of the thread to be created
	if(!::ResetEvent(m_hKillThread))
		THROW(_T("Cannot reset the kill event"), 0, GetLastError(), 0);

	boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);

	m_hThread = ::CreateThread(NULL, 0, pThreadFunction, pThreadParam, CREATE_SUSPENDED, NULL);
	if(!m_hThread)
		THROW(_T("Cannot create thread"), 0, GetLastError(), 0);

	if(!::SetThreadPriority(m_hThread, iPriority))
	{
		CloseHandle(m_hThread);
		m_hThread = NULL;

		THROW(_T("Cannot set thread priority"), 0, GetLastError(), 0);
	}

	if(::ResumeThread(m_hThread) == (DWORD)-1)
	{
		VERIFY(CloseHandle(m_hThread));
		m_hThread = NULL;

		THROW(_T("Cannot resume thread"), 0, GetLastError(), 0);
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
		VERIFY(ResetEvent(m_hKillThread));

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(lock);
		VERIFY(CloseHandle(m_hThread));
		m_hThread = NULL;
		return;
	}
	else
		THROW(_T("Problem waiting for thread to finish"), 0, GetLastError(), 0);
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
			THROW(_T("Cannot suspend thread"), 0, GetLastError(), 0);

		if(!::SetThreadPriority(m_hThread, iPriority))
		{
			// resume thread if cannot change priority
			VERIFY(::ResumeThread(m_hThread) != (DWORD)-1);

			THROW(_T("Cannot change the thread priority"), 0, GetLastError(), 0);
		}

		if(::ResumeThread(m_hThread) == (DWORD)-1)
			THROW(_T("Cannot resume thread"), 0, GetLastError(), 0);
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
		VERIFY(ResetEvent(m_hKillThread));

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(rUpgradeLock);

		VERIFY(CloseHandle(m_hThread));

		m_hThread = NULL;
	}
}

void TWorkerThreadController::SignalThreadToStop(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock)
{
	RemoveZombieData(rUpgradeLock);
	if(!m_hThread)
		return;

	if(!SetEvent(m_hKillThread))
		THROW(_T("Cannot set the kill event for thread"), 0, GetLastError(), 0);
}

void TWorkerThreadController::WaitForThreadToExit(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock)
{
	if(!m_hThread)
		return;

	DWORD dwRes = WaitForSingleObject(m_hThread, INFINITE);
	if(dwRes == WAIT_OBJECT_0)
	{
		VERIFY(ResetEvent(m_hKillThread));

		boost::upgrade_to_unique_lock<boost::shared_mutex> lock_upgraded(rUpgradeLock);
		VERIFY(CloseHandle(m_hThread));
		m_hThread = NULL;
		return;
	}
	else
		THROW(_T("Problem waiting for thread to finish"), 0, GetLastError(), 0);
}
