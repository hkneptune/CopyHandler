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
#include "TFailedBufferQueue.h"
#include "TWriteBufferQueueWrapper.h"

namespace chcore
{
	class TOverlappedWriter
	{
	private:
		static const unsigned long long NoIoError = 0xffffffffffffffff;

	public:
		explicit TOverlappedWriter(const logger::TLogFileDataPtr& spLogFileData, const TOrderedBufferQueuePtr& spBuffersToWrite,
			unsigned long long ullFilePos);
		TOverlappedWriter(const TOverlappedWriter&) = delete;
		~TOverlappedWriter();

		TOverlappedWriter& operator=(const TOverlappedWriter&) = delete;

		// buffer management - writer
		void AddFailedWriteBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFailedWriteBuffer();

		void AddFinishedBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFinishedBuffer();

		// processing info
		bool IsDataWritingFinished() const { return m_bDataWritingFinished; }

		// event access
		HANDLE GetEventWritePossibleHandle() const { return m_tBuffersToWrite.GetHasBuffersEvent(); }
		HANDLE GetEventWriteFailedHandle() const { return m_tFailedWriteBuffers.GetHasBuffersEvent(); }
		HANDLE GetEventWriteFinishedHandle() const { return m_tFinishedBuffers.GetHasBuffersEvent(); }

		size_t GetBufferCount() const;

	private:
		logger::TLoggerPtr m_spLog;

		TWriteBufferQueueWrapper m_tBuffersToWrite;

		TOrderedBufferQueue m_tFailedWriteBuffers;
		TOrderedBufferQueue m_tFinishedBuffers;

		bool m_bDataWritingFinished = false;	// output file was already written to the end
	};
}

#endif
