/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/

#include "stdafx.h"
#include "resource.h"
#include "CustomCopyDlg.h"
#include "structs.h"
#include "dialogs.h"
#include "BufferSizeDlg.h"
#include "FilterDlg.h"
#include "StringHelpers.h"
#include "ch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomCopyDlg dialog


CCustomCopyDlg::CCustomCopyDlg() : CHLanguageDialog(CCustomCopyDlg::IDD)
{
	//{{AFX_DATA_INIT(CCustomCopyDlg)
	m_ucCount = 1;
	m_bOnlyCreate = FALSE;
	m_bIgnoreFolders = FALSE;
	m_bFilters = FALSE;
	m_bAdvanced = FALSE;
	m_bForceDirectories = FALSE;
	//}}AFX_DATA_INIT
	
//	m_ccData.m_astrPaths.RemoveAll();	// unneeded
//	m_ccData.m_strDestPath.Empty();
	
	m_ccData.m_iOperation=0;
	m_ccData.m_iPriority=THREAD_PRIORITY_NORMAL;
	m_ccData.m_ucCount=1;

	// m_ccData.m_bsSizes stays uninitialized
	// m_ccData.m_afFilters - this too
	
	m_ccData.m_bIgnoreFolders=false;
	m_ccData.m_bForceDirectories=false;
	m_ccData.m_bCreateStructure=false;

	m_bActualisation=false;
}

void CCustomCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CHLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomCopyDlg)
	DDX_Control(pDX, IDC_DESTPATH_COMBOBOXEX, m_ctlDstPath);
	DDX_Control(pDX, IDC_COUNT_SPIN, m_ctlCountSpin);
	DDX_Control(pDX, IDC_FILTERS_LIST, m_ctlFilters);
	DDX_Control(pDX, IDC_BUFFERSIZES_LIST, m_ctlBufferSizes);
	DDX_Control(pDX, IDC_OPERATION_COMBO, m_ctlOperation);
	DDX_Control(pDX, IDC_PRIORITY_COMBO, m_ctlPriority);
	DDX_Control(pDX, IDC_FILES_LIST, m_ctlFiles);
	DDX_Text(pDX, IDC_COUNT_EDIT, m_ucCount);
	DDV_MinMaxByte(pDX, m_ucCount, 1, 255);
	DDX_Check(pDX, IDC_ONLYSTRUCTURE_CHECK, m_bOnlyCreate);
	DDX_Check(pDX, IDC_IGNOREFOLDERS_CHECK, m_bIgnoreFolders);
	DDX_Check(pDX, IDC_FORCEDIRECTORIES_CHECK, m_bForceDirectories);
	DDX_Check(pDX, IDC_FILTERS_CHECK, m_bFilters);
	DDX_Check(pDX, IDC_ADVANCED_CHECK, m_bAdvanced);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCustomCopyDlg, CHLanguageDialog)
	//{{AFX_MSG_MAP(CCustomCopyDlg)
	ON_BN_CLICKED(IDC_ADDDIR_BUTTON, OnAddDirectoryButton)
	ON_BN_CLICKED(IDC_ADDFILE_BUTTON, OnAddFilesButton)
	ON_BN_CLICKED(IDC_REMOVEFILEFOLDER_BUTTON, OnRemoveButton)
	ON_BN_CLICKED(IDC_DESTBROWSE_BUTTON, OnBrowseButton)
	ON_BN_CLICKED(IDC_BUFFERSIZES_BUTTON, OnChangebufferButton)
	ON_BN_CLICKED(IDC_ADDFILTER_BUTTON, OnAddfilterButton)
	ON_BN_CLICKED(IDC_REMOVEFILTER_BUTTON, OnRemovefilterButton)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_FILTERS_CHECK, OnFiltersCheck)
	ON_BN_CLICKED(IDC_STANDARD_CHECK, OnStandardCheck)
	ON_BN_CLICKED(IDC_ADVANCED_CHECK, OnAdvancedCheck)
	ON_NOTIFY(NM_DBLCLK, IDC_FILTERS_LIST, OnDblclkFiltersList)
	ON_LBN_DBLCLK(IDC_BUFFERSIZES_LIST, OnDblclkBuffersizesList)
	ON_CBN_EDITCHANGE(IDC_DESTPATH_COMBOBOXEX, OnEditchangeDestpathComboboxex)
	ON_BN_CLICKED(IDC_IMPORT_BUTTON, OnImportButton)
	ON_BN_CLICKED(IDC_IGNOREFOLDERS_CHECK, OnIgnorefoldersCheck)
	ON_BN_CLICKED(IDC_FORCEDIRECTORIES_CHECK, OnForcedirectoriesCheck)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomCopyDlg message handlers
