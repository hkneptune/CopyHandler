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

#include "CDragDropListCtrl.h"
#include "CDragDropComboEx.h"
#include "../libchengine/FeedbackRules.h"

/////////////////////////////////////////////////////////////////////////////
// RuleEditDlg dialog

class RuleEditDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	explicit RuleEditDlg(const chengine::FeedbackRules& rRules);

	chengine::FeedbackRules GetRules() const { return m_rules; }

protected:
	void DoDataExchange(CDataExchange* pDX) override;

	void OnLanguageChanged() override;

	// Generated message map functions
	BOOL OnInitDialog() override;

	void InitAlreadyExistsColumns();
	void InitErrorColumns();
	void InitNotEnoughSpaceColumns();

	void FillAlreadyExistsList();
	void FillErrorList();
	void FillNotEnoughSpaceList();

	void AddAlreadyExistsRule(const chengine::FeedbackAlreadyExistsRule& rRule, int iPos);
	void AddErrorRule(const chengine::FeedbackErrorRule& rRule, int iPos);
	void AddNotEnoughSpaceRule(const chengine::FeedbackNotEnoughSpaceRule& rRule, int iPos);

	void OnOK() override;

	void EnableControls();
	void EnableAlreadyExistsControls();
	void EnableErrorControls();
	void EnableNotEnoughSpaceControls();

	afx_msg void OnAlreadyExistsItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnErrorItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNotEnoughSpaceItemChanged(NMHDR* pNMHDR, LRESULT* pResult);

	afx_msg void OnDblclkAlreadyExistsList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAlreadyExistsChangeButton();
	afx_msg void OnAlreadyExistsAddButton();
	afx_msg void OnAlreadyExistsRemoveButton();
	afx_msg void OnAlreadyExistsUpButton();
	afx_msg void OnAlreadyExistsDownButton();

	afx_msg void OnDblclkErrorList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnErrorChangeButton();
	afx_msg void OnErrorAddButton();
	afx_msg void OnErrorRemoveButton();
	afx_msg void OnErrorUpButton();
	afx_msg void OnErrorDownButton();

	afx_msg void OnDblclkNotEnoughSpaceList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNotEnoughSpaceChangeButton();
	afx_msg void OnNotEnoughSpaceAddButton();
	afx_msg void OnNotEnoughSpaceRemoveButton();
	afx_msg void OnNotEnoughSpaceUpButton();
	afx_msg void OnNotEnoughSpaceDownButton();

	DECLARE_MESSAGE_MAP()

public:
	chengine::FeedbackRules m_rules;

	CListCtrl m_ctlAlreadyExistsRulesList;
	CListCtrl m_ctlErrorRulesList;
	CListCtrl m_ctlNotEnoughSpaceRulesList;
};
