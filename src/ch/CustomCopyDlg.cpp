/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include "stdafx.h"
#include <boost/shared_array.hpp>
#include "resource.h"
#include "CustomCopyDlg.h"
#include "structs.h"
#include "dialogs.h"
#include "BufferSizeDlg.h"
#include "FilterDlg.h"
#include "StringHelpers.h"
#include "ch.h"
#include "CfgProperties.h"
#include "../libchengine/TTaskDefinition.h"
#include "../libchengine/TTaskConfigBufferSizes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCustomCopyDlg dialog


CCustomCopyDlg::CCustomCopyDlg() :
	ictranslate::CLanguageDialog(IDD_CUSTOM_COPY_DIALOG)
{
	GetConfig().ExtractSubConfig(BRANCH_TASK_SETTINGS, m_tTaskDefinition.GetConfiguration());
}

CCustomCopyDlg::CCustomCopyDlg(const chengine::TTaskDefinition& rTaskDefinition) :
	ictranslate::CLanguageDialog(IDD_CUSTOM_COPY_DIALOG),
	m_tTaskDefinition(rTaskDefinition)
{
}

void CCustomCopyDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCustomCopyDlg)
	DDX_Control(pDX, IDC_DESTPATH_COMBOBOXEX, m_ctlDstPath);
	DDX_Control(pDX, IDC_FILTERS_LIST, m_ctlFilters);
	DDX_Control(pDX, IDC_BUFFERSIZES_LIST, m_ctlBufferSizes);
	DDX_Control(pDX, IDC_OPERATION_COMBO, m_ctlOperation);
	DDX_Control(pDX, IDC_PRIORITY_COMBO, m_ctlPriority);
	DDX_Control(pDX, IDC_FILES_LIST, m_ctlFiles);
	DDX_Check(pDX, IDC_ONLYSTRUCTURE_CHECK, m_bOnlyCreate);
	DDX_Check(pDX, IDC_IGNOREFOLDERS_CHECK, m_bIgnoreFolders);
	DDX_Check(pDX, IDC_FORCEDIRECTORIES_CHECK, m_bForceDirectories);
	DDX_Check(pDX, IDC_FILTERS_CHECK, m_bFilters);
	DDX_Check(pDX, IDC_ADVANCED_CHECK, m_bAdvanced);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCustomCopyDlg,ictranslate::CLanguageDialog)
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
	ON_BN_CLICKED(IDC_EXPORT_BUTTON, OnExportButtonClicked)
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCustomCopyDlg message handlers
BOOL CCustomCopyDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	// set dialog icon
	HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	SetIcon(hIcon, FALSE);

	AddResizableControl(IDC_001_STATIC, 0.0, 0.0, 1.0, 0.0);
	AddResizableControl(IDC_FILES_LIST, 0.0, 0.0, 1.0, 0.5);
	AddResizableControl(IDC_ADDFILE_BUTTON, 1.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_ADDDIR_BUTTON, 1.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_REMOVEFILEFOLDER_BUTTON, 1.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_IMPORT_BUTTON, 1.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_002_STATIC, 0.0, 0.5, 1.0, 0.0);
	AddResizableControl(IDC_DESTPATH_COMBOBOXEX, 0.0, 0.5, 1.0, 0.0);
	AddResizableControl(IDC_DESTBROWSE_BUTTON, 1.0, 0.5, 0.0, 0.0);

	AddResizableControl(IDC_BAR1_STATIC, 0.0, 0.5, 0.5, 0.0);
	AddResizableControl(IDC_007_STATIC, 0.5, 0.5, 0.0, 0.0);
	AddResizableControl(IDC_BAR2_STATIC, 0.5, 0.5, 0.5, 0.0);
	
	AddResizableControl(IDC_003_STATIC, 0.0, 0.5, 0.5, 0.0);
	AddResizableControl(IDC_004_STATIC, 0.5, 0.5, 0.5, 0.0);

	AddResizableControl(IDC_OPERATION_COMBO, 0.0, 0.5, 0.5, 0.0);
	AddResizableControl(IDC_PRIORITY_COMBO, 0.5, 0.5, 0.5, 0.0);

	AddResizableControl(IDC_006_STATIC, 0.0, 0.5, 1.0, 0.0);
	AddResizableControl(IDC_BUFFERSIZES_LIST, 0.0, 0.5, 1.0, 0.0);
	AddResizableControl(IDC_BUFFERSIZES_BUTTON, 1.0, 0.5, 0.0, 0.0);

	AddResizableControl(IDC_FILTERS_CHECK, 0.0, 0.5, 0.0, 0.0);
	AddResizableControl(IDC_BAR3_STATIC, 0.0, 0.5, 1.0, 0.0);
	AddResizableControl(IDC_FILTERS_LIST, 0.0, 0.5, 1.0, 0.5);
	AddResizableControl(IDC_ADDFILTER_BUTTON, 1.0, 0.5, 0.0, 0.0);
	AddResizableControl(IDC_REMOVEFILTER_BUTTON, 1.0, 0.5, 0.0, 0.0);

	AddResizableControl(IDC_ADVANCED_CHECK, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_BAR4_STATIC, 0.0, 1.0, 1.0, 0.0);

	AddResizableControl(IDC_IGNOREFOLDERS_CHECK, 0.0, 1.0, 1.0, 0.0);
	AddResizableControl(IDC_ONLYSTRUCTURE_CHECK, 0.0, 1.0, 1.0, 0.0);
	AddResizableControl(IDC_FORCEDIRECTORIES_CHECK, 0.0, 1.0, 1.0, 0.0);

	AddResizableControl(IDC_BAR5_STATIC, 0.0, 1.0, 1.0, 0.0);
	AddResizableControl(IDOK, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDCANCEL, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_HELP_BUTTON, 1.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_EXPORT_BUTTON, 0.0, 1.0, 0.0, 0.0);

	InitializeResizableControls();

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
	m_ctlFilters.SetExtendedStyle(m_ctlFilters.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// paths' listbox - add one column
	LVCOLUMN lvc;
	lvc.mask=LVCF_FMT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;
	lvc.cx=rc.Width();
	m_ctlFiles.InsertColumn(1, &lvc);
	
	// fill paths' listbox
	for(size_t stIndex = 0; stIndex < m_tTaskDefinition.GetSourcePathCount(); stIndex++)
	{
		AddPath(m_tTaskDefinition.GetSourcePathAt(stIndex).ToString());
	}

	// image list for a combo with recent paths
	m_ctlDstPath.SetImageList(&m_ilImages);

	// recent paths addition
	COMBOBOXEXITEM cbi;
	CString strText;
	cbi.mask=CBEIF_IMAGE | CBEIF_TEXT;

	for(size_t stIndex = 0; stIndex < m_vRecent.size(); ++stIndex)
	{
		cbi.iItem = stIndex;
		strText = m_vRecent.at(stIndex);
		cbi.pszText = strText.GetBuffer(1);
		sfi.iIcon = -1;
		SHGetFileInfo(strText, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		cbi.iImage = sfi.iIcon;

		m_ctlDstPath.InsertItem(&cbi);
	}

	// destination path
	m_ctlDstPath.SetPath(m_tTaskDefinition.GetDestinationPath().ToString());

	// operation type
	m_ctlOperation.AddString(GetResManager().LoadString(IDS_CCDCOPY_STRING));
	m_ctlOperation.AddString(GetResManager().LoadString(IDS_CCDMOVE_STRING));

	// copying/moving
	m_ctlOperation.SetCurSel(m_tTaskDefinition.GetOperationType() == chengine::eOperation_Move ? 1 : 0);

	// fill priority combo
	for (int stIndex=0;stIndex<7;stIndex++)
	{
		m_ctlPriority.AddString(GetResManager().LoadString(IDS_PRIORITY0_STRING+stIndex));
	}

	m_ctlPriority.SetCurSel(PriorityToIndex(chengine::GetTaskPropValue<chengine::eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration())));

	// fill buffer sizes listbox
	SetBuffersizesString();

	// list width
	m_ctlFilters.GetWindowRect(&rc);
	rc.right -= GetSystemMetrics(SM_CXEDGE)*2;

	// filter - some columns in a header
	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;

	// mask
	lvc.iSubItem=-1;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDRMASK_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.15*rc.Width());
	m_ctlFilters.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem=0;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.15*rc.Width());
	m_ctlFilters.InsertColumn(2, &lvc);

	// size
	lvc.iSubItem=1;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDRSIZE_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.3*rc.Width());
	m_ctlFilters.InsertColumn(3, &lvc);

	// time
	lvc.iSubItem=2;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDRDATE_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.3*rc.Width());
	m_ctlFilters.InsertColumn(4, &lvc);

	// attributes
	lvc.iSubItem=3;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDRATTRIB_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.1*rc.Width());
	m_ctlFilters.InsertColumn(5, &lvc);

	// -attributes
	lvc.iSubItem=4;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEATTRIB_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.1*rc.Width());
	m_ctlFilters.InsertColumn(6, &lvc);

	m_bFilters = !m_tTaskDefinition.GetFilters().IsEmpty();

	// other custom flags
	m_bIgnoreFolders = chengine::GetTaskPropValue<chengine::eTO_IgnoreDirectories>(m_tTaskDefinition.GetConfiguration());
	m_bForceDirectories = chengine::GetTaskPropValue<chengine::eTO_CreateDirectoriesRelativeToRoot>(m_tTaskDefinition.GetConfiguration());
	m_bOnlyCreate = chengine::GetTaskPropValue<chengine::eTO_CreateEmptyFiles>(m_tTaskDefinition.GetConfiguration());
	m_bAdvanced = (m_bIgnoreFolders | m_bForceDirectories | m_bOnlyCreate);

	UpdateData(FALSE);

	EnableControls();

	return TRUE;
}

