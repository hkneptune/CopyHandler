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
#include "DestPath.h"

void GetDriveData(LPCTSTR lpszPath, int* piDrvNum, UINT* puiDrvType)
{
	TCHAR drv[_MAX_DRIVE+1];
	
	_tsplitpath(lpszPath, drv, NULL, NULL, NULL);
	if(lstrlen(drv) != 0)
	{
		// add '\\'
		lstrcat(drv, _T("\\"));
		_tcsupr(drv);
		
		// disk number
		if (piDrvNum)
			*piDrvNum=drv[0]-_T('A');

		// disk type
		if (puiDrvType)
		{
			*puiDrvType=GetDriveType(drv);
			if (*puiDrvType == DRIVE_NO_ROOT_DIR)
				*puiDrvType=DRIVE_UNKNOWN;
		}
	}
	else
	{
		// there's no disk in a path
		if (piDrvNum)
			*piDrvNum=-1;

		if (puiDrvType)
		{
			// check for unc path
			if (_tcsncmp(lpszPath, _T("\\\\"), 2) == 0)
				*puiDrvType=DRIVE_REMOTE;
			else
				*puiDrvType=DRIVE_UNKNOWN;
		}
	}
}

void CDestPath::SetPath(LPCTSTR lpszPath)
{
	m_strPath=lpszPath;

	// make sure '\\' has been added
	if (m_strPath.Right(1) != _T('\\'))
		m_strPath+=_T('\\');

	GetDriveData(m_strPath, &m_iDriveNumber, &m_uiDriveType);
}

void CDestPath::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar<<m_strPath;
		ar<<m_iDriveNumber;
		ar<<m_uiDriveType;
	}
	else
	{
		ar>>m_strPath;
		ar>>m_iDriveNumber;
		ar>>m_uiDriveType;
	}
}
