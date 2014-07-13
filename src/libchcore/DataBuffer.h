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

#define DEFAULT_SIZE	65536

#define ROUNDTODS(number)\
	((number + DEFAULT_SIZE - 1) & ~(DEFAULT_SIZE-1))

#define ROUNDUP(number, to)\
	((number + to - 1) & ~(to-1))

#define ROUNDDOWN(number, to)\
	(number & ~(to-1))

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

public:
	TBufferSizes();

	bool operator==(const TBufferSizes& bsSizes) const;

	void Clear();

	bool IsOnlyDefault() const { return m_bOnlyDefault; }
	UINT GetDefaultSize() const { return m_auiSizes[eBuffer_Default]; }
	UINT GetOneDiskSize() const { return m_auiSizes[eBuffer_OneDisk]; }
	UINT GetTwoDisksSize() const { return m_auiSizes[eBuffer_TwoDisks]; }
	UINT GetCDSize() const { return m_auiSizes[eBuffer_CD]; }
	UINT GetLANSize() const { return m_auiSizes[eBuffer_LAN]; }
	UINT GetSizeByType(EBufferType eType) const;

	void SetOnlyDefault(bool bOnlyDefault) { m_bOnlyDefault = bOnlyDefault; }
	void SetDefaultSize(UINT uiSize) { m_auiSizes[eBuffer_Default] = uiSize; }
	void SetOneDiskSize(UINT uiSize) { m_auiSizes[eBuffer_OneDisk] = uiSize; }
	void SetTwoDisksSize(UINT uiSize) { m_auiSizes[eBuffer_TwoDisks] = uiSize; }
	void SetCDSize(UINT uiSize) { m_auiSizes[eBuffer_CD] = uiSize; }
	void SetLANSize(UINT uiSize) { m_auiSizes[eBuffer_LAN] = uiSize; }
	void SetSizeByType(EBufferType eType, UINT uiSize);

	UINT GetMaxSize() const;

private:
	UINT m_auiSizes[eBuffer_Last];
	bool m_bOnlyDefault;
};

END_CHCORE_NAMESPACE

#endif