// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#ifndef __TOVERLAPPEDREADER_H__
#define __TOVERLAPPEDREADER_H__

#include "../liblogger/TLogFileData.h"
#include "../liblogger/TLogger.h"
#include "TOrderedBufferQueue.h"
#include "TReadBufferQueueWrapper.h"

namespace chcore
{
	class TOverlappedReader
	{
	public:
		explicit TOverlappedReader(const logger::TLogFileDataPtr& spLogFileData, const TBufferListPtr& spEmptyBuffers,
			unsigned long long ullFilePos, DWORD dwChunkSize);
		TOverlappedReader(const TOverlappedReader&) = delete;
		~TOverlappedReader();

		TOverlappedReader& operator=(const TOverlappedReader&) = delete;

		// buffer management
		void AddEmptyBuffer(TOverlappedDataBuffer* pBuffer, bool bKeepPosition);
		TOverlappedDataBuffer* GetEmptyBuffer();

		void AddFailedReadBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFailedReadBuffer();

		void AddFinishedReadBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFinishedReadBuffer();

		TOrderedBufferQueuePtr GetFinishedQueue() const;

		// processing info
		bool IsDataSourceFinished() const { return m_tInputBuffers.IsDataSourceFinished(); }

		// event access
		HANDLE GetEventReadPossibleHandle() const { return m_tInputBuffers.GetHasBuffersEvent(); }
		HANDLE GetEventReadFailedHandle() const { return m_spFullBuffers->GetHasErrorEvent(); }
		HANDLE GetEventReadFinishedHandle() const { return m_spFullBuffers->GetHasBuffersEvent(); }

		void ReleaseBuffers();

	private:
		logger::TLoggerPtr m_spLog;

		// queues
		TBufferListPtr m_spEmptyBuffers;
		TReadBufferQueueWrapper m_tInputBuffers;
		TOrderedBufferQueuePtr m_spFullBuffers;			// buffers with data

		bool m_bReleaseMode = false;		// when set, all incoming buffers will go to empty buffers
	};

	using TOverlappedReaderPtr = std::shared_ptr<TOverlappedReader>;
}

#endif
