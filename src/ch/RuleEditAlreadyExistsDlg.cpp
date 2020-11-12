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
#include "RuleEditAlreadyExistsDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg dialog

RuleEditAlreadyExistsDlg::RuleEditAlreadyExistsDlg(const chengine::FeedbackAlreadyExistsRule& rRule) :
	CLanguageDialog(IDD_RULE_EDIT_ALREADYEXISTS_DIALOG),
	m_rule(rRule)
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

BEGIN_MESSAGE_MAP(RuleEditAlreadyExistsDlg,ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_INCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_EXCLUDE_MASK_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_FILTER_BY_DATE_CHECK, EnableControls)
	ON_BN_CLICKED(IDC_FILTER_BY_SIZE_CHECK, EnableControls)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg message handlers

BOOL RuleEditAlreadyExistsDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	// fill the combos with data

	// strings <, <=, ...
	for(int iIndex = IDS_LT_STRING; iIndex <= IDS_DT_STRING; iIndex++)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlSizeCompareType.AddString(pszData);
		m_ctlDateCompareType.AddString(pszData);
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
	m_bUseSizeCompareType = m_rule.GetUseSizeCompare();
	m_ctlSizeCompareType.SetCurSel(m_rule.GetSizeCompareType());
	
	// date
	m_bUseDateCompareType = m_rule.GetUseDateCompare();
	m_ctlDateCompareType.SetCurSel(m_rule.GetDateCompareType());

	// response
	for(int iIndex = IDS_FEEDBACK_RESPONSE_UNKNOWN; iIndex <= IDS_FEEDBACK_RESPONSE_RENAME; ++iIndex)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlResponse.AddString(pszData);
	}
	
	UpdateData(FALSE);

	EnableControls();

	return TRUE;
}

void RuleEditAlreadyExistsDlg::OnLanguageChanged()
{
	// selection
	int iSizeCompareTypeIndex = m_ctlSizeCompareType.GetCurSel();
	int iDateCompareTypeIndex = m_ctlDateCompareType.GetCurSel();
	int iResponseIndex = m_ctlResponse.GetCurSel();

	m_ctlSizeCompareType.ResetContent();
	m_ctlDateCompareType.ResetContent();
	m_ctlResponse.ResetContent();

	// strings <, <=, ...
	for(int iIndex = IDS_LT_STRING; iIndex <= IDS_DT_STRING; iIndex++)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlSizeCompareType.AddString(pszData);
		m_ctlDateCompareType.AddString(pszData);
	}

	for(int iIndex = IDS_FEEDBACK_RESPONSE_UNKNOWN; iIndex <= IDS_FEEDBACK_RESPONSE_RENAME; ++iIndex)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlResponse.AddString(pszData);
	}

	m_ctlSizeCompareType.SetCurSel(iSizeCompareTypeIndex);
	m_ctlDateCompareType.SetCurSel(iDateCompareTypeIndex);
	m_ctlResponse.SetCurSel(iResponseIndex);
}

void RuleEditAlreadyExistsDlg::EnableControls()
{
	UpdateData(TRUE);
	// mask
	m_ctlIncludeMask.EnableWindow(m_bUseIncludeMask);
	m_ctlExcludeMask.EnableWindow(m_bUseExcludeMask);

	// size
	m_ctlSizeCompareType.EnableWindow(m_bUseSizeCompareType);

	// date
	m_ctlDateCompareType.EnableWindow(m_bUseDateCompareType);
}

void RuleEditAlreadyExistsDlg::OnOK() 
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
	m_rule.SetUseSizeCompare(m_bUseSizeCompareType != 0);
	m_rule.SetSizeCompareType((chengine::ECompareType)m_ctlSizeCompareType.GetCurSel());

	// date
	m_rule.SetUseDateCompare(m_bUseDateCompareType != 0);
	m_rule.SetDateCompareType((chengine::ECompareType)m_ctlDateCompareType.GetCurSel());

	// response
	m_rule.SetResult((chengine::EFeedbackResult)m_ctlResponse.GetCurSel());

	CLanguageDialog::OnOK();
}