BOOL CCustomCopyDlg::OnInitDialog() 
{
	CHLanguageDialog::OnInitDialog();

	// make this dialog on top
	SetWindowPos(&wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE /*| SWP_SHOWWINDOW*/);

	// paths' listbox - init images - system image list
    SHFILEINFO sfi;
    HIMAGELIST hImageList = (HIMAGELIST)SHGetFileInfo(_T("C:\\"), FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO), 
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

	m_ilImages.Attach(hImageList);
	m_ctlFiles.SetImageList(&m_ilImages, LVSIL_SMALL);

	// calc list width
	CRect rc;
	m_ctlFiles.GetWindowRect(&rc);
	rc.right-=GetSystemMetrics(SM_CXEDGE)*2;

	// some styles
	m_ctlFiles.SetExtendedStyle(m_ctlFiles.GetExtendedStyle() | LVS_EX_FULLROWSELECT);
	m_ctlFilters.SetExtendedStyle(m_ctlFiles.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// paths' listbox - add one column
	LVCOLUMN lvc;
	lvc.mask=LVCF_FMT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;
	lvc.cx=rc.Width();
	m_ctlFiles.InsertColumn(1, &lvc);
	
	// fill paths' listbox
	for (int i=0;i<m_ccData.m_astrPaths.GetSize();i++)
		AddPath(m_ccData.m_astrPaths.GetAt(i));

	// image list for a combo with recent paths
	m_ctlDstPath.SetImageList(&m_ilImages);

	// recent paths addition
	COMBOBOXEXITEM cbi;
	CString strText;
	cbi.mask=CBEIF_IMAGE | CBEIF_TEXT;

	for (i=0;i<(int)m_ccData.m_vRecent.size();i++)
	{
		cbi.iItem=i;
		strText=m_ccData.m_vRecent.at(i);
		cbi.pszText=strText.GetBuffer(1);
		sfi.iIcon=-1;
		SHGetFileInfo(strText, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		cbi.iImage=sfi.iIcon;

		m_ctlDstPath.InsertItem(&cbi);
	}

	// destination path
	SetComboPath(m_ccData.m_strDestPath);
//	m_strDest=m_ccData.m_strDestPath;	//**

	// operation type
	m_ctlOperation.AddString(GetResManager()->LoadString(IDS_CCDCOPY_STRING));
	m_ctlOperation.AddString(GetResManager()->LoadString(IDS_CCDMOVE_STRING));

	// copying/moving
	m_ctlOperation.SetCurSel(m_ccData.m_iOperation);

	// fill priority combo
	for (i=0;i<7;i++)
	{
		m_ctlPriority.AddString(GetResManager()->LoadString(IDS_PRIORITY0_STRING+i));
	}

	m_ctlPriority.SetCurSel(PriorityToIndex(m_ccData.m_iPriority));

	// count of copies
	m_ucCount=m_ccData.m_ucCount;
	m_ctlCountSpin.SetRange(1, 255);

	// fill buffer sizes listbox
	SetBuffersizesString();

	// list width
	m_ctlFilters.GetWindowRect(&rc);
	rc.right-=GetSystemMetrics(SM_CXEDGE)*2;

	// filter - some columns in a header
	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;

	// mask
	lvc.iSubItem=-1;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDRMASK_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.15*rc.Width());
	m_ctlFilters.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem=0;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.15*rc.Width());
	m_ctlFilters.InsertColumn(2, &lvc);

	// size
	lvc.iSubItem=1;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDRSIZE_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.3*rc.Width());
	m_ctlFilters.InsertColumn(3, &lvc);

	// time
	lvc.iSubItem=2;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDRDATE_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.3*rc.Width());
	m_ctlFilters.InsertColumn(4, &lvc);

	// attributes
	lvc.iSubItem=3;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDRATTRIB_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.1*rc.Width());
	m_ctlFilters.InsertColumn(5, &lvc);

	// -attributes
	lvc.iSubItem=4;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDREXCLUDEATTRIB_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.1*rc.Width());
	m_ctlFilters.InsertColumn(6, &lvc);
	
	m_bFilters=!!m_ccData.m_afFilters.GetSize();

	// other custom flags
	m_bAdvanced=(m_ccData.m_bIgnoreFolders | m_ccData.m_bCreateStructure);
	m_bIgnoreFolders=m_ccData.m_bIgnoreFolders;
	m_bForceDirectories=m_ccData.m_bForceDirectories;
	m_bOnlyCreate=m_ccData.m_bCreateStructure;

	UpdateData(FALSE);

	EnableControls();

	return TRUE;
}

