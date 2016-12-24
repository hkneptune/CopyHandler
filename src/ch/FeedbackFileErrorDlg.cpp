// FeedbackOpenFileErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "FeedbackFileErrorDlg.h"
#include "FeedbackHandler.h"
#include "../libchcore/TWin32ErrorFormatter.h"
#include "resource.h"

// CFeedbackFileErrorDlg dialog

IMPLEMENT_DYNAMIC(CFeedbackFileErrorDlg, ictranslate::CLanguageDialog)

CFeedbackFileErrorDlg::CFeedbackFileErrorDlg(const wchar_t* pszSrcPath, const wchar_t* pszDstPath, unsigned long ulSysError, CWnd* pParent /*=nullptr*/)
	: ictranslate::CLanguageDialog(IDD_FEEDBACK_FILE_ERROR_DIALOG, pParent),
	m_bAllItems(FALSE),
	m_strSrcPath(pszSrcPath),
	m_strDstPath(pszDstPath),
	m_ulSysError(ulSysError)
{
}

CFeedbackFileErrorDlg::~CFeedbackFileErrorDlg()
{
}

void CFeedbackFileErrorDlg::DoDataExchange(CDataExchange* pDX)
{
	ictranslate::CLanguageDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ALL_ITEMS_CHECK, m_bAllItems);
	DDX_Control(pDX, IDC_INFO_STATIC, m_ctlErrorInfo);
}

BEGIN_MESSAGE_MAP(CFeedbackFileErrorDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_RETRY_BUTTON, &CFeedbackFileErrorDlg::OnBnClickedRetryButton)
	ON_BN_CLICKED(IDC_SKIP_BUTTON, &CFeedbackFileErrorDlg::OnBnClickedSkipButton)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, &CFeedbackFileErrorDlg::OnBnClickedPauseButton)
	ON_BN_CLICKED(IDCANCEL, &CFeedbackFileErrorDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CFeedbackFileErrorDlg message handlers
BOOL CFeedbackFileErrorDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	// set dialog icon
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, FALSE);

	AddResizableControl(IDC_001_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_DESC_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_INFO_STATIC, 0.0, 0.0, 1.0, 1.0);
	AddResizableControl(IDC_RETRY_BUTTON, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_SKIP_BUTTON, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_PAUSE_BUTTON, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDCANCEL, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_ALL_ITEMS_CHECK, 0.0, 1.0, 1.0, 0.0);

	InitializeResizableControls();

	ictranslate::CResourceManager& rResManager = GetResManager();
	CString strFmt;
	strFmt = rResManager.LoadString(m_strDstPath.IsEmpty() ? IDS_INFO_FILE_STRING : IDS_INFO_TWO_FILE_STRING);
	strFmt += _T("\r\n");
	strFmt += rResManager.LoadString(IDS_INFO_REASON_STRING);

	// get system error string
	string::TString strError = chcore::TWin32ErrorFormatter::FormatWin32ErrorCode(m_ulSysError, true);

	ictranslate::CFormat fmt(strFmt);
	fmt.SetParam(_T("%filename"), m_strSrcPath);
	fmt.SetParam(_T("%dstfilename"), m_strDstPath);
	fmt.SetParam(_T("%reason"), strError.c_str());

	m_ctlErrorInfo.SetWindowText(fmt.ToString());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFeedbackFileErrorDlg::OnBnClickedRetryButton()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Retry);
}

void CFeedbackFileErrorDlg::OnBnClickedSkipButton()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Skip);
}

void CFeedbackFileErrorDlg::OnBnClickedPauseButton()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Pause);
}

void CFeedbackFileErrorDlg::OnBnClickedCancel()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Cancel);
}

void CFeedbackFileErrorDlg::OnCancel()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Cancel);
}
