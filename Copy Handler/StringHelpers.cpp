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
#include "stdafx.h"
#include "Copy Handler.h"
#include "StringHelpers.h"
#include "stdio.h"

#ifdef _MFC_VER
void ExpandFormatString(CString* pstrFmt, DWORD dwError)
{
	// replace strings %errnum & %errdesc to something else
	TCHAR xx[_MAX_PATH];
	pstrFmt->Replace(_T("%errnum"), _itot(dwError, xx, 10));
	
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, xx, _MAX_PATH, NULL);

	while (xx[_tcslen(xx)-1] == _T('\n') || xx[_tcslen(xx)-1] == _T('\r'))
		xx[_tcslen(xx)-1] = _T('\0');

	pstrFmt->Replace(_T("%errdesc"), xx);
}
#endif

LPTSTR GetSizeString(double dData, LPTSTR pszBuffer)
{
	if (dData < 0.0)
		dData=0.0;

	if (dData < 1200.0)
		_stprintf(pszBuffer, _T("%.2f %s"), dData, GetResManager()->LoadString(IDS_BYTE_STRING));
	else if (dData < 1228800.0)
		_stprintf(pszBuffer, _T("%.2f %s"), static_cast<double>(dData)/1024.0, GetResManager()->LoadString(IDS_KBYTE_STRING));
	else if (dData < 1258291200.0)
		_stprintf(pszBuffer, _T("%.2f %s"), static_cast<double>(dData)/1048576.0, GetResManager()->LoadString(IDS_MBYTE_STRING));
	else
		_stprintf(pszBuffer, _T("%.2f %s"), static_cast<double>(dData)/1073741824.0, GetResManager()->LoadString(IDS_GBYTE_STRING));

	return pszBuffer;
}

LPTSTR GetSizeString(__int64 llData, LPTSTR pszBuffer, bool bStrict)
{
	if (llData < 0)
		llData=0;

	if (llData >= 1258291200 && (!bStrict || (llData % 1073741824) == 0))
		_stprintf(pszBuffer, _T("%.2f %s"), (double)(llData/1073741824.0), GetResManager()->LoadString(IDS_GBYTE_STRING));
	else if (llData >= 1228800 && (!bStrict || (llData % 1048576) == 0))
		_stprintf(pszBuffer, _T("%.2f %s"), (double)(llData/1048576.0), GetResManager()->LoadString(IDS_MBYTE_STRING));
	else if (llData >= 1200 && (!bStrict || (llData % 1024) == 0))
		_stprintf(pszBuffer, _T("%.2f %s"), (double)(llData/1024.0), GetResManager()->LoadString(IDS_KBYTE_STRING));
	else
		_stprintf(pszBuffer, _T("%I64u %s"), llData, GetResManager()->LoadString(IDS_BYTE_STRING));

	return pszBuffer;
}
