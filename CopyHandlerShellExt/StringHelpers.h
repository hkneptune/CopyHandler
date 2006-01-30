/************************************************************************
	Copy Handler 1.x - program for copying data	in Microsoft Windows
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

#include "..\common\ipcstructs.h"

extern CSharedConfigStruct* g_pscsShared;

LPTSTR GetSizeString(double dData, LPTSTR pszBuffer);

template<class T> LPTSTR GetSizeString(T tData, LPTSTR pszBuffer, bool bStrict=false)
{
	if (tData < 0)
		tData=0;

	if (tData >= 1258291200 && (!bStrict || (tData % 1073741824) == 0))
	{
		_stprintf(pszBuffer, _T("%.2f %s"), static_cast<double>(tData)/1073741824.0, g_pscsShared->szSizes[3]);
		return pszBuffer;
	}
	else if (tData >= 1228800 && (!bStrict || (tData % 1048576) == 0))
	{
		_stprintf(pszBuffer, _T("%.2f %s"), static_cast<double>(tData)/1048576.0, g_pscsShared->szSizes[2]);
		return pszBuffer;
	}
	else if (tData >= 1200 && (!bStrict || (tData % 1024) == 0))
	{
		_stprintf(pszBuffer, _T("%.2f %s"), static_cast<double>(tData)/1024.0, g_pscsShared->szSizes[1]);
		return pszBuffer;
	}
	else
	{
		_stprintf(pszBuffer, _T("%d %s"), tData, g_pscsShared->szSizes[0]);
		return pszBuffer;
	}
}

#endif
