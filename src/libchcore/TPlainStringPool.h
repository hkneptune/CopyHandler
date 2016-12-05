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
#ifndef __TPLAINSTRINGPOOL_H__
#define __TPLAINSTRINGPOOL_H__

#include "libchcore.h"

namespace chcore
{
	class LIBCHCORE_API TPlainStringPool
	{
	public:
		static const size_t BlockSize = 256 * 1024;

	public:
		TPlainStringPool();
		TPlainStringPool(const TPlainStringPool&) = delete;
		~TPlainStringPool();

		TPlainStringPool& operator=(const TPlainStringPool&) = delete;

		wchar_t* Alloc(size_t stCount);
		wchar_t* AllocForString(const wchar_t* pszString);

		void Clear(bool bLeaveSingleEmptyBlock = true);

	private:
		wchar_t* AllocNewBlock();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::vector<std::pair<wchar_t*, size_t> > m_vBlocks;	// memory blocks of size BlockSize => remaining size
#pragma warning(pop)
	};
}

#endif
