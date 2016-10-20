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
#include "stdafx.h"
#include "TOrderedBufferQueue.h"
#include "TOverlappedDataBuffer.h"
#include "TCoreException.h"

namespace chcore
{
	bool CompareBufferPositions::operator()(const TOverlappedDataBuffer* pBufferA, const TOverlappedDataBuffer* pBufferB) const
	{
		if(!pBufferA)
			throw TCoreException(eErr_InvalidArgument, L"pBufferA", LOCATION);
		if(!pBufferB)
			throw TCoreException(eErr_InvalidArgument, L"pBufferB", LOCATION);

		return pBufferA->GetFilePosition() < pBufferB->GetFilePosition();
	}
}
