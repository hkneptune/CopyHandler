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
#ifndef __STRINGHELPERS_H__
#define __STRINGHELPERS_H__

// formatting routines
#ifdef _MFC_VER
void ExpandFormatString(CString* pstrFmt, DWORD dwError);
#endif

LPTSTR GetSizeString(double dData, LPTSTR pszBuffer, size_t stMaxBufferSize);
LPTSTR GetSizeString(__int64 llData, LPTSTR pszBuffer, size_t stMaxBufferSize, bool bStrict=false);

#endif
