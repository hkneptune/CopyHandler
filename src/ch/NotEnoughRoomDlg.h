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
#ifndef __NOTENOUGHROOMDLG_H__
#define __NOTENOUGHROOMDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CNotEnoughRoomDlg dialog

class CNotEnoughRoomDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	CNotEnoughRoomDlg();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CNotEnoughRoomDlg)
	enum { IDD = IDD_FEEDBACK_NOTENOUGHPLACE_DIALOG };
	CListBox	m_ctlFiles;
	//}}AFX_DATA

	CString m_strTitle;
	CStringArray m_strFiles;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNotEnoughRoomDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	bool m_bEnableTimer;
	int m_iTime;
	int m_iDefaultOption;
	CString	m_strDisk;
	__int64 m_llRequired;

protected:
	void UpdateDialog();
	virtual void OnLanguageChanged(WORD wOld, WORD wNew);

	// Generated message map functions
	//{{AFX_MSG(CNotEnoughRoomDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRetryButton();
	afx_msg void OnIgnoreButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
