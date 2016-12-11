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
#ifndef __TCONFIGARRAY_H__
#define __TCONFIGARRAY_H__

#include "TConfig.h"

namespace chengine
{
	class LIBCHENGINE_API TConfigArray
	{
	public:
		TConfigArray();
		TConfigArray(const TConfigArray& rSrc);
		~TConfigArray();

		TConfigArray& operator=(const TConfigArray& rSrc);

		size_t GetCount() const;
		bool IsEmpty() const;

		const TConfig& GetAt(size_t stIndex) const;
		TConfig& GetAt(size_t stIndex);

		void Add(const TConfig& rSrc);

		void RemoveAt(size_t stIndex);
		void Clear();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<TConfig> m_vConfigs;
#pragma warning(pop)
	};
}

#endif
