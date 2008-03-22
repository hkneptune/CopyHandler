// ICTranslateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ictranslate.h"
#include "ICTranslateDlg.h"
#include <assert.h>
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IMAGE_INVALID 0
#define IMAGE_NONEXISTENT 1
#define IMAGE_OVERFLUOUS 2
#define IMAGE_VALID 3

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CICTranslateDlg dialog




CICTranslateDlg::CICTranslateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CICTranslateDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CICTranslateDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SRCDATA_LIST, m_ctlBaseLanguageList);
	DDX_Control(pDX, IDC_DSTDATA_LIST, m_ctlCustomLanguageList);
	DDX_Control(pDX, IDC_SRCDATA_EDIT, m_ctlSrcText);
	DDX_Control(pDX, IDC_DSTDATA_EDIT, m_ctlDstText);
	DDX_Control(pDX, IDC_SRC_FILENAME_EDIT, m_ctlSrcFilename);
	DDX_Control(pDX, IDC_SRC_AUTHOR_EDIT, m_ctlSrcAuthor);
	DDX_Control(pDX, IDC_SRC_LANGUAGE_NAME_EDIT, m_ctlSrcLanguageName);
	DDX_Control(pDX, IDC_SRC_HELP_FILENAME_EDIT, m_ctlSrcHelpFilename);
	DDX_Control(pDX, IDC_SRC_FONT_EDIT, m_ctlSrcFont);
	DDX_Control(pDX, IDC_SRC_RTL_CHECK, m_ctlSrcRTL);
	DDX_Control(pDX, IDC_DST_FILENAME_EDIT, m_ctlDstFilename);
	DDX_Control(pDX, IDC_DST_AUTHOR_EDIT, m_ctlDstAuthor);
	DDX_Control(pDX, IDC_DST_LANGUAGE_NAME_EDIT, m_ctlDstLanguageName);
	DDX_Control(pDX, IDC_DST_HELP_FILENAME_EDIT, m_ctlDstHelpFilename);
	DDX_Control(pDX, IDC_DST_FONT_EDIT, m_ctlDstFont);
	DDX_Control(pDX, IDC_DST_RTL_CHECK, m_ctlDstRTL);
}

BEGIN_MESSAGE_MAP(CICTranslateDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_COMMAND(ID_FILE_OPENBASETRANSLATION, &CICTranslateDlg::OnFileOpenBaseTranslation)
	ON_COMMAND(ID_FILE_OPENYOURTRANSLATION, &CICTranslateDlg::OnFileOpenYourTranslation)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SRCDATA_LIST, &CICTranslateDlg::OnItemChangedSrcDataList)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_DSTDATA_LIST, &CICTranslateDlg::OnItemChangedDstDataList)
	ON_BN_CLICKED(IDC_COPY_BUTTON, &CICTranslateDlg::OnBnClickedCopyButton)
	ON_BN_CLICKED(IDAPPLY, &CICTranslateDlg::OnBnClickedApply)
	ON_BN_CLICKED(IDC_CHOOSE_FONT_BUTTON, &CICTranslateDlg::OnBnClickedChooseFontButton)
	ON_COMMAND(ID_EDIT_CLEANUP_TRANSLATION, &CICTranslateDlg::OnEditCleanupTranslation)
	ON_COMMAND(ID_FILE_NEWTRANSLATION, &CICTranslateDlg::OnFileNewTranslation)
	ON_COMMAND(ID_FILE_SAVETRANSLATIONAS, &CICTranslateDlg::OnFileSaveTranslationAs)
	ON_COMMAND(ID_FILE_SAVETRANSLATION, &CICTranslateDlg::OnFileSaveTranslation)
	ON_EN_KILLFOCUS(IDC_DST_AUTHOR_EDIT, &CICTranslateDlg::OnEnKillFocusDstAuthorEdit)
	ON_EN_KILLFOCUS(IDC_DST_LANGUAGE_NAME_EDIT, &CICTranslateDlg::OnEnKillFocusDstLanguageNameEdit)
	ON_EN_KILLFOCUS(IDC_DST_HELP_FILENAME_EDIT, &CICTranslateDlg::OnEnKillFocusDstHelpFilenameEdit)
	ON_BN_CLICKED(IDC_DST_RTL_CHECK, &CICTranslateDlg::OnBnClickedDstRtlCheck)
