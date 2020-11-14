/***************************************************************************
*   Copyright (C) 2001-2020 by Józef Starosczyk                           *
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
#include "stdafx.h"
#include "ch.h"
#include "RuleEditErrorDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg dialog

RuleEditErrorDlg::RuleEditErrorDlg(const chengine::FeedbackErrorRule& rRule) :
	CLanguageDialog(IDD_RULE_EDIT_ERROR_DIALOG),
	m_rule(rRule)
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

BEGIN_MESSAGE_MAP(RuleEditErrorDlg,ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_FILTER_BY_OPERATION_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_FILTER_BY_SYSTEMERROR_CHECK, EnableControls)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg message handlers

BOOL RuleEditErrorDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	// fill the combos with data

	// strings <, <=, ...
	for(int iIndex = IDS_OPERATION_DELETEERROR; iIndex <= IDS_OPERATION_RETRIEVEFILEINFO; iIndex++)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlOperationType.AddString(pszData);
	}

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
	m_ctlOperationType.SetCurSel(m_rule.GetErrorType());

	// response
	for(int iIndex = IDS_FEEDBACK_RESPONSE_UNKNOWN; iIndex <= IDS_FEEDBACK_RESPONSE_RENAME; ++iIndex)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlResponse.AddString(pszData);
	}
	m_ctlResponse.SetCurSel(IDS_FEEDBACK_RESPONSE_SKIP - IDS_FEEDBACK_RESPONSE_UNKNOWN);

	UpdateData(FALSE);

	EnableControls();

	return TRUE;
}

void RuleEditErrorDlg::OnLanguageChanged()
{
	// selection
	int iDateCompareTypeIndex = m_ctlOperationType.GetCurSel();
	int iResponseIndex = m_ctlResponse.GetCurSel();

	m_ctlOperationType.ResetContent();
	m_ctlResponse.ResetContent();

	// strings <, <=, ...
	for(int iIndex = IDS_OPERATION_DELETEERROR; iIndex <= IDS_OPERATION_RETRIEVEFILEINFO; iIndex++)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlOperationType.AddString(pszData);
	}

	for(int iIndex = IDS_FEEDBACK_RESPONSE_UNKNOWN; iIndex <= IDS_FEEDBACK_RESPONSE_RENAME; ++iIndex)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlResponse.AddString(pszData);
	}

	m_ctlOperationType.SetCurSel(iDateCompareTypeIndex);
	m_ctlResponse.SetCurSel(iResponseIndex);
}

void RuleEditErrorDlg::EnableControls()
{
	UpdateData(TRUE);
	// mask
	m_ctlIncludeMask.EnableWindow(m_bUseIncludeMask);
	m_ctlExcludeMask.EnableWindow(m_bUseExcludeMask);

	// size
	m_ctlSystemError.EnableWindow(m_bUseSystemError);

	// date
	m_ctlOperationType.EnableWindow(m_bUseOperationType);
}

void RuleEditErrorDlg::OnOK()
{
	UpdateData(TRUE);
	
	// TFileFilter --> dialogu - mask
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
	m_rule.SetErrorType((chengine::EFileError)m_ctlOperationType.GetCurSel());

	// response
	m_rule.SetResult((chengine::EFeedbackResult)m_ctlResponse.GetCurSel());

	CLanguageDialog::OnOK();
}
