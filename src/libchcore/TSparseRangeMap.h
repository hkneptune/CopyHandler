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
#ifndef __TSparseRangeMap_H__
#define __TSparseRangeMap_H__

#include "libchcore.h"
#include "CommonDataTypes.h"

namespace chcore
{
	class LIBCHCORE_API TSparseRangeMap
	{
	public:
		TSparseRangeMap();
		~TSparseRangeMap();

		void Insert(file_size_t fsRangeStart, file_size_t fsRangeEnd);

		size_t GetRangeCount() const;
		void GetRangeAt(size_t stIndex, file_size_t& rfsRangeStart, file_size_t& rfsRangeEnd) const;

		bool OverlapsRange(file_size_t fsRangeStart, file_size_t fsRangeEnd) const;

	private:
		using RangePair = std::pair<file_size_t, file_size_t>;
		using RangeMap = std::map<file_size_t, file_size_t>;

#pragma warning(push)
#pragma warning(disable: 4251)
		RangeMap m_mapRanges;
#pragma warning(pop)
	};
}

#endif
