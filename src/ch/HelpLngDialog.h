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
#pragma once

#include "LanguageDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CHLanguageDialog dialog

class CHLanguageDialog : public CLanguageDialog
{
// Construction
public:
	CHLanguageDialog(bool* pLock=NULL) : CLanguageDialog(pLock) { };
	CHLanguageDialog(PCTSTR lpszTemplateName, CWnd* pParent = NULL, bool* pLock=NULL) : CLanguageDialog(lpszTemplateName, pParent, pLock) { };   // standard constructor
	CHLanguageDialog(UINT uiIDTemplate, CWnd* pParent = NULL, bool* pLock=NULL) : CLanguageDialog(uiIDTemplate, pParent, pLock) { };   // standard constructor
	
	BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	void OnContextMenu(CWnd* pWnd, CPoint point);
	void OnHelpButton();

	DECLARE_MESSAGE_MAP()
};