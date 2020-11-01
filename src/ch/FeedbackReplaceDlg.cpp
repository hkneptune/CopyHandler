// FeedbackReplaceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "FeedbackReplaceDlg.h"
#include "../libictranslate/ResourceManager.h"
#include "FeedbackHandler.h"
#include "resource.h"
#include "../libchengine/TFileInfo.h"
#include "StringHelpers.h"

// CFeedbackReplaceDlg dialog

IMPLEMENT_DYNAMIC(CFeedbackReplaceDlg, ictranslate::CLanguageDialog)

CFeedbackReplaceDlg::CFeedbackReplaceDlg(const chengine::TFileInfo& spSrcFile, const chengine::TFileInfo& spDstFile, CWnd* pParent /*=nullptr*/)
	: ictranslate::CLanguageDialog(IDD_FEEDBACK_REPLACE_DIALOG, pParent),
	m_bAllItems(FALSE),
	m_rSrcFile(spSrcFile),
	m_rDstFile(spDstFile)
{
}

CFeedbackReplaceDlg::~CFeedbackReplaceDlg()
{
}

void CFeedbackReplaceDlg::DoDataExchange(CDataExchange* pDX)
{
	ictranslate::CLanguageDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SRC_ICON_STATIC, m_ctlSrcIcon);
	DDX_Control(pDX, IDC_DST_ICON_STATIC, m_ctlDstIcon);

	DDX_Control(pDX, IDC_SRC_FILENAME_EDIT, m_ctlSrcName);
	DDX_Control(pDX, IDC_SRC_PATH_EDIT, m_ctlSrcPath);
	DDX_Control(pDX, IDC_SRC_MODIFIEDDATE_EDIT, m_ctlSrcDate);
	DDX_Control(pDX, IDC_SRC_FILESIZE_EDIT, m_ctlSrcSize);

	DDX_Control(pDX, IDC_DST_FILENAME_EDIT, m_ctlDstName);
	DDX_Control(pDX, IDC_DST_PATH_EDIT, m_ctlDstPath);
	DDX_Control(pDX, IDC_DST_MODIFIEDDATE_EDIT, m_ctlDstDate);
	DDX_Control(pDX, IDC_DST_FILESIZE_EDIT, m_ctlDstSize);

	DDX_Control(pDX, IDC_REPLACE_BUTTON, m_btnReplace);
	DDX_Control(pDX, IDC_RENAME_BUTTON, m_btnRename);
	DDX_Control(pDX, IDC_RESUME_BUTTON, m_btnResume);
	DDX_Control(pDX, IDC_SKIP_BUTTON, m_btnSkip);
	DDX_Control(pDX, IDC_PAUSE_BUTTON, m_btnPause);
	DDX_Control(pDX, IDC_CANCEL_BUTTON, m_btnCancel);

	DDX_Control(pDX, IDC_MASS_REPLACE_MENUBUTTON, m_btnMassReplace);
	DDX_Control(pDX, IDC_MASS_RENAME_MENUBUTTON, m_btnMassRename);
	DDX_Control(pDX, IDC_MASS_RESUME_MENUBUTTON, m_btnMassResume);
	DDX_Control(pDX, IDC_MASS_SKIP_MENUBUTTON, m_btnMassSkip);
}

BEGIN_MESSAGE_MAP(CFeedbackReplaceDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_REPLACE_BUTTON, &CFeedbackReplaceDlg::OnBnClickedReplaceButton)
	ON_BN_CLICKED(IDC_RESUME_BUTTON, &CFeedbackReplaceDlg::OnBnClickedCopyRestButton)
	ON_BN_CLICKED(IDC_SKIP_BUTTON, &CFeedbackReplaceDlg::OnBnClickedSkipButton)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, &CFeedbackReplaceDlg::OnBnClickedPauseButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, &CFeedbackReplaceDlg::OnBnClickedCancelButton)
	ON_BN_CLICKED(IDC_MASS_REPLACE_MENUBUTTON, &CFeedbackReplaceDlg::OnBnMassReplace)
END_MESSAGE_MAP()


// CFeedbackReplaceDlg message handlers

