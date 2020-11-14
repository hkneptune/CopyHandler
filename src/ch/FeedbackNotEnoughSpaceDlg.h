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

#include "../libchengine/TLocalFilesystem.h"
#include "../libchengine/FeedbackNotEnoughSpaceRuleList.h"
#include "../libchengine/FeedbackRules.h"

/////////////////////////////////////////////////////////////////////////////
// CFeedbackNotEnoughSpaceDlg dialog

class CFeedbackNotEnoughSpaceDlg : public ictranslate::CLanguageDialog
{
public:
	CFeedbackNotEnoughSpaceDlg(chengine::FeedbackRules& currentRules, unsigned long long ullSizeRequired, const wchar_t* pszSrcPath, const wchar_t* pszDstPath);   // standard constructor

	bool IsApplyToAllItemsChecked() const;

	const chengine::FeedbackRules& GetRules() const;

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	void UpdateDialog();
	void OnLanguageChanged() override;
	void OnCancel() override;

	BOOL OnInitDialog() override;

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnRetryButton();
	afx_msg void OnIgnoreButton();
	afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bAllItems = FALSE;
	CString m_strDisk;
	unsigned long long m_ullRequired = 0;
	std::vector<std::wstring> m_vstrFiles;

	CListBox m_ctlFiles;

	chengine::TLocalFilesystem m_fsLocal;
	chengine::FeedbackRules& m_rules;
};

#endif
