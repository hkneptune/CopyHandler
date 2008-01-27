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
#include "clipboard.h"

void GetDataFromClipboard(HDROP hdrop, LPCTSTR pszDstPath, LPTSTR *pszBuffer, UINT* pSize)
{
	// get clipboard data
	UINT uiBufferSize=(pszDstPath == NULL) ? 0 : _tcslen(pszDstPath)+1;
	UINT uiFilesCount=DragQueryFile(hdrop, 0xffffffff, NULL, 0);
	
	// count size
	for (UINT i=0;i<uiFilesCount;i++)
		uiBufferSize+=DragQueryFile(hdrop, i, NULL, 0)+1;
	
	// new buffer
	*pszBuffer=new TCHAR[uiBufferSize];
	*pSize=uiBufferSize;
	TCHAR szPath[_MAX_PATH];
	
	// copy pszDstPath
	if (pszDstPath != NULL)
		_tcscpy(*pszBuffer, pszDstPath);
	
	// size of pszDstPath
	UINT uiOffset=(pszDstPath == NULL) ? 0 : _tcslen(pszDstPath)+1;
	
	// get files and put it in a table
	UINT uiSize;
	for (UINT i=0;i<uiFilesCount;i++)
	{
		uiSize=DragQueryFile(hdrop, i, szPath, _MAX_PATH);
		_tcscpy(*pszBuffer+uiOffset, szPath);
		uiOffset+=uiSize+1;
	}
}
