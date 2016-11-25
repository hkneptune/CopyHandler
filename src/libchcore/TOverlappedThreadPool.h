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
#ifndef __TOVERLAPPEDTHREADPOOL_H__
#define __TOVERLAPPEDTHREADPOOL_H__

#include <functional>
#include "TThreadedQueueRunner.h"
#include "TOverlappedReaderFB.h"
#include "TOverlappedWriterFB.h"

namespace chcore
{
	using TReaderThread = TThreadedQueueRunner<std::function<void()>>;
	using TWriterThread = TThreadedQueueRunner<std::function<void()>>;

	class TOverlappedThreadPool
	{
	public:
		explicit TOverlappedThreadPool(HANDLE hKill);

		TReaderThread& ReaderThread();
		TWriterThread& WriterThread();

		void QueueRead(const TOverlappedReaderFBPtr& spReader);
		void QueueWrite(const TOverlappedWriterFBPtr& spWriter);

	private:
		TReaderThread m_threadReader;
		TWriterThread m_threadWriter;
	};
}

#endif
