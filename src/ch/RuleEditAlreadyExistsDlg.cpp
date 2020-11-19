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
#include "RuleEditAlreadyExistsDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace chengine;

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg dialog

RuleEditAlreadyExistsDlg::RuleEditAlreadyExistsDlg(const chengine::FeedbackAlreadyExistsRule& rRule) :
	CLanguageDialog(IDD_RULE_EDIT_ALREADYEXISTS_DIALOG),
	m_rule(rRule),
	m_comboResponse(m_ctlResponse, eResult_Overwrite, eResult_Last),
	m_comboDateCompare(m_ctlDateCompareType, eCmp_Equal, eCmp_Last),
	m_comboSizeCompare(m_ctlSizeCompareType, eCmp_Equal, eCmp_Last)
{
}

void RuleEditAlreadyExistsDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_INCLUDE_MASK_CHECK, m_bUseIncludeMask);
	DDX_Control(pDX, IDC_INCLUDE_MASK_COMBO, m_ctlIncludeMask);

	DDX_Check(pDX, IDC_EXCLUDE_MASK_CHECK, m_bUseExcludeMask);
	DDX_Control(pDX, IDC_EXCLUDE_MASK_COMBO, m_ctlExcludeMask);

	DDX_Check(pDX, IDC_FILTER_BY_DATE_CHECK, m_bUseDateCompareType);
	DDX_Control(pDX, IDC_FILTER_BY_DATE_COMBO, m_ctlDateCompareType);

	DDX_Check(pDX, IDC_FILTER_BY_SIZE_CHECK, m_bUseSizeCompareType);
	DDX_Control(pDX, IDC_FILTER_BY_SIZE_COMBO, m_ctlSizeCompareType);

	DDX_Control(pDX, IDC_RESPONSE_COMBO, m_ctlResponse);
}

void RuleEditAlreadyExistsDlg::OnIncludeMaskButton()
{
	m_filterTypesWrapper.StartTracking(true, *this, IDC_INCLUDE_MASK_BUTTON);
}

void RuleEditAlreadyExistsDlg::OnExcludeMaskButton()
{
	m_filterTypesWrapper.StartTracking(false, *this, IDC_EXCLUDE_MASK_BUTTON);
}

BEGIN_MESSAGE_MAP(RuleEditAlreadyExistsDlg,ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_FILTER_BY_DATE_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_FILTER_BY_SIZE_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_BUTTON, OnIncludeMaskButton)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_BUTTON, OnExcludeMaskButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg message handlers

BOOL RuleEditAlreadyExistsDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	// fill the combos with data
	FillCompareCombos();
	FillResponseCombo();

	// copy data from TFileFilter to a dialog - mask
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

	// size&date
	m_bUseDateCompareType = m_rule.GetUseDateCompare();
	m_comboDateCompare.SelectComboResult(m_rule.GetDateCompareType());
	m_bUseSizeCompareType = m_rule.GetUseSizeCompare();
	m_comboSizeCompare.SelectComboResult(m_rule.GetSizeCompareType());
	
	// response
	EFeedbackResult eResult = m_rule.GetResult();
	m_comboResponse.SelectComboResult(eResult == eResult_Unknown ? eResult_Overwrite : eResult);

	m_filterTypesWrapper.Init();

	UpdateData(FALSE);

	EnableControls();

	return TRUE;
}

void RuleEditAlreadyExistsDlg::FillCompareCombos()
{
	m_ctlSizeCompareType.ResetContent();
	m_ctlDateCompareType.ResetContent();

	for(int iIndex = IDS_LT_STRING; iIndex <= IDS_DT_STRING; iIndex++)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		int iPos = m_ctlSizeCompareType.AddString(pszData);
		m_ctlSizeCompareType.SetItemData(iPos, iIndex - IDS_LT_STRING);

		iPos = m_ctlDateCompareType.AddString(pszData);
		m_ctlDateCompareType.SetItemData(iPos, iIndex - IDS_LT_STRING);
	}
}

void RuleEditAlreadyExistsDlg::FillResponseCombo()
{
	m_ctlResponse.ResetContent();

	std::vector<int> vEntries = {
		IDS_FEEDBACK_RESPONSE_OVERWRITE,
		IDS_FEEDBACK_RESPONSE_RESUME,
		IDS_FEEDBACK_RESPONSE_SKIP,
		IDS_FEEDBACK_RESPONSE_CANCEL,
		IDS_FEEDBACK_RESPONSE_PAUSE,
		IDS_FEEDBACK_RESPONSE_RENAME
	};
	for(int entry : vEntries)
	{
		const wchar_t* pszData = GetResManager().LoadString(entry);
		int iPos = m_ctlResponse.AddString(pszData);
		m_ctlResponse.SetItemData(iPos, entry - IDS_FEEDBACK_RESPONSE_UNKNOWN);
	}
}

void RuleEditAlreadyExistsDlg::OnLanguageChanged()
{
	// combo result
	EFeedbackResult eResult = m_comboResponse.GetSelectedValue();
	FillResponseCombo();
	m_comboResponse.SelectComboResult(eResult);

	// size&date
	ECompareType eSizeCmp = m_comboSizeCompare.GetSelectedValue();
	ECompareType eDateCmp = m_comboDateCompare.GetSelectedValue();

	FillCompareCombos();

	m_comboSizeCompare.SelectComboResult(eSizeCmp);
	m_comboDateCompare.SelectComboResult(eDateCmp);
}

void RuleEditAlreadyExistsDlg::EnableControls()
{
	UpdateData(TRUE);
	// mask
	m_ctlIncludeMask.EnableWindow(m_bUseIncludeMask);
	m_ctlExcludeMask.EnableWindow(m_bUseExcludeMask);
	GetDlgItem(IDC_INCLUDE_MASK_BUTTON)->EnableWindow(m_bUseIncludeMask);
	GetDlgItem(IDC_EXCLUDE_MASK_BUTTON)->EnableWindow(m_bUseExcludeMask);

	// size
	m_ctlSizeCompareType.EnableWindow(m_bUseSizeCompareType);

	// date
	m_ctlDateCompareType.EnableWindow(m_bUseDateCompareType);
}

void RuleEditAlreadyExistsDlg::OnOK() 
{
	UpdateData(TRUE);
	
	CString strText;
	m_ctlIncludeMask.GetWindowText(strText);
	m_rule.SetUseMask(((m_bUseIncludeMask != 0) && !strText.IsEmpty()));
	m_rule.SetCombinedMask((PCTSTR)strText);

	m_ctlExcludeMask.GetWindowText(strText);
	m_rule.SetUseExcludeMask((m_bUseExcludeMask != 0) && !strText.IsEmpty());
	m_rule.SetCombinedExcludeMask((PCTSTR)strText);

	// size
	m_rule.SetUseSizeCompare(m_bUseSizeCompareType != 0);
	m_rule.SetSizeCompareType(m_comboSizeCompare.GetSelectedValue());

	// date
	m_rule.SetUseDateCompare(m_bUseDateCompareType != 0);
	m_rule.SetDateCompareType(m_comboDateCompare.GetSelectedValue());

	// response
	m_rule.SetResult(m_comboResponse.GetSelectedValue());

	CLanguageDialog::OnOK();
}

BOOL RuleEditAlreadyExistsDlg::OnCommand(WPARAM wParam, LPARAM lParam)
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
