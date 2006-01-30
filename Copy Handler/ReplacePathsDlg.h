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
#ifndef __REPLACEPATHSDLG_H__
#define __REPLACEPATHSDLG_H__

#include "structs.h"

/////////////////////////////////////////////////////////////////////////////
// CReplacePathsDlg dialog

class CReplacePathsDlg : public CHLanguageDialog
{
// Construction
public:
	CReplacePathsDlg();   // standard constructor

	CTask* m_pTask;
// Dialog Data
	//{{AFX_DATA(CReplacePathsDlg)
	enum { IDD = IDD_REPLACE_PATHS_DIALOG };
	CListBox	m_ctlPathsList;
	CString	m_strDest;
	CString	m_strSource;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReplacePathsDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReplacePathsDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePathsList();
	virtual void OnOK();
	afx_msg void OnBrowseButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
