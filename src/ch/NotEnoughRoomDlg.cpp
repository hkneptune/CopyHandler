/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
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
#include "ch.h"
#include "NotEnoughRoomDlg.h"
#include "btnIDs.h"
#include "StringHelpers.h"
#include "..\Common\FileSupport.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNotEnoughRoomDlg dialog


CNotEnoughRoomDlg::CNotEnoughRoomDlg()
	:ictranslate::CLanguageDialog(CNotEnoughRoomDlg::IDD)
{
	//{{AFX_DATA_INIT(CNotEnoughRoomDlg)
	//}}AFX_DATA_INIT
	m_bEnableTimer=false;
	m_iTime=30000;
	m_iDefaultOption=ID_IGNORE;
}


void CNotEnoughRoomDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNotEnoughRoomDlg)
	DDX_Control(pDX, IDC_FILES_LIST, m_ctlFiles);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNotEnoughRoomDlg,ictranslate::CLanguageDialog)
	//{{AFX_MSG_MAP(CNotEnoughRoomDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RETRY_BUTTON, OnRetryButton)
	ON_BN_CLICKED(IDC_IGNORE_BUTTON, OnIgnoreButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNotEnoughRoomDlg message handlers
void CNotEnoughRoomDlg::UpdateDialog()
{
	// format needed text
	ictranslate::CFormat fmt(GetResManager()->LoadString(IDS_NERPATH_STRING));
	fmt.SetParam(_t("%path"), m_strDisk);

	CWnd* pWnd=GetDlgItem(IDC_HEADER_STATIC);
	if (pWnd)
		pWnd->SetWindowText(fmt);

	// now the sizes
	TCHAR szData[128];
	pWnd=GetDlgItem(IDC_REQUIRED_STATIC);
	if (pWnd)
		pWnd->SetWindowText(GetSizeString(m_llRequired, szData, 128));
	__int64 llFree;
	pWnd=GetDlgItem(IDC_AVAILABLE_STATIC);
	if (pWnd && GetDynamicFreeSpace(m_strDisk, &llFree, NULL))
		pWnd->SetWindowText(GetSizeString(llFree, szData, 128));
}

BOOL CNotEnoughRoomDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();
	
	// set to top
	SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE /*| SWP_SHOWWINDOW*/);

	// needed data
	for (int i=0;i<m_strFiles.GetSize();i++)
		m_ctlFiles.AddString(m_strFiles.GetAt(i));

	// format needed text
	UpdateDialog();

	GetWindowText(m_strTitle);
	
	SetTimer(1601, 1000, NULL);

	return TRUE;
}

void CNotEnoughRoomDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == 1601)
	{
		// count the time if needed
		if (m_bEnableTimer)
		{
			m_iTime-=1000;
			if (m_iTime < 0)
				EndDialog(m_iDefaultOption);
			
			TCHAR xx[16];
			SetWindowText(m_strTitle+_T(" [")+CString(_itot(m_iTime/1000, xx, 10))+_T("]"));
		}

		// update free space
		__int64 llFree;
		CWnd *pWnd=GetDlgItem(IDC_AVAILABLE_STATIC);
		if (pWnd && GetDynamicFreeSpace(m_strDisk, &llFree, NULL))
		{
			TCHAR szData[128];
			pWnd->SetWindowText(GetSizeString(llFree, szData, 128));

			// end dialog if this is enough
			if (m_llRequired <= llFree)
			{
				CLanguageDialog::OnTimer(nIDEvent);
				EndDialog(ID_RETRY);
			}
		}
	}
	
	CLanguageDialog::OnTimer(nIDEvent);
}

void CNotEnoughRoomDlg::OnRetryButton() 
{
	EndDialog(ID_RETRY);	
}

void CNotEnoughRoomDlg::OnIgnoreButton() 
{
	EndDialog(ID_IGNORE);	
}

void CNotEnoughRoomDlg::OnLanguageChanged()
{
	UpdateDialog();
}
