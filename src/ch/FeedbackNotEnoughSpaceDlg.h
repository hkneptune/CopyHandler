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
#ifndef __FEEDBACKNOTENOUGHSPACEDLG_H__
#define __FEEDBACKNOTENOUGHSPACEDLG_H__
#include "../libchcore/TLocalFilesystem.h"

/////////////////////////////////////////////////////////////////////////////
// CFeedbackNotEnoughSpaceDlg dialog

class CFeedbackNotEnoughSpaceDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	CFeedbackNotEnoughSpaceDlg(unsigned long long ullSizeRequired, const wchar_t* pszSrcPath, const wchar_t* pszDstPath);   // standard constructor

// Overrides
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
public:
	CString	m_strDisk;
	unsigned long long m_ullRequired;
	std::vector<std::wstring> m_vstrFiles;
	CListBox	m_ctlFiles;

protected:
	void UpdateDialog();
	void OnLanguageChanged() override;
	void OnCancel() override;

	BOOL OnInitDialog() override;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRetryButton();
	afx_msg void OnIgnoreButton();
	afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()

public:
	BOOL m_bAllItems;

private:
	chcore::TLocalFilesystem m_fsLocal;
};

#endif
