/***************************************************************************
*   Copyright (C) 2001-2008 by J�zef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "stdafx.h"
#include "resource.h"
#include "SmallReplaceFilesDlg.h"
#include "btnIDs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSmallReplaceFilesDlg dialog


CSmallReplaceFilesDlg::CSmallReplaceFilesDlg()
	:ictranslate::CLanguageDialog(CSmallReplaceFilesDlg::IDD)
{
	//{{AFX_DATA_INIT(CSmallReplaceFilesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// zeruj zmienne
	m_pfiSource=NULL;
	m_pfiDest=NULL;
	m_bEnableTimer=false;
	m_iDefaultOption=ID_RECOPY;
	m_iTime=30000;
}

void CSmallReplaceFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSmallReplaceFilesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSmallReplaceFilesDlg,ictranslate::CLanguageDialog)
	//{{AFX_MSG_MAP(CSmallReplaceFilesDlg)
	ON_BN_CLICKED(IDC_RECOPY_BUTTON, OnRecopyButton)
	ON_BN_CLICKED(IDC_RECOPY_ALL_BUTTON, OnRecopyAllButton)
	ON_BN_CLICKED(IDC_IGNORE_BUTTON, OnIgnoreButton)
	ON_BN_CLICKED(IDC_IGNORE_ALL_BUTTON, OnIgnoreAllButton)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSmallReplaceFilesDlg message handlers

BOOL CSmallReplaceFilesDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();
	
	// set on top
	SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE /*| SWP_SHOWWINDOW*/);

	ASSERT(m_pfiSource);
	ASSERT(m_pfiDest);

	// show attributes
	TCHAR xx[64];
	GetDlgItem(IDC_FILENAME_EDIT)->SetWindowText(m_pfiSource->GetFullFilePath());
	GetDlgItem(IDC_FILESIZE_EDIT)->SetWindowText(_i64tot(m_pfiSource->GetLength64(), xx, 10));
	GetDlgItem(IDC_CREATETIME_EDIT)->SetWindowText(m_pfiSource->GetCreationTime().Format(_T("%x %X")));
	GetDlgItem(IDC_MODIFY_TIME_EDIT)->SetWindowText(m_pfiSource->GetLastWriteTime().Format(_T("%x %X")));

	GetDlgItem(IDC_DEST_FILENAME_EDIT)->SetWindowText(m_pfiDest->GetFullFilePath());
	GetDlgItem(IDC_DEST_FILESIZE_EDIT)->SetWindowText(_i64tot(m_pfiDest->GetLength64(), xx, 10));
	GetDlgItem(IDC_DEST_CREATETIME_EDIT)->SetWindowText(m_pfiDest->GetCreationTime().Format(_T("%x %X")));
	GetDlgItem(IDC_DEST_MODIFYTIME_EDIT)->SetWindowText(m_pfiDest->GetLastWriteTime().Format(_T("%x %X")));
	
	GetWindowText(m_strTitle);

	if (m_bEnableTimer)
		SetTimer(1601, 1000, NULL);

	return TRUE;
}

void CSmallReplaceFilesDlg::OnRecopyButton() 
{
	EndDialog(ID_RECOPY);	
}

void CSmallReplaceFilesDlg::OnRecopyAllButton() 
{
	EndDialog(ID_RECOPYALL);	
}

void CSmallReplaceFilesDlg::OnIgnoreButton() 
{
	EndDialog(ID_IGNORE);	
}

void CSmallReplaceFilesDlg::OnIgnoreAllButton() 
{
	EndDialog(ID_IGNOREALL);	
}

void CSmallReplaceFilesDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == 1601)
	{
		m_iTime-=1000;
		if (m_iTime < 0)
			EndDialog(m_iDefaultOption);
		
		TCHAR xx[16];
		SetWindowText(m_strTitle+_T(" [")+CString(_itot(m_iTime/1000, xx, 10))+_T("]"));
	}
	
	CLanguageDialog::OnTimer(nIDEvent);
}
