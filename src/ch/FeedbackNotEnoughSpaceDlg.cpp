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
#include "FeedbackNotEnoughSpaceDlg.h"
#include "resource.h"
#include "../libchengine/EFeedbackResult.h"
#include "RuleEditDlg.h"
#include "StringHelpers.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFeedbackNotEnoughSpaceDlg dialog

CFeedbackNotEnoughSpaceDlg::CFeedbackNotEnoughSpaceDlg(chengine::FeedbackRules& currentRules, unsigned long long ullSizeRequired, const wchar_t* pszDstPath)
	:ictranslate::CLanguageDialog(IDD_FEEDBACK_NOTENOUGHSPACE_DIALOG),
	m_strDstPath(pszDstPath),
	m_ullRequired(ullSizeRequired),
	m_fsLocal(GetLogFileData()),
	m_rules(currentRules)
{
}

const chengine::FeedbackRules& CFeedbackNotEnoughSpaceDlg::GetRules() const
{
	return m_rules;
}

void CFeedbackNotEnoughSpaceDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LOCATION_EDIT, m_ctlLocationEdit);
	DDX_Control(pDX, IDC_REQUIRED_SPACE_STATIC, m_ctlRequiredSpaceStatic);
	DDX_Control(pDX, IDC_AVAILABLE_SPACE_STATIC, m_ctlAvailableSpaceStatic);
	DDX_Control(pDX, IDC_RETRY_BUTTON, m_btnRetry);
	DDX_Control(pDX, IDC_IGNORE_BUTTON, m_btnIgnore);
	DDX_Control(pDX, IDC_CUSTOM_RULES_BUTTON, m_btnCustomRules);
	DDX_Control(pDX, IDC_PAUSE_BUTTON, m_btnPause);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CFeedbackNotEnoughSpaceDlg,ictranslate::CLanguageDialog)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_RETRY_BUTTON, OnBnRetry)
	ON_BN_CLICKED(IDC_IGNORE_BUTTON, OnBnIgnore)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CUSTOM_RULES_BUTTON, OnBnCustomRules)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, OnBnPause)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFeedbackNotEnoughSpaceDlg message handlers
void CFeedbackNotEnoughSpaceDlg::UpdateDialog()
{
	// update size information
	m_ctlRequiredSpaceStatic.SetWindowText(GetSizeString(m_ullRequired));

	unsigned long long ullFree = 0, ullTotal = 0;
	try
	{
		m_fsLocal.GetDynamicFreeSpace(chcore::PathFromString(m_strDstPath), ullFree, ullTotal);
	}
	catch(const std::exception&)
	{
		ullFree = 0;
	}
	m_ctlAvailableSpaceStatic.SetWindowText(GetSizeString(ullFree));

	// location
	m_ctlLocationEdit.SetWindowText(m_strDstPath);
}

BOOL CFeedbackNotEnoughSpaceDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	// set dialog icon
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, FALSE);

	AddResizableControl(IDC_HEADER_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DETAILS_STATIC, 0.0, 0.0, 1.0, 1.0);
	AddResizableControl(IDC_ICON_STATIC, 0.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_LOCATION_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_LOCATION_EDIT, 0.0, 0.0, 0.0, 1.0);

	AddResizableControl(IDC_REQUIRED_SPACE_HDR_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_REQUIRED_SPACE_STATIC, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_AVAILABLE_SPACE_HDR_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_AVAILABLE_SPACE_STATIC, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_RETRY_BUTTON, 0.34, 1.0, 0.33, 0.0);
	AddResizableControl(IDC_IGNORE_BUTTON, 0.67, 1.0, 0.33, 0.0);
	AddResizableControl(IDC_CUSTOM_RULES_BUTTON, 0.0, 1.0, 0.34, 0.0);
	AddResizableControl(IDC_PAUSE_BUTTON, 0.34, 1.0, 0.33, 0.0);
	AddResizableControl(IDCANCEL, 0.67, 1.0, 0.33, 0.0);

	InitializeResizableControls();

	// set to top
	//SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE /*| SWP_SHOWWINDOW*/);

	// format needed text
	UpdateDialog();

	SetTimer(1601, 1000, nullptr);

	return TRUE;
}

void CFeedbackNotEnoughSpaceDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == 1601)
	{
		// update free space
		
		unsigned long long ullFree = 0;
		try
		{
			unsigned long long ullTotal = 0;
			m_fsLocal.GetDynamicFreeSpace(chcore::PathFromString(m_strDstPath), ullFree, ullTotal);
		}
		catch(const std::exception&)
		{
			ullFree = 0;
		}

		m_ctlAvailableSpaceStatic.SetWindowText(GetSizeString(ullFree));

		// end dialog if this is enough
		if (m_ullRequired <= ullFree)
		{
			KillTimer(1601);
			EndDialog(chengine::EFeedbackResult::eResult_Retry);
		}
	}
	
	CLanguageDialog::OnTimer(nIDEvent);
}

void CFeedbackNotEnoughSpaceDlg::OnLanguageChanged()
{
	UpdateDialog();
}

void CFeedbackNotEnoughSpaceDlg::OnBnRetry() 
{
	EndDialog(chengine::EFeedbackResult::eResult_Retry);
}

void CFeedbackNotEnoughSpaceDlg::OnBnIgnore() 
{
	EndDialog(chengine::EFeedbackResult::eResult_Ignore);
}

void CFeedbackNotEnoughSpaceDlg::OnCancel()
{
	EndDialog(chengine::EFeedbackResult::eResult_Cancel);
}

void CFeedbackNotEnoughSpaceDlg::OnBnCustomRules()
{
	RuleEditDlg dlg(m_rules);
	if(dlg.DoModal() == IDOK)
	{
		m_rules = dlg.GetRules();
	}
}

void CFeedbackNotEnoughSpaceDlg::OnBnPause()
{
	EndDialog(chengine::EFeedbackResult::eResult_Pause);
}
