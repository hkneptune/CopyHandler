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
#ifndef __TOVERLAPPEDDATABUFFERQUEUE_H__
#define __TOVERLAPPEDDATABUFFERQUEUE_H__

#include "libchcore.h"
#include <deque>
#include "TEvent.h"
#include "IOverlappedDataBufferQueue.h"

namespace chcore
{
	class TOverlappedDataBuffer;

	struct CompareBufferPositions
	{
		bool operator()(const TOverlappedDataBuffer* rBufferA, const TOverlappedDataBuffer* rBufferB);
	};

	class TOverlappedDataBufferQueue : public IOverlappedDataBufferQueue
	{
	public:
		TOverlappedDataBufferQueue();
		TOverlappedDataBufferQueue(size_t stCount, size_t stBufferSize);
		~TOverlappedDataBufferQueue();

		void ReinitializeBuffers(size_t stCount, size_t stBufferSize);
		size_t GetTotalBufferCount() const;
		size_t GetSingleBufferSize() const;

		// buffer management
		virtual void AddEmptyBuffer(TOverlappedDataBuffer* pBuffer) override;
		virtual TOverlappedDataBuffer* GetEmptyBuffer() override;

		virtual void AddFullBuffer(TOverlappedDataBuffer* pBuffer) override;
		virtual TOverlappedDataBuffer* GetFullBuffer() override;

		virtual void AddFinishedBuffer(TOverlappedDataBuffer* pBuffer) override;
		virtual TOverlappedDataBuffer* GetFinishedBuffer() override;

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

		void WaitForMissingBuffers(HANDLE hKillEvent);

	private:
		void CleanupBuffers();
		void UpdateReadPossibleEvent();
		void UpdateWritePossibleEvent();
		void UpdateWriteFinishedEvent();
		void UpdateAllBuffersAccountedFor();

	private:
		std::deque<std::unique_ptr<TOverlappedDataBuffer>> m_listAllBuffers;

		std::list<TOverlappedDataBuffer*> m_listEmptyBuffers;

		using FullBuffersSet = std::set < TOverlappedDataBuffer*, CompareBufferPositions >;
		FullBuffersSet m_setFullBuffers;

		using FinishedBuffersSet = std::set < TOverlappedDataBuffer*, CompareBufferPositions >;
		FinishedBuffersSet m_setFinishedBuffers;

		bool m_bDataSourceFinished;		// input file was already read to the end
		bool m_bDataWritingFinished;	// output file was already written to the end

		unsigned long long m_ullNextReadBufferOrder;	// next order id for read buffers
		unsigned long long m_ullNextWriteBufferOrder;	// next order id to be processed when writing
		unsigned long long m_ullNextFinishedBufferOrder;	// next order id to be processed when finishing writing

		TEvent m_eventReadPossible;
		TEvent m_eventWritePossible;
		TEvent m_eventWriteFinished;
		TEvent m_eventAllBuffersAccountedFor;
	};
}

#endif
