// UpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "UpdaterDlg.h"
#include "UpdateChecker.h"
#include "../common/version.h"

#define UPDATER_TIMER 639

BEGIN_MESSAGE_MAP(CUpdaterDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_OPEN_WEBPAGE_BUTTON, &CUpdaterDlg::OnBnClickedOpenWebpageButton)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CUpdaterDlg dialog

IMPLEMENT_DYNAMIC(CUpdaterDlg, ictranslate::CLanguageDialog)

CUpdaterDlg::CUpdaterDlg(CWnd* pParent /*=NULL*/)
: ictranslate::CLanguageDialog(CUpdaterDlg::IDD, pParent)
{
}

CUpdaterDlg::~CUpdaterDlg()
{
}

void CUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	ictranslate::CLanguageDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFO_STATIC, m_ctlText);
}

BOOL CUpdaterDlg::OnInitDialog()
{
	ictranslate::CLanguageDialog::OnInitDialog();

	ictranslate::CFormat fmt(GetResManager().LoadString(IDS_UPDATER_WAITING_STRING));
	fmt.SetParam(_t("%site"), _T(PRODUCT_SITE));
	m_ctlText.SetWindowText(fmt);

	SetTimer(UPDATER_TIMER, 10, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdaterDlg::StartChecking()
{
	m_ucChecker.CheckForUpdates(_T(PRODUCT_SITE), false);

	ictranslate::CResourceManager& rResManager = GetResManager();
	ictranslate::CFormat fmt;

	CString strFmt;
	switch(m_ucChecker.GetResult())
	{
	case CUpdateChecker::eResult_Error:
		strFmt = rResManager.LoadString(IDS_UPDATER_ERROR_STRING);
		break;
	case CUpdateChecker::eResult_VersionNewer:
		strFmt = rResManager.LoadString(IDS_UPDATER_NEW_VERSION_STRING);
		break;
	case CUpdateChecker::eResult_VersionCurrent:
		strFmt = rResManager.LoadString(IDS_UPDATER_EQUAL_VERSION_STRING);
		break;
	case CUpdateChecker::eResult_VersionOlder:
		strFmt = rResManager.LoadString(IDS_UPDATER_OLD_VERSION_STRING);
		break;
	}

	fmt.SetFormat(strFmt);
	fmt.SetParam(_t("%errdesc"), m_ucChecker.GetLastError());
	fmt.SetParam(_t("%thisver"), _T(PRODUCT_VERSION));
	fmt.SetParam(_t("%officialver"), m_ucChecker.GetReadableVersion());

	m_ctlText.SetWindowText(fmt);
}

void CUpdaterDlg::OnBnClickedOpenWebpageButton()
{
	ShellExecute(NULL, _T("open"), m_ucChecker.GetDownloadAddress(), NULL, NULL, SW_SHOW);
}

void CUpdaterDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == UPDATER_TIMER)
	{
		KillTimer(UPDATER_TIMER);
		StartChecking();
	}

	CLanguageDialog::OnTimer(nIDEvent);
}
