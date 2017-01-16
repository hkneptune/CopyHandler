// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TWorkerThreadController.h
/// @date  2010/09/04
/// @brief Contains class TWorkerThreadController.
// ============================================================================
#ifndef __TWORKERTHREADCONTROLLER_H__
#define __TWORKERTHREADCONTROLLER_H__

#include "libchcore.h"

///////////////////////////////////////////////////////////////////////////
// TWorkerThreadController
namespace boost
{
	template<class T> class upgrade_lock;
}
namespace chcore
{
	class LIBCHCORE_API TWorkerThreadController
	{
	public:
		TWorkerThreadController();
		TWorkerThreadController(const TWorkerThreadController&) = delete;
		~TWorkerThreadController();

		TWorkerThreadController& operator=(const TWorkerThreadController&) = delete;

		// methods to be used outside of the thread being controlled
		void StartThread(PTHREAD_START_ROUTINE pThreadFunction, PVOID pThreadParam, int iPriority = THREAD_PRIORITY_NORMAL);
		void SignalThreadToStop();
		void WaitForThreadToExit(DWORD dwMiliseconds = INFINITE);

		void StopThread(DWORD dwMiliseconds = INFINITE);
		void ChangePriority(int iPriority);

		// methods to be used only inside the thread being controlled
		bool KillRequested(DWORD dwWaitForSignal = 0);
		HANDLE GetKillThreadHandle() const;

	protected:
		void RemoveZombieData(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock);

		void SignalThreadToStop(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock);
		void WaitForThreadToExit(boost::upgrade_lock<boost::shared_mutex>& rUpgradeLock, DWORD dwMiliseconds = INFINITE);

	private:
		HANDLE m_hThread;
		HANDLE m_hKillThread;
#pragma warning(push)
#pragma warning(disable: 4251)
		boost::shared_mutex m_lock;
#pragma warning(pop)
	};
}

#endif
