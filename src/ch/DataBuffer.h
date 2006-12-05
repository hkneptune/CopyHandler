/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#define DEFAULT_SIZE	65536

#define ROUNDTODS(number)\
	((number + DEFAULT_SIZE - 1) & ~(DEFAULT_SIZE-1))

#define ROUNDUP(number, to)\
	((number + to - 1) & ~(to-1))

#define ROUNDDOWN(number, to)\
	(number & ~(to-1))

#define BI_DEFAULT		0
#define BI_ONEDISK		1
#define BI_TWODISKS		2
#define BI_CD			3
#define BI_LAN			4

#pragma warning (disable: 4201) 
struct BUFFERSIZES
{
	void Serialize(CArchive& ar);
	bool operator==(const BUFFERSIZES& bsSizes) const;
	union
	{
		struct
		{
			UINT m_uiDefaultSize;		// default buffer size
			UINT m_uiOneDiskSize;		// inside one disk boundary
			UINT m_uiTwoDisksSize;		// two disks
			UINT m_uiCDSize;			// CD<->anything
			UINT m_uiLANSize;			// LAN<->anything
		};
		UINT m_auiSizes[5];
	};
	bool m_bOnlyDefault;
};
#pragma warning (default: 4201)

class CDataBuffer
{
public:
	CDataBuffer() { m_pBuffer=NULL; m_uiRealSize=0; m_bsSizes.m_uiDefaultSize=0; m_bsSizes.m_uiOneDiskSize=0; m_bsSizes.m_uiTwoDisksSize=0; m_bsSizes.m_uiCDSize=0; m_bsSizes.m_uiLANSize=0; m_bsSizes.m_bOnlyDefault=false; };
	~CDataBuffer() { Delete(); };

	const BUFFERSIZES* Create(const BUFFERSIZES* pbsSizes);	// (re)allocates the buffer; if there's an error - restores previous buffer size
	void Delete();				// deletes buffer

	UINT GetRealSize() { return m_uiRealSize; };
	UINT GetDefaultSize() { return m_bsSizes.m_uiDefaultSize; };
	UINT GetOneDiskSize() { return m_bsSizes.m_uiOneDiskSize; };
	UINT GetTwoDisksSize() { return m_bsSizes.m_uiTwoDisksSize; };
	UINT GetCDSize() { return m_bsSizes.m_uiCDSize; };
	UINT GetLANSize() { return m_bsSizes.m_uiLANSize; };
	const BUFFERSIZES* GetSizes() { return &m_bsSizes; }; 

	// operators
	operator unsigned char*() { return m_pBuffer; };
protected:
	unsigned char *m_pBuffer;	// buffer address
	UINT m_uiRealSize;			// real buffer size
	BUFFERSIZES m_bsSizes;
};

#endif