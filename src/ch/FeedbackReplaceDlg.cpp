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

CFeedbackReplaceDlg::CFeedbackReplaceDlg(const chcore::TFileInfoPtr& spSrcFile, const chcore::TFileInfoPtr& spDstFile, CWnd* pParent /*=NULL*/)
	: ictranslate::CLanguageDialog(IDD_FEEDBACK_REPLACE_DIALOG, pParent),
	m_spSrcFile(spSrcFile),
	m_spDstFile(spDstFile),
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
	DDX_Control(pDX, IDC_SRC_INFO_STATIC, m_ctlSrcInfo);
	DDX_Control(pDX, IDC_DST_INFO_STATIC, m_ctlDstInfo);
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
	AddResizableControl(IDC_SRC_INFO_STATIC, 0.0, 0.0, 1.0, 0.5);

	AddResizableControl(IDC_01_STATIC, 0.0, 0.5, 1.0, 0.0);
	AddResizableControl(IDC_DST_ICON_STATIC, 0.0, 0.5, 0.0, 0.0);
	AddResizableControl(IDC_DST_INFO_STATIC, 0.0, 0.5, 1.0, 0.5);

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
	BOOST_ASSERT(m_spSrcFile && m_spDstFile);
	if(!m_spSrcFile || !m_spDstFile)
		return;

	// load template
	ictranslate::CResourceManager& rManager = GetResManager();

	CString strTemplate;
	strTemplate += rManager.LoadString(IDS_INFO_FILE_STRING);
	strTemplate += _T("\r\n");
	strTemplate += rManager.LoadString(IDS_INFO_SIZE_STRING);
	strTemplate += _T("\r\n");
	strTemplate += rManager.LoadString(IDS_INFO_MODIFIED_STRING);

	ictranslate::CFormat fmt(strTemplate);
	fmt.SetParam(_T("%filename"), m_spSrcFile->GetFullFilePath().ToString());
	fmt.SetParam(_T("%size"), m_spSrcFile->GetLength64());

	COleDateTime dtTemp = m_spSrcFile->GetLastWriteTime();
	fmt.SetParam(_T("%datemod"), dtTemp.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT));

	m_ctlSrcInfo.SetWindowText(fmt);

	fmt.SetFormat(strTemplate);
	fmt.SetParam(_T("%filename"), m_spDstFile->GetFullFilePath().ToString());
	fmt.SetParam(_T("%size"), m_spDstFile->GetLength64());
	dtTemp = m_spDstFile->GetLastWriteTime();
	fmt.SetParam(_T("%datemod"), dtTemp.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT));

	m_ctlDstInfo.SetWindowText(fmt);
}

void CFeedbackReplaceDlg::RefreshImages()
{
	BOOST_ASSERT(m_spSrcFile && m_spDstFile);
	if(!m_spSrcFile || !m_spDstFile)
		return;

	SHFILEINFO shfi;
	DWORD_PTR dwRes = SHGetFileInfo(m_spSrcFile->GetFullFilePath().ToString(), 0, &shfi, sizeof(shfi), SHGFI_ICON);
	if(dwRes)
		m_ctlSrcIcon.SetIcon(shfi.hIcon);

	dwRes = SHGetFileInfo(m_spDstFile->GetFullFilePath().ToString(), 0, &shfi, sizeof(shfi), SHGFI_ICON);
	if(dwRes)
		m_ctlDstIcon.SetIcon(shfi.hIcon);
}

void CFeedbackReplaceDlg::OnBnClickedReplaceButton()
{
	UpdateData(TRUE);
	EndDialog(CFeedbackHandler::eResult_Overwrite);
}

void CFeedbackReplaceDlg::OnBnClickedCopyRestButton()
{
	UpdateData(TRUE);
	EndDialog(CFeedbackHandler::eResult_CopyRest);
}

void CFeedbackReplaceDlg::OnBnClickedSkipButton()
{
	UpdateData(TRUE);
	EndDialog(CFeedbackHandler::eResult_Skip);
}

void CFeedbackReplaceDlg::OnBnClickedPauseButton()
{
	UpdateData(TRUE);
	EndDialog(CFeedbackHandler::eResult_Pause);
}

void CFeedbackReplaceDlg::OnBnClickedCancelButton()
{
	UpdateData(TRUE);
	EndDialog(CFeedbackHandler::eResult_Cancel);
}