void CCustomCopyDlg::OnLanguageChanged()
{
	UpdateData(TRUE);

	// count the width of a list
	UpdateFilesListCtrlHeaderWidth();

	// operation
	int iPos=m_ctlOperation.GetCurSel();
	m_ctlOperation.ResetContent();
	m_ctlOperation.AddString(GetResManager().LoadString(IDS_CCDCOPY_STRING));
	m_ctlOperation.AddString(GetResManager().LoadString(IDS_CCDMOVE_STRING));
	m_ctlOperation.SetCurSel(iPos);

	// priority combo
	iPos=m_ctlPriority.GetCurSel();
	m_ctlPriority.ResetContent();
	for (int i=0;i<7;i++)
	{
		m_ctlPriority.AddString(GetResManager().LoadString(IDS_PRIORITY0_STRING+i));
	}
	m_ctlPriority.SetCurSel(iPos);

	// fill the listbox with buffers
	SetBuffersizesString();

	// filter section (filter, size, date, attributes)
	while(m_ctlFilters.DeleteColumn(0));		// delete all columns

	CRect rc;
	m_ctlFilters.GetWindowRect(&rc);
	rc.right -= GetSystemMetrics(SM_CXEDGE) * 2;

	LVCOLUMN lvc;
	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;

	// mask
	lvc.iSubItem=-1;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDRMASK_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.15*rc.Width());
	m_ctlFilters.InsertColumn(1, &lvc);

	// exclude mask
	lvc.iSubItem=0;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEMASK_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.15*rc.Width());
	m_ctlFilters.InsertColumn(2, &lvc);

	// size
	lvc.iSubItem=1;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDRSIZE_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.3*rc.Width());
	m_ctlFilters.InsertColumn(3, &lvc);

	// time
	lvc.iSubItem=2;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDRDATE_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.3*rc.Width());
	m_ctlFilters.InsertColumn(4, &lvc);

	// attributes
	lvc.iSubItem=3;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDRATTRIB_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.1*rc.Width());
	m_ctlFilters.InsertColumn(5, &lvc);

	// -attributes
	lvc.iSubItem=4;
	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_HDREXCLUDEATTRIB_STRING);
	lvc.cchTextMax=lstrlen(lvc.pszText);
	lvc.cx=static_cast<int>(0.1*rc.Width());
	m_ctlFilters.InsertColumn(6, &lvc);

	// refresh the entries in filters' list
	const chengine::TFileFiltersArray& afFilters = m_tTaskDefinition.GetFilters();
	m_ctlFilters.DeleteAllItems();
	for(size_t stIndex = 0; stIndex < afFilters.GetCount(); ++stIndex)
	{
		const chengine::TFileFilter& rFilter = afFilters.GetAt(stIndex);
		AddFilter(rFilter, boost::numeric_cast<int>(stIndex));
	}
}

void CCustomCopyDlg::OnAddDirectoryButton() 
{
	CString strPath;
	if (BrowseForFolder(GetResManager().LoadString(IDS_BROWSE_STRING), &strPath))
		AddPath(strPath);
}

void CCustomCopyDlg::OnAddFilesButton() 
{
	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_ALLOWMULTISELECT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NODEREFERENCELINKS | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, GetResManager().LoadString(IDS_FILEDLGALLFILTER_STRING), this);
	
	TCHAR *pszBuffer = new TCHAR[65535];
	memset(pszBuffer, 0, 65535*sizeof(TCHAR));
	dlg.m_ofn.lpstrFile=pszBuffer;
	dlg.m_ofn.nMaxFile=65535;

	if(dlg.DoModal() == IDOK)
	{
		pszBuffer[65534] = _T('\0');
		// first element is the path
		CString strPath=pszBuffer;

		size_t stOffset = _tcslen(pszBuffer) + 1;
		
		// get filenames
		if(pszBuffer[stOffset] == _T('\0'))
			AddPath(strPath);
		else
		{
			if(strPath.Right(1) != _T("\\"))
				strPath += _T("\\");
			while(pszBuffer[stOffset] != _T('\0'))
			{
				AddPath(strPath + CString(pszBuffer + stOffset));
				stOffset += _tcslen(pszBuffer + stOffset) + 1;
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
		if (pos == nullptr)
			break;

		iItem=m_ctlFiles.GetNextSelectedItem(pos);
		m_ctlFiles.DeleteItem(iItem);
	}
}

void CCustomCopyDlg::OnBrowseButton() 
{
	CString strPath;
	if (BrowseForFolder(GetResManager().LoadString(IDS_DSTFOLDERBROWSE_STRING), &strPath))
	{
		m_ctlDstPath.SetPath(strPath);
	}
}

void CCustomCopyDlg::OnOK() 
{
	UpdateData(TRUE);

	if(!HasBasicTaskData())
	{
		MsgBox(IDS_MISSINGDATA_STRING);
		return;
	}

	UpdateInternalTaskDefinition();

	CLanguageDialog::OnOK();
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
	ictranslate::CFormat fmt;

	chengine::TBufferSizes bsSizes = GetTaskPropBufferSizes(m_tTaskDefinition.GetConfiguration());

	fmt.SetFormat(GetResManager().LoadString(IDS_BSEDEFAULT_STRING));
	fmt.SetParam(_T("%size"), GetSizeString(bsSizes.GetDefaultSize(), true));
	m_ctlBufferSizes.AddString(fmt.ToString());
	
	if (!bsSizes.IsOnlyDefault())
	{
		fmt.SetFormat(GetResManager().LoadString(IDS_BSEONEDISK_STRING));
		fmt.SetParam(_T("%size"), GetSizeString(bsSizes.GetOneDiskSize(), true));
		m_ctlBufferSizes.AddString(fmt.ToString());
		
		fmt.SetFormat(GetResManager().LoadString(IDS_BSETWODISKS_STRING));
		fmt.SetParam(_T("%size"), GetSizeString(bsSizes.GetTwoDisksSize(), true));
		m_ctlBufferSizes.AddString(fmt.ToString());
		
		fmt.SetFormat(GetResManager().LoadString(IDS_BSECD_STRING));
		fmt.SetParam(_T("%size"), GetSizeString(bsSizes.GetCDSize(), true));
		m_ctlBufferSizes.AddString(fmt.ToString());
		
		fmt.SetFormat(GetResManager().LoadString(IDS_BSELAN_STRING));
		fmt.SetParam(_T("%size"), GetSizeString(bsSizes.GetLANSize(), true));
		m_ctlBufferSizes.AddString(fmt.ToString());
	}
}

void CCustomCopyDlg::OnChangebufferButton()
{
	chengine::TBufferSizes tBufferSizes = GetTaskPropBufferSizes(m_tTaskDefinition.GetConfiguration());

	CBufferSizeDlg dlg(&tBufferSizes);
	if(dlg.DoModal() == IDOK)
	{
		SetTaskPropBufferSizes(m_tTaskDefinition.GetConfiguration(), dlg.GetBufferSizes());
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

	chengine::TFileFiltersArray& afFilters = m_tTaskDefinition.GetFilters();
	for (size_t i = 0; i < afFilters.GetCount(); i++)
	{
		const chengine::TFileFilter& rFilter = afFilters.GetAt(i);
		if(rFilter.GetUseMask())
			dlg.m_astrAddMask.Add(rFilter.GetCombinedMask().c_str());
		if(rFilter.GetUseExcludeMask())
			dlg.m_astrAddExcludeMask.Add(rFilter.GetCombinedExcludeMask().c_str());
	}
	
	if(dlg.DoModal() == IDOK)
	{
		if(dlg.m_ffFilter.GetUseMask() || dlg.m_ffFilter.GetUseExcludeMask() || dlg.m_ffFilter.GetUseSize1() || dlg.m_ffFilter.GetUseDateTime1() || dlg.m_ffFilter.GetUseAttributes())
		{
			afFilters.Add(dlg.m_ffFilter);
			AddFilter(dlg.m_ffFilter);
		}
		else
			MsgBox(IDS_EMPTYFILTER_STRING, MB_OK | MB_ICONINFORMATION);
	}
}

void CCustomCopyDlg::AddFilter(const chengine::TFileFilter &rFilter, int iPos)
{
	LVITEM lvi;
	CString strLoaded;

	lvi.mask=LVIF_TEXT;
	lvi.iItem=(iPos == -1) ? m_ctlFilters.GetItemCount() : iPos;

	/////////////////////
	lvi.iSubItem=0;
	
	if (rFilter.GetUseMask())
	{
		string::TString strData = rFilter.GetCombinedMask();
		strLoaded = strData.c_str();
	}
	else
		strLoaded = GetResManager().LoadString(IDS_FILTERMASKEMPTY_STRING);
	
	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.InsertItem(&lvi);

	/////////////////////
	lvi.iSubItem=1;
	
	if(rFilter.GetUseExcludeMask())
	{
		string::TString strData = rFilter.GetCombinedExcludeMask();
		strLoaded = strData.c_str();
	}
	else
		strLoaded = GetResManager().LoadString(IDS_FILTERMASKEMPTY_STRING);
	
	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);

	/////////////////
	lvi.iSubItem=2;
	
	if (rFilter.GetUseSize1())
	{
		strLoaded.Format(_T("%s %s"), GetResManager().LoadString(IDS_LT_STRING+rFilter.GetSizeType1()), (PCTSTR)GetSizeString(rFilter.GetSize1(), true));
		if (rFilter.GetUseSize2())
		{
			strLoaded += GetResManager().LoadString(IDS_AND_STRING);
			strLoaded.AppendFormat(_T("%s %s"), GetResManager().LoadString(IDS_LT_STRING+rFilter.GetSizeType2()), (PCTSTR)GetSizeString(rFilter.GetSize2(), true));
		}
	}
	else
		strLoaded = GetResManager().LoadString(IDS_FILTERSIZE_STRING);
	
	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);

	///////////////////
	lvi.iSubItem=3;
	
	if (rFilter.GetUseDateTime1())
	{
		strLoaded.Format(_T("%s %s"), GetResManager().LoadString(IDS_DATECREATED_STRING+rFilter.GetDateType()), GetResManager().LoadString(IDS_LT_STRING+rFilter.GetDateCmpType1()));

		string::TString strFmtDateTime = rFilter.GetDateTime1().Format(rFilter.GetUseDate1(), rFilter.GetUseTime1());
		strLoaded += strFmtDateTime.c_str();

		if (rFilter.GetUseDateTime2())
		{
			strLoaded += GetResManager().LoadString(IDS_AND_STRING);
			strLoaded += GetResManager().LoadString(IDS_LT_STRING + rFilter.GetDateCmpType2());

			strFmtDateTime = rFilter.GetDateTime2().Format(rFilter.GetUseDate2(), rFilter.GetUseTime2());
			strLoaded += strFmtDateTime.c_str();
		}
	}
	else
		strLoaded = GetResManager().LoadString(IDS_FILTERDATE_STRING);

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);

	/////////////////////
	lvi.iSubItem=4;
	strLoaded.Empty();
	if(rFilter.GetUseAttributes())
	{
		if(rFilter.GetArchive() == 1)
			strLoaded += L"A";
		if(rFilter.GetReadOnly() == 1)
			strLoaded += _T("R");
		if(rFilter.GetHidden() == 1)
			strLoaded += _T("H");
		if(rFilter.GetSystem() == 1)
			strLoaded += _T("S");
		if(rFilter.GetDirectory() == 1)
			strLoaded += _T("D");
	}

	if (!rFilter.GetUseAttributes() || strLoaded.IsEmpty())
		strLoaded = GetResManager().LoadString(IDS_FILTERATTRIB_STRING);
	
	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);

	/////////////////////
	lvi.iSubItem=5;
	strLoaded.Empty();
	if(rFilter.GetUseAttributes())
	{
		if(rFilter.GetArchive() == 0)
			strLoaded += _T("A");
		if(rFilter.GetReadOnly() == 0)
			strLoaded += _T("R");
		if(rFilter.GetHidden() == 0)
			strLoaded += _T("H");
		if(rFilter.GetSystem() == 0)
			strLoaded += _T("S");
		if(rFilter.GetDirectory() == 0)
			strLoaded += _T("D");
	}

	if(!rFilter.GetUseAttributes() || strLoaded.IsEmpty())
		strLoaded = GetResManager().LoadString(IDS_FILTERATTRIB_STRING);

	lvi.pszText = (PTSTR)(PCTSTR)strLoaded;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlFilters.SetItem(&lvi);
}