END_MESSAGE_MAP()


// CICTranslateDlg message handlers

BOOL CICTranslateDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		if(strAboutMenu.LoadString(IDS_ABOUTBOX))
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// use image list
	m_ilImages.Create(16, 16, ILC_COLOR4 | ILC_MASK, 0, 4);
	m_ilImages.Add(AfxGetApp()->LoadIcon(IDI_INVALID_ICON));
	m_ilImages.Add(AfxGetApp()->LoadIcon(IDI_NONEXISTENT_ICON));
	m_ilImages.Add(AfxGetApp()->LoadIcon(IDI_OVERFLUOUS_ICON));
	m_ilImages.Add(AfxGetApp()->LoadIcon(IDI_VALID_ICON));

	m_ctlCustomLanguageList.SetImageList(&m_ilImages, LVSIL_SMALL);

	// full row selection
	m_ctlBaseLanguageList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ctlCustomLanguageList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// setup the lists' headers
	// first the width of list (assuming both have the same width)
	CRect rcList;
	m_ctlBaseLanguageList.GetWindowRect(&rcList);
	uint_t uiWidth = rcList.Width();

	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = uiWidth / 5;
	lvc.pszText = _T("ID");

	m_ctlBaseLanguageList.InsertColumn(0, &lvc);
	m_ctlCustomLanguageList.InsertColumn(0, &lvc);

	lvc.cx = 4 * uiWidth / 5;
	lvc.pszText = _T("Text");

	m_ctlBaseLanguageList.InsertColumn(1, &lvc);
	m_ctlCustomLanguageList.InsertColumn(1, &lvc);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CICTranslateDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CICTranslateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CICTranslateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CICTranslateDlg::OnFileOpenBaseTranslation()
{
	CFileDialog fd(TRUE, _T(".lng"), _T(""), OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, _T("Language files (*.lng)|*.lng|All files(*.*)|*.*||"), this);
	if(fd.DoModal() == IDOK)
	{
		if(!m_ldBase.ReadTranslation(fd.GetPathName()))
		{
			AfxMessageBox(_T("Reading file failed"));
			return;
		}

		UpdateBaseLanguageList();
		UpdateCustomLanguageList();
	}
}

void CICTranslateDlg::OnFileOpenYourTranslation()
{
	// check for modification flag
	if(!WarnModified())
		return;

	CFileDialog fd(TRUE, _T(".lng"), _T(""), OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, _T("Language files (*.lng)|*.lng|All files(*.*)|*.*||"), this);
	if(fd.DoModal() == IDOK)
	{
		if(!m_ldCustom.ReadTranslation(fd.GetPathName()))
		{
			AfxMessageBox(_T("Reading file failed"));
			return;
		}

		UpdateCustomLanguageList();
	}
}

void CICTranslateDlg::EnumLngStrings(uint_t uiID, const ictranslate::CTranslationItem* pTranslationItem, ptr_t pData)
{
	CListCtrl* pList = (CListCtrl*)pData;
	assert(pTranslationItem);
	if(!pTranslationItem)
		return;
	CString strID;
	strID.Format(UIFMT, uiID);

	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.pszText = (PTSTR)(PCTSTR)strID;
	lvi.iItem = 0;
	lvi.iSubItem = 0;
	lvi.lParam = uiID;
	
	pList->InsertItem(&lvi);

	lvi.mask = LVIF_TEXT;
	lvi.pszText = (PTSTR)pTranslationItem->GetText();
	lvi.iItem = 0;
	lvi.iSubItem = 1;

	pList->SetItem(&lvi);
}

