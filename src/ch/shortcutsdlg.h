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
#ifndef __SHORTCUTSDLG_H__
#define __SHORTCUTSDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CShortcutsDlg dialog

class CShortcutsDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	explicit CShortcutsDlg(CWnd* pParent = NULL);   // standard constructor

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	void UpdateComboIcon();
	void SetComboPath(LPCTSTR lpszPath);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnItemchangedShortcutList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEditchangePathComboboxex();
	afx_msg void OnAddButton();
	afx_msg void OnChangeButton();
	afx_msg void OnDeleteButton();
	afx_msg void OnBrowseButton();
	afx_msg void OnUpButton();
	afx_msg void OnDownButton();

	DECLARE_MESSAGE_MAP()

public:
	const std::vector<CString> *m_pcvRecent = nullptr;	// one way only
	std::vector<CString> m_cvShortcuts;		// two-way - shortcuts are being returned through this member

private:
	HIMAGELIST m_himl = nullptr;
	HIMAGELIST m_hliml = nullptr;
	bool m_bActualisation = false;
	CComboBoxEx	m_ctlPath;
	CListCtrl	m_ctlShortcuts;
	CString	m_strName;
};

#endif