void CCustomCopyDlg::OnLanguageChanged(WORD /*wOld*/, WORD /*wNew*/)
{
	UpdateData(TRUE);

	// count the width of a list
	CRect rc;
	m_ctlFiles.GetWindowRect(&rc);
	rc.right-=GetSystemMetrics(SM_CXEDGE)*2;

	// change the width of a column
	LVCOLUMN lvc;
	lvc.mask=LVCF_WIDTH;
	lvc.cx=rc.Width();
	m_ctlFiles.SetColumn(0, &lvc);

	// operation
	int iPos=m_ctlOperation.GetCurSel();
	m_ctlOperation.ResetContent();
	m_ctlOperation.AddString(GetResManager()->LoadString(IDS_CCDCOPY_STRING));
	m_ctlOperation.AddString(GetResManager()->LoadString(IDS_CCDMOVE_STRING));
	m_ctlOperation.SetCurSel(iPos);

	// priority combo
	iPos=m_ctlPriority.GetCurSel();
	m_ctlPriority.ResetContent();
	for (int i=0;i<7;i++)
	{
		m_ctlPriority.AddString(GetResManager()->LoadString(IDS_PRIORITY0_STRING+i));
	}
	m_ctlPriority.SetCurSel(iPos);

	// fill the listbox with buffers
	SetBuffersizesString();

	// filter section (filter, size, date, attributes)
	while(m_ctlFilters.DeleteColumn(0));		// delete all columns

	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;

	// mask
	lvc.iSubItem=-1;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDRMASK_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.15*rc.Width());
	m_ctlFilters.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem=0;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.15*rc.Width());
	m_ctlFilters.InsertColumn(2, &lvc);

	// size
	lvc.iSubItem=1;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDRSIZE_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.3*rc.Width());
	m_ctlFilters.InsertColumn(3, &lvc);

	// time
	lvc.iSubItem=2;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDRDATE_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.3*rc.Width());
	m_ctlFilters.InsertColumn(4, &lvc);

	// attributes
	lvc.iSubItem=3;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDRATTRIB_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.1*rc.Width());
	m_ctlFilters.InsertColumn(5, &lvc);

	// -attributes
	lvc.iSubItem=4;
	lvc.pszText=(PTSTR)GetResManager()->LoadString(IDS_HDREXCLUDEATTRIB_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.1*rc.Width());
	m_ctlFilters.InsertColumn(6, &lvc);

	// refresh the entries in filters' list
	m_ctlFilters.DeleteAllItems();
	for (i=0;i<m_ccData.m_afFilters.GetSize();i++)
		AddFilter(m_ccData.m_afFilters.GetAt(i), i);
}

void CCustomCopyDlg::OnAddDirectoryButton() 
{
	CString strPath;
	if (BrowseForFolder(GetResManager()->LoadString(IDS_BROWSE_STRING), &strPath))
		AddPath(strPath);
}

