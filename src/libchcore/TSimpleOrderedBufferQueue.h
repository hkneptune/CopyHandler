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

namespace chcore
{
	class TSimpleOrderedBufferQueue : public std::set<TOverlappedDataBuffer*, CompareBufferPositions>
	{
	public:
		void Push(TOverlappedDataBuffer* pBuffer)
		{
			if(!pBuffer)
				throw TCoreException(eErr_InvalidArgument, L"pBuffer is NULL", LOCATION);
			insert(pBuffer);
		}

		TOverlappedDataBuffer* Pop()
		{
			if(empty())
				return nullptr;
			TOverlappedDataBuffer* pBuffer = *begin();
			erase(begin());

			return pBuffer;
		}

		const TOverlappedDataBuffer* const Peek() const
		{
			if(empty())
				return nullptr;
			return *begin();
		}

		void ReleaseBuffers(const TBufferListPtr& spBuffers)
		{
			for (TOverlappedDataBuffer* pBuffer : *this)
			{
				spBuffers->Push(pBuffer);
			}
			clear();
		}
	};
}

#endif
