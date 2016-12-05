// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#ifndef __TSIMPLEORDEREDBUFFERQUEUE_H__
#define __TSIMPLEORDEREDBUFFERQUEUE_H__

#include <set>
#include "TCoreException.h"
#include "TOverlappedDataBuffer.h"
#include "TBufferList.h"
#include "TSharedCount.h"

namespace chcore
{
	class TSimpleOrderedBufferQueue : private std::set<TOverlappedDataBuffer*, CompareBufferPositions>
	{
	public:
		TSimpleOrderedBufferQueue() : m_spCount(std::make_shared<TSharedCount<size_t>>())
		{
		}

		void Push(TOverlappedDataBuffer* pBuffer)
		{
			if(!pBuffer)
				throw TCoreException(eErr_InvalidArgument, L"pBuffer is NULL", LOCATION);
			if(!insert(pBuffer).second)
				throw TCoreException(eErr_InvalidArgument, L"Buffer already exists in the collection", LOCATION);
			m_spCount->Increase();
		}

		TOverlappedDataBuffer* Pop()
		{
			if(empty())
				return nullptr;
			TOverlappedDataBuffer* pBuffer = *begin();
			erase(begin());

			m_spCount->Decrease();

			return pBuffer;
		}

		const TOverlappedDataBuffer* Peek() const
		{
			if(empty())
				return nullptr;
			return *begin();
		}

		void ClearBuffers(const TBufferListPtr& spBuffers)
		{
			if(!spBuffers)
				throw TCoreException(eErr_InvalidArgument, L"spBuffers is NULL", LOCATION);

			for (TOverlappedDataBuffer* pBuffer : *this)
			{
				spBuffers->Push(pBuffer);
			}
			clear();

			m_spCount->SetValue(0);
		}

		bool IsEmpty() const
		{
			return empty();
		}

		size_t GetCount() const
		{
			return m_spCount->GetValue();
		}

		TSharedCountPtr<size_t> GetSharedCount()
		{
			return m_spCount;
		}

	private:
		TSharedCountPtr<size_t> m_spCount;
	};
}

#endif