void CICTranslateDlg::OnItemChangedSrcDataList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if(pNMLV->uNewState & LVIS_SELECTED)
	{
		// set the text to the edit box
		m_ctlSrcText.SetWindowText(m_ctlBaseLanguageList.GetItemText(pNMLV->iItem, 1));

		uint_t uiID = pNMLV->lParam;

		// to avoid infinite loop of selections, check if the current selection is already valid
		POSITION pos = m_ctlCustomLanguageList.GetFirstSelectedItemPosition();
		if(pos)
		{
			int iPos = m_ctlCustomLanguageList.GetNextSelectedItem(pos);
			uint_t uiCurrentID = m_ctlCustomLanguageList.GetItemData(iPos);
			if(uiID == uiCurrentID)
				return;
		}
//		m_ctlCustomLanguageList.SetItemState(-1, 0, LVIS_SELECTED);

		// search in the second list for the specified id
		int iCount = m_ctlCustomLanguageList.GetItemCount();
		for(int i = 0; i < iCount; i++)
		{
			uint_t uiCustomID = m_ctlCustomLanguageList.GetItemData(i);
			if(uiCustomID == uiID)
			{
				m_ctlCustomLanguageList.EnsureVisible(i, FALSE);
				m_ctlCustomLanguageList.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
				break;
			}
		}
	}
	else if(pNMLV->uOldState & LVIS_SELECTED)
	{
		m_ctlCustomLanguageList.SetItemState(-1, 0, LVIS_SELECTED);
		m_ctlDstText.SetWindowText(_T(""));
	}
}

void CICTranslateDlg::OnItemChangedDstDataList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	*pResult = 0;

	if(pNMLV->uNewState & LVIS_SELECTED)
	{
		// set the text to the edit box
		m_ctlDstText.SetWindowText(m_ctlCustomLanguageList.GetItemText(pNMLV->iItem, 1));

		uint_t uiID = pNMLV->lParam;

		// to avoid infinite loop of selections, check if the current selection is already valid
		POSITION pos = m_ctlBaseLanguageList.GetFirstSelectedItemPosition();
		if(pos)
		{
			int iPos = m_ctlBaseLanguageList.GetNextSelectedItem(pos);
			uint_t uiCurrentID = m_ctlBaseLanguageList.GetItemData(iPos);
			if(uiID == uiCurrentID)
				return;
		}
//		m_ctlBaseLanguageList.SetItemState(-1, 0, LVIS_SELECTED);

		// search in the second list for the specified id
		int iCount = m_ctlBaseLanguageList.GetItemCount();
		for(int i = 0; i < iCount; i++)
		{
			uint_t uiCustomID = m_ctlBaseLanguageList.GetItemData(i);
			if(uiCustomID == uiID)
			{
				m_ctlBaseLanguageList.EnsureVisible(i, FALSE);
				m_ctlBaseLanguageList.SetItemState(i, LVIS_SELECTED, LVIS_SELECTED);
				break;
			}
		}
	}
	else if(pNMLV->uOldState & LVIS_SELECTED)
	{
		m_ctlBaseLanguageList.SetItemState(-1, 0, LVIS_SELECTED);
		m_ctlSrcText.SetWindowText(_T(""));
	}
}
void CICTranslateDlg::UpdateBaseLanguageList()
{
	// fill the informations about the translation
	m_ctlSrcFilename.SetWindowText(m_ldBase.GetFilename(true));
	m_ctlSrcAuthor.SetWindowText(m_ldBase.GetAuthor());
	m_ctlSrcLanguageName.SetWindowText(m_ldBase.GetLangName());
	m_ctlSrcHelpFilename.SetWindowText(m_ldBase.GetHelpName());
	CString strFont;
	if(m_ldBase.GetFontFace())
		strFont.Format(TSTRFMT _T(", ") UIFMT, m_ldBase.GetFontFace(), m_ldBase.GetPointSize());
	m_ctlSrcFont.SetWindowText(strFont);
	m_ctlSrcRTL.SetCheck(m_ldBase.GetDirection() ? BST_CHECKED : BST_UNCHECKED);

	// add texts to the list
	m_ctlBaseLanguageList.DeleteAllItems();
	m_ldBase.EnumStrings(&EnumLngStrings, &m_ctlBaseLanguageList);
	m_ctlBaseLanguageList.SortItems(&ListSortFunc, NULL);

	UpdateCustomListImages();
}

