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

BEGIN_CHCORE_NAMESPACE

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

	// buffer management
	virtual void AddEmptyBuffer(TOverlappedDataBuffer* pBuffer) override;
	virtual TOverlappedDataBuffer* GetEmptyBuffer() override;

	virtual void AddFullBuffer(TOverlappedDataBuffer* pBuffer) override;
	virtual TOverlappedDataBuffer* GetFullBuffer() override;

	virtual void AddFinishedBuffer(TOverlappedDataBuffer* pBuffer) override;
	virtual TOverlappedDataBuffer* GetFinishedBuffer() override;

	// data source change
	void DataSourceChanged();

	// event access
	HANDLE GetEventReadPossibleHandle() { return m_eventReadPossible.Handle(); }
	HANDLE GetEventWritePossibleHandle() { return m_eventWritePossible.Handle(); }
	HANDLE GetEventWriteFinishedHandle() { return m_eventWriteFinished.Handle(); }

private:
	void CleanupBuffers();
	void UpdateReadPossibleEvent();
	void UpdateWritePossibleEvent();
	void UpdateWriteFinishedEvent();

private:
	std::deque<std::unique_ptr<TOverlappedDataBuffer>> m_listAllBuffers;
	size_t m_stBufferSize;

	std::list<TOverlappedDataBuffer*> m_listEmptyBuffers;
	std::set<TOverlappedDataBuffer*, CompareBufferPositions> m_setFullBuffers;
	std::set<TOverlappedDataBuffer*, CompareBufferPositions> m_setFinishedBuffers;

	bool m_bDataSourceFinished;		// input file was already read to the end
	unsigned long long m_ullNextExpectedWritePosition;	// current write file pointer
	unsigned long long m_ullNextReadBufferOrder;	// next order id for read buffers
	unsigned long long m_ullNextWriteBufferOrder;	// next order id to be processed when writing
	unsigned long long m_ullNextFinishedBufferOrder;	// next order id to be processed when finishing writing

	TEvent m_eventReadPossible;
	TEvent m_eventWritePossible;
	TEvent m_eventWriteFinished;
};

END_CHCORE_NAMESPACE

#endif
