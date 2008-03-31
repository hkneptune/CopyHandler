// UpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "UpdaterDlg.h"
#include "UpdateChecker.h"
#include "../common/version.h"
#include "StaticEx.h"

// CUpdaterDlg dialog

IMPLEMENT_DYNAMIC(CUpdaterDlg, ictranslate::CLanguageDialog)

CUpdaterDlg::CUpdaterDlg(CUpdateChecker::ECheckResult eResult, PCTSTR pszVersion, PCTSTR pszError, CWnd* pParent /*=NULL*/)
	: ictranslate::CLanguageDialog(CUpdaterDlg::IDD, pParent),
	m_eResult(eResult),
	m_strVersion(pszVersion),
	m_strError(pszError)
{
	RegisterStaticExControl(AfxGetInstanceHandle());
}

CUpdaterDlg::~CUpdaterDlg()
{
}

void CUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	ictranslate::CLanguageDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_INFO_STATIC, m_ctlText);
}

BEGIN_MESSAGE_MAP(CUpdaterDlg, ictranslate::CLanguageDialog)
END_MESSAGE_MAP()


// CUpdaterDlg message handlers
BOOL CUpdaterDlg::OnInitDialog()
{
	ictranslate::CLanguageDialog::OnInitDialog();
	ictranslate::CFormat fmt;
	ictranslate::CResourceManager* pResManager = GetResManager();
	_ASSERTE(pResManager);
	if(!pResManager)
		return FALSE;

	CString strFmt;
	switch(m_eResult)
	{
	case CUpdateChecker::eResult_Error:
		strFmt = pResManager->LoadString(IDS_UPDATER_ERROR_STRING);
		fmt.SetFormat(strFmt);
		fmt.SetParam(_t("%errdesc"), m_strError);

		m_ctlText.SetWindowText(fmt);
		break;
	case CUpdateChecker::eResult_VersionNewer:
		strFmt = pResManager->LoadString(IDS_UPDATER_NEW_VERSION_STRING);
		fmt.SetFormat(strFmt);
		fmt.SetParam(_t("%thisver"), _T(PRODUCT_VERSION));
		fmt.SetParam(_t("%officialver"), m_strVersion);

		m_ctlText.SetWindowText(fmt);
		break;
	case CUpdateChecker::eResult_VersionCurrent:
		strFmt = pResManager->LoadString(IDS_UPDATER_EQUAL_VERSION_STRING);
		fmt.SetFormat(strFmt);
		fmt.SetParam(_t("%thisver"), _T(PRODUCT_VERSION));
		fmt.SetParam(_t("%officialver"), m_strVersion);

		m_ctlText.SetWindowText(fmt);
		break;
	case CUpdateChecker::eResult_VersionOlder:
		strFmt = pResManager->LoadString(IDS_UPDATER_OLD_VERSION_STRING);
		fmt.SetFormat(strFmt);
		fmt.SetParam(_t("%thisver"), _T(PRODUCT_VERSION));
		fmt.SetParam(_t("%officialver"), m_strVersion);

		m_ctlText.SetWindowText(fmt);
		break;
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