void CCustomCopyDlg::OnAddFilesButton() 
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NODEREFERENCELINKS | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, GetResManager()->LoadString(IDS_FILEDLGALLFILTER_STRING), this);
	
	TCHAR *pszBuffer=new TCHAR[65535];
	memset(pszBuffer, 0, 65535*sizeof(TCHAR));
	dlg.m_ofn.lpstrFile=pszBuffer;
	dlg.m_ofn.nMaxFile=65535;

	if (dlg.DoModal() == IDOK)
	{
		// first element is the path
		CString strPath=pszBuffer;

		int iOffset=_tcslen(pszBuffer)+1;
		
		// get filenames
		if (pszBuffer[iOffset] == _T('\0'))
			AddPath(strPath);
		else
		{
			if (strPath.Right(1) != _T("\\"))
				strPath+=_T("\\");
			while (pszBuffer[iOffset] != _T('\0'))
			{
				AddPath(strPath+CString(pszBuffer+iOffset));
				iOffset+=_tcslen(pszBuffer+iOffset)+1;
			}
		}
	}

	// delete buffer
	delete [] pszBuffer;
}

void CCustomCopyDlg::OnRemoveButton() 
{
	POSITION pos;
	int iItem;
	while (true)
	{
		pos = m_ctlFiles.GetFirstSelectedItemPosition();
		if (pos == NULL)
			break;
		else
		{
			iItem=m_ctlFiles.GetNextSelectedItem(pos);
			m_ctlFiles.DeleteItem(iItem);
		}
	}
}

void CCustomCopyDlg::OnBrowseButton() 
{
	CString strPath;
	if (BrowseForFolder(GetResManager()->LoadString(IDS_DSTFOLDERBROWSE_STRING), &strPath))
	{
		SetComboPath(strPath);
//		m_strDest=strPath;	//**
	}
}

void CCustomCopyDlg::OnOK() 
{
	UpdateData(TRUE);

	// copy files from listctrl to an array
	m_ccData.m_astrPaths.RemoveAll();
	CString strPath;

	for (int i=0;i<m_ctlFiles.GetItemCount();i++)
		m_ccData.m_astrPaths.Add(m_ctlFiles.GetItemText(i, 0));

	// dest path
	m_ctlDstPath.GetWindowText(m_ccData.m_strDestPath);
//	m_ccData.m_strDestPath=m_strDest;	//**

	// operation type
	m_ccData.m_iOperation=m_ctlOperation.GetCurSel();

	// priority
	m_ccData.m_iPriority=IndexToPriority(m_ctlPriority.GetCurSel());

	// count of copies
	m_ccData.m_ucCount=m_ucCount;

	// buffersize is being changed realtime
	// so as filter

	m_ccData.m_bIgnoreFolders=(m_bIgnoreFolders != 0);
	m_ccData.m_bForceDirectories=(m_bForceDirectories != 0);
	m_ccData.m_bCreateStructure=(m_bOnlyCreate != 0);

	if (m_ccData.m_strDestPath.IsEmpty() || m_ccData.m_astrPaths.GetSize() == 0)
		MsgBox(IDS_MISSINGDATA_STRING);
	else
		CHLanguageDialog::OnOK();
}

