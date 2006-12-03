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
#include "ReplacePathsDlg.h"
#include "dialogs.h"
#include "ch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CReplacePathsDlg dialog


CReplacePathsDlg::CReplacePathsDlg()
	: CHLanguageDialog(CReplacePathsDlg::IDD)
{
	//{{AFX_DATA_INIT(CReplacePathsDlg)
	m_strDest = _T("");
	m_strSource = _T("");
	//}}AFX_DATA_INIT
}


void CReplacePathsDlg::DoDataExchange(CDataExchange* pDX)
{
	CHLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CReplacePathsDlg)
	DDX_Control(pDX, IDC_PATHS_LIST, m_ctlPathsList);
	DDX_Text(pDX, IDC_DESTINATION_EDIT, m_strDest);
	DDX_Text(pDX, IDC_SOURCE_EDIT, m_strSource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CReplacePathsDlg, CHLanguageDialog)
	//{{AFX_MSG_MAP(CReplacePathsDlg)
	ON_LBN_SELCHANGE(IDC_PATHS_LIST, OnSelchangePathsList)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, OnBrowseButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReplacePathsDlg message handlers

BOOL CReplacePathsDlg::OnInitDialog() 
{
	CHLanguageDialog::OnInitDialog();

	for (int i=0;i<m_pTask->GetClipboardDataSize();i++)
		m_ctlPathsList.AddString(m_pTask->GetClipboardData(i)->GetPath());
	
	return TRUE;
}

void CReplacePathsDlg::OnSelchangePathsList() 
{
	int iSel=m_ctlPathsList.GetCurSel();
	if (iSel == LB_ERR)
		return;

	m_ctlPathsList.GetText(iSel, m_strSource);
	UpdateData(FALSE);
}

void CReplacePathsDlg::OnOK() 
{
	UpdateData(TRUE);
	if (m_strSource.IsEmpty())
		MsgBox(IDS_SOURCESTRINGMISSING_STRING);
	else
		CHLanguageDialog::OnOK();
}

void CReplacePathsDlg::OnBrowseButton() 
{
	CString strPath;
	if (BrowseForFolder(GetResManager()->LoadString(IDS_BROWSE_STRING), &strPath))
	{
		UpdateData(TRUE);
		m_strDest=strPath;
		UpdateData(FALSE);
	}
}
