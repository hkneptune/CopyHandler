// FeedbackOpenFileErrorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "FeedbackFileErrorDlg.h"
#include "FeedbackHandler.h"

// CFeedbackFileErrorDlg dialog

IMPLEMENT_DYNAMIC(CFeedbackFileErrorDlg, ictranslate::CLanguageDialog)

CFeedbackFileErrorDlg::CFeedbackFileErrorDlg(const tchar_t* pszPath, ulong_t ulSysError, CWnd* pParent /*=NULL*/)
	: ictranslate::CLanguageDialog(CFeedbackFileErrorDlg::IDD, pParent),
	m_bAllItems(FALSE),
	m_strPath(pszPath),
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

	ictranslate::CResourceManager* pResManager = GetResManager();
	BOOST_ASSERT(pResManager);
	if(pResManager)
	{
		CString strFmt;
		strFmt = pResManager->LoadString(IDS_INFO_FILE_STRING);
		strFmt += _T("\r\n");
		strFmt += pResManager->LoadString(IDS_INFO_REASON_STRING);

		// get system error string
		TCHAR szSystem[1024];
		DWORD dwPos=FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, m_ulSysError, 0, szSystem, 1023, NULL);
		szSystem[1023] = _T('\0');

		// get rid of \r\n at the end of szSystem
		while(--dwPos && (szSystem[dwPos] == 0x0a || szSystem[dwPos] == 0x0d))
			szSystem[dwPos]=_T('\0');

		ictranslate::CFormat fmt(strFmt);
		fmt.SetParam(_t("%filename"), m_strPath);
		fmt.SetParam(_t("%reason"), szSystem);

		m_ctlErrorInfo.SetWindowText(fmt);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFeedbackFileErrorDlg::OnBnClickedRetryButton()
{
	EndDialog(CFeedbackHandler::eResult_Retry);
}

void CFeedbackFileErrorDlg::OnBnClickedSkipButton()
{
	EndDialog(CFeedbackHandler::eResult_Skip);
}

void CFeedbackFileErrorDlg::OnBnClickedPauseButton()
{
	EndDialog(CFeedbackHandler::eResult_Pause);
}

void CFeedbackFileErrorDlg::OnBnClickedCancel()
{
	EndDialog(CFeedbackHandler::eResult_Cancel);
}