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

#include "../liblogger/TLogFileData.h"
#include "../liblogger/TLogger.h"
#include "TOverlappedMemoryPool.h"
#include "TOverlappedReader.h"
#include "TOverlappedWriter.h"

namespace chcore
{
	class TOverlappedReaderWriter
	{
	public:
		explicit TOverlappedReaderWriter(const logger::TLogFileDataPtr& spLogFileData, const TOverlappedMemoryPoolPtr& spBuffers,
			unsigned long long ullFilePos, DWORD dwChunkSize);
		TOverlappedReaderWriter(const TOverlappedReaderWriter&) = delete;
		~TOverlappedReaderWriter();

		TOverlappedReaderWriter& operator=(const TOverlappedReaderWriter&) = delete;

		// reader/writer
		TOverlappedReaderPtr GetReader() const { return m_spReader; }
		TOverlappedWriterPtr GetWriter() const { return m_spWriter; }

		// event access
		void WaitForMissingBuffersAndResetState(HANDLE hKillEvent);

	private:
		logger::TLoggerPtr m_spLog;

		TOverlappedMemoryPoolPtr m_spMemoryPool;
		TOverlappedReaderPtr m_spReader;
		TOverlappedWriterPtr m_spWriter;
	};
}

#endif
