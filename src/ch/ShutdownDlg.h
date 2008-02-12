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
#ifndef __SHUTDOWNDLG_H__
#define __SHUTDOWNDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CShutdownDlg dialog

class CShutdownDlg : public CHLanguageDialog
{
// Construction
public:
	CShutdownDlg();   // standard constructor

	int m_iOverallTime;

// Dialog Data
	//{{AFX_DATA(CShutdownDlg)
	enum { IDD = IDD_SHUTDOWN_DIALOG };
	CProgressCtrl	m_ctlProgress;
	CString	m_strTime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShutdownDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FormatTimeString(int iTime, CString* pstrData);
	int m_iTime;		// czas w sekundach

	// Generated message map functions
	//{{AFX_MSG(CShutdownDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