void CICTranslateDlg::UpdateCustomLanguageList()
{
	// fill the informations about the translation
	m_ctlDstFilename.SetWindowText(m_ldCustom.GetFilename(true));
	m_ctlDstAuthor.SetWindowText(m_ldCustom.GetAuthor());
	m_ctlDstLanguageName.SetWindowText(m_ldCustom.GetLangName());
	m_ctlDstHelpFilename.SetWindowText(m_ldCustom.GetHelpName());
	m_ctlDstRTL.SetCheck(m_ldCustom.GetDirection() ? BST_CHECKED : BST_UNCHECKED);
	CString strFont;
	if(m_ldCustom.GetFontFace())
		strFont.Format(TSTRFMT _T(", ") UIFMT, m_ldCustom.GetFontFace(), m_ldCustom.GetPointSize());
	m_ctlDstFont.SetWindowText(strFont);

	// add texts to the list
	m_ctlCustomLanguageList.DeleteAllItems();
	m_ldCustom.EnumStrings(&EnumLngStrings, &m_ctlCustomLanguageList);

	// now add the items that exists in the base language and does not exist in the custom one
	std::set<uint_t> setCustomKeys;

	// enum items from custom list
	int iCount = m_ctlCustomLanguageList.GetItemCount();
	for(int i = 0; i < iCount; i++)
	{
		setCustomKeys.insert(m_ctlCustomLanguageList.GetItemData(i));
	}

	// add to custom list values from base that does not exist
	iCount = m_ctlBaseLanguageList.GetItemCount();
	for(int i = 0; i < iCount; i++)
	{
		uint_t uiID = m_ctlBaseLanguageList.GetItemData(i);
		if(setCustomKeys.find(uiID) == setCustomKeys.end())
		{
			// string does not exist in the custom list - add
			CString strID;
			strID.Format(UIFMT, uiID);

			LVITEM lvi;
			lvi.mask = LVIF_TEXT | LVIF_PARAM;
			lvi.pszText = (PTSTR)(PCTSTR)strID;
			lvi.iItem = 0;
			lvi.iSubItem = 0;
			lvi.lParam = uiID;

			m_ctlCustomLanguageList.InsertItem(&lvi);

			lvi.mask = LVIF_TEXT;
			lvi.pszText = _T("");
			lvi.iItem = 0;
			lvi.iSubItem = 1;

			m_ctlCustomLanguageList.SetItem(&lvi);
		}
	}
	m_ctlCustomLanguageList.SortItems(&ListSortFunc, NULL);

	UpdateCustomListImages();
}

void CICTranslateDlg::UpdateCustomListImages()
{
	int iCount = m_ctlCustomLanguageList.GetItemCount();
	for(int i = 0; i < iCount; i++)
	{
		UpdateCustomListImage(i, false);
	}
}

void CICTranslateDlg::UpdateCustomListImage(int iItem, bool bUpdateText)
{
	uint_t uiID = m_ctlCustomLanguageList.GetItemData(iItem);
	ictranslate::CTranslationItem* pBaseItem = m_ldBase.GetTranslationItem(uiID, false);
	ictranslate::CTranslationItem* pCustomItem = m_ldCustom.GetTranslationItem(uiID, false);
	LVITEM lvi;
	if(pCustomItem)
	{
		if(pBaseItem)
		{
			if(pCustomItem->GetChecksum() != pBaseItem->GetChecksum())
				lvi.iImage = IMAGE_INVALID;
			else
				lvi.iImage = IMAGE_VALID;
		}
		else
			lvi.iImage = IMAGE_OVERFLUOUS;
	}
	else
	{
		if(pBaseItem)
			lvi.iImage = IMAGE_NONEXISTENT;
		else
			assert(false);
	}
	lvi.mask = LVIF_IMAGE;
	lvi.iItem = iItem;
	lvi.iSubItem = 0;
	m_ctlCustomLanguageList.SetItem(&lvi);

	if(bUpdateText)
	{
		lvi.iItem = iItem;
		lvi.iSubItem = 1;
		lvi.mask = LVIF_TEXT;
		if(pCustomItem)
			lvi.pszText = (PTSTR)pCustomItem->GetText();
		else
			lvi.pszText = _T("");

		m_ctlCustomLanguageList.SetItem(&lvi);
	}
}

