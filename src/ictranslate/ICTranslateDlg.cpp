// ICTranslateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ictranslate.h"
#include "ICTranslateDlg.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


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

	m_ctlBaseLanguageList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_ctlCustomLanguageList.SetExtendedStyle(LVS_EX_FULLROWSELECT);

	// setup the lists
	LVCOLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = 50;
	lvc.pszText = _T("ID");

	m_ctlBaseLanguageList.InsertColumn(0, &lvc);
	m_ctlCustomLanguageList.InsertColumn(0, &lvc);

	lvc.cx = 150;
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
			AfxMessageBox(_T("Reading file failed"));

		// add texts to the list
		m_ldBase.EnumStrings(&EnumLngStrings, &m_ctlBaseLanguageList);
	}
}

void CICTranslateDlg::OnFileOpenYourTranslation()
{
	CFileDialog fd(TRUE, _T(".lng"), _T(""), OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, _T("Language files (*.lng)|*.lng|All files(*.*)|*.*||"), this);
	if(fd.DoModal() == IDOK)
	{
		if(!m_ldCustom.ReadTranslation(fd.GetPathName()))
			AfxMessageBox(_T("Reading file failed"));

		// add texts to the list
		m_ctlCustomLanguageList.DeleteAllItems();
		m_ldCustom.EnumStrings(&EnumLngStrings, &m_ctlCustomLanguageList);
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

void CICTranslateDlg::UpdateCustomListImages()
{
	int iCount = m_ctlCustomLanguageList.GetItemCount();

}