void CCustomCopyDlg::OnRemovefilterButton() 
{
	chengine::TFileFiltersArray& afFilters = m_tTaskDefinition.GetFilters();

	POSITION pos;
	int iItem;
	while(true)
	{
		pos=m_ctlFilters.GetFirstSelectedItemPosition();
		if (pos == nullptr)
			break;

		iItem=m_ctlFilters.GetNextSelectedItem(pos);
		m_ctlFilters.DeleteItem(iItem);
		afFilters.RemoveAt(iItem);
	}
}

void CCustomCopyDlg::OnDestroy() 
{
	m_ctlFiles.SetImageList(nullptr, LVSIL_SMALL);
	m_ilImages.Detach();

	CLanguageDialog::OnDestroy();
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
	POSITION pos = m_ctlFilters.GetFirstSelectedItemPosition();
	if(pos != nullptr)
	{
		chengine::TFileFiltersArray& afFilters = m_tTaskDefinition.GetFilters();

		int iItem = m_ctlFilters.GetNextSelectedItem(pos);
		CFilterDlg dlg;
		const chengine::TFileFilter& rFilter = afFilters.GetAt(iItem);
		dlg.m_ffFilter = rFilter;
		
		for(size_t stIndex = 0; stIndex < afFilters.GetCount(); ++stIndex)
		{
			const chengine::TFileFilter& rFilter = afFilters.GetAt(stIndex);
			if(rFilter.GetUseMask() && boost::numeric_cast<int>(stIndex) != iItem)
				dlg.m_astrAddMask.Add(rFilter.GetCombinedMask().c_str());
			if (rFilter.GetUseExcludeMask() && boost::numeric_cast<int>(stIndex) != iItem)
				dlg.m_astrAddExcludeMask.Add(rFilter.GetCombinedExcludeMask().c_str());
		}

		if (dlg.DoModal() == IDOK)
		{
			// delete old element
			m_ctlFilters.DeleteItem(iItem);
			//m_ccData.m_afFilters.RemoveAt(iItem);

			// insert new if needed
			if (dlg.m_ffFilter.GetUseMask() || dlg.m_ffFilter.GetUseExcludeMask() || dlg.m_ffFilter.GetUseSize1()
				|| dlg.m_ffFilter.GetUseDateTime1() || dlg.m_ffFilter.GetUseAttributes())
			{
				afFilters.SetAt(iItem, dlg.m_ffFilter);
				AddFilter(dlg.m_ffFilter, iItem);
			}
		}
	}

	*pResult = 0;
}

