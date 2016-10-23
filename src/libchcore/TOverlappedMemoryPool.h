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

#include "TEvent.h"
#include "TBufferList.h"

namespace chcore
{
	class TOverlappedDataBuffer;

	class TOverlappedMemoryPool
	{
	public:
		TOverlappedMemoryPool();
		TOverlappedMemoryPool(size_t stCount, size_t stBufferSize);
		TOverlappedMemoryPool(const TOverlappedMemoryPool&) = delete;
		~TOverlappedMemoryPool();

		TOverlappedMemoryPool& operator=(const TOverlappedMemoryPool&) = delete;

		void ReinitializeBuffers(size_t stCount, size_t stBufferSize);
		size_t GetTotalBufferCount() const;
		size_t GetAvailableBufferCount() const;
		size_t GetSingleBufferSize() const;

		TBufferListPtr GetBufferList() const;;

	private:
		std::vector<std::unique_ptr<TOverlappedDataBuffer>> m_listAllBuffers;

		TBufferListPtr m_spQueueBuffers;

		TEvent m_eventAllBuffersAccountedFor;
	};

	using TOverlappedMemoryPoolPtr = std::shared_ptr<TOverlappedMemoryPool>;
}

#endif
