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
#include "DstFileErrorDlg.h"
#include "btnIDs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDstFileErrorDlg dialog

CDstFileErrorDlg::CDstFileErrorDlg()
	:ictranslate::CLanguageDialog(CDstFileErrorDlg::IDD)
{
	//{{AFX_DATA_INIT(CDstFileErrorDlg)
	m_strMessage = _T("");
	m_strFilename = _T("");
	//}}AFX_DATA_INIT
	m_bEnableTimer=false;
	m_iDefaultOption=ID_RECOPY;
	m_iTime=30000;
}


void CDstFileErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDstFileErrorDlg)
	DDX_Text(pDX, IDC_MESSAGE_EDIT, m_strMessage);
	DDX_Text(pDX, IDC_FILENAME_EDIT, m_strFilename);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDstFileErrorDlg,ictranslate::CLanguageDialog)
	//{{AFX_MSG_MAP(CDstFileErrorDlg)
	ON_BN_CLICKED(IDC_RETRY_BUTTON, OnRetryButton)
	ON_BN_CLICKED(IDC_IGNORE_BUTTON, OnIgnoreButton)
	ON_BN_CLICKED(IDC_WAIT_BUTTON, OnWaitButton)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_IGNORE_ALL_BUTTON, OnIgnoreAllButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDstFileErrorDlg message handlers

void CDstFileErrorDlg::OnRetryButton() 
{
	EndDialog(ID_RETRY);	
}

void CDstFileErrorDlg::OnIgnoreButton() 
{
	EndDialog(ID_IGNORE);
}

void CDstFileErrorDlg::OnWaitButton() 
{
	EndDialog(ID_WAIT);	
}

void CDstFileErrorDlg::OnTimer(UINT_PTR nIDEvent) 
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

BOOL CDstFileErrorDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	// make this dialog on top
	SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE /*| SWP_SHOWWINDOW*/);
	
	// remember title
	GetWindowText(m_strTitle);

	if (m_bEnableTimer)
		SetTimer(1601, 1000, NULL);
	
	return TRUE;
}

void CDstFileErrorDlg::OnIgnoreAllButton() 
{
	EndDialog(ID_IGNOREALL);	
}