int CALLBACK CICTranslateDlg::ListSortFunc(LPARAM lParam1, LPARAM lParam2, LPARAM /*lParamSort*/)
{
	uint_t uiID1 = (uint_t)lParam1;
	uint_t uiID2 = (uint_t)lParam2;

	if(uiID1 < uiID2)
		return -1;
	else if(uiID1 == uiID2)
		return 0;
	else
		return 1;
}

void CICTranslateDlg::OnBnClickedCopyButton()
{
	CString strText;
	m_ctlSrcText.GetWindowText(strText);
	m_ctlDstText.SetWindowText(strText);
}

void CICTranslateDlg::OnBnClickedApply()
{
	// set the current text as the properly translated one
	CString strText;
	m_ctlDstText.GetWindowText(strText);

	// locate base entry for the current text
	POSITION pos = m_ctlCustomLanguageList.GetFirstSelectedItemPosition();
	if(!pos)
	{
		AfxMessageBox(_T("No text selected."));
		return;
	}

	int iPos = m_ctlCustomLanguageList.GetNextSelectedItem(pos);
	uint_t uiID = m_ctlCustomLanguageList.GetItemData(iPos);

	ictranslate::CTranslationItem* pBaseItem = m_ldBase.GetTranslationItem(uiID, false);
	if(!pBaseItem)
	{
		AfxMessageBox(_T("No base translation available for the item. Perform translation cleanup."));
		return;
	}

	// retrieve item for custom translation if exists, else create new
	ictranslate::CTranslationItem* pCustomItem = m_ldCustom.GetTranslationItem(uiID, true);
	if(pCustomItem)
	{
		pCustomItem->SetText(strText, false);
		pCustomItem->SetChecksum(pBaseItem->GetChecksum());
		m_ldCustom.SetModified();
	}

	UpdateCustomListImage(iPos, true);
}

void CICTranslateDlg::OnBnClickedChooseFontButton()
{
	CClientDC dc(this);

	LOGFONT lf;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfEscapement = 0;
	lf.lfItalic = 0;
	lf.lfOrientation = 0;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfStrikeOut = 0;
	lf.lfUnderline = 0;
	lf.lfWeight = FW_NORMAL;
	lf.lfWidth = 0;
	const tchar_t* pszFontFace = m_ldCustom.GetFontFace();
	if(pszFontFace)
	{
		lf.lfHeight = -MulDiv(m_ldCustom.GetPointSize(), GetDeviceCaps(dc.m_hDC, LOGPIXELSY), 72);
		size_t stLen = _tcslen(pszFontFace);
		if(stLen >= LF_FACESIZE)
			stLen = LF_FACESIZE - 1;
		_tcsncpy(lf.lfFaceName, pszFontFace, stLen);
		lf.lfFaceName[stLen] = _T('\0');
	}
	else
	{
		lf.lfHeight = 0;
		lf.lfFaceName[0] = _T('\0');
	}

	CFontDialog dlg(&lf);
	if(dlg.DoModal() == IDOK)
	{
		// set font info
		dlg.GetCurrentFont(&lf);
		WORD uiPointSize = (WORD)-MulDiv(lf.lfHeight, 72, GetDeviceCaps(dc.m_hDC, LOGPIXELSY));
		m_ldCustom.SetFontFace(lf.lfFaceName);
		m_ldCustom.SetPointSize(uiPointSize);

		CString strFont;
		strFont.Format(TSTRFMT _T(", ") UIFMT, m_ldCustom.GetFontFace(), m_ldCustom.GetPointSize());
		m_ctlDstFont.SetWindowText(strFont);
	}
}

