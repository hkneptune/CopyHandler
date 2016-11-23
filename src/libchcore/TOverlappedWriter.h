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
#ifndef __TOVERLAPPEDWRITER_H__
#define __TOVERLAPPEDWRITER_H__

#include "../liblogger/TLogFileData.h"
#include "../liblogger/TLogger.h"
#include "TOrderedBufferQueue.h"
#include "TWriteBufferQueueWrapper.h"
#include "TBufferList.h"
#include "TOverlappedProcessorRange.h"

namespace chcore
{
	class TOverlappedWriter
	{
	public:
		explicit TOverlappedWriter(const logger::TLogFileDataPtr& spLogFileData, const TOrderedBufferQueuePtr& spBuffersToWrite,
			const TOverlappedProcessorRangePtr& spRange, const TBufferListPtr& spEmptyBuffers);
		TOverlappedWriter(const TOverlappedWriter&) = delete;
		~TOverlappedWriter();

		TOverlappedWriter& operator=(const TOverlappedWriter&) = delete;

		void AddEmptyBuffer(TOverlappedDataBuffer* pBuffer);

		void AddRetryBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetWriteBuffer();

		// buffer management - writer
		void AddFailedWriteBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFailedWriteBuffer();

		void AddFinishedBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFinishedBuffer();

		// processing info
		void MarkAsFinalized(TOverlappedDataBuffer* pBuffer);

		// event access
		HANDLE GetEventWritePossibleHandle() const;
		HANDLE GetEventWriteFailedHandle() const;
		HANDLE GetEventWriteFinishedHandle() const;

		void UpdateProcessingRange(unsigned long long ullNewPosition);
		void ClearBuffers();

	private:
		logger::TLoggerPtr m_spLog;

		TBufferListPtr m_spEmptyBuffers;

		TWriteBufferQueueWrapper m_tBuffersToWrite;
		TOrderedBufferQueue m_tFinishedBuffers;

		TOverlappedDataBuffer* m_pLastPartBuffer = nullptr;

		boost::signals2::connection m_dataRangeChanged;
	};

	using TOverlappedWriterPtr = std::shared_ptr<TOverlappedWriter>;
}

#endif
