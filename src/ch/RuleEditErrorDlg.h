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

#include "../libchengine/FeedbackErrorRule.h"
#include "ComboDataWrapper.h"
#include "FilterTypesMenuWrapper.h"

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg dialog

class RuleEditErrorDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	RuleEditErrorDlg(const chengine::FeedbackErrorRule& rRule);

	chengine::FeedbackErrorRule GetRule() const { return m_rule; }

protected:
	void OnLanguageChanged() override;
	void EnableControls();

	BOOL OnInitDialog() override;

	void FillOperationCombo();

	void FillResponseCombo();
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

	afx_msg void OnIncludeMaskButton();
	afx_msg void OnExcludeMaskButton();

	DECLARE_MESSAGE_MAP()

private:
	BOOL m_bUseIncludeMask = FALSE;
	CComboBox m_ctlIncludeMask;
	BOOL m_bUseExcludeMask = FALSE;
	CComboBox m_ctlExcludeMask;
	BOOL m_bUseOperationType = FALSE;
	CComboBox m_ctlOperationType;
	BOOL m_bUseSystemError = FALSE;
	CEdit m_ctlSystemError;

	CComboBox m_ctlResponse;

	chengine::FeedbackErrorRule m_rule;

	CStringArray m_astrAddMask;
	CStringArray m_astrAddExcludeMask;

	ComboDataWrapper<chengine::EFeedbackResult> m_comboResponse;
	ComboDataWrapper<chengine::EFileError> m_comboOperationType;

	FilterTypesMenuWrapper m_filterTypesWrapper;
};
