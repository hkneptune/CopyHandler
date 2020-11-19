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
#include "RuleEditErrorDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace chengine;

RuleEditErrorDlg::RuleEditErrorDlg(const chengine::FeedbackErrorRule& rRule) :
	CLanguageDialog(IDD_RULE_EDIT_ERROR_DIALOG),
	m_rule(rRule),
	m_comboResponse(m_ctlResponse, eResult_Skip, eResult_Last),
	m_comboOperationType(m_ctlOperationType, EFileError::eCheckForFreeSpace, EFileError::eOperation_Last)
{
}

void RuleEditErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_INCLUDE_MASK_CHECK, m_bUseIncludeMask);
	DDX_Control(pDX, IDC_INCLUDE_MASK_COMBO, m_ctlIncludeMask);

	DDX_Check(pDX, IDC_EXCLUDE_MASK_CHECK, m_bUseExcludeMask);
	DDX_Control(pDX, IDC_EXCLUDE_MASK_COMBO, m_ctlExcludeMask);

	DDX_Check(pDX, IDC_FILTER_BY_OPERATION_CHECK, m_bUseOperationType);
	DDX_Control(pDX, IDC_FILTER_BY_OPERATION_COMBO, m_ctlOperationType);

	DDX_Check(pDX, IDC_FILTER_BY_SYSTEMERROR_CHECK, m_bUseSystemError);
	DDX_Control(pDX, IDC_FILTER_BY_SYSTEMERROR_EDIT, m_ctlSystemError);

	DDX_Control(pDX, IDC_RESPONSE_COMBO, m_ctlResponse);
}

void RuleEditErrorDlg::OnIncludeMaskButton()
{
	m_filterTypesWrapper.StartTracking(true, *this, IDC_INCLUDE_MASK_BUTTON);
}

void RuleEditErrorDlg::OnExcludeMaskButton()
{
	m_filterTypesWrapper.StartTracking(false, *this, IDC_EXCLUDE_MASK_BUTTON);
}

BEGIN_MESSAGE_MAP(RuleEditErrorDlg,ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_FILTER_BY_OPERATION_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_FILTER_BY_SYSTEMERROR_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_BUTTON, OnIncludeMaskButton)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_BUTTON, OnExcludeMaskButton)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg message handlers

BOOL RuleEditErrorDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	// fill the combos with data

	// strings <, <=, ...
	FillOperationCombo();
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

	// size
	m_bUseSystemError = m_rule.GetUseSystemErrorNo();
	CString strValue;
	strValue.Format(L"%u", m_rule.GetSystemErrorNo());
	m_ctlSystemError.SetWindowText(strValue);
	
	// date
	m_bUseOperationType = m_rule.GetUseErrorType();
	m_comboOperationType.SelectComboResult(m_rule.GetErrorType());

	// result
	EFeedbackResult eResult = m_rule.GetResult();
	m_comboResponse.SelectComboResult(eResult == eResult_Unknown ? eResult_Skip : eResult);

	m_filterTypesWrapper.Init();

	UpdateData(FALSE);

	EnableControls();

	return TRUE;
}

void RuleEditErrorDlg::FillOperationCombo()
{
	m_ctlOperationType.ResetContent();

	for(int iIndex = IDS_OPERATION_DELETEERROR; iIndex <= IDS_OPERATION_RETRIEVEFILEINFO; iIndex++)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		int iPos = m_ctlOperationType.AddString(pszData);
		m_ctlOperationType.SetItemData(iPos, iIndex - IDS_OPERATION_DELETEERROR);
	}
}

void RuleEditErrorDlg::FillResponseCombo()
{
	m_ctlResponse.ResetContent();

	std::vector<int> vEntries = {
		IDS_FEEDBACK_RESPONSE_SKIP,
		IDS_FEEDBACK_RESPONSE_CANCEL,
		IDS_FEEDBACK_RESPONSE_PAUSE,
		IDS_FEEDBACK_RESPONSE_RETRY
	};
	for(int entry : vEntries)
	{
		const wchar_t* pszData = GetResManager().LoadString(entry);
		int iPos = m_ctlResponse.AddString(pszData);
		m_ctlResponse.SetItemData(iPos, entry - IDS_FEEDBACK_RESPONSE_UNKNOWN);
	}
}

void RuleEditErrorDlg::OnLanguageChanged()
{
	EFeedbackResult eResult = m_comboResponse.GetSelectedValue();
	FillResponseCombo();
	m_comboResponse.SelectComboResult(eResult);

	EFileError eOperation = m_comboOperationType.GetSelectedValue();

	FillOperationCombo();

	m_comboOperationType.SelectComboResult(eOperation);
}

void RuleEditErrorDlg::EnableControls()
{
	UpdateData(TRUE);
	// mask
	m_ctlIncludeMask.EnableWindow(m_bUseIncludeMask);
	m_ctlExcludeMask.EnableWindow(m_bUseExcludeMask);
	GetDlgItem(IDC_INCLUDE_MASK_BUTTON)->EnableWindow(m_bUseIncludeMask);
	GetDlgItem(IDC_EXCLUDE_MASK_BUTTON)->EnableWindow(m_bUseExcludeMask);

	// size
	m_ctlSystemError.EnableWindow(m_bUseSystemError);

	// date
	m_ctlOperationType.EnableWindow(m_bUseOperationType);
}

void RuleEditErrorDlg::OnOK()
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
	m_rule.SetUseSystemErrorNo(m_bUseSystemError != 0);

	CString strErrorNo;
	m_ctlSystemError.GetWindowText(strErrorNo);
	unsigned int uiErrorNo = boost::lexical_cast<unsigned int>((PCTSTR)strErrorNo);
	m_rule.SetSystemErrorNo(uiErrorNo);

	// date
	m_rule.SetUseErrorType(m_bUseOperationType != 0);
	m_rule.SetErrorType(m_comboOperationType.GetSelectedValue());

	// response
	m_rule.SetResult(m_comboResponse.GetSelectedValue());

	CLanguageDialog::OnOK();
}

BOOL RuleEditErrorDlg::OnCommand(WPARAM wParam, LPARAM lParam)
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
