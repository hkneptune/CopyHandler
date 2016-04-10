// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#include "stdafx.h"
#include "ch.h"
#include "StringHelpers.h"
#include "stdio.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CString GetSizeString(double dData)
{
	if (dData < 0.0)
		dData = 0.0;

	CString strResult;
	if (dData < 1200.0)
		strResult.Format(_T("%.2f %s"), dData, GetResManager().LoadString(IDS_BYTE_STRING));
	else if (dData < 1228800.0)
		strResult.Format(_T("%.2f %s"), static_cast<double>(dData) / 1024.0, GetResManager().LoadString(IDS_KBYTE_STRING));
	else if (dData < 1258291200.0)
		strResult.Format(_T("%.2f %s"), static_cast<double>(dData) / 1048576.0, GetResManager().LoadString(IDS_MBYTE_STRING));
	else
		strResult.Format(_T("%.2f %s"), static_cast<double>(dData) / 1073741824.0, GetResManager().LoadString(IDS_GBYTE_STRING));

	return strResult;
}

CString GetSizeString(unsigned long long ullData, bool bStrict)
{
	CString strResult;
	if (ullData >= 1258291200 && (!bStrict || (ullData % 1073741824) == 0))
		strResult.Format(_T("%.2f %s"), (double)(ullData / 1073741824.0), GetResManager().LoadString(IDS_GBYTE_STRING));
	else if (ullData >= 1228800 && (!bStrict || (ullData % 1048576) == 0))
		strResult.Format(_T("%.2f %s"), (double)(ullData / 1048576.0), GetResManager().LoadString(IDS_MBYTE_STRING));
	else if (ullData >= 1200 && (!bStrict || (ullData % 1024) == 0))
		strResult.Format(_T("%.2f %s"), (double)(ullData / 1024.0), GetResManager().LoadString(IDS_KBYTE_STRING));
	else
		strResult.Format(_T("%I64u %s"), ullData, GetResManager().LoadString(IDS_BYTE_STRING));

	return strResult;
}
