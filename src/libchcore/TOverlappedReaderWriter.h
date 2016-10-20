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
#include "TOrderedBufferQueue.h"
#include "IFilesystemFile.h"

namespace chcore
{
	class TOverlappedReaderWriter
	{
	private:
		static const unsigned long long NoIoError = 0xffffffffffffffff;

	public:
		explicit TOverlappedReaderWriter(const logger::TLogFileDataPtr& spLogFileData, const TOverlappedMemoryPoolPtr& spBuffers,
			file_size_t ullFilePos, DWORD dwChunkSize);
		TOverlappedReaderWriter(const TOverlappedReaderWriter&) = delete;
		~TOverlappedReaderWriter();

		TOverlappedReaderWriter& operator=(const TOverlappedReaderWriter&) = delete;

		// buffer management
		void AddFailedReadBuffer(TOverlappedDataBuffer* pBuffer);
		void AddEmptyBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetEmptyBuffer();

		void AddFullBuffer(TOverlappedDataBuffer* pBuffer);
		void AddFailedFullBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFullBuffer();

		void AddFinishedBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFinishedBuffer();
		void MarkFinishedBufferAsComplete(TOverlappedDataBuffer* pBuffer);

		// data source change
		void DataSourceChanged();

		// processing info
		bool IsDataSourceFinished() const { return m_bDataSourceFinished; }
		bool IsDataWritingFinished() const { return m_bDataWritingFinished; }

		// event access
		HANDLE GetEventReadPossibleHandle() const { return m_eventReadPossible.Handle(); }
		HANDLE GetEventWritePossibleHandle() const { return m_eventWritePossible.Handle(); }
		HANDLE GetEventWriteFinishedHandle() const { return m_eventWriteFinished.Handle(); }
		HANDLE GetEventAllBuffersAccountedFor() const { return m_eventAllBuffersAccountedFor.Handle(); }

		void WaitForMissingBuffersAndResetState(HANDLE hKillEvent);

	private:
		void CleanupBuffers();
		void UpdateReadPossibleEvent();
		void UpdateWritePossibleEvent();
		void UpdateWriteFinishedEvent();
		void UpdateAllBuffersAccountedFor();

	private:
		logger::TLoggerPtr m_spLog;

		TOverlappedMemoryPoolPtr m_spMemoryPool;

		TOrderedBufferQueue m_setEmptyBuffers;	// initialized empty buffers
		TOrderedBufferQueue m_setFullBuffers;
		TOrderedBufferQueue m_setFinishedBuffers;

		bool m_bDataSourceFinished = false;		// input file was already read to the end
		bool m_bDataWritingFinished = false;	// output file was already written to the end

		DWORD m_dwDataChunkSize = 0;

		unsigned long long m_ullNextReadBufferOrder = 0;	// next order id for read buffers
		unsigned long long m_ullReadErrorOrder = NoIoError;

		unsigned long long m_ullNextWriteBufferOrder = 0;	// next order id to be processed when writing
		unsigned long long m_ullNextFinishedBufferOrder = 0;	// next order id to be processed when finishing writing

		TEvent m_eventReadPossible;
		TEvent m_eventWritePossible;
		TEvent m_eventWriteFinished;
		TEvent m_eventAllBuffersAccountedFor;
	};
}

#endif
