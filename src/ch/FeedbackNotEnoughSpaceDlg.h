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

#include "../libchengine/TLocalFilesystem.h"
#include "../libchengine/FeedbackNotEnoughSpaceRuleList.h"
#include "../libchengine/FeedbackRules.h"

/////////////////////////////////////////////////////////////////////////////
// CFeedbackNotEnoughSpaceDlg dialog

class CFeedbackNotEnoughSpaceDlg : public ictranslate::CLanguageDialog
{
public:
	CFeedbackNotEnoughSpaceDlg(chengine::FeedbackRules& currentRules, unsigned long long ullSizeRequired, const wchar_t* pszDstPath);   // standard constructor

	const chengine::FeedbackRules& GetRules() const;

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void UpdateDialog();
	void OnLanguageChanged() override;
	void OnCancel() override;

	BOOL OnInitDialog() override;

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnRetry();
	afx_msg void OnBnIgnore();
	afx_msg void OnBnCustomRules();
	afx_msg void OnBnPause();

	DECLARE_MESSAGE_MAP()

private:
	CString m_strDstPath;
	unsigned long long m_ullRequired = 0;

	CMFCButton m_btnRetry;
	CMFCButton m_btnIgnore;
	CMFCButton m_btnCustomRules;
	CMFCButton m_btnPause;
	CMFCButton m_btnCancel;

	CEdit m_ctlLocationEdit;
	CStatic m_ctlRequiredSpaceStatic;
	CStatic m_ctlAvailableSpaceStatic;


	chengine::TLocalFilesystem m_fsLocal;
	chengine::FeedbackRules& m_rules;
};
