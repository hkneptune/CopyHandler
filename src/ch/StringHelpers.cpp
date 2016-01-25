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
#include "ch.h"
#include "StringHelpers.h"
#include "stdio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

LPCTSTR GetSizeString(double dData, LPTSTR pszBuffer, size_t stMaxBufferSize)
{
	if (dData < 0.0)
		dData=0.0;

	if (dData < 1200.0)
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), dData, GetResManager().LoadString(IDS_BYTE_STRING));
	else if (dData < 1228800.0)
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), static_cast<double>(dData)/1024.0, GetResManager().LoadString(IDS_KBYTE_STRING));
	else if (dData < 1258291200.0)
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), static_cast<double>(dData)/1048576.0, GetResManager().LoadString(IDS_MBYTE_STRING));
	else
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), static_cast<double>(dData)/1073741824.0, GetResManager().LoadString(IDS_GBYTE_STRING));

	return pszBuffer;
}

LPCTSTR GetSizeString(unsigned long long ullData, LPTSTR pszBuffer, size_t stMaxBufferSize, bool bStrict)
{
	if (ullData < 0)
		ullData=0;

	if (ullData >= 1258291200 && (!bStrict || (ullData % 1073741824) == 0))
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), (double)(ullData/1073741824.0), GetResManager().LoadString(IDS_GBYTE_STRING));
	else if (ullData >= 1228800 && (!bStrict || (ullData % 1048576) == 0))
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), (double)(ullData/1048576.0), GetResManager().LoadString(IDS_MBYTE_STRING));
	else if (ullData >= 1200 && (!bStrict || (ullData % 1024) == 0))
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%.2f %s"), (double)(ullData/1024.0), GetResManager().LoadString(IDS_KBYTE_STRING));
	else
		_sntprintf(pszBuffer, stMaxBufferSize, _T("%I64u %s"), ullData, GetResManager().LoadString(IDS_BYTE_STRING));

	return pszBuffer;
}
