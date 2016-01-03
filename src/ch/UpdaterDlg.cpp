// UpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "UpdaterDlg.h"
#include "UpdateChecker.h"
#include "../common/version.h"
#include "StaticEx.h"

#define UPDATER_TIMER 639

BEGIN_MESSAGE_MAP(CUpdaterDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_OPEN_WEBPAGE_BUTTON, &CUpdaterDlg::OnBnClickedOpenWebpageButton)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CUpdaterDlg dialog

IMPLEMENT_DYNAMIC(CUpdaterDlg, ictranslate::CLanguageDialog)

CUpdaterDlg::CUpdaterDlg(bool bBackgroundMode, CWnd* pParent /*=NULL*/) :
	ictranslate::CLanguageDialog(CUpdaterDlg::IDD, pParent),
	m_eLastState(CUpdateChecker::eResult_Undefined),
	m_bBackgroundMode(bBackgroundMode)
{
	RegisterStaticExControl(AfxGetInstanceHandle());
}

CUpdaterDlg::~CUpdaterDlg()
{
	m_ucChecker.Cleanup();
}

void CUpdaterDlg::DoDataExchange(CDataExchange* pDX)
{
	ictranslate::CLanguageDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ICON_STATIC, m_ctlImage);
	DDX_Control(pDX, IDC_MAINUPDATEINFO_CUSTOM, m_ctlMainText);
	DDX_Control(pDX, IDC_CHANGELOG_RICHEDIT, m_ctlRichEdit);
}

