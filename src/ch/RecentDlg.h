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
#ifndef __RECENTDLG_H__
#define __RECENTDLG_H__

#include "afxtempl.h"
#include "charvect.h"

/////////////////////////////////////////////////////////////////////////////
// CRecentDlg dialog

class CRecentDlg : public CHLanguageDialog
{
// Construction
public:
	CRecentDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRecentDlg)
	enum { IDD = IDD_RECENTEDIT_DIALOG };
	CListCtrl	m_ctlRecent;
	CString	m_strPath;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecentDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	char_vector m_cvRecent;
	HIMAGELIST m_himl, m_hliml;

protected:

	// Generated message map functions
	//{{AFX_MSG(CRecentDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedRecentList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBrowseButton();
	afx_msg void OnAddButton();
	afx_msg void OnChangeButton();
	afx_msg void OnDeleteButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