void CCustomCopyDlg::OnDblclkBuffersizesList() 
{
	int iItem = m_ctlBufferSizes.GetCurSel();
	if(iItem != LB_ERR)
	{
		chengine::TBufferSizes tBufferSizes = GetTaskPropBufferSizes(m_tTaskDefinition.GetConfiguration());
		CBufferSizeDlg dlg(&tBufferSizes, (chengine::TBufferSizes::EBufferType)iItem);

		if(dlg.DoModal() == IDOK)
		{
			SetTaskPropBufferSizes(m_tTaskDefinition.GetConfiguration(), dlg.GetBufferSizes());
			SetBuffersizesString();
		}
	}
}

void CCustomCopyDlg::UpdateComboIcon()
{
	// get text from combo
	COMBOBOXEXITEM cbi;
	TCHAR szPath[_MAX_PATH];
	memset(szPath, 0, _MAX_PATH);
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
	boost::shared_array<BYTE> spBuffer;

	CFileDialog dlg(TRUE, nullptr, nullptr, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, GetResManager().LoadString(IDS_FLTALLFILTER_STRING));
	if(dlg.DoModal() == IDOK)
	{
		unsigned long ulSize = 0;
		UINT uiCount=0;
		try
		{
			CFile file(dlg.GetPathName(), CFile::modeRead);

			// load files max 1MB in size;
			unsigned long long ullSize = file.GetLength();
			if(ullSize > 1*1024*1024 || ullSize < 2)
			{
				AfxMessageBox(GetResManager().LoadString(IDS_IMPORTERROR_STRING));
				return;
			}

			ulSize = boost::numeric_cast<unsigned long>(ullSize);
			spBuffer.reset(new BYTE[ulSize + 3]);	// guarantee that we have null at the end of the string (3 bytes to compensate for possible odd number of bytes and for unicode)
			memset(spBuffer.get(), 0, ulSize + 3);

			ulSize = file.Read(spBuffer.get(), ulSize);
			file.Close();
		}
		catch(...)
		{
			AfxMessageBox(GetResManager().LoadString(IDS_IMPORTERROR_STRING));
			return;
		}

		// parse text from buffer (there is no point processing files with size < 3 - stIndex.e. "c:")
		if(!spBuffer || ulSize < 3)
		{
			AfxMessageBox(GetResManager().LoadString(IDS_IMPORTERROR_STRING));
			return;
		}

		// which format?
		CString strData;
		if(spBuffer[0] == 0xff && spBuffer[1] == 0xfe)
		{
			// utf-16 (native)
			strData = (wchar_t*)(spBuffer.get() + 2);
			
		}
		else if(ulSize >= 3 && spBuffer[0] == 0xef && spBuffer[1] == 0xbb && spBuffer[2] == 0xbf)
		{
			boost::shared_array<wchar_t> spWideBuffer(new wchar_t[ulSize + 1]);
			memset(spWideBuffer.get(), 0, (ulSize + 1) * sizeof(wchar_t));

			// utf-8 - needs conversion
			int iRes = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, (char*)(spBuffer.get() + 3), ulSize - 3, spWideBuffer.get(), ulSize);
			if(iRes == 0)
				return;		// failed to convert

			spWideBuffer[iRes] = L'\0';
			strData = spWideBuffer.get();
		}
		else
		{
			// assuming ansi
			strData = (char*)spBuffer.get();
		}

		CString strToken;
		int iPos = 0;
		strToken = strData.Tokenize(_T("\n"), iPos);
		while(strToken != _T(""))
		{
			strToken.TrimLeft(_T("\" \t\r\n"));
			strToken.TrimRight(_T("\" \t\r\n"));

			AddPath(strToken);
			uiCount++;

			strToken = strData.Tokenize(_T("\n"), iPos);
		}

		// report
		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_IMPORTREPORT_STRING));
		fmt.SetParam(_T("%count"), uiCount);
		AfxMessageBox(fmt.ToString());
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

