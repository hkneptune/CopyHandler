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
#ifndef __BUFFERSIZEDLG_H__
#define __BUFFERSIZEDLG_H__

#include "DataBuffer.h"

/////////////////////////////////////////////////////////////////////////////
// CBufferSizeDlg dialog

class CBufferSizeDlg : public CHLanguageDialog
{
// Construction
public:
	CBufferSizeDlg();   // standard constructor

	void SetLANSize(UINT uiSize);
	void SetCDSize(UINT uiSize);
	void SetTwoDisksSize(UINT uiSize);
	void SetOneDiskSize(UINT uiSize);
	void SetDefaultSize(UINT uiSize);
	UINT IndexToValue(int iIndex);

	int m_iActiveIndex;
	BUFFERSIZES m_bsSizes;

// Dialog Data
	//{{AFX_DATA(CBufferSizeDlg)
	enum { IDD = IDD_BUFFERSIZE_DIALOG };
	CComboBox	m_ctlTwoDisksMulti;
	CComboBox	m_ctlOneDiskMulti;
	CComboBox	m_ctlLANMulti;
	CComboBox	m_ctlDefaultMulti;
	CComboBox	m_ctlCDROMMulti;
	UINT	m_uiDefaultSize;
	UINT	m_uiLANSize;
	UINT	m_uiCDROMSize;
	UINT	m_uiOneDiskSize;
	UINT	m_uiTwoDisksSize;
	BOOL	m_bOnlyDefaultCheck;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBufferSizeDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual void OnLanguageChanged(WORD wOld, WORD wNew);

	void EnableControls(bool bEnable=true);
	// Generated message map functions
	//{{AFX_MSG(CBufferSizeDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnOnlydefaultCheck();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
