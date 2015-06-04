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

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TBufferSizes
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

public:
	TBufferSizes();
	TBufferSizes(bool bOnlyDefault, UINT uiBufferCount, UINT uiDefaultSize,
		UINT uiOneDiskSize, UINT uiTwoDisksSize, UINT uiCDSize, UINT uiLANSize);

	void Clear();

	bool IsOnlyDefault() const { return m_bOnlyDefault; }
	UINT GetDefaultSize() const { return m_uiDefaultSize; }
	UINT GetOneDiskSize() const { return m_uiOneDiskSize; }
	UINT GetTwoDisksSize() const { return m_uiTwoDisksSize; }
	UINT GetCDSize() const { return m_uiCDSize; }
	UINT GetLANSize() const { return m_uiLANSize; }

	void SetOnlyDefault(bool bOnlyDefault) { m_bOnlyDefault = bOnlyDefault; }
	void SetDefaultSize(UINT uiSize);
	void SetOneDiskSize(UINT uiSize);
	void SetTwoDisksSize(UINT uiSize);
	void SetCDSize(UINT uiSize);
	void SetLANSize(UINT uiSize);

	UINT GetBufferCount() const { return m_uiBufferCount; }
	void SetBufferCount(UINT uiBufferCount);

	UINT GetSizeByType(EBufferType eType) const;
	void SetSizeByType(EBufferType eType, UINT uiSize);

	UINT GetMaxSize() const;

private:
	UINT m_uiDefaultSize;
	UINT m_uiOneDiskSize;
	UINT m_uiTwoDisksSize;
	UINT m_uiCDSize;
	UINT m_uiLANSize;

	bool m_bOnlyDefault;
	UINT m_uiBufferCount;
};

END_CHCORE_NAMESPACE

#endif