void CCustomCopyDlg::SetBuffersizesString()
{
	CRect rcList;
	m_ctlBufferSizes.GetWindowRect(&rcList);
	rcList.right-=2*GetSystemMetrics(SM_CXEDGE);

	m_ctlBufferSizes.SetColumnWidth(rcList.Width()/3);

	// erase everything
	m_ctlBufferSizes.ResetContent();

	// fill the list
	TCHAR szData[160], szSize[64];
	
	_stprintf(szData, GetResManager()->LoadString(IDS_BSEDEFAULT_STRING), GetSizeString(m_ccData.m_bsSizes.m_uiDefaultSize, szSize, true));
	m_ctlBufferSizes.AddString(szData);
	
	if (!m_ccData.m_bsSizes.m_bOnlyDefault)
	{
		_stprintf(szData, GetResManager()->LoadString(IDS_BSEONEDISK_STRING), GetSizeString(m_ccData.m_bsSizes.m_uiOneDiskSize, szSize, true));
		m_ctlBufferSizes.AddString(szData);
		
		_stprintf(szData, GetResManager()->LoadString(IDS_BSETWODISKS_STRING), GetSizeString(m_ccData.m_bsSizes.m_uiTwoDisksSize, szSize, true));
		m_ctlBufferSizes.AddString(szData);
		
		_stprintf(szData, GetResManager()->LoadString(IDS_BSECD_STRING), GetSizeString(m_ccData.m_bsSizes.m_uiCDSize, szSize, true));
		m_ctlBufferSizes.AddString(szData);
		
		_stprintf(szData, GetResManager()->LoadString(IDS_BSELAN_STRING), GetSizeString(m_ccData.m_bsSizes.m_uiLANSize, szSize, true));
		m_ctlBufferSizes.AddString(szData);
	}
}

void CCustomCopyDlg::OnChangebufferButton() 
{
	CBufferSizeDlg dlg;
	dlg.m_bsSizes=m_ccData.m_bsSizes;
	if (dlg.DoModal() == IDOK)
	{
		m_ccData.m_bsSizes=dlg.m_bsSizes;
		SetBuffersizesString();
	}
}

void CCustomCopyDlg::AddPath(CString strPath)
{
	// fill listbox with paths
	LVITEM lvi;
	lvi.mask=LVIF_TEXT | LVIF_IMAGE;
	lvi.iItem=m_ctlFiles.GetItemCount();
	lvi.iSubItem=0;
	
	// there's no need for a high speed so get the images
	SHFILEINFO sfi;
	SHGetFileInfo(strPath, FILE_ATTRIBUTE_NORMAL, &sfi,  sizeof(SHFILEINFO), 
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON/* | SHGFI_USEFILEATTRIBUTES*/);
	
	// fill the list
	lvi.pszText=strPath.GetBuffer(0);
	strPath.ReleaseBuffer();
	lvi.cchTextMax=lstrlen(lvi.pszText);
	lvi.iImage=sfi.iIcon;
	m_ctlFiles.InsertItem(&lvi);
}

void CCustomCopyDlg::OnAddfilterButton() 
{
	CFilterDlg dlg;
	CString strData;
	for (int i=0;i<m_ccData.m_afFilters.GetSize();i++)
	{
		if (m_ccData.m_afFilters.GetAt(i).m_bUseMask)
			dlg.m_astrAddMask.Add(m_ccData.m_afFilters.GetAt(i).GetCombinedMask(strData));
		if (m_ccData.m_afFilters.GetAt(i).m_bUseExcludeMask)
			dlg.m_astrAddExcludeMask.Add(m_ccData.m_afFilters.GetAt(i).GetCombinedExcludeMask(strData));
	}
	
	if (dlg.DoModal() == IDOK)
	{
		if (dlg.m_ffFilter.m_bUseMask || dlg.m_ffFilter.m_bUseExcludeMask || dlg.m_ffFilter.m_bUseSize 
			|| dlg.m_ffFilter.m_bUseDate || dlg.m_ffFilter.m_bUseAttributes)
		{
			m_ccData.m_afFilters.Add(dlg.m_ffFilter);
			AddFilter(dlg.m_ffFilter);
		}
		else
			MsgBox(IDS_EMPTYFILTER_STRING, MB_OK | MB_ICONINFORMATION);
	}
}

