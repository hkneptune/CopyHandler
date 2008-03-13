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
#include "ch.h"
#include "resource.h"
#include "AboutDlg.h"
#include "StaticEx.h"
#include "../common/version.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CAboutDlg::m_bLock=false;

CAboutDlg::CAboutDlg() :ictranslate::CLanguageDialog(CAboutDlg::IDD, NULL, &m_bLock)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
	RegisterStaticExControl(AfxGetInstanceHandle());
}

CAboutDlg::~CAboutDlg()
{
}

BEGIN_MESSAGE_MAP(CAboutDlg,ictranslate::CLanguageDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAboutDlg::UpdateProgramVersion()
{
	CWnd* pCtl=(CWnd*)GetDlgItem(IDC_PROGRAM_STATICEX);
	CWnd* pCtl2=(CWnd*)GetDlgItem(IDC_FULLVERSION_STATICEX);
	CWnd* pWndCopyright = GetDlgItem(IDC_COPYRIGHT_STATIC);
	if (!pCtl || !pCtl2 || !pWndCopyright)
		return;
	else
	{
		TCHAR szFull[256];
		_sntprintf(szFull, 256, GetResManager()->LoadString(IDS_ABOUTVERSION_STRING), GetApp()->GetAppVersion());

		pCtl->SetWindowText(GetApp()->GetAppNameVer());
		pCtl2->SetWindowText(szFull);
		pWndCopyright->SetWindowText(_T(COPYRIGHT_INFO));
	}
}

BOOL CAboutDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	UpdateProgramVersion();

	return TRUE;
}

void CAboutDlg::OnLanguageChanged(WORD /*wOld*/, WORD /*wNew*/)
{
	UpdateProgramVersion();
}

BOOL CAboutDlg::OnTooltipText(UINT uiID, TOOLTIPTEXT* pTip)
{
	switch(uiID)
	{
	case IDC_HOMEPAGELINK_STATIC:
	case IDC_CONTACT1LINK_STATIC:
	case IDC_CONTACT2LINK_STATIC:
		{
			HWND hWnd=::GetDlgItem(this->m_hWnd, uiID);
			if (!hWnd)
				return FALSE;
			::SendMessage(hWnd, SEM_GETLINK, (WPARAM)79, (LPARAM)pTip->szText);
			pTip->szText[79]=_T('\0');
			return TRUE;
		}
	default:
		return FALSE;
	}
}
