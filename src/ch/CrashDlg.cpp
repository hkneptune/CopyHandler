// CrashDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "CrashDlg.h"
#include "version.h"

#define IDS_CRASH_TITLE			_T("Application crashed")
#define IDS_STATIC_INFO			_T("Copy Handler encountered an internal problem and will be closed.\n\nIf you want to help correct this problem in the future releases of program you can send the crash information to the author of this program (e-mail it to ixen@copyhandler.com).")
#define IDS_VERSIONINFO_STATIC	_T("Program version:")
#define IDS_LOCATIONINFO_STATIC _T("Crash dump location:")
#define IDS_LOCATION_STATIC		_T("Error encountered while trying to create crash dump")
#define IDS_OK					_T("&Close")

// CCrashDlg dialog

IMPLEMENT_DYNAMIC(CCrashDlg, CHLanguageDialog)

CCrashDlg::CCrashDlg(bool bResult, PCTSTR pszFilename, CWnd* pParent /*=NULL*/)
	: CDialog(CCrashDlg::IDD, pParent),
	m_bResult(bResult),
	m_strFilename(pszFilename)
{
}

CCrashDlg::~CCrashDlg()
{
}

void CCrashDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VERSION_STATIC, m_ctlVersion);
	DDX_Control(pDX, IDC_LOCATION_EDIT, m_ctlLocation);
	DDX_Control(pDX, IDOK, m_ctlOKButton);
	DDX_Control(pDX, IDC_STATIC_INFO, m_ctlInfo);
	DDX_Control(pDX, IDC_VERSIONINFO_STATIC, m_ctlVersionInfo);
	DDX_Control(pDX, IDC_LOCATIONINFO_STATIC, m_ctlLocationInfo);
}


BEGIN_MESSAGE_MAP(CCrashDlg, CDialog)
END_MESSAGE_MAP()


// CCrashDlg message handlers

BOOL CCrashDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(IDS_CRASH_TITLE);
	m_ctlInfo.SetWindowText(IDS_STATIC_INFO);
	m_ctlVersionInfo.SetWindowText(IDS_VERSIONINFO_STATIC);
	m_ctlVersion.SetWindowText(PRODUCT_FULL_VERSION);
	m_ctlLocationInfo.SetWindowText(IDS_LOCATIONINFO_STATIC);
	if(m_bResult)
		m_ctlLocation.SetWindowText(m_strFilename);
	else
		m_ctlLocation.SetWindowText(IDS_LOCATION_STATIC);

	m_ctlOKButton.SetWindowText(IDS_OK);
	m_ctlOKButton.EnableWindow(m_bResult);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
