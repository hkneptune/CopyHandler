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
#include "HelpLngDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_ICTRANSLATE_NAMESPACE

/////////////////////////////////////////////////////////////////////////////
// CHLanguageDialog dialog

BEGIN_MESSAGE_MAP(CHLanguageDialog, CLanguageDialog)
	ON_WM_HELPINFO()
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDHELP, OnHelpButton)
END_MESSAGE_MAP()

BOOL CHLanguageDialog::OnHelpInfo(HELPINFO* pHelpInfo)
{
	if (pHelpInfo->iContextType == HELPINFO_WINDOW)
	{
		pHelpInfo->dwContextId=(m_uiResID << 16) | pHelpInfo->iCtrlId;
		AfxGetApp()->HtmlHelp((DWORD_PTR)pHelpInfo, HH_DISPLAY_TEXT_POPUP);
		return true;
	}
	else
		return false;
}

void CHLanguageDialog::OnContextMenu(CWnd* pWnd, CPoint point)
{
	HELPINFO hi;
	hi.cbSize=sizeof(HELPINFO);
	hi.iCtrlId=pWnd->GetDlgCtrlID();
	hi.dwContextId=(m_uiResID << 16) | hi.iCtrlId;
	hi.hItemHandle=pWnd->m_hWnd;
	hi.iContextType=HELPINFO_WINDOW;
	hi.MousePos=point;

	HtmlHelp((DWORD_PTR)&hi, HH_DISPLAY_TEXT_POPUP);
}

void CHLanguageDialog::OnHelpButton() 
{
	HtmlHelp(m_uiResID+0x20000, HH_HELP_CONTEXT);
}

END_ICTRANSLATE_NAMESPACE
