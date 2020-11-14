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
#ifndef __FEEDBACKFILEERRORDLG_H__
#define __FEEDBACKFILEERRORDLG_H__

#include "../libchengine/FeedbackErrorRuleList.h"
#include "../libchengine/FeedbackRules.h"

// CFeedbackFileErrorDlg dialog
class CFeedbackFileErrorDlg : public ictranslate::CLanguageDialog
{
	DECLARE_DYNAMIC(CFeedbackFileErrorDlg)

public:
	CFeedbackFileErrorDlg(chengine::FeedbackRules& currentRules, const wchar_t* pszSrcPath, const wchar_t* pszDstPath, unsigned long ulSysError, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CFeedbackFileErrorDlg();

	bool IsApplyToAllItemsChecked() const;

	const chengine::FeedbackRules& GetRules() const;

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void OnCancel() override;
	BOOL OnInitDialog() override;

	afx_msg void OnBnClickedRetryButton();
	afx_msg void OnBnClickedSkipButton();
	afx_msg void OnBnClickedPauseButton();
	afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bAllItems = FALSE;
	CStatic m_ctlErrorInfo;
	CString m_strSrcPath;
	CString m_strDstPath;
	unsigned long m_ulSysError = 0;

	chengine::FeedbackRules& m_rules;
};

#endif
