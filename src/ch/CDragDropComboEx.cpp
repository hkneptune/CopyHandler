// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#include "CDragDropComboEx.h"

BEGIN_MESSAGE_MAP(CDragDropComboEx, CComboBoxEx)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

CDragDropComboEx::CDragDropComboEx() : CComboBoxEx()
{
}

void CDragDropComboEx::PreSubclassWindow()
{
	DragAcceptFiles(TRUE);
}

void CDragDropComboEx::OnDropFiles(HDROP hDrop)
{
	UINT uiPathsCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

	wchar_t szFilename[ MAX_PATH + 1 ];
	for(UINT iIndex = 0; iIndex < uiPathsCount; ++iIndex)
	{
		if(DragQueryFile(hDrop, iIndex, szFilename, MAX_PATH + 1) != 0)
		{
			DWORD dwAttributes = GetFileAttributes(szFilename);
			if(dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				SetPath(szFilename);
				break;
			}
		}
	}

	DragFinish(hDrop);
}

void CDragDropComboEx::SetPath(const CString& strPath)
{
	// set current select to -1
	SetCurSel(-1);

	SHFILEINFO sfi;
	sfi.iIcon = -1;
	SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_SYSICONINDEX);

	COMBOBOXEXITEM cbi;
	cbi.mask = CBEIF_TEXT | CBEIF_IMAGE;
	cbi.iItem = -1;
	cbi.pszText = (PTSTR)(PCTSTR)strPath;
	cbi.iImage = sfi.iIcon;

	SetItem(&cbi);
}
