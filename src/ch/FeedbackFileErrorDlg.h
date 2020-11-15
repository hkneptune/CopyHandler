// ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================

#pragma once

#include "../libchengine/FeedbackErrorRuleList.h"
#include "../libchengine/FeedbackRules.h"
#include "../libchengine/EFileError.h"

// CFeedbackFileErrorDlg dialog
class CFeedbackFileErrorDlg : public ictranslate::CLanguageDialog
{
	DECLARE_DYNAMIC(CFeedbackFileErrorDlg)

public:
	CFeedbackFileErrorDlg(chengine::FeedbackRules& currentRules, const wchar_t* pszSrcPath, const wchar_t* pszDstPath, chengine::EFileError eOperationType, unsigned long ulSysError, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFeedbackFileErrorDlg();

	const chengine::FeedbackRules& GetRules() const;

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void OnCancel() override;
	BOOL OnInitDialog() override;

	afx_msg void OnBnRetry();
	afx_msg void OnBnSkip();
	afx_msg void OnBnPause();
	afx_msg void OnBnCustomRules();

	DECLARE_MESSAGE_MAP()

private:
	CEdit m_ctlFirstName;
	CEdit m_ctlSecondName;
	CEdit m_ctlOperationType;
	CEdit m_ctlSystemError;

	CMFCMenuButton m_btnRetry;
	CMFCMenuButton m_btnSkip;
	CMFCButton m_btnCustomRules;
	CMFCButton m_btnPause;
	CMFCButton m_btnCancel;

	CMenu m_menuMassRetry;
	CMenu m_menuMassSkip;

	CString m_strSrcPath;
	CString m_strDstPath;
	chengine::EFileError m_eOperationType = chengine::eCheckForFreeSpace;
	unsigned long m_ulSysError = 0;

	chengine::FeedbackRules& m_rules;
};
