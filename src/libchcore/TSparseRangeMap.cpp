// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "TSparseRangeMap.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

namespace chcore
{
	TSparseRangeMap::TSparseRangeMap()
	{
	}

	TSparseRangeMap::~TSparseRangeMap()
	{
	}

	void TSparseRangeMap::Insert(file_size_t fsRangeStart, file_size_t fsRangeEnd)
	{
		if (fsRangeEnd < fsRangeStart)
			std::swap(fsRangeStart, fsRangeEnd);

		std::pair<RangeMap::iterator, bool> pairInsert = m_mapRanges.emplace(std::make_pair(fsRangeStart, fsRangeEnd));
		RangeMap::iterator iterInsertedRange = pairInsert.first;
		if (!pairInsert.second)
		{
			// range with fsRangeStart already exists; update it with the increased range (if bigger than the existing one)
			if (fsRangeEnd > iterInsertedRange->second)
				iterInsertedRange->second = fsRangeEnd;
			else
				return;	// new range overlaps with old one and is smaller; does not change the state of internal map and no reorganization is needed
		}
		else
		{
			// element inserted successfully; since it can overlap the previous range in the map, we need to start merging ranges from this previous element
			if (iterInsertedRange != m_mapRanges.begin())
				iterInsertedRange = std::prev(iterInsertedRange);
			else if (m_mapRanges.size() == 1)
				return;	// this is the only element currently in the map; no need to process further.
		}

		// merge subsequent ranges
		file_size_t fsCurrentRangeEnd = iterInsertedRange->second;

		RangeMap::iterator iterRange = std::next(iterInsertedRange);

		while (iterRange != m_mapRanges.end())
		{
			if (iterRange->first <= fsCurrentRangeEnd + 1)
			{
				// next range overlaps with the inserted one - merge them
				if (iterRange->second > iterInsertedRange->second)
					iterInsertedRange->second = iterRange->second;
				iterRange = m_mapRanges.erase(iterRange);
			}
			else
				break;
		}
	}

	size_t TSparseRangeMap::GetRangeCount() const
	{
		return m_mapRanges.size();
	}

	void TSparseRangeMap::GetRangeAt(size_t stIndex, file_size_t& rfsRangeStart, file_size_t& rfsRangeEnd) const
	{
		if (stIndex >= m_mapRanges.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		RangeMap::const_iterator iterMap = m_mapRanges.begin();
		std::advance(iterMap, stIndex);

		rfsRangeStart = iterMap->first;
		rfsRangeEnd = iterMap->second;
	}

	bool TSparseRangeMap::OverlapsRange(file_size_t fsRangeStart, file_size_t fsRangeEnd) const
	{
		if (fsRangeEnd < fsRangeStart)
			std::swap(fsRangeStart, fsRangeEnd);

		RangeMap::const_iterator iterStart = m_mapRanges.lower_bound(fsRangeStart);
		if (iterStart != m_mapRanges.begin())
			iterStart = std::prev(iterStart);

		RangeMap::const_iterator iterEnd = m_mapRanges.upper_bound(fsRangeEnd);
		while (iterStart != iterEnd)
		{
			if (fsRangeStart <= iterStart->second && iterStart->first <= fsRangeEnd)
				return true;
			++iterStart;
		}

		return false;
	}
}
