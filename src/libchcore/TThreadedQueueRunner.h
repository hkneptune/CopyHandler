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
#ifndef __THREADEDQUEUERUNNER_H__
#define __THREADEDQUEUERUNNER_H__

#include "WaitableQueue.h"
#include <thread>
#include <array>

namespace chcore
{
	template<class T>
	class TThreadedQueueRunner
	{
	public:
		explicit TThreadedQueueRunner(HANDLE hKill) :
			m_hKill(hKill),
			m_eventLocalKill(true, false)
		{
		}

		TThreadedQueueRunner(const TThreadedQueueRunner& rSrc) = delete;

		~TThreadedQueueRunner()
		{
			Stop();
		}

		TThreadedQueueRunner& operator=(const TThreadedQueueRunner& rSrc) = delete;

		void Start()
		{
			if(m_thread.joinable())
				return;
			m_eventLocalKill.ResetEvent();

			m_thread = std::thread(&TThreadedQueueRunner::ThreadProc, this);
		}

		void Stop()
		{
			m_eventLocalKill.SetEvent();
			if(m_thread.joinable())
				m_thread.join();
		}

		void PushTask(T&& func)
		{
			if(!m_thread.joinable())
				Start();

			m_queue.PushBack(std::forward<T>(func));
		}

	private:
		void ThreadProc()
		{
			enum { eKill, eLocalKill, eQueue, eCount };
			std::array<HANDLE, eCount> arrHandles = { m_hKill, m_eventLocalKill.Handle(), m_queue.GetWaitHandle() };

			bool bStop = false;
			do
			{
				DWORD dwResult = WaitForMultipleObjectsEx(eCount, arrHandles.data(), FALSE, INFINITE, TRUE);
				switch(dwResult)
				{
				case STATUS_USER_APC:
					break;

				case WAIT_OBJECT_0 + eQueue:
					{
						T func = m_queue.PopFront();
						func();
						break;
					}

				case WAIT_OBJECT_0 + eKill:
				case WAIT_OBJECT_0 + eLocalKill:
				default:
					{
						bStop = true;
						break;
					}
				}
			}
			while(!bStop);
		}

	private:
		WaitableQueue<T> m_queue;
		std::thread m_thread;
		HANDLE m_hKill = nullptr;
		TEvent m_eventLocalKill;
	};
}

#endif