void CCustomCopyDlg::AddFilter(const CFileFilter &rFilter, int iPos)
{
	LVITEM lvi;
	TCHAR szLoaded[1024];
	TCHAR szData[64];

	lvi.mask=LVIF_TEXT;
	lvi.iItem=(iPos == -1) ? m_ctlFilters.GetItemCount() : iPos;

	/////////////////////
	lvi.iSubItem=0;
	
	if (rFilter.m_bUseMask)
	{
		CString strData;
		rFilter.GetCombinedMask(strData);
		_tcscpy(szLoaded, strData);
	}
	else
		_tcscpy(szLoaded, GetResManager()->LoadString(IDS_FILTERMASKEMPTY_STRING));
	
	lvi.pszText=szLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.InsertItem(&lvi);

	/////////////////////
	lvi.iSubItem=1;
	
	if (rFilter.m_bUseExcludeMask)
	{
		CString strData;
		rFilter.GetCombinedExcludeMask(strData);
		_tcscpy(szLoaded, strData);
	}
	else
		_tcscpy(szLoaded, GetResManager()->LoadString(IDS_FILTERMASKEMPTY_STRING));
	
	lvi.pszText=szLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);

	/////////////////
	lvi.iSubItem=2;
	
	if (rFilter.m_bUseSize)
	{
		_stprintf(szLoaded, _T("%s %s"), GetResManager()->LoadString(IDS_LT_STRING+rFilter.m_iSizeType1), GetSizeString(static_cast<__int64>(rFilter.m_ullSize1), szData, true));

		if (rFilter.m_bUseSize2)
		{
			_tcscat(szLoaded, GetResManager()->LoadString(IDS_AND_STRING));
			CString strLoaded2;
			strLoaded2.Format(_T("%s %s"), GetResManager()->LoadString(IDS_LT_STRING+rFilter.m_iSizeType2), GetSizeString(static_cast<__int64>(rFilter.m_ullSize2), szData, true));
			_tcscat(szLoaded, strLoaded2);
		}
	}
	else
		_tcscpy(szLoaded, GetResManager()->LoadString(IDS_FILTERSIZE_STRING));
	
	lvi.pszText=szLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);

	///////////////////
	lvi.iSubItem=3;
	
	if (rFilter.m_bUseDate)
	{
		_stprintf(szLoaded, _T("%s %s"), GetResManager()->LoadString(IDS_DATECREATED_STRING+rFilter.m_iDateType), GetResManager()->LoadString(IDS_LT_STRING+rFilter.m_iDateType1));
		if (rFilter.m_bDate1)
			_tcscat(szLoaded, rFilter.m_tDate1.Format(_T(" %x")));
		if (rFilter.m_bTime1)
			_tcscat(szLoaded, rFilter.m_tTime1.Format(_T(" %X")));

		if (rFilter.m_bUseDate2)
		{
			_tcscat(szLoaded, GetResManager()->LoadString(IDS_AND_STRING));
			_tcscat(szLoaded, GetResManager()->LoadString(IDS_LT_STRING+rFilter.m_iDateType2));
			if (rFilter.m_bDate2)
				_tcscat(szLoaded, rFilter.m_tDate2.Format(_T(" %x")));
			if (rFilter.m_bTime2)
				_tcscat(szLoaded, rFilter.m_tTime2.Format(_T(" %X")));
		}
	}
	else
		_tcscpy(szLoaded, GetResManager()->LoadString(IDS_FILTERDATE_STRING));

	lvi.pszText=szLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);

	/////////////////////
	lvi.iSubItem=4;
	szLoaded[0]=_T('\0');
	if (rFilter.m_bUseAttributes)
	{
		if (rFilter.m_iArchive == 1)
			_tcscat(szLoaded, _T("A"));
		if (rFilter.m_iReadOnly == 1)
			_tcscat(szLoaded, _T("R"));
		if (rFilter.m_iHidden == 1)
			_tcscat(szLoaded, _T("H"));
		if (rFilter.m_iSystem == 1)
			_tcscat(szLoaded, _T("S"));
		if (rFilter.m_iDirectory == 1)
			_tcscat(szLoaded, _T("D"));
	}

	if (!rFilter.m_bUseAttributes || szLoaded[0] == _T('\0'))
		_tcscpy(szLoaded, GetResManager()->LoadString(IDS_FILTERATTRIB_STRING));
	
	lvi.pszText=szLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);

	/////////////////////
	lvi.iSubItem=5;
	szLoaded[0]=_T('\0');
	if (rFilter.m_bUseAttributes)
	{
		if (rFilter.m_iArchive == 0)
			_tcscat(szLoaded, _T("A"));
		if (rFilter.m_iReadOnly == 0)
			_tcscat(szLoaded, _T("R"));
		if (rFilter.m_iHidden == 0)
			_tcscat(szLoaded, _T("H"));
		if (rFilter.m_iSystem == 0)
			_tcscat(szLoaded, _T("S"));
		if (rFilter.m_iDirectory == 0)
			_tcscat(szLoaded, _T("D"));
	}

	if (!rFilter.m_bUseAttributes || szLoaded[0] == _T('0'))
		_tcscpy(szLoaded, GetResManager()->LoadString(IDS_FILTERATTRIB_STRING));

	lvi.pszText=szLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);
}