BOOL CUpdaterDlg::OnInitDialog()
{
	ictranslate::CLanguageDialog::OnInitDialog();

	InitRichEdit();

	// disable button initially
	CWnd* pWnd = GetDlgItem(IDC_OPEN_WEBPAGE_BUTTON);
	if(pWnd)
		pWnd->EnableWindow(FALSE);

	if(!m_bBackgroundMode)
		ShowWindow(SW_SHOW);

	// start the updater
	m_ucChecker.AsyncCheckForUpdates(_T(PRODUCT_SITE), GetPropValue<PP_PLANGUAGE>(GetConfig()), (UpdateVersionInfo::EVersionType)GetPropValue<PP_PUPDATECHANNEL>(GetConfig()), m_bBackgroundMode);

	// start a timer to display progress
	SetTimer(UPDATER_TIMER, 10, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CUpdaterDlg::OnBnClickedOpenWebpageButton()
{
	CString strDownloadAddr = m_ucChecker.GetDownloadAddress();
	if(!strDownloadAddr.IsEmpty())
	{
		CString str;
		str.Format(_T("Opening a browser with address %s..."), (PCTSTR)strDownloadAddr);
		LOG_DEBUG(str);

		str.Format(_T("url.dll,FileProtocolHandler %s"), (PCTSTR)strDownloadAddr);
		ULONG_PTR ulRes = (ULONG_PTR)ShellExecute(nullptr, _T("open"), _T("rundll32.exe"), str, nullptr, SW_SHOW);

		str.Format(_T("ShellExecute returned %I64u"), (unsigned long long)ulRes);
		LOG_DEBUG(str);

		// close the dialog if succeeded; 32 is some arbitrary value from ms docs
		if(ulRes > 32)
			CUpdaterDlg::OnOK();
	}
}

void CUpdaterDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == UPDATER_TIMER)
	{
		ictranslate::CResourceManager& rResManager = GetResManager();
		ictranslate::CFormat fmt;
		CUpdateChecker::ECheckResult eResult = m_ucChecker.GetResult();
		CString strFmt;
		EBkModeResult eBkMode = eRes_None;
		bool bEnableButton = false;

		if(eResult != m_eLastState)
		{
			switch(m_ucChecker.GetResult())
			{
			case CUpdateChecker::eResult_Undefined:
				TRACE(_T("CUpdateChecker::eResult_Undefined\n"));
				eBkMode = eRes_Exit;
				UpdateIcon(eIcon_Info);
				UpdateMainText(rResManager.LoadString(IDS_UPDATER_CHECKINGFORUPDATES));
				strFmt = rResManager.LoadString(IDS_UPDATER_WAITING_STRING);
				break;

			case CUpdateChecker::eResult_Pending:
				TRACE(_T("CUpdateChecker::eResult_Pending\n"));
				UpdateIcon(eIcon_Info);
				UpdateMainText(rResManager.LoadString(IDS_UPDATER_CHECKINGFORUPDATES));
				strFmt = rResManager.LoadString(IDS_UPDATER_WAITING_STRING);
				break;

			case CUpdateChecker::eResult_Killed:
				TRACE(_T("CUpdateChecker::eResult_Killed\n"));
				eBkMode = eRes_Exit;
				UpdateIcon(eIcon_Error);
				UpdateMainText(rResManager.LoadString(IDS_UPDATER_ERROR_STRING));
				strFmt = rResManager.LoadString(IDS_UPDATER_KILLEDERROR);
				break;

			case CUpdateChecker::eResult_Error:
				TRACE(_T("CUpdateChecker::eResult_Error\n"));
				eBkMode = eRes_Exit;
				UpdateIcon(eIcon_Error);
				UpdateMainText(rResManager.LoadString(IDS_UPDATER_ERROR_STRING));
				strFmt = m_ucChecker.GetLastError();
				break;

			case CUpdateChecker::eResult_VersionCurrent:
				TRACE(_T("CUpdateChecker::eResult_VersionCurrent\n"));
				eBkMode = eRes_Exit;
				bEnableButton = false;
				UpdateIcon(eIcon_Info);
				UpdateMainText(rResManager.LoadString(IDS_UPDATER_ALREADYNEWESTVERSION));
				strFmt = rResManager.LoadString(IDS_UPDATER_EQUAL_VERSION_STRING);
				break;

			case CUpdateChecker::eResult_RemoteVersionNewer:
				TRACE(_T("CUpdateChecker::eResult_RemoteVersionNewer\n"));
				eBkMode = eRes_Show;
				bEnableButton = true;
				UpdateIcon(eIcon_Warning);
				UpdateMainText(rResManager.LoadString(IDS_UPDATER_NEWVERSIONEXISTS));
				strFmt = rResManager.LoadString(IDS_UPDATER_NEW_VERSION_STRING);
				break;

			default:
				_ASSERTE(FALSE);
				eBkMode = eRes_Exit;
				return;
			}

			fmt.SetFormat(strFmt);
			fmt.SetParam(_t("%site"), _t(PRODUCT_SITE));
			fmt.SetParam(_t("%thisver"), _T(PRODUCT_VERSION));
			fmt.SetParam(L"%numericver", PRODUCT_NUMERIC_VERSION);
			fmt.SetParam(_t("%officialver"), m_ucChecker.GetReadableVersion());
			fmt.SetParam(L"%reldate", m_ucChecker.GetReleaseDate());

			CString strEntireText = fmt;
			CString strReleaseNotes = m_ucChecker.GetReleaseNotes();
			strReleaseNotes = strReleaseNotes.Trim();
			if(!strReleaseNotes.IsEmpty())
			{
				fmt.SetFormat(L"\n\n%relnoteshdr\n%relnotestxt");
				fmt.SetParam(L"%relnoteshdr", rResManager.LoadString(IDS_UPDATER_RELEASENOTES));
				fmt.SetParam(L"%relnotestxt", m_ucChecker.GetReleaseNotes());
				strEntireText += fmt;
			}

			UpdateSecondaryText(strEntireText);
			
			// Update button state
			CWnd* pWnd = GetDlgItem(IDC_OPEN_WEBPAGE_BUTTON);
			if(pWnd)
				pWnd->EnableWindow(bEnableButton);

			m_eLastState = eResult;

			// handle background mode
			if(m_bBackgroundMode)
			{
				switch(eBkMode)
				{
				case eRes_None:
					break;
				case eRes_Exit:
					KillTimer(UPDATER_TIMER);
					EndDialog(IDCANCEL);
					return;
				case eRes_Show:
					ShowWindow(SW_SHOW);
					break;
				default:
					BOOST_ASSERT(FALSE);
				}
			}
		}
	}

	CLanguageDialog::OnTimer(nIDEvent);
}

void CUpdaterDlg::UpdateIcon(EUpdateType eType)
{
	HICON hIcon = NULL;
	switch(eType)
	{
	case eIcon_Warning:
		hIcon = AfxGetApp()->LoadStandardIcon(IDI_WARNING);
		break;

	case eIcon_Error:
		hIcon = AfxGetApp()->LoadStandardIcon(IDI_ERROR);
		break;

	case eIcon_Info:
	default:
		hIcon = AfxGetApp()->LoadStandardIcon(IDI_INFORMATION);
		break;
	}

	m_ctlImage.SetIcon(hIcon);
}

void CUpdaterDlg::UpdateMainText(const wchar_t* pszText)
{
	m_ctlMainText.SetWindowText(pszText);
}

void CUpdaterDlg::UpdateSecondaryText(const wchar_t* pszText)
{
	m_ctlRichEdit.SetWindowText(pszText);
}

void CUpdaterDlg::InitRichEdit()
{
	COLORREF crTextColor = GetSysColor(COLOR_BTNTEXT);
	CHARFORMAT2 cf;
	cf.cbSize = sizeof(CHARFORMAT2);

	m_ctlRichEdit.GetDefaultCharFormat(cf);
	cf.dwMask |= CFM_COLOR;
	cf.dwEffects &= ~CFE_AUTOCOLOR;
	cf.crTextColor = crTextColor;
	m_ctlRichEdit.SetDefaultCharFormat(cf);
}
