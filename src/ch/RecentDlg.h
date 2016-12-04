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
#ifndef __RECENTDLG_H__
#define __RECENTDLG_H__

/////////////////////////////////////////////////////////////////////////////
// CRecentDlg dialog

class CRecentDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	explicit CRecentDlg(CWnd* pParent = nullptr);   // standard constructor

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support

protected:
	BOOL OnInitDialog() override;
	afx_msg void OnItemchangedRecentList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBrowseButton();
	afx_msg void OnAddButton();
	afx_msg void OnChangeButton();
	afx_msg void OnDeleteButton();

	DECLARE_MESSAGE_MAP()

public:
	std::vector<CString> m_cvRecent;

private:
	HIMAGELIST m_himl = nullptr;
	HIMAGELIST m_hliml = nullptr;
	CListCtrl m_ctlRecent;
	CString m_strPath;
};

#endif
