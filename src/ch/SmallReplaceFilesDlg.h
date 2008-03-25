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
#ifndef __SMALLREPLACEFILESDLG_H__
#define __SMALLREPLACEFILESDLG_H__

#include "FileInfo.h"

/////////////////////////////////////////////////////////////////////////////
// CSmallReplaceFilesDlg dialog

class CSmallReplaceFilesDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	CSmallReplaceFilesDlg();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSmallReplaceFilesDlg)
	enum { IDD = IDD_FEEDBACK_SMALL_REPLACE_FILES_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	CFileInfo *m_pfiSource, *m_pfiDest;

	CString m_strTitle;
	bool m_bEnableTimer;
	int m_iTime;
	int m_iDefaultOption;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSmallReplaceFilesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSmallReplaceFilesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnRecopyButton();
	afx_msg void OnRecopyAllButton();
	afx_msg void OnIgnoreButton();
	afx_msg void OnIgnoreAllButton();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
