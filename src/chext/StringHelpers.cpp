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
#include "stdafx.h"
#include "StringHelpers.h"
#include "stdio.h"

LPTSTR GetSizeString(double dData, LPTSTR pszBuffer, size_t stMaxBufferSize)
{
	if (dData < 0.0)
		dData=0.0;

	if (dData < 1200.0)
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), dData, g_pscsShared->szSizes[0]);
	else if (dData < 1228800.0)
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), static_cast<double>(dData)/1024.0, g_pscsShared->szSizes[1]);
	else if (dData < 1258291200.0)
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), static_cast<double>(dData)/1048576.0, g_pscsShared->szSizes[2]);
	else
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), static_cast<double>(dData)/1073741824.0, g_pscsShared->szSizes[3]);

	return pszBuffer;
}