BOOL CFeedbackReplaceDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	// set dialog icon
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, FALSE);

	AddResizableControl(IDC_INFO_STATIC, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_SRC_ICON_STATIC, 0.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_SRCFILE_STATIC, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_SRC_NAME_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_SRC_FILENAME_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_LOCATION_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_SRC_PATH_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_SIZE_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_SRC_FILESIZE_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_TIME_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_SRC_MODIFIEDDATE_EDIT, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_DST_ICON_STATIC, 0.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_DSTFILE_STATIC, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_DST_NAME_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_DST_FILENAME_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_LOCATION_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_DST_PATH_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_SIZE_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_DST_FILESIZE_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_TIME_STATIC, 0.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_DST_MODIFIEDDATE_EDIT, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_REPLACE_BUTTON, 0.0, 0.0, 0.25, 0.0);
	AddResizableControl(IDC_MASS_REPLACE_MENUBUTTON, 0.25, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_RENAME_BUTTON, 0.25, 0.0, 0.25, 0.0);
	AddResizableControl(IDC_MASS_RENAME_MENUBUTTON, 0.5, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_RESUME_BUTTON, 0.5, 0.0, 0.25, 0.0);
	AddResizableControl(IDC_MASS_RESUME_MENUBUTTON, 0.75, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_SKIP_BUTTON, 0.75, 0.0, 0.25, 0.0);
	AddResizableControl(IDC_MASS_SKIP_MENUBUTTON, 1.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_PAUSE_BUTTON, 0.5, 0.0, 0.25, 0.0);
	AddResizableControl(IDC_CANCEL_BUTTON, 0.75, 0.0, 0.25, 0.0);

	InitializeResizableControls();

	// load the informations about files
	RefreshFilesInfo();
	RefreshImages();

	HMENU hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_PRIORITY_MENU));
	m_mfcMenu.Attach(hMenu);

	m_btnMassReplace.m_hMenu = m_mfcMenu.GetSubMenu(0)->GetSafeHmenu();
	m_btnMassResume.m_hMenu = m_mfcMenu.GetSubMenu(0)->GetSafeHmenu();
	m_btnMassRename.m_hMenu = m_mfcMenu.GetSubMenu(0)->GetSafeHmenu();
	m_btnMassSkip.m_hMenu = m_mfcMenu.GetSubMenu(0)->GetSafeHmenu();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFeedbackReplaceDlg::RefreshFilesInfo()
{
	/////////////////////////////////////////////////////////////
	// src file
	chcore::TSmartPath pathSrc = m_rSrcFile.GetFullFilePath();

	// name
	m_ctlSrcName.SetWindowText(pathSrc.GetFileName().ToString());

	// path
	m_ctlSrcPath.SetWindowText(pathSrc.GetParent().ToString());

	// size
	m_ctlSrcSize.SetWindowText(GetSizeString(m_rSrcFile.GetLength64()));

	// modified date
	COleDateTime dtTemp = m_rSrcFile.GetLastWriteTime().GetAsFiletime();
	m_ctlSrcDate.SetWindowText(dtTemp.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT));

	/////////////////////////////////////////////////////////////
	// dst file
	chcore::TSmartPath pathDst = m_rDstFile.GetFullFilePath();

	// name
	m_ctlDstName.SetWindowText(pathDst.GetFileName().ToString());

	// path
	m_ctlDstPath.SetWindowText(pathDst.GetParent().ToString());

	// size
	m_ctlDstSize.SetWindowText(GetSizeString(m_rDstFile.GetLength64()));

	// modified date
	dtTemp = m_rDstFile.GetLastWriteTime().GetAsFiletime();
	m_ctlDstDate.SetWindowText(dtTemp.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT));

	// button captions
	m_btnResume.EnableWindow(m_rDstFile.GetLength64() < m_rSrcFile.GetLength64());
	m_btnResume.SetTooltip(L"Some tooltip");
	m_btnReplace.SetTooltip(L"Replace tooltip");
}

void CFeedbackReplaceDlg::RefreshImages()
{
	SHFILEINFO shfi;
	DWORD_PTR dwRes = SHGetFileInfo(m_rSrcFile.GetFullFilePath().ToString(), 0, &shfi, sizeof(shfi), SHGFI_ICON);
	if(dwRes)
		m_ctlSrcIcon.SetIcon(shfi.hIcon);

	dwRes = SHGetFileInfo(m_rDstFile.GetFullFilePath().ToString(), 0, &shfi, sizeof(shfi), SHGFI_ICON);
	if(dwRes)
		m_ctlDstIcon.SetIcon(shfi.hIcon);
}

void CFeedbackReplaceDlg::OnBnClickedReplaceButton()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Overwrite);
}

void CFeedbackReplaceDlg::OnBnClickedCopyRestButton()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_CopyRest);
}

void CFeedbackReplaceDlg::OnBnClickedSkipButton()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Skip);
}

void CFeedbackReplaceDlg::OnBnClickedPauseButton()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Pause);
}

void CFeedbackReplaceDlg::OnBnClickedCancelButton()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Cancel);
}

void CFeedbackReplaceDlg::OnBnMassReplace()
{
	CString str;
	switch (m_btnMassReplace.m_nMenuResult)
	{
	case ID_POPUP_TIME_CRITICAL:
		str = L"first menu item clicked";
		break;
	case ID_POPUP_HIGHEST:
		str = L"second menu item clicked";
		break;
	default:
		str = L"Default";
		break;
	}
	MessageBox(str);
}

void CFeedbackReplaceDlg::OnCancel()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Cancel);
}

bool CFeedbackReplaceDlg::IsApplyToAllItemsChecked() const
{
	return m_bAllItems != FALSE;
}
