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

namespace chcore
{
	class TOverlappedWriter
	{
	private:
		static const unsigned long long NoIoError = 0xffffffffffffffff;

	public:
		explicit TOverlappedWriter(const logger::TLogFileDataPtr& spLogFileData, const TOrderedBufferQueuePtr& spBuffersToWrite,
			unsigned long long ullFilePos, const TBufferListPtr& spEmptyBuffers);
		TOverlappedWriter(const TOverlappedWriter&) = delete;
		~TOverlappedWriter();

		TOverlappedWriter& operator=(const TOverlappedWriter&) = delete;

		TOverlappedDataBuffer* GetWriteBuffer();

		// buffer management - writer
		void AddFailedWriteBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFailedWriteBuffer();

		void AddFinishedBuffer(TOverlappedDataBuffer* pBuffer);
		TOverlappedDataBuffer* GetFinishedBuffer();

		// processing info
		void MarkAsFinalized(TOverlappedDataBuffer* pBuffer);

		// event access
		HANDLE GetEventWritePossibleHandle() const { return m_tBuffersToWrite.GetHasBuffersEvent(); }
		HANDLE GetEventWriteFailedHandle() const { return m_tFinishedBuffers.GetHasErrorEvent(); }
		HANDLE GetEventWriteFinishedHandle() const { return m_tFinishedBuffers.GetHasBuffersEvent(); }

		void ReleaseBuffers();

	private:
		logger::TLoggerPtr m_spLog;

		TBufferListPtr m_spEmptyBuffers;

		TWriteBufferQueueWrapper m_tBuffersToWrite;
		TOrderedBufferQueue m_tFinishedBuffers;

		TOverlappedDataBuffer* m_pLastPartBuffer = nullptr;

		bool m_bReleaseMode = false;
	};
}

#endif
