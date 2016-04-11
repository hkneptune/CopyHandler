// UpdaterDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "UpdaterDlg.h"
#include "UpdateChecker.h"
#include "../common/version.h"
#include "StaticEx.h"
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "WindowsVersion.h"
#include "../libchcore/TLogger.h"

#define UPDATER_TIMER 639

BEGIN_MESSAGE_MAP(CUpdaterDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_OPEN_WEBPAGE_BUTTON, &CUpdaterDlg::OnBnClickedOpenWebpageButton)
	ON_CBN_SELCHANGE(IDC_UPDATESFREQ_COMBO, OnSelchangeFreqCombo)
	ON_CBN_SELCHANGE(IDC_UPDATECHANNEL_COMBO, OnSelchangeChannelCombo)
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
	DDX_Control(pDX, IDC_UPDATESFREQ_COMBO, m_ctlUpdateFreq);
	DDX_Control(pDX, IDC_UPDATECHANNEL_COMBO, m_ctlUpdateChannel);

}

BOOL CUpdaterDlg::OnInitDialog()
{
	ictranslate::CLanguageDialog::OnInitDialog();

	InitRichEdit();
	InitUpdateFreqCombo();
	InitUpdateChannelCombo();

	// disable button initially
	EnableOpenWebPageButton(false);

	if(!m_bBackgroundMode)
		ShowWindow(SW_SHOW);

	// start the updater
	CheckForUpdates();

	// start a timer to display progress
	SetTimer(UPDATER_TIMER, 50, NULL);

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

		if(eResult != m_eLastState)
		{
			bool bEnableButton = false;
			m_eLastState = eResult;

			switch(eResult)
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
			fmt.SetParam(_T("%site"), _T(PRODUCT_SITE));
			fmt.SetParam(_T("%thisver"), _T(PRODUCT_VERSION));
			fmt.SetParam(L"%thisnumericver", PRODUCT_NUMERIC_VERSION);
			fmt.SetParam(L"%numericver", m_ucChecker.GetNumericVersion());
			fmt.SetParam(_T("%officialver"), m_ucChecker.GetReadableVersion());
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
			EnableOpenWebPageButton(bEnableButton);
			EnableUpdateRelatedControls(eResult > CUpdateChecker::eResult_Pending);

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
					m_bBackgroundMode = false;		// when we show this window for the first time the user is responsible for closing the dialog;
													// otherwise window might close by itself when checking for updates from within the open window
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

void CUpdaterDlg::InitUpdateChannelCombo()
{
	ictranslate::CResourceManager& rResManager = GetResManager();

	std::wstring strText = rResManager.LoadString(IDS_CFGUPDATECHANNELITEMS_STRING);
	std::vector<std::wstring> vItems;
	for(std::wstring strItem : boost::split(vItems, strText, boost::is_any_of(L"!")))
	{
		m_ctlUpdateChannel.AddString(strItem.c_str());
	}

	UpdateVersionInfo::EVersionType eUpdateChannel = (UpdateVersionInfo::EVersionType)GetPropValue<PP_PUPDATECHANNEL>(GetConfig());
	if(eUpdateChannel < vItems.size())
		m_ctlUpdateChannel.SetCurSel(eUpdateChannel);
	else
		m_ctlUpdateChannel.SetCurSel(0);
}

void CUpdaterDlg::InitUpdateFreqCombo()
{
	ictranslate::CResourceManager& rResManager = GetResManager();

	std::wstring strText = rResManager.LoadString(IDS_UPDATE_FREQUENCIES);
	std::vector<std::wstring> vItems;
	for(std::wstring strItem : boost::split(vItems, strText, boost::is_any_of(L"!")))
	{
		m_ctlUpdateFreq.AddString(strItem.c_str());
	}

	EUpdatesFrequency eFrequency = (EUpdatesFrequency)GetPropValue<PP_PCHECK_FOR_UPDATES_FREQUENCY>(GetConfig());
	if(eFrequency < vItems.size())
		m_ctlUpdateFreq.SetCurSel(eFrequency);
	else
		m_ctlUpdateFreq.SetCurSel(0);
}

void CUpdaterDlg::OnSelchangeFreqCombo()
{
	int iCurSel = m_ctlUpdateFreq.GetCurSel();
	if(iCurSel == CB_ERR)
		return;

	EUpdatesFrequency eFrequency = eFreq_Weekly;
	if(iCurSel < EUpdatesFrequency::eFreq_Max)
		eFrequency = (EUpdatesFrequency)iCurSel;

	SetPropValue<PP_PCHECK_FOR_UPDATES_FREQUENCY>(GetConfig(), eFrequency);
	GetConfig().Write();
}

void CUpdaterDlg::OnSelchangeChannelCombo()
{
	int iCurSel = m_ctlUpdateChannel.GetCurSel();
	if(iCurSel == CB_ERR)
		return;

	UpdateVersionInfo::EVersionType eVersionType = UpdateVersionInfo::eReleaseCandidate;
	if(iCurSel < UpdateVersionInfo::EVersionType::eMax)
		eVersionType = (UpdateVersionInfo::EVersionType)iCurSel;

	SetPropValue<PP_PUPDATECHANNEL>(GetConfig(), eVersionType);
	GetConfig().Write();

	CheckForUpdates();
}

void CUpdaterDlg::EnableOpenWebPageButton(bool bEnable)
{
	CWnd* pWnd = GetDlgItem(IDC_OPEN_WEBPAGE_BUTTON);
	if(pWnd)
		pWnd->EnableWindow(bEnable ? TRUE : FALSE);
}

void CUpdaterDlg::CheckForUpdates()
{
	EnableUpdateRelatedControls(false);
	m_eLastState = CUpdateChecker::eResult_Undefined;

	CString strSite;

	EUseSecureConnection eUseSecureConnection = (EUseSecureConnection)GetPropValue<PP_PUPDATE_USE_SECURE_CONNECTION>(GetConfig());
	switch(eUseSecureConnection)
	{
	case eSecure_No:
		strSite = _T(UPDATE_CHECK_LINK_NONSECURE);
		break;

	case eSecure_Yes:
		strSite = _T(UPDATE_CHECK_LINK_SECURE);
		break;

	case eSecure_Auto:
	default:
		{
			bool bUseSecureConnection = WindowsVersion::IsWindows7Or2008R2OrGreater();
			if(bUseSecureConnection)
				strSite = _T(UPDATE_CHECK_LINK_SECURE);
			else
				strSite = _T(UPDATE_CHECK_LINK_NONSECURE);

			break;
		}
	}

	CString strError;
	try
	{
		m_ucChecker.AsyncCheckForUpdates(strSite,
			GetPropValue<PP_PLANGUAGE>(GetConfig()),
			(UpdateVersionInfo::EVersionType)GetPropValue<PP_PUPDATECHANNEL>(GetConfig()),
			m_bBackgroundMode,
			false	// disabled sending headers as it is causing issues with WinInet on WinXP and Win Vista
		);
	}
	catch (const std::exception& e)
	{
		strError = e.what();
	}

	if(!strError.IsEmpty())
	{
		LOG_ERROR(strError);
	}
}

void CUpdaterDlg::EnableUpdateRelatedControls(bool bEnable)
{
	m_ctlUpdateChannel.EnableWindow(bEnable ? TRUE : FALSE);
}
