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
#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include "libchengine.h"

namespace chengine
{
	class LIBCHENGINE_API TBufferSizes
	{
	public:
		enum EBufferType
		{
			eBuffer_Default = 0,
			eBuffer_OneDisk = 1,
			eBuffer_TwoDisks = 2,
			eBuffer_CD = 3,
			eBuffer_LAN = 4,

			// do not remove this marker
			eBuffer_Last
		};

		static const unsigned int BufferGranularity = 4096;
		static const unsigned int MinBufferCount = 1;
		static const unsigned int MinReadAhead = 1;
		static const unsigned int MinConcurrentReads = 1;
		static const unsigned int MinConcurrentWrites = 1;

	public:
		TBufferSizes();
		TBufferSizes(bool bOnlyDefault, unsigned int uiBufferCount, unsigned int uiDefaultSize,
			unsigned int uiOneDiskSize, unsigned int uiTwoDisksSize, unsigned int uiCDSize, unsigned int uiLANSize,
			unsigned int uiMaxReadAheadBuffers, unsigned int uiMaxConcurrentReads, unsigned int uiMaxConcurrentWrites);

		void Clear();

		bool IsOnlyDefault() const { return m_bOnlyDefault; }
		unsigned int GetDefaultSize() const { return m_uiDefaultSize; }
		unsigned int GetOneDiskSize() const { return m_uiOneDiskSize; }
		unsigned int GetTwoDisksSize() const { return m_uiTwoDisksSize; }
		unsigned int GetCDSize() const { return m_uiCDSize; }
		unsigned int GetLANSize() const { return m_uiLANSize; }

		void SetOnlyDefault(bool bOnlyDefault) { m_bOnlyDefault = bOnlyDefault; }
		void SetDefaultSize(unsigned int uiSize);
		void SetOneDiskSize(unsigned int uiSize);
		void SetTwoDisksSize(unsigned int uiSize);
		void SetCDSize(unsigned int uiSize);
		void SetLANSize(unsigned int uiSize);

		unsigned int GetBufferCount() const { return m_uiBufferCount; }
		void SetBufferCount(unsigned int uiBufferCount);

		unsigned int GetSizeByType(EBufferType eType) const;
		void SetSizeByType(EBufferType eType, unsigned int uiSize);

		unsigned int GetMaxReadAheadBuffers() const { return m_uiMaxReadAheadBuffers; }
		void SetMaxReadAheadBuffers(unsigned int uiMaxReadAhead) { m_uiMaxReadAheadBuffers = uiMaxReadAhead; }

		unsigned int GetMaxConcurrentReads() const { return m_uiMaxConcurrentReads; }
		void SetMaxConcurrentReads(unsigned int uiMaxConcurrentReads) { m_uiMaxConcurrentReads = uiMaxConcurrentReads; }

		unsigned int GetMaxConcurrentWrites() const { return m_uiMaxConcurrentWrites; }
		void SetMaxConcurrentWrites(unsigned int uiMaxConcurrentWrites) { m_uiMaxConcurrentWrites = uiMaxConcurrentWrites; }

		unsigned int GetMaxSize() const;

	private:
		unsigned int m_uiDefaultSize = 0;
		unsigned int m_uiOneDiskSize = 0;
		unsigned int m_uiTwoDisksSize = 0;
		unsigned int m_uiCDSize = 0;
		unsigned int m_uiLANSize = 0;

		bool m_bOnlyDefault = false;
		unsigned int m_uiBufferCount = 0;

		unsigned int m_uiMaxReadAheadBuffers = 0;
		unsigned int m_uiMaxConcurrentReads = 0;
		unsigned int m_uiMaxConcurrentWrites = 0;
	};
}

#endif
