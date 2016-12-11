/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "stdafx.h"
#include "TBufferSizes.h"
#include "../libchcore/RoundingFunctions.h"
#include "../libchcore/TCoreException.h"

using namespace chcore;

namespace chengine
{
	TBufferSizes::TBufferSizes() :
		m_uiDefaultSize(BufferGranularity),
		m_uiOneDiskSize(BufferGranularity),
		m_uiTwoDisksSize(BufferGranularity),
		m_uiCDSize(BufferGranularity),
		m_uiLANSize(BufferGranularity),
		m_bOnlyDefault(false),
		m_uiBufferCount(MinBufferCount),
		m_uiMaxReadAheadBuffers(MinReadAhead),
		m_uiMaxConcurrentReads(MinConcurrentReads),
		m_uiMaxConcurrentWrites(MinConcurrentWrites)
	{
	}

	TBufferSizes::TBufferSizes(bool bOnlyDefault, unsigned int uiBufferCount, unsigned int uiDefaultSize, unsigned int uiOneDiskSize, unsigned int uiTwoDisksSize, unsigned int uiCDSize, unsigned int uiLANSize,
		unsigned int uiMaxReadAheadBuffers, unsigned int uiMaxConcurrentReads, unsigned int uiMaxConcurrentWrites) :
		m_uiDefaultSize(std::max(BufferGranularity, RoundUp(uiDefaultSize, BufferGranularity))),
		m_uiOneDiskSize(std::max(BufferGranularity, RoundUp(uiOneDiskSize, BufferGranularity))),
		m_uiTwoDisksSize(std::max(BufferGranularity, RoundUp(uiTwoDisksSize, BufferGranularity))),
		m_uiCDSize(std::max(BufferGranularity, RoundUp(uiCDSize, BufferGranularity))),
		m_uiLANSize(std::max(BufferGranularity, RoundUp(uiLANSize, BufferGranularity))),
		m_bOnlyDefault(bOnlyDefault),
		m_uiBufferCount(std::max(uiBufferCount, MinBufferCount)),
		m_uiMaxReadAheadBuffers(std::max(uiMaxReadAheadBuffers, MinReadAhead)),
		m_uiMaxConcurrentReads(std::max(uiMaxConcurrentReads, MinConcurrentReads)),
		m_uiMaxConcurrentWrites(std::max(uiMaxConcurrentWrites, MinConcurrentWrites))
	{
	}

	void TBufferSizes::Clear()
	{
		m_uiDefaultSize = BufferGranularity;
		m_uiOneDiskSize = BufferGranularity;
		m_uiTwoDisksSize = BufferGranularity;
		m_uiCDSize = BufferGranularity;
		m_uiLANSize = BufferGranularity;
		m_bOnlyDefault = false;
		m_uiBufferCount = MinBufferCount;
		m_uiMaxReadAheadBuffers = MinReadAhead;
		m_uiMaxConcurrentReads = MinConcurrentReads;
		m_uiMaxConcurrentWrites = MinConcurrentWrites;
	}

	unsigned int TBufferSizes::GetSizeByType(EBufferType eType) const
	{
		switch (eType)
		{
		case eBuffer_Default:
			return m_uiDefaultSize;
		case eBuffer_OneDisk:
			return m_uiOneDiskSize;
		case eBuffer_TwoDisks:
			return m_uiTwoDisksSize;
		case eBuffer_CD:
			return m_uiCDSize;
		case eBuffer_LAN:
			return m_uiLANSize;
		default:
			throw TCoreException(eErr_BoundsExceeded, L"Unsupported buffer type", LOCATION);
		}
	}

	void TBufferSizes::SetSizeByType(EBufferType eType, unsigned int uiSize)
	{
		switch (eType)
		{
		case eBuffer_Default:
			m_uiDefaultSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
			break;
		case eBuffer_OneDisk:
			m_uiOneDiskSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
			break;
		case eBuffer_TwoDisks:
			m_uiTwoDisksSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
			break;
		case eBuffer_CD:
			m_uiCDSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
			break;
		case eBuffer_LAN:
			m_uiLANSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
			break;
		default:
			throw TCoreException(eErr_BoundsExceeded, L"Unsupported buffer type", LOCATION);
		}
	}

	void TBufferSizes::SetDefaultSize(unsigned int uiSize)
	{
		m_uiDefaultSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
	}

	void TBufferSizes::SetOneDiskSize(unsigned int uiSize)
	{
		m_uiOneDiskSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
	}

	void TBufferSizes::SetTwoDisksSize(unsigned int uiSize)
	{
		m_uiTwoDisksSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
	}

	void TBufferSizes::SetCDSize(unsigned int uiSize)
	{
		m_uiCDSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
	}

	void TBufferSizes::SetLANSize(unsigned int uiSize)
	{
		m_uiLANSize = std::max(BufferGranularity, RoundUp(uiSize, BufferGranularity));
	}

	void TBufferSizes::SetBufferCount(unsigned int uiBufferCount)
	{
		m_uiBufferCount = std::max(uiBufferCount, MinBufferCount);
	}

	unsigned int TBufferSizes::GetMaxSize() const
	{
		if (m_bOnlyDefault)
			return m_uiDefaultSize;

		return std::max({ m_uiDefaultSize, m_uiOneDiskSize, m_uiTwoDisksSize, m_uiCDSize, m_uiLANSize });
	}
}
