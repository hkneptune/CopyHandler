// FeedbackReplaceDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ch.h"
#include "FeedbackReplaceDlg.h"
#include "../libictranslate/ResourceManager.h"
#include "FeedbackHandler.h"
#include "resource.h"
#include "../libchengine/TFileInfo.h"

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

	DDX_Check(pDX, IDC_ALL_ITEMS_CHECK, m_bAllItems);
}

BEGIN_MESSAGE_MAP(CFeedbackReplaceDlg, ictranslate::CLanguageDialog)
	ON_BN_CLICKED(IDC_REPLACE_BUTTON, &CFeedbackReplaceDlg::OnBnClickedReplaceButton)
	ON_BN_CLICKED(IDC_COPY_REST_BUTTON, &CFeedbackReplaceDlg::OnBnClickedCopyRestButton)
	ON_BN_CLICKED(IDC_SKIP_BUTTON, &CFeedbackReplaceDlg::OnBnClickedSkipButton)
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, &CFeedbackReplaceDlg::OnBnClickedPauseButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, &CFeedbackReplaceDlg::OnBnClickedCancelButton)
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

	AddResizableControl(IDC_00_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_ICON_STATIC, 0.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_SRCFILE_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_FILENAME_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_PATH_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_FILESIZE_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_SRC_MODIFIEDDATE_EDIT, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_01_STATIC, 0.0, 0.5, 1.0, 0.0);
	AddResizableControl(IDC_DST_ICON_STATIC, 0.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_DSTFILE_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_FILENAME_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_PATH_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_FILESIZE_EDIT, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_DST_MODIFIEDDATE_EDIT, 0.0, 0.0, 1.0, 0.0);

	AddResizableControl(IDC_REPLACE_BUTTON, 0.0, 0.0, 0.2, 0.0);
	AddResizableControl(IDC_COPY_REST_BUTTON, 0.2, 0.0, 0.2, 0.0);
	AddResizableControl(IDC_SKIP_BUTTON, 0.4, 0.0, 0.2, 0.0);
	AddResizableControl(IDC_PAUSE_BUTTON, 0.6, 0.0, 0.2, 0.0);
	AddResizableControl(IDC_CANCEL_BUTTON, 0.8, 0.0, 0.2, 0.0);

	AddResizableControl(IDC_ALL_ITEMS_CHECK, 0.0, 0.0, 1.0, 0.0);

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
	m_ctlSrcPath.SetWindowText(fmt.ToString());

	// size
	strTemplate = rManager.LoadString(IDS_INFO_SIZE_STRING);
	fmt.SetFormat(strTemplate);
	fmt.SetParam(_T("%size"), m_rSrcFile.GetLength64());
	m_ctlSrcSize.SetWindowText(fmt.ToString());

	// modified date
	strTemplate = rManager.LoadString(IDS_INFO_MODIFIED_STRING);
	fmt.SetFormat(strTemplate);
	COleDateTime dtTemp = m_rSrcFile.GetLastWriteTime().GetAsFiletime();
	fmt.SetParam(_T("%datemod"), dtTemp.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT));
	m_ctlSrcDate.SetWindowText(fmt.ToString());

	/////////////////////////////////////////////////////////////
	// dst file
	chcore::TSmartPath pathDst = m_rDstFile.GetFullFilePath();

	// name
	m_ctlDstName.SetWindowText(pathDst.GetFileName().ToString());

	// path
	strTemplate = rManager.LoadString(IDS_INFO_PATH_STRING);
	fmt.SetFormat(strTemplate);
	fmt.SetParam(_T("%pathname"), pathDst.GetParent().ToString());
	m_ctlDstPath.SetWindowText(fmt.ToString());

	// size
	strTemplate = rManager.LoadString(IDS_INFO_SIZE_STRING);
	fmt.SetFormat(strTemplate);
	fmt.SetParam(_T("%size"), m_rDstFile.GetLength64());
	m_ctlDstSize.SetWindowText(fmt.ToString());

	// modified date
	strTemplate = rManager.LoadString(IDS_INFO_MODIFIED_STRING);
	fmt.SetFormat(strTemplate);
	dtTemp = m_rDstFile.GetLastWriteTime().GetAsFiletime();
	fmt.SetParam(_T("%datemod"), dtTemp.Format(LOCALE_NOUSEROVERRIDE, LANG_USER_DEFAULT));
	m_ctlDstDate.SetWindowText(fmt.ToString());

	// button captions
	CWnd* pAppendButton = GetDlgItem(IDC_COPY_REST_BUTTON);
	if(pAppendButton)
	{
		if(m_rDstFile.GetLength64() > m_rSrcFile.GetLength64())
		{
			CString strAltButtonCaption = rManager.LoadString(IDS_BUTTON_TRUNCATE_STRING);
			pAppendButton->SetWindowText(strAltButtonCaption);
		}
	}
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

void CFeedbackReplaceDlg::OnCancel()
{
	UpdateData(TRUE);
	EndDialog(chengine::EFeedbackResult::eResult_Cancel);
}

bool CFeedbackReplaceDlg::IsApplyToAllItemsChecked() const
{
	return m_bAllItems != FALSE;
}

void CFeedbackReplaceDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	if(m_rcInitial.Width() != 0 && m_rcInitial.Height() != 0)
	{
		lpMMI->ptMinTrackSize.y = m_rcInitial.Height();
		lpMMI->ptMaxTrackSize.y = m_rcInitial.Height();
	}
}
