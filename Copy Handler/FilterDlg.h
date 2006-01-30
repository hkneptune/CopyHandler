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
#ifndef __FILTERDLG_H__
#define __FILTERDLG_H__

#include "FileInfo.h"
#include <afxdtctl.h>

/////////////////////////////////////////////////////////////////////////////
// CFilterDlg dialog

class CFilterDlg : public CHLanguageDialog
{
// Construction
public:
	CFilterDlg();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFilterDlg)
	enum { IDD = IDD_FILTER_DIALOG };
	CComboBox	m_ctlExcludeMask;
	CSpinButtonCtrl	m_ctlSpin2;
	CSpinButtonCtrl	m_ctlSpin1;
	CDateTimeCtrl	m_ctlTime2;
	CDateTimeCtrl	m_ctlTime1;
	CComboBox	m_ctlSizeType2;
	CComboBox	m_ctlSizeType1;
	CComboBox	m_ctlSize2Multi;
	CComboBox	m_ctlSize1Multi;
	CComboBox	m_ctlFilter;
	CComboBox	m_ctlDateType;
	CComboBox	m_ctlDateType2;
	CDateTimeCtrl	m_ctlDate2;
	CComboBox	m_ctlDateType1;
	CDateTimeCtrl	m_ctlDate1;
	int		m_iArchive;
	BOOL	m_bAttributes;
	BOOL	m_bDate;
	BOOL	m_bDate2;
	int		m_iDirectory;
	BOOL	m_bFilter;
	int		m_iHidden;
	int		m_iReadOnly;
	BOOL	m_bSize;
	UINT	m_uiSize1;
	BOOL	m_bSize2;
	UINT	m_uiSize2;
	int		m_iSystem;
	BOOL	m_bExclude;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFilterDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	void SetSize2(unsigned __int64 ullSize);
	CFileFilter m_ffFilter;
	CStringArray m_astrAddMask;
	CStringArray m_astrAddExcludeMask;

protected:
	virtual void OnLanguageChanged(WORD wOld, WORD wNew);
	int GetMultiplier(int iIndex);
	void EnableControls();
	void SetSize1(unsigned __int64 ullSize);

	// Generated message map functions
	//{{AFX_MSG(CFilterDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnAttributesCheck();
	afx_msg void OnDateCheck();
	afx_msg void OnDate2Check();
	afx_msg void OnFilterCheck();
	afx_msg void OnSizeCheck();
	afx_msg void OnSize2Check();
	afx_msg void OnExcludemaskCheck();
	afx_msg void OnDatetimechangeTime1Datetimepicker(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDatetimechangeDate1Datetimepicker(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