void CCustomCopyDlg::OnRemovefilterButton() 
{
	POSITION pos;
	int iItem;
	while (true)
	{
		pos=m_ctlFilters.GetFirstSelectedItemPosition();
		if (pos == NULL)
			break;
		else
		{
			iItem=m_ctlFilters.GetNextSelectedItem(pos);
			m_ctlFilters.DeleteItem(iItem);
			m_ccData.m_afFilters.RemoveAt(iItem);
		}
	}
}

void CCustomCopyDlg::OnDestroy() 
{
	m_ctlFiles.SetImageList(NULL, LVSIL_SMALL);
	m_ilImages.Detach();

	CHLanguageDialog::OnDestroy();
}

void CCustomCopyDlg::EnableControls()
{
	UpdateData(TRUE);

	m_ctlFilters.EnableWindow(m_bFilters);
	GetDlgItem(IDC_ADDFILTER_BUTTON)->EnableWindow(m_bFilters);
	GetDlgItem(IDC_REMOVEFILTER_BUTTON)->EnableWindow(m_bFilters);
	
	GetDlgItem(IDC_IGNOREFOLDERS_CHECK)->EnableWindow(m_bAdvanced && !m_bForceDirectories);
	GetDlgItem(IDC_FORCEDIRECTORIES_CHECK)->EnableWindow(m_bAdvanced && !m_bIgnoreFolders);
	GetDlgItem(IDC_ONLYSTRUCTURE_CHECK)->EnableWindow(m_bAdvanced);
}

void CCustomCopyDlg::OnFiltersCheck() 
{
	EnableControls();
}

void CCustomCopyDlg::OnStandardCheck() 
{
	EnableControls();
}

void CCustomCopyDlg::OnAdvancedCheck() 
{
	EnableControls();
}

void CCustomCopyDlg::OnDblclkFiltersList(NMHDR* /*pNMHDR*/, LRESULT* pResult) 
{
	POSITION pos=m_ctlFilters.GetFirstSelectedItemPosition();
	if (pos != NULL)
	{
		int iItem=m_ctlFilters.GetNextSelectedItem(pos);
		CFilterDlg dlg;
		dlg.m_ffFilter=m_ccData.m_afFilters.GetAt(iItem);
		
		CString strData;
		for (int i=0;i<m_ccData.m_afFilters.GetSize();i++)
		{
			if (m_ccData.m_afFilters.GetAt(i).m_bUseMask && i != iItem)
				dlg.m_astrAddMask.Add(m_ccData.m_afFilters.GetAt(i).GetCombinedMask(strData));
			if (m_ccData.m_afFilters.GetAt(i).m_bUseExcludeMask && i != iItem)
				dlg.m_astrAddExcludeMask.Add(m_ccData.m_afFilters.GetAt(i).GetCombinedExcludeMask(strData));
		}

		if (dlg.DoModal() == IDOK)
		{
			// delete old element
			m_ctlFilters.DeleteItem(iItem);
			m_ccData.m_afFilters.RemoveAt(iItem);

			// insert new if needed
			if (dlg.m_ffFilter.m_bUseMask || dlg.m_ffFilter.m_bUseExcludeMask || dlg.m_ffFilter.m_bUseSize 
				|| dlg.m_ffFilter.m_bUseDate || dlg.m_ffFilter.m_bUseAttributes)
			{
				m_ccData.m_afFilters.InsertAt(iItem, dlg.m_ffFilter);
				AddFilter(dlg.m_ffFilter, iItem);
			}
		}
	}

	*pResult = 0;
}

