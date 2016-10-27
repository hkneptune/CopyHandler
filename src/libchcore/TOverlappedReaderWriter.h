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
#ifndef __TOVERLAPPEDREADERWRITER_H__
#define __TOVERLAPPEDREADERWRITER_H__

#include "TEvent.h"
#include "../liblogger/TLogFileData.h"
#include "../liblogger/TLogger.h"
#include "TOverlappedMemoryPool.h"
#include "TOverlappedReader.h"
#include "TOverlappedWriter.h"

namespace chcore
{
	class TOverlappedReaderWriter
	{
	private:
		static const unsigned long long NoIoError = 0xffffffffffffffff;

	public:
		explicit TOverlappedReaderWriter(const logger::TLogFileDataPtr& spLogFileData, const TOverlappedMemoryPoolPtr& spBuffers,
			unsigned long long ullFilePos, DWORD dwChunkSize);
		TOverlappedReaderWriter(const TOverlappedReaderWriter&) = delete;
		~TOverlappedReaderWriter();

		TOverlappedReaderWriter& operator=(const TOverlappedReaderWriter&) = delete;

		// buffer management - reader
		TOverlappedDataBuffer* GetEmptyBuffer();
		void AddEmptyBuffer(TOverlappedDataBuffer* pBuffer, bool bKeepPosition);

		TOverlappedDataBuffer* GetFailedReadBuffer();
		void AddFailedReadBuffer(TOverlappedDataBuffer* pBuffer);

		void AddFinishedReadBuffer(TOverlappedDataBuffer* pBuffer);

		// buffer management - writer
		TOverlappedDataBuffer* GetWriteBuffer();

		TOverlappedDataBuffer* GetFailedWriteBuffer();
		void AddFailedWriteBuffer(TOverlappedDataBuffer* pBuffer);

		void AddFinishedWriteBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFinishedWriteBuffer();

		void MarkFinishedBufferAsComplete(TOverlappedDataBuffer* pBuffer);

		// processing info
		bool IsDataSourceFinished() const { return m_tReader.IsDataSourceFinished(); }

		// event access
		HANDLE GetEventReadPossibleHandle() const { return m_tReader.GetEventReadPossibleHandle(); }
		HANDLE GetEventReadFailedHandle() const { return m_tReader.GetEventReadFailedHandle(); }
		HANDLE GetEventWritePossibleHandle() const { return m_tReader.GetEventReadFinishedHandle(); }

		HANDLE GetEventWriteFailedHandle() const { return m_tWriter.GetEventWriteFailedHandle(); }
		HANDLE GetEventWriteFinishedHandle() const { return m_tWriter.GetEventWriteFinishedHandle(); }

		HANDLE GetEventAllBuffersAccountedFor() const { return m_eventAllBuffersAccountedFor.Handle(); }

		void WaitForMissingBuffersAndResetState(HANDLE hKillEvent);

	private:
		logger::TLoggerPtr m_spLog;

		TOverlappedMemoryPoolPtr m_spMemoryPool;
		TOverlappedReader m_tReader;
		TOverlappedWriter m_tWriter;

		TEvent m_eventAllBuffersAccountedFor;
	};
}

#endif
