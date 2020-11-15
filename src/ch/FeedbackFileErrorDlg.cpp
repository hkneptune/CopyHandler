// FeedbackOpenFileErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "FeedbackFileErrorDlg.h"
#include "FeedbackHandler.h"
#include "../libchcore/TWin32ErrorFormatter.h"
#include "resource.h"
#include "RuleEditDlg.h"
#include "../libchengine/FeedbackPredefinedRules.h"

using namespace chengine;

IMPLEMENT_DYNAMIC(CFeedbackFileErrorDlg, ictranslate::CLanguageDialog)

CFeedbackFileErrorDlg::CFeedbackFileErrorDlg(chengine::FeedbackRules& currentRules, const wchar_t* pszSrcPath, const wchar_t* pszDstPath, EFileError eOperationType, unsigned long ulSysError, CWnd* pParent /*=nullptr*/)
	: ictranslate::CLanguageDialog(IDD_FEEDBACK_FILE_ERROR_DIALOG, pParent),
	m_strSrcPath(pszSrcPath),
	m_strDstPath(pszDstPath),
	m_ulSysError(ulSysError),
	m_rules(currentRules),
	m_eOperationType(eOperationType)
{
}

CFeedbackFileErrorDlg::~CFeedbackFileErrorDlg()
{
}

const chengine::FeedbackRules& CFeedbackFileErrorDlg::GetRules() const
{
	return m_rules;
}

void CFeedbackFileErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	ictranslate::CLanguageDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILENAME_EDIT, m_ctlFirstName);
	DDX_Control(pDX, IDC_SECOND_FILENAME_EDIT, m_ctlSecondName);
	DDX_Control(pDX, IDC_OPERATION_EDIT, m_ctlOperationType);
	DDX_Control(pDX, IDC_SYSTEM_ERROR_EDIT, m_ctlSystemError);

	DDX_Control(pDX, IDC_RETRY_BUTTON, m_btnRetry);
	DDX_Control(pDX, IDC_SKIP_BUTTON, m_btnSkip);
	DDX_Control(pDX, IDC_CUSTOM_RULES_BUTTON, m_btnCustomRules);
	DDX_Control(pDX, IDC_PAUSE_BUTTON, m_btnPause);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CFeedbackFileErrorDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_RETRY_BUTTON, OnBnRetry)
	ON_BN_CLICKED(IDC_SKIP_BUTTON, OnBnSkip)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, OnBnPause)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
	ON_BN_CLICKED(IDC_CUSTOM_RULES_BUTTON, OnBnCustomRules)
END_MESSAGE_MAP()

BOOL CFeedbackFileErrorDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	// set dialog icon
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, FALSE);

	AddResizableControl(IDC_HEADER_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_ERROR_DETAILS_STATIC, 0.0, 0.0, 1.0, 1.0);
	AddResizableControl(IDC_SRC_NAME_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_FILENAME_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_NAME_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_SECOND_FILENAME_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_OPERATION_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_OPERATION_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SYSTEM_ERROR_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_SYSTEM_ERROR_EDIT, 0.0, 0.0, 1.0, 1.0);

	AddResizableControl(IDC_RETRY_BUTTON, 0.34, 1.0, 0.33, 0.0);
	AddResizableControl(IDC_SKIP_BUTTON, 0.67, 1.0, 0.33, 0.0);
	AddResizableControl(IDC_CUSTOM_RULES_BUTTON, 0.0, 1.0, 0.34, 0.0);
	AddResizableControl(IDC_PAUSE_BUTTON, 0.34, 1.0, 0.33, 0.0);
	AddResizableControl(IDCANCEL, 0.67, 1.0, 0.33, 0.0);

	InitializeResizableControls();

	// set menus on buttons
	HMENU hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_ERROR_MASS_RETRY_MENU));
	m_menuMassRetry.Attach(hMenu);
	m_btnRetry.m_hMenu = m_menuMassRetry.GetSubMenu(0)->GetSafeHmenu();
	m_btnRetry.m_bDefaultClick = TRUE;

	hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_ERROR_MASS_SKIP_MENU));
	m_menuMassSkip.Attach(hMenu);
	m_btnSkip.m_hMenu = m_menuMassSkip.GetSubMenu(0)->GetSafeHmenu();
	m_btnSkip.m_bDefaultClick = TRUE;

	// set data
	ictranslate::CResourceManager& rResManager = GetResManager();
	CString strOperationType = rResManager.LoadString(IDS_OPERATION_DELETEERROR + m_eOperationType);
	m_ctlOperationType.SetWindowText(strOperationType);

	m_ctlFirstName.SetWindowText(m_strSrcPath);
	m_ctlSecondName.SetWindowText(m_strDstPath.IsEmpty() ? CString(L"-") : m_strDstPath);

	// get system error string
	string::TString strError = chcore::TWin32ErrorFormatter::FormatWin32ErrorCode(m_ulSysError, true);
	
	ictranslate::CFormat fmt(L"%errno - %msg");
	fmt.SetParam(_T("%errno"), m_ulSysError);
	fmt.SetParam(_T("%msg"), strError.c_str());

	m_ctlSystemError.SetWindowText(fmt.ToString());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFeedbackFileErrorDlg::OnBnRetry()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Retry);
}

void CFeedbackFileErrorDlg::OnBnSkip()
{
	UpdateData(TRUE);
	switch(m_btnSkip.m_nMenuResult)
	{
	case ID_FEEDBACK_SKIP_WHEN_SAME_ERROR:
		m_rules.GetErrorRules().Merge(FeedbackPredefinedRules::CreateErrorRule(EErrorPredefinedRuleCondition::eCondition_WhenSameError, m_ulSysError, eResult_Skip));
		break;
	}

	EndDialog(chengine::EFeedbackResult::eResult_Skip);
}

void CFeedbackFileErrorDlg::OnBnPause()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Pause);
}

void CFeedbackFileErrorDlg::OnBnCustomRules()
{
	RuleEditDlg dlg(m_rules);
	if(dlg.DoModal() == IDOK)
	{
		m_rules = dlg.GetRules();
	}
}

void CFeedbackFileErrorDlg::OnCancel()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Cancel);
}
