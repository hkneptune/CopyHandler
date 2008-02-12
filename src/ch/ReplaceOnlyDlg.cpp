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
#include "resource.h"
#include "ReplaceOnlyDlg.h"
#include "btnIDs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReplaceOnlyDlg dialog


CReplaceOnlyDlg::CReplaceOnlyDlg()
	: CHLanguageDialog(CReplaceOnlyDlg::IDD)
{
	//{{AFX_DATA_INIT(CReplaceOnlyDlg)
	m_strMessage = _T("");
	//}}AFX_DATA_INIT
	m_pfiSource=NULL;
	m_pfiDest=NULL;
	m_bEnableTimer=false;
	m_iDefaultOption=ID_RECOPY;
	m_iTime=30000;
}


void CReplaceOnlyDlg::DoDataExchange(CDataExchange* pDX)
{
	CHLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplaceOnlyDlg)
	DDX_Text(pDX, IDC_MESSAGE_EDIT, m_strMessage);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReplaceOnlyDlg, CHLanguageDialog)
	//{{AFX_MSG_MAP(CReplaceOnlyDlg)
	ON_BN_CLICKED(IDC_IGNORE_BUTTON, OnIgnoreButton)
	ON_BN_CLICKED(IDC_IGNORE_ALL_BUTTON, OnIgnoreAllButton)
	ON_BN_CLICKED(IDC_WAIT_BUTTON, OnWaitButton)
	ON_BN_CLICKED(IDC_RETRY_BUTTON, OnRetryButton)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplaceOnlyDlg message handlers

BOOL CReplaceOnlyDlg::OnInitDialog() 
{
	CHLanguageDialog::OnInitDialog();
	
	// make on top
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

	// remember the title
	GetWindowText(m_strTitle);

	if (m_bEnableTimer)
		SetTimer(1601, 1000, NULL);

	return TRUE;
}

void CReplaceOnlyDlg::OnIgnoreButton() 
{
	EndDialog(ID_IGNORE);
}

void CReplaceOnlyDlg::OnIgnoreAllButton() 
{
	EndDialog(ID_IGNOREALL);	
}

void CReplaceOnlyDlg::OnWaitButton() 
{
	EndDialog(ID_WAIT);	
}

void CReplaceOnlyDlg::OnRetryButton() 
{
	EndDialog(ID_RETRY);	
}

void CReplaceOnlyDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == 1601)
	{
		m_iTime-=1000;
		if (m_iTime < 0)
			EndDialog(m_iDefaultOption);
		
		TCHAR xx[16];
		SetWindowText(m_strTitle+_T(" [")+CString(_itot(m_iTime/1000, xx, 10))+_T("]"));
	}
	
	CHLanguageDialog::OnTimer(nIDEvent);
}
