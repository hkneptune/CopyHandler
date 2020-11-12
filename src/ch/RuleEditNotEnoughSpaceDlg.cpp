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
#include "RuleEditNotEnoughSpaceDlg.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg dialog

RuleEditNotEnoughSpaceDlg::RuleEditNotEnoughSpaceDlg(const chengine::FeedbackNotEnoughSpaceRule& rRule) :
	CLanguageDialog(IDD_RULE_EDIT_NOTENOUGHSPACE_DIALOG),
	m_rule(rRule)
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
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FeedbackRuleEditorDlg message handlers

BOOL RuleEditNotEnoughSpaceDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

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

void RuleEditNotEnoughSpaceDlg::OnLanguageChanged()
{
	// selection
	int iResponseIndex = m_ctlResponse.GetCurSel();

	m_ctlResponse.ResetContent();

	for(int iIndex = IDS_FEEDBACK_RESPONSE_UNKNOWN; iIndex <= IDS_FEEDBACK_RESPONSE_RENAME; ++iIndex)
	{
		const wchar_t* pszData = GetResManager().LoadString(iIndex);
		m_ctlResponse.AddString(pszData);
	}

	m_ctlResponse.SetCurSel(iResponseIndex);
}

void RuleEditNotEnoughSpaceDlg::EnableControls()
{
	UpdateData(TRUE);
	// mask
	m_ctlIncludeMask.EnableWindow(m_bUseIncludeMask);
	m_ctlExcludeMask.EnableWindow(m_bUseExcludeMask);
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
	m_rule.SetResult((chengine::EFeedbackResult)m_ctlResponse.GetCurSel());

	CLanguageDialog::OnOK();
}