void CCustomCopyDlg::OnDblclkBuffersizesList() 
{
	int iItem=m_ctlBufferSizes.GetCurSel();
	if (iItem != LB_ERR)
	{
		CBufferSizeDlg dlg;
		dlg.m_bsSizes=m_ccData.m_bsSizes;
		dlg.m_iActiveIndex=iItem;
		if (dlg.DoModal() == IDOK)
		{
			m_ccData.m_bsSizes=dlg.m_bsSizes;
			SetBuffersizesString();
		}
	}
}

void CCustomCopyDlg::SetComboPath(LPCTSTR lpszText)
{
	// set current select to -1
	m_ctlDstPath.SetCurSel(-1);

	SHFILEINFO sfi;
	sfi.iIcon=-1;

	COMBOBOXEXITEM cbi;
	TCHAR szPath[_MAX_PATH];

	cbi.mask=CBEIF_TEXT | CBEIF_IMAGE;
	cbi.iItem=-1;
	_tcscpy(szPath, lpszText);
	cbi.pszText=szPath;
	SHGetFileInfo(cbi.pszText, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
	cbi.iImage=sfi.iIcon;
	m_ctlDstPath.SetItem(&cbi);
}

void CCustomCopyDlg::UpdateComboIcon()
{
	// get text from combo
	COMBOBOXEXITEM cbi;
	TCHAR szPath[_MAX_PATH];
	cbi.mask=CBEIF_TEXT;
	cbi.iItem=m_ctlDstPath.GetCurSel()/*-1*/;
	cbi.pszText=szPath;
	cbi.cchTextMax=_MAX_PATH;

	if (!m_ctlDstPath.GetItem(&cbi))
		return;

	// select no item
	m_ctlDstPath.SetCurSel(-1);

	// icon update
	SHFILEINFO sfi;
	sfi.iIcon=-1;

	cbi.mask |= CBEIF_IMAGE;
	cbi.iItem=-1;

	CString str=(LPCTSTR)szPath;
	if (str.Left(2) != _T("\\\\") || str.Find(_T('\\'), 2) != -1)
		SHGetFileInfo(cbi.pszText, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_SYSICONINDEX);
	
	cbi.iImage=sfi.iIcon;
	m_ctlDstPath.SetItem(&cbi);

	// unselect text in combo's edit
	CEdit* pEdit=m_ctlDstPath.GetEditCtrl();
	if (!pEdit)
		return;

	pEdit->SetSel(-1, -1);
}

void CCustomCopyDlg::OnEditchangeDestpathComboboxex() 
{
	if (m_bActualisation)
		return;
	m_bActualisation=true;
	UpdateComboIcon();
	m_bActualisation=false;
}

void CCustomCopyDlg::OnImportButton() 
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, GetResManager()->LoadString(IDS_FLTALLFILTER_STRING));
	if (dlg.DoModal() == IDOK)
	{
		UINT uiCount=0;
		CString strData;
		try
		{
			CFile file(dlg.GetPathName(), CFile::modeRead);
			CArchive ar(&file, CArchive::load);

			while (ar.ReadString(strData))
			{
				strData.TrimLeft(_T("\" \t"));
				strData.TrimRight(_T("\" \t"));
				AddPath(strData);
				uiCount++;
			}

			ar.Close();
			file.Close();
		}
		catch(CException* e)
		{
			e->Delete();
		}

		// report
		CString strFmt;
		strFmt.Format(GetResManager()->LoadString(IDS_IMPORTREPORT_STRING), uiCount);
		AfxMessageBox(strFmt);
	}
}

void CCustomCopyDlg::OnForcedirectoriesCheck() 
{
	UpdateData(TRUE);

	GetDlgItem(IDC_IGNOREFOLDERS_CHECK)->EnableWindow(!m_bForceDirectories);
}

void CCustomCopyDlg::OnIgnorefoldersCheck()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_FORCEDIRECTORIES_CHECK)->EnableWindow(!m_bIgnoreFolders);
}
