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
#include "resource.h"
#include "AboutDlg.h"
#include "StaticEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

bool CAboutDlg::m_bLock=false;

CAboutDlg::CAboutDlg() : CHLanguageDialog(CAboutDlg::IDD, NULL, &m_bLock)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
	RegisterStaticExControl(AfxGetInstanceHandle());
}

CAboutDlg::~CAboutDlg()
{
}

BEGIN_MESSAGE_MAP(CAboutDlg, CHLanguageDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CAboutDlg::UpdateProgramVersion()
{
	CWnd* pCtl=(CWnd*)GetDlgItem(IDC_PROGRAM_STATICEX);
	CWnd* pCtl2=(CWnd*)GetDlgItem(IDC_FULLVERSION_STATICEX);
	if (!pCtl || !pCtl2)
		return;
	else
	{
		TCHAR szFull[256];
		_stprintf(szFull, GetResManager()->LoadString(IDS_ABOUTVERSION_STRING), GetApp()->GetAppVersion());

		pCtl->SetWindowText(GetApp()->GetAppNameVer());
		pCtl2->SetWindowText(szFull);
	}
}

void CAboutDlg::UpdateThanks()
{
	CEdit* pEdit=(CEdit*)GetDlgItem(IDC_THANX_EDIT);
	if (pEdit == NULL)
		return;

	// get the info about current translations
	TCHAR szData[1024];
	GetConfig()->GetStringValue(PP_PLANGDIR, szData, 1024);
	GetApp()->ExpandPath(szData);
	vector<CLangData> vData;
	GetResManager()->Scan(szData, &vData);

	// format the info
	TCHAR szTI[8192];
	szTI[0]=_T('\0');
	for (vector<CLangData>::iterator it=vData.begin();it!=vData.end();it++)
	{
		_stprintf(szData, _T("%s\t\t%s [%s%lu, %s%s]\r\n"), it->GetAuthor(), it->GetLangName(), GetResManager()->LoadString(IDS_LANGCODE_STRING), it->GetLangCode(), GetResManager()->LoadString(IDS_LANGVER_STRING), it->GetVersion());
		_tcscat(szTI, szData);
	}

	TCHAR szText[16384];
	_sntprintf(szText, 16384, GetResManager()->LoadString(IDR_THANKS_TEXT), szTI);
	szText[16383]=0;
	pEdit->SetWindowText(szText);
}

BOOL CAboutDlg::OnInitDialog()
{
	CHLanguageDialog::OnInitDialog();

	UpdateProgramVersion();
	UpdateThanks();

	return TRUE;
}

void CAboutDlg::OnLanguageChanged(WORD /*wOld*/, WORD /*wNew*/)
{
	UpdateProgramVersion();
	UpdateThanks();
}

BOOL CAboutDlg::OnTooltipText(UINT uiID, TOOLTIPTEXT* pTip)
{
	switch(uiID)
	{
	case IDC_HOMEPAGELINK_STATIC:
	case IDC_HOMEPAGELINK2_STATIC:
	case IDC_CONTACT1LINK_STATIC:
	case IDC_CONTACT2LINK_STATIC:
	case IDC_CONTACT3LINK_STATIC:
	case IDC_GENFORUMPAGELINK_STATIC:
	case IDC_GENFORUMSUBSCRIBELINK_STATIC:
	case IDC_GENFORUMUNSUBSCRIBELINK_STATIC:
	case IDC_GENFORUMSENDLINK_STATIC:
	case IDC_DEVFORUMPAGELINK_STATIC:
	case IDC_DEVFORUMSUBSCRIBELINK_STATIC:
	case IDC_DEVFORUMUNSUBSCRIBELINK_STATIC:
	case IDC_DEVFORUMSENDLINK_STATIC:
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