void CICTranslateDlg::OnEditCleanupTranslation()
{
	m_ldCustom.CleanupTranslation(m_ldBase);
	UpdateCustomLanguageList();
}

void CICTranslateDlg::OnFileNewTranslation()
{
	// check for modification flag
	if(!WarnModified())
		return;

	// clear the custom translation
	m_ldCustom.Clear();
	UpdateCustomLanguageList();
}

void CICTranslateDlg::OnFileSaveTranslationAs()
{
	CString strFilename = m_ldCustom.GetFilename(false);
	CString strPath = m_ldCustom.GetFilename(true);
	
	CFileDialog dlg(FALSE, _T(".lng"), strFilename, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Language files (*.lng)|*.lng|All files (*.*)|*.*||"), this);
	if(dlg.DoModal() == IDOK)
	{
		// store additional informations from the dialog box
		CString str;
		m_ctlDstAuthor.GetWindowText(str);
		m_ldCustom.SetAuthor(str);
		m_ctlDstLanguageName.GetWindowText(str);
		m_ldCustom.SetLangName(str);
		m_ctlDstHelpFilename.GetWindowText(str);
		m_ldCustom.SetHelpName(str);
		bool bRTL = (m_ctlDstRTL.GetCheck() == BST_CHECKED);
		m_ldCustom.SetDirection(bRTL);

		// store translation with new name
		m_ldCustom.WriteTranslation(dlg.GetPathName());
		m_ctlDstFilename.SetWindowText(m_ldCustom.GetFilename(true));
	}
}

void CICTranslateDlg::OnFileSaveTranslation()
{
	CString strPath = m_ldCustom.GetFilename(true);
	if(strPath.IsEmpty())
	{
		OnFileSaveTranslationAs();
	}
	else
	{
		// store additional informations from the dialog box
		CString str;
		m_ctlDstAuthor.GetWindowText(str);
		m_ldCustom.SetAuthor(str);
		m_ctlDstLanguageName.GetWindowText(str);
		m_ldCustom.SetLangName(str);
		m_ctlDstHelpFilename.GetWindowText(str);
		m_ldCustom.SetHelpName(str);
		bool bRTL = (m_ctlDstRTL.GetCheck() == BST_CHECKED);
		m_ldCustom.SetDirection(bRTL);

		m_ldCustom.WriteTranslation(NULL);
		m_ctlDstFilename.SetWindowText(m_ldCustom.GetFilename(true));
	}
}

bool CICTranslateDlg::WarnModified() const
{
	// check the modification flag
	if(m_ldCustom.IsModified())
	{
		int iRes = AfxMessageBox(_T("You have modified the translation file. If you continue, the changes might be lost. Do you want to continue ?"), MB_YESNO | MB_ICONQUESTION);
		return iRes == IDYES;
	}
	else
		return true;
}

void CICTranslateDlg::OnCancel()
{
	if(WarnModified())
		CDialog::OnCancel();
}

void CICTranslateDlg::OnEnKillFocusDstAuthorEdit()
{
	CString str;
	m_ctlDstAuthor.GetWindowText(str);
	const tchar_t* psz = m_ldCustom.GetAuthor();
	if(psz && psz != str)
		m_ldCustom.SetAuthor(str);
}

void CICTranslateDlg::OnEnKillFocusDstLanguageNameEdit()
{
	CString str;
	m_ctlDstLanguageName.GetWindowText(str);
	const tchar_t* psz = m_ldCustom.GetLangName();
	if(psz && psz != str)
		m_ldCustom.SetLangName(str);
}

void CICTranslateDlg::OnEnKillFocusDstHelpFilenameEdit()
{
	CString str;
	m_ctlDstHelpFilename.GetWindowText(str);
	const tchar_t* psz = m_ldCustom.GetHelpName();
	if(psz && psz != str)
		m_ldCustom.SetHelpName(str);
}

void CICTranslateDlg::OnBnClickedDstRtlCheck()
{
	bool bRTL = (m_ctlDstRTL.GetCheck() == BST_CHECKED);
	m_ldCustom.SetDirection(bRTL);
}
