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

#include "../libchengine/FeedbackRules.h"

namespace chengine
{
	class TFileInfo;
}

class CFeedbackReplaceDlg : public ictranslate::CLanguageDialog
{
	DECLARE_DYNAMIC(CFeedbackReplaceDlg)

public:
	CFeedbackReplaceDlg(chengine::FeedbackRules& currentRules, const chengine::TFileInfo& spSrcFile, const chengine::TFileInfo& spDstFile, const string::TString& strSuggestedName, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFeedbackReplaceDlg();

	BOOL OnInitDialog() override;

	const chengine::FeedbackRules& GetRules() const;
	string::TString GetNewName() const { return m_strNewName; }

protected:
	void DoDataExchange(CDataExchange* pDX) override;
	void OnCancel() override;

	void RefreshFilesInfo();
	void RefreshImages();

	afx_msg void OnBnClickedReplaceButton();
	afx_msg void OnBnClickedRenameButton();
	afx_msg void OnBnClickedCopyRestButton();
	afx_msg void OnBnClickedSkipButton();
	afx_msg void OnBnClickedPauseButton();
	afx_msg void OnBnClickedCancelButton();
	afx_msg void OnBnCustomRulesButton();

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);

	DECLARE_MESSAGE_MAP()

private:
	CStatic m_ctlSrcIcon;
	CStatic m_ctlDstIcon;

	CEdit m_ctlSrcName;
	CEdit m_ctlSrcPath;
	CEdit m_ctlSrcSize;
	CEdit m_ctlSrcDate;

	CEdit m_ctlDstName;
	CEdit m_ctlDstRename;
	CEdit m_ctlDstPath;
	CEdit m_ctlDstSize;
	CEdit m_ctlDstDate;

	CMFCMenuButton m_btnReplace;
	CMFCMenuButton m_btnRename;
	CMFCMenuButton m_btnResume;
	CMFCMenuButton m_btnSkip;
	CMFCButton m_btnPause;
	CMFCButton m_btnCancel;
	CMFCButton m_btnCustomRules;

	CMenu m_menuMassReplace;
	CMenu m_menuMassRename;
	CMenu m_menuMassResume;
	CMenu m_menuMassSkip;

	CRect m_rcInitial;

	const chengine::TFileInfo& m_rSrcFile;
	const chengine::TFileInfo& m_rDstFile;

	chengine::FeedbackRules& m_rules;
	string::TString m_strNewName;
};
