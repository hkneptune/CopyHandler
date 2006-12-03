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
#ifndef __DESTPATH_H__
#define __DESTPATH_H__

class CDestPath
{
public:
	CDestPath() { m_iDriveNumber=-1; m_uiDriveType=static_cast<UINT>(-1); };
	void SetPath(LPCTSTR lpszPath);
	const CString& GetPath() const { return m_strPath; };

	int GetDriveNumber() const { return m_iDriveNumber; };
	UINT GetDriveType() const { return m_uiDriveType; };

	void Serialize(CArchive& ar);

protected:
	CString m_strPath;	// always with ending '\\'
	int m_iDriveNumber;	// initialized within setpath (std -1)
	UINT m_uiDriveType;	// disk type - -1 if none, the rest like in GetDriveType
};

#endif