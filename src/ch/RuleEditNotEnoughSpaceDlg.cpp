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
#include "stdafx.h"
#include "ch.h"
#include "RuleEditNotEnoughSpaceDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace chengine;

RuleEditNotEnoughSpaceDlg::RuleEditNotEnoughSpaceDlg(const chengine::FeedbackNotEnoughSpaceRule& rRule) :
	CLanguageDialog(IDD_RULE_EDIT_NOTENOUGHSPACE_DIALOG),
	m_rule(rRule),
	m_comboResponse(m_ctlResponse, eResult_Ignore, eResult_Last)
{
}

void RuleEditNotEnoughSpaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_INCLUDE_MASK_CHECK, m_bUseIncludeMask);
	DDX_Control(pDX, IDC_INCLUDE_MASK_COMBO, m_ctlIncludeMask);

	DDX_Check(pDX, IDC_EXCLUDE_MASK_CHECK, m_bUseExcludeMask);
	DDX_Control(pDX, IDC_EXCLUDE_MASK_COMBO, m_ctlExcludeMask);

	DDX_Control(pDX, IDC_RESPONSE_COMBO, m_ctlResponse);
}

BEGIN_MESSAGE_MAP(RuleEditNotEnoughSpaceDlg,ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_BUTTON, OnIncludeMaskButton)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_BUTTON, OnExcludeMaskButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg message handlers

BOOL RuleEditNotEnoughSpaceDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	FillResponseCombo();

	m_bUseIncludeMask = m_rule.GetUseMask();

	m_ctlIncludeMask.SetCurSel(m_ctlIncludeMask.AddString(m_rule.GetCombinedMask().c_str()));
	for(int i = 0; i < m_astrAddMask.GetSize(); i++)
	{
		m_ctlIncludeMask.AddString(m_astrAddMask.GetAt(i));
	}

	m_bUseExcludeMask = m_rule.GetUseExcludeMask();
	m_ctlExcludeMask.SetCurSel(m_ctlExcludeMask.AddString(m_rule.GetCombinedExcludeMask().c_str()));
	for(int i = 0; i < m_astrAddExcludeMask.GetSize(); i++)
	{
		m_ctlExcludeMask.AddString(m_astrAddExcludeMask.GetAt(i));
	}

	// response
	EFeedbackResult eResult = m_rule.GetResult();
	m_comboResponse.SelectComboResult(eResult == eResult_Unknown ? eResult_Ignore : eResult);

	m_filterTypesWrapper.Init();

	UpdateData(FALSE);

	EnableControls();

	return TRUE;
}

void RuleEditNotEnoughSpaceDlg::FillResponseCombo()
{
	m_ctlResponse.ResetContent();

	std::vector<int> vEntries = {
		IDS_FEEDBACK_RESPONSE_CANCEL,
		IDS_FEEDBACK_RESPONSE_PAUSE,
		IDS_FEEDBACK_RESPONSE_RETRY,
		IDS_FEEDBACK_RESPONSE_IGNORE
	};
	for(int entry : vEntries)
	{
		const wchar_t* pszData = GetResManager().LoadString(entry);
		int iPos = m_ctlResponse.AddString(pszData);
		m_ctlResponse.SetItemData(iPos, entry - IDS_FEEDBACK_RESPONSE_UNKNOWN);
	}
}

void RuleEditNotEnoughSpaceDlg::OnIncludeMaskButton()
{
	m_filterTypesWrapper.StartTracking(true, *this, IDC_INCLUDE_MASK_BUTTON);
}

void RuleEditNotEnoughSpaceDlg::OnExcludeMaskButton()
{
	m_filterTypesWrapper.StartTracking(false, *this, IDC_EXCLUDE_MASK_BUTTON);
}

void RuleEditNotEnoughSpaceDlg::OnLanguageChanged()
{
	// combo result
	EFeedbackResult eResult = m_comboResponse.GetSelectedValue();
	FillResponseCombo();
	m_comboResponse.SelectComboResult(eResult);
}

void RuleEditNotEnoughSpaceDlg::EnableControls()
{
	UpdateData(TRUE);
	// mask
	m_ctlIncludeMask.EnableWindow(m_bUseIncludeMask);
	m_ctlExcludeMask.EnableWindow(m_bUseExcludeMask);
	GetDlgItem(IDC_INCLUDE_MASK_BUTTON)->EnableWindow(m_bUseIncludeMask);
	GetDlgItem(IDC_EXCLUDE_MASK_BUTTON)->EnableWindow(m_bUseExcludeMask);
}

void RuleEditNotEnoughSpaceDlg::OnOK() 
{
	UpdateData(TRUE);
	
	CString strText;
	m_ctlIncludeMask.GetWindowText(strText);
	m_rule.SetUseMask(((m_bUseIncludeMask != 0) && !strText.IsEmpty()));
	m_rule.SetCombinedMask((PCTSTR)strText);

	m_ctlExcludeMask.GetWindowText(strText);
	m_rule.SetUseExcludeMask((m_bUseExcludeMask != 0) && !strText.IsEmpty());
	m_rule.SetCombinedExcludeMask((PCTSTR)strText);

	// response
	m_rule.SetResult(m_comboResponse.GetSelectedValue());

	CLanguageDialog::OnOK();
}

BOOL RuleEditNotEnoughSpaceDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(HIWORD(wParam) == 0)
	{
		if(LOWORD(wParam) >= ID_POPUP_FILTER_FILE_WILDCARD && LOWORD(wParam) <= ID_POPUP_FILTER_SEPARATOR_CHAR)
		{
			CComboBox& rCombo = m_filterTypesWrapper.IsTrackingIncludeMask() ? m_ctlIncludeMask : m_ctlExcludeMask;
			m_filterTypesWrapper.OnCommand(LOWORD(wParam), rCombo);
		}
	}
	return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);
}