void CCustomCopyDlg::OnExportButtonClicked()
{
	UpdateData(TRUE);

	if (!HasBasicTaskData())
	{
		MsgBox(IDS_MISSINGDATA_STRING);
		return;
	}

	UpdateInternalTaskDefinition();

	CFileDialog dlg(FALSE, _T("xml"), _T("Task"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, GetResManager().LoadString(IDS_FLTALLFILTER_STRING));
	if (dlg.DoModal() == IDOK)
	{
		CString strError;
		try
		{
			m_tTaskDefinition.Store(chcore::PathFromString(dlg.GetPathName()));
		}
		catch (const std::exception& e)
		{
			strError = e.what();
		}

		if (!strError.IsEmpty())
		{
			ictranslate::CFormat fmt;
			fmt.SetFormat(GetResManager().LoadString(IDS_EXPORTING_TASK_FAILED));
			fmt.SetParam(_T("%reason"), strError);

			AfxMessageBox(fmt.ToString(), MB_OK | MB_ICONERROR);
		}
	}
}

void CCustomCopyDlg::UpdateInternalTaskDefinition()
{
	CString strDstPath;
	m_ctlDstPath.GetWindowText(strDstPath);

	// copy files from listctrl to an array
	m_tTaskDefinition.ClearSourcePaths();

	// dest path
	m_tTaskDefinition.SetDestinationPath(chcore::PathFromString(strDstPath));

	for (int i = 0; i < m_ctlFiles.GetItemCount(); i++)
	{
		m_tTaskDefinition.AddSourcePath(chcore::PathFromString(m_ctlFiles.GetItemText(i, 0)));
	}

	// operation type
	m_tTaskDefinition.SetOperationType(m_ctlOperation.GetCurSel() == 0 ? chengine::eOperation_Copy : chengine::eOperation_Move);

	// priority
	chengine::SetTaskPropValue<chengine::eTO_ThreadPriority>(m_tTaskDefinition.GetConfiguration(), IndexToPriority(m_ctlPriority.GetCurSel()));

	chengine::SetTaskPropValue<chengine::eTO_IgnoreDirectories>(m_tTaskDefinition.GetConfiguration(), (m_bIgnoreFolders != 0));
	chengine::SetTaskPropValue<chengine::eTO_CreateDirectoriesRelativeToRoot>(m_tTaskDefinition.GetConfiguration(), (m_bForceDirectories != 0));
	chengine::SetTaskPropValue<chengine::eTO_CreateEmptyFiles>(m_tTaskDefinition.GetConfiguration(), (m_bOnlyCreate != 0));
}

bool CCustomCopyDlg::HasBasicTaskData()
{
	CString strDstPath;
	m_ctlDstPath.GetWindowText(strDstPath);

	if (strDstPath.IsEmpty() || m_ctlFiles.GetItemCount() == 0)
		return false;

	return true;
}

void CCustomCopyDlg::UpdateFilesListCtrlHeaderWidth()
{
	CRect rc;
	m_ctlFiles.GetWindowRect(&rc);
	rc.right -= GetSystemMetrics(SM_CXEDGE) * 2;

	// change the width of a column
	LVCOLUMN lvc;
	lvc.mask = LVCF_WIDTH;
	lvc.cx = rc.Width();
	m_ctlFiles.SetColumn(0, &lvc);
}

void CCustomCopyDlg::OnSize(UINT nType, int /*cx*/, int /*cy*/)
{
	if(nType == SIZE_RESTORED || nType == SIZE_MAXIMIZED)
	{
		CWnd* pWnd = GetDlgItem(IDC_FILES_LIST);
		if(pWnd)
			UpdateFilesListCtrlHeaderWidth();
	}
}
