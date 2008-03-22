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
#ifndef __OPTIONSDLG_H__
#define __OPTIONSDLG_H__

#include "PropertyListCtrl.h"
#include "structs.h"
#include "shortcuts.h"
#include "charvect.h"

#define WM_CONFIGNOTIFY		WM_USER+13

/////////////////////////////////////////////////////////////////////////////
// COptionsDlg dialog

class COptionsDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	void SendClosingNotify();
	COptionsDlg(CWnd* pParent = NULL);   // standard constructor

	virtual void OnLanguageChanged();

	static bool m_bLock;				// locker

	char_vector m_cvRecent;
	char_vector m_cvShortcuts;

	// for languages
	vector<ictranslate::CLangData> m_vld;
	TCHAR m_szLangPath[_MAX_PATH];	// the full path to a folder with langs (@read)

	friend void CustomPropertyCallbackProc(LPVOID lpParam, int iParam, CPtrList* pList, int iIndex);
	friend void ShortcutsPropertyCallbackProc(LPVOID lpParam, int iParam, CPtrList* pList, int iIndex);
	friend void RecentPropertyCallbackProc(LPVOID lpParam, int iParam, CPtrList* pList, int iIndex);

// Dialog Data
	//{{AFX_DATA(COptionsDlg)
	enum { IDD = IDD_OPTIONS_DIALOG };
	CPropertyListCtrl	m_ctlProperties;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COptionsDlg)
	public:
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void FillPropertyList();
	void ApplyProperties();

	int GetIndexProp(int iPosition);
	CString GetStringProp(int iPosition);
	UINT GetUintProp(int iPosition);
	bool GetBoolProp(int iPosition);
	CString MakeCompoundString(UINT uiBase, int iCount, LPCTSTR lpszSeparator);

	TCHAR m_szBuffer[_MAX_PATH];	// for macro use
	CString m_strTemp;
	int m_iSel;

	// Generated message map functions
	//{{AFX_MSG(COptionsDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnApplyButton();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
