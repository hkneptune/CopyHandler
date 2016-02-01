// FeedbackReplaceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "../libchcore/TFileInfo.h"
#include "FeedbackReplaceDlg.h"
#include "../libictranslate/ResourceManager.h"
#include "../libchcore/TFileInfo.h"
#include "FeedbackHandler.h"

// CFeedbackReplaceDlg dialog

IMPLEMENT_DYNAMIC(CFeedbackReplaceDlg, ictranslate::CLanguageDialog)

CFeedbackReplaceDlg::CFeedbackReplaceDlg(const chcore::TFileInfo& spSrcFile, const chcore::TFileInfo& spDstFile, CWnd* pParent /*=NULL*/)
	: ictranslate::CLanguageDialog(IDD_FEEDBACK_REPLACE_DIALOG, pParent),
	m_rSrcFile(spSrcFile),
	m_rDstFile(spDstFile),
	m_bAllItems(FALSE)
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

	DDX_Check(pDX, IDC_ALL_ITEMS_CHECK, m_bAllItems);
}


BEGIN_MESSAGE_MAP(CFeedbackReplaceDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_REPLACE_BUTTON, &CFeedbackReplaceDlg::OnBnClickedReplaceButton)
	ON_BN_CLICKED(IDC_COPY_REST_BUTTON, &CFeedbackReplaceDlg::OnBnClickedCopyRestButton)
	ON_BN_CLICKED(IDC_SKIP_BUTTON, &CFeedbackReplaceDlg::OnBnClickedSkipButton)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, &CFeedbackReplaceDlg::OnBnClickedPauseButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, &CFeedbackReplaceDlg::OnBnClickedCancelButton)
END_MESSAGE_MAP()


// CFeedbackReplaceDlg message handlers

BOOL CFeedbackReplaceDlg::OnInitDialog()
{
	CLanguageDialog::OnInitDialog();

	AddResizableControl(IDC_INFO_STATIC, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_00_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_ICON_STATIC, 0.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_SRCFILE_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_FILENAME_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_PATH_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_FILESIZE_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_MODIFIEDDATE_EDIT, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_01_STATIC, 0.0, 0.5, 1.0, 0.0);
	AddResizableControl(IDC_DST_ICON_STATIC, 0.0, 0.5, 0.0, 0.0);

	AddResizableControl(IDC_DSTFILE_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_FILENAME_EDIT, 0.0, 1.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_PATH_EDIT, 0.0, 1.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_FILESIZE_EDIT, 0.0, 1.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_MODIFIEDDATE_EDIT, 0.0, 1.0, 1.0, 0.0);

	AddResizableControl(IDC_COPY_REST_BUTTON, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_SKIP_BUTTON, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_PAUSE_BUTTON, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_CANCEL_BUTTON, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_REPLACE_BUTTON, 0.0, 1.0, 0.0, 0.0);

	AddResizableControl(IDC_ALL_ITEMS_CHECK, 0.0, 1.0, 1.0, 0.0);

	InitializeResizableControls();

	// load the informations about files
	RefreshFilesInfo();
	RefreshImages();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFeedbackReplaceDlg::RefreshFilesInfo()
{
	// load template
	ictranslate::CResourceManager& rManager = GetResManager();

	CString strTemplate;

	/////////////////////////////////////////////////////////////
	// src file
	chcore::TSmartPath pathSrc = m_rSrcFile.GetFullFilePath();

	// name
	m_ctlSrcName.SetWindowText(pathSrc.GetFileName().ToString());

	// path
	strTemplate = rManager.LoadString(IDS_INFO_PATH_STRING);
	ictranslate::CFormat fmt(strTemplate);
	fmt.SetParam(_T("%pathname"), pathSrc.GetParent().ToString());
	m_ctlSrcPath.SetWindowText(fmt);

	// size
	strTemplate = rManager.LoadString(IDS_INFO_SIZE_STRING);
	fmt.SetFormat(strTemplate);
	fmt.SetParam(_T("%size"), m_rSrcFile.GetLength64());
	m_ctlSrcSize.SetWindowText(fmt);

	// modified date
	strTemplate = rManager.LoadString(IDS_INFO_MODIFIED_STRING);
	fmt.SetFormat(strTemplate);
	COleDateTime dtTemp = m_rSrcFile.GetLastWriteTime().GetAsFiletime();
	fmt.SetParam(_T("%datemod"), dtTemp.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT));
	m_ctlSrcDate.SetWindowText(fmt);

	/////////////////////////////////////////////////////////////
	// dst file
	chcore::TSmartPath pathDst = m_rDstFile.GetFullFilePath();

	// name
	m_ctlDstName.SetWindowText(pathDst.GetFileName().ToString());

	// path
	strTemplate = rManager.LoadString(IDS_INFO_PATH_STRING);
	fmt.SetFormat(strTemplate);
	fmt.SetParam(_T("%pathname"), pathDst.GetParent().ToString());
	m_ctlDstPath.SetWindowText(fmt);

	// size
	strTemplate = rManager.LoadString(IDS_INFO_SIZE_STRING);
	fmt.SetFormat(strTemplate);
	fmt.SetParam(_T("%size"), m_rDstFile.GetLength64());
	m_ctlDstSize.SetWindowText(fmt);

	// modified date
	strTemplate = rManager.LoadString(IDS_INFO_MODIFIED_STRING);
	fmt.SetFormat(strTemplate);
	dtTemp = m_rDstFile.GetLastWriteTime().GetAsFiletime();
	fmt.SetParam(_T("%datemod"), dtTemp.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT));
	m_ctlDstDate.SetWindowText(fmt);
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
	EndDialog(chcore::EFeedbackResult::eResult_Overwrite);
}

void CFeedbackReplaceDlg::OnBnClickedCopyRestButton()
{
	UpdateData(TRUE);
	EndDialog(chcore::EFeedbackResult::eResult_CopyRest);
}

void CFeedbackReplaceDlg::OnBnClickedSkipButton()
{
	UpdateData(TRUE);
	EndDialog(chcore::EFeedbackResult::eResult_Skip);
}

void CFeedbackReplaceDlg::OnBnClickedPauseButton()
{
	UpdateData(TRUE);
	EndDialog(chcore::EFeedbackResult::eResult_Pause);
}

void CFeedbackReplaceDlg::OnBnClickedCancelButton()
{
	UpdateData(TRUE);
	EndDialog(chcore::EFeedbackResult::eResult_Cancel);
}

bool CFeedbackReplaceDlg::IsApplyToAllItemsChecked() const
{
	return m_bAllItems != FALSE;
}
