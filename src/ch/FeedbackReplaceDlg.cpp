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
#include "../libchengine/FeedbackPredefinedRules.h"

using namespace chengine;

IMPLEMENT_DYNAMIC(CFeedbackReplaceDlg, ictranslate::CLanguageDialog)

CFeedbackReplaceDlg::CFeedbackReplaceDlg(const chengine::TFileInfo& spSrcFile, const chengine::TFileInfo& spDstFile, CWnd* pParent /*=nullptr*/)
	: ictranslate::CLanguageDialog(IDD_FEEDBACK_REPLACE_DIALOG, pParent),
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
	ON_BN_CLICKED(IDC_MASS_RENAME_MENUBUTTON, &CFeedbackReplaceDlg::OnBnMassRename)
	ON_BN_CLICKED(IDC_MASS_RESUME_MENUBUTTON, &CFeedbackReplaceDlg::OnBnMassResume)
	ON_BN_CLICKED(IDC_MASS_SKIP_MENUBUTTON, &CFeedbackReplaceDlg::OnBnMassSkip)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CFeedbackReplaceDlg message handlers

BOOL CFeedbackReplaceDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	GetWindowRect(&m_rcInitial);

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

	HMENU hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_FEEDBACK_MASS_REPLACE_MENU));
	m_menuMassReplace.Attach(hMenu);
	hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_FEEDBACK_MASS_RENAME_MENU));
	m_menuMassRename.Attach(hMenu);
	hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_FEEDBACK_MASS_RESUME_MENU));
	m_menuMassResume.Attach(hMenu);
	hMenu = GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_FEEDBACK_MASS_SKIP_MENU));
	m_menuMassSkip.Attach(hMenu);

	m_btnMassReplace.m_hMenu = m_menuMassReplace.GetSubMenu(0)->GetSafeHmenu();
	m_btnMassResume.m_hMenu = m_menuMassResume.GetSubMenu(0)->GetSafeHmenu();
	m_btnMassRename.m_hMenu = m_menuMassRename.GetSubMenu(0)->GetSafeHmenu();
	m_btnMassSkip.m_hMenu = m_menuMassSkip.GetSubMenu(0)->GetSafeHmenu();

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
	switch (m_btnMassReplace.m_nMenuResult)
	{
	case ID_FEEDBACK_REPLACE_ALLEXISTINGFILES:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_ApplyToAll, eResult_Overwrite);
		break;
	case ID_FEEDBACK_REPLACE_FILESWITHDIFFERENTDATESORSIZES:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenDifferentDateOrSize, eResult_Overwrite);
		break;
	case ID_FEEDBACK_REPLACE_OLDERFILESWITHNEWERVERSIONS:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenNewerThanDst, eResult_Overwrite);
		break;
	case ID_FEEDBACK_REPLACE_NEWERFILESWITHOLDERVERSIONS:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenOlderThanDst, eResult_Overwrite);
		break;
	}

	EndDialog(chengine::EFeedbackResult::eResult_Overwrite);
}

void CFeedbackReplaceDlg::OnBnMassRename()
{
	switch (m_btnMassRename.m_nMenuResult)
	{
	case ID_FEEDBACK_RENAME_WHENDESTIONATIONFILEEXISTS:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_ApplyToAll, eResult_Rename);
		break;
	case ID_FEEDBACK_RENAME_WHENDATEORSIZEDIFFERS:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenDifferentDateOrSize, eResult_Rename);
		break;
	case ID_FEEDBACK_RENAME_WHENDATEANDSZEARESAME:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenSameDateAndSize, eResult_Rename);
		break;
	case ID_FEEDBACK_RENAME_WHENNEWERTHANDESTINATION:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenNewerThanDst, eResult_Rename);
		break;
	case ID_FEEDBACK_RENAME_WHENOLDERTHANDESTINATION:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenOlderThanDst, eResult_Rename);
		break;
	}

	EndDialog(chengine::EFeedbackResult::eResult_Rename);
}

void CFeedbackReplaceDlg::OnBnMassResume()
{
	switch (m_btnMassResume.m_nMenuResult)
	{
	case ID_FEEDBACK_RESUME_WHENFILEBIGGERTHANDESTINATION:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenBiggerThanDst, eResult_CopyRest);
		break;
	}

	EndDialog(chengine::EFeedbackResult::eResult_CopyRest);
}

void CFeedbackReplaceDlg::OnBnMassSkip()
{
	switch (m_btnMassSkip.m_nMenuResult)
	{
	case ID_FEEDBACK_SKIP_ALLEXISTINGDESTINATIONFILES:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_ApplyToAll, eResult_Skip);
		break;
	case ID_FEEDBACK_SKIP_ALLFILESWITHSAMEDATESANDSIZES:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenSameDateAndSize, eResult_Skip);
		break;
	case ID_FEEDBACK_SKIP_FILESTHATAREOLDERTHANDESTINATION:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenOlderThanDst, eResult_Skip);
		break;
	case ID_FEEDBACK_SKIP_FILESTHATARENEWERTHANDESTINATION:
		m_feedbackRules = FeedbackPredefinedRules::CreateAlreadyExistsRule(EPredefinedRuleCondition::eCondition_WhenNewerThanDst, eResult_Skip);
		break;
	}

	EndDialog(chengine::EFeedbackResult::eResult_Skip);
}

void CFeedbackReplaceDlg::OnCancel()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Cancel);
}

const chengine::FeedbackAlreadyExistsRuleList& CFeedbackReplaceDlg::GetRules() const
{
	return m_feedbackRules;
}

void CFeedbackReplaceDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if (m_rcInitial.Width() != 0 && m_rcInitial.Height() != 0)
	{
		lpMMI->ptMinTrackSize.y = m_rcInitial.Height();
		lpMMI->ptMaxTrackSize.y = m_rcInitial.Height();
	}
}
