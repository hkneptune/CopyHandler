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
#include "CDragDropListCtrl.h"

BEGIN_MESSAGE_MAP(CDragDropListCtrl, CListCtrl)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

CDragDropListCtrl::CDragDropListCtrl() :
	CListCtrl()
{
}

void CDragDropListCtrl::PreSubclassWindow()
{
	// some styles
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	DragAcceptFiles(TRUE);
}

void CDragDropListCtrl::OnDropFiles(HDROP hDrop)
{
	UINT uiPathsCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

	wchar_t szFilename[MAX_PATH + 1];
	for(UINT iIndex = 0; iIndex < uiPathsCount; ++iIndex)
	{
		if(DragQueryFile(hDrop, iIndex, szFilename, MAX_PATH + 1) != 0)
			AddPath(szFilename);
	}

	DragFinish(hDrop);
}

void CDragDropListCtrl::AddPath(const CString& strPath)
{
	// fill listbox with paths
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_IMAGE;
	lvi.iItem = GetItemCount();
	lvi.iSubItem = 0;

	// there's no need for a high speed so get the images
	SHFILEINFO sfi;
	SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	// fill the list
	lvi.pszText = (PTSTR)(PCTSTR)strPath;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	lvi.iImage = sfi.iIcon;
	InsertItem(&lvi);
}
