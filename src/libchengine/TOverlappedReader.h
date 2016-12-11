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
#include "TOverlappedProcessorRange.h"

namespace chengine
{
	class TOverlappedReader
	{
	public:
		explicit TOverlappedReader(const logger::TLogFileDataPtr& spLogFileData,
			const TBufferListPtr& spEmptyBuffers,
			const TOverlappedProcessorRangePtr& spDataRange,
			DWORD dwChunkSize,
			size_t stMaxOtfBuffers, size_t stMaxReadAheadBuffers,
			TSharedCountPtr<size_t> spOtfBuffersCount);
		TOverlappedReader(const TOverlappedReader&) = delete;
		~TOverlappedReader();

		TOverlappedReader& operator=(const TOverlappedReader&) = delete;

		// buffer management
		void AddEmptyBuffer(TOverlappedDataBuffer* pBuffer);
		void AddRetryBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetEmptyBuffer();

		void AddFailedReadBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFailedReadBuffer();

		void AddFinishedReadBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFinishedReadBuffer();

		TOrderedBufferQueuePtr GetFinishedQueue() const;

		// processing info
		bool IsDataSourceFinished() const;

		// event access
		HANDLE GetEventReadPossibleHandle() const;
		HANDLE GetEventReadFailedHandle() const;
		HANDLE GetEventDataSourceFinishedHandle() const;

		void ClearBuffers();

		void UpdateProcessingRange(unsigned long long ullNewPosition);

	private:
		logger::TLoggerPtr m_spLog;

		// queues
		TOrderedBufferQueuePtr m_spFullBuffers;			// buffers with data
		TReadBufferQueueWrapper m_tInputBuffers;

		boost::signals2::connection m_dataRangeChanged;
	};

	using TOverlappedReaderPtr = std::shared_ptr<TOverlappedReader>;
}

#endif
