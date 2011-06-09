/***************************************************************************
*   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
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
#include "ch.h"
#include "resource.h"
#include "StatusDlg.h"
#include "BufferSizeDlg.h"
#include "StringHelpers.h"
#include "StaticEx.h"
#include "Structs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool CStatusDlg::m_bLock=false;

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog

CStatusDlg::CStatusDlg(CTaskArray* pTasks, CWnd* pParent /*=NULL*/)
	: ictranslate::CLanguageDialog(CStatusDlg::IDD, pParent, &m_bLock)
{
	//{{AFX_DATA_INIT(CStatusDlg)
	//}}AFX_DATA_INIT
	m_i64LastProcessed=0;
	m_i64LastAllTasksProcessed=0;
	m_pTasks=pTasks;
	m_dwLastUpdate=0;

	RegisterStaticExControl(AfxGetInstanceHandle());
}

CStatusDlg::~CStatusDlg()
{

}

void CStatusDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStatusDlg)
	DDX_Control(pDX, IDC_TASK_PROGRESS, m_ctlCurrentProgress);
	DDX_Control(pDX, IDC_STATUS_LIST, m_ctlStatusList);
	DDX_Control(pDX, IDC_ALL_PROGRESS, m_ctlProgressAll);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CStatusDlg,ictranslate::CLanguageDialog)
	//{{AFX_MSG_MAP(CStatusDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, OnPauseButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnCancelButton)
	ON_BN_CLICKED(IDC_ROLL_UNROLL_BUTTON, OnRollUnrollButton)
	ON_BN_CLICKED(IDC_SET_PRIORITY_BUTTON, OnSetPriorityButton)
	ON_BN_CLICKED(IDC_SET_BUFFERSIZE_BUTTON, OnSetBuffersizeButton)
	ON_BN_CLICKED(IDC_START_ALL_BUTTON, OnStartAllButton)
	ON_BN_CLICKED(IDC_RESTART_BUTTON, OnRestartButton)
	ON_BN_CLICKED(IDC_DELETE_BUTTON, OnDeleteButton)
	ON_BN_CLICKED(IDC_PAUSE_ALL_BUTTON, OnPauseAllButton)
	ON_BN_CLICKED(IDC_RESTART_ALL_BUTTON, OnRestartAllButton)
	ON_BN_CLICKED(IDC_CANCEL_ALL_BUTTON, OnCancelAllButton)
	ON_BN_CLICKED(IDC_REMOVE_FINISHED_BUTTON, OnRemoveFinishedButton)
	ON_NOTIFY(LVN_KEYDOWN, IDC_STATUS_LIST, OnKeydownStatusList)
	ON_NOTIFY(LVN_CHANGEDSELECTION, IDC_STATUS_LIST, OnSelectionChanged)
	ON_BN_CLICKED(IDC_SHOW_LOG_BUTTON, OnShowLogButton)
	ON_BN_CLICKED(IDC_STICK_BUTTON, OnStickButton)
	ON_BN_CLICKED(IDC_RESUME_BUTTON, OnResumeButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg message handlers

BOOL CStatusDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	PrepareResizableControls();

	// get size of list ctrl
	CRect rcList;
	m_ctlStatusList.GetWindowRect(&rcList);
	int iWidth=rcList.Width();

	// set additional styles
	m_ctlStatusList.SetExtendedStyle(m_ctlStatusList.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	// add columns
	LVCOLUMN lvc;
	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNSTATUS_STRING); /*_T("Status")*/;
	lvc.cchTextMax = lstrlen(lvc.pszText); 
	lvc.cx = static_cast<int>(0.27*iWidth);
	lvc.iSubItem=-1;
	m_ctlStatusList.InsertColumn(1, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNSOURCE_STRING);/*_T("File");*/
	lvc.cchTextMax = lstrlen(lvc.pszText); 
	lvc.cx = static_cast<int>(0.3*iWidth);
	lvc.iSubItem=0;
	m_ctlStatusList.InsertColumn(2, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNDESTINATION_STRING);/*_T("To:");*/
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.27*iWidth);
	lvc.iSubItem=1;
	m_ctlStatusList.InsertColumn(3, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNPROGRESS_STRING);/*_T("Progress");*/
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15*iWidth);
	lvc.iSubItem=2;
	m_ctlStatusList.InsertColumn(4, &lvc);

	// images
	m_images.Create(16, 16, ILC_COLOR16 | ILC_MASK, 0, 3);
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_WORKING_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_ERROR_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_PAUSED_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_FINISHED_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_CANCELLED_ICON)));
	m_images.Add(AfxGetApp()->LoadIcon(MAKEINTRESOURCE(IDI_WAITING_ICON)));

	m_ctlStatusList.SetImageList(&m_images, LVSIL_SMALL);

	// set fixed progresses ranges
	m_ctlCurrentProgress.SetRange32(0, 100);
	m_ctlProgressAll.SetRange32(0, 100);

	// change the size of a dialog
	ApplyDisplayDetails(true);
//	ApplyButtonsState();
//	EnableControls(false);

	// refresh data
	RefreshStatus();

	// select needed element
	size_t stIndex = 0;
	while(stIndex < m_pTasks->GetSize())
	{
		if(m_pTasks->GetAt(stIndex) == m_spInitialSelection)
		{
			m_ctlStatusList.SetItemState(boost::numeric_cast<int>(stIndex), LVIS_SELECTED, LVIS_SELECTED);
			break;
		}

		stIndex++;
	}

	// refresh data timer
	SetTimer(777, GetPropValue<PP_STATUSREFRESHINTERVAL>(GetConfig()), NULL);

	return TRUE;
}

void CStatusDlg::EnableControls(bool bEnable)
{
	// enable/disable controls
	GetDlgItem(IDC_SET_BUFFERSIZE_BUTTON)->EnableWindow(bEnable);
	GetDlgItem(IDC_SET_PRIORITY_BUTTON)->EnableWindow(bEnable);

	if (!bEnable)
	{
		// get rid of text id disabling
		GetDlgItem(IDC_OPERATION_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYOPERATIONTEXT_STRING));
		GetDlgItem(IDC_SOURCE_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYSOURCETEXT_STRING));
		GetDlgItem(IDC_DESTINATION_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYDESTINATIONTEXT_STRING));
		GetDlgItem(IDC_BUFFERSIZE_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYBUFFERSIZETEXT_STRING));
		GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPRIORITYTEXT_STRING));
		
		GetDlgItem(IDC_PROGRESS_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPROCESSEDTEXT_STRING));
		GetDlgItem(IDC_TRANSFER_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTRANSFERTEXT_STRING));
		GetDlgItem(IDC_TIME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTIMETEXT_STRING));
		GetDlgItem(IDC_ASSOCIATEDFILES__STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYASSOCFILE_STRING));

		m_ctlCurrentProgress.SetPos(0);
	}
}

void CStatusDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == 777)	// refreshing data
	{
		// turn off timer for some time
		KillTimer(777);

		RefreshStatus();

		// reenable
		SetTimer(777, GetPropValue<PP_STATUSREFRESHINTERVAL>(GetConfig()), NULL);
	}

	CLanguageDialog::OnTimer(nIDEvent);
}

void CStatusDlg::AddTaskInfo(int nPos, const CTaskPtr& spTask, DWORD dwCurrentTime)
{
	_ASSERTE(spTask != NULL);
	if(spTask == NULL)
		return;

	// index to string
	_itot(nPos, m_szData, 10);

	// get data snapshot from task
	TASK_DISPLAY_DATA td;
	spTask->GetSnapshot(&td);

	// index subitem
	CString strStatusText = GetStatusString(td);
	CString strTemp;
	LVITEM lvi;
	lvi.mask=LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem=nPos;
	lvi.iSubItem=0;
	lvi.pszText = (PTSTR)(PCTSTR)strStatusText;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	lvi.lParam = spTask->GetSessionUniqueID();
	lvi.iImage=GetImageFromStatus(td.m_eTaskState);
	if (nPos < m_ctlStatusList.GetItemCount())
		m_ctlStatusList.SetItem(&lvi);
	else
		m_ctlStatusList.InsertItem(&lvi);

	// status subitem
	lvi.mask=LVIF_TEXT;
	lvi.iSubItem=1;
	if(td.m_strFileName.IsEmpty())
		strTemp = GetResManager().LoadString(IDS_NONEINPUTFILE_STRING);
	else
		strTemp = td.m_strFileName;
	lvi.pszText=strTemp.GetBuffer(0);
	strTemp.ReleaseBuffer();
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);

	// insert 'file' subitem
	lvi.iSubItem=2;
	strTemp = td.m_pathDstPath.ToString();
	lvi.pszText=strTemp.GetBuffer(0);
	strTemp.ReleaseBuffer();
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);

	// insert dest subitem
	lvi.iSubItem=3;
	_itot( td.m_nPercent, m_szData, 10 );
	_tcscat(m_szData, _T(" %"));
	lvi.pszText=m_szData;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);

	// right side update
	if(spTask == m_spSelectedItem && GetPropValue<PP_STATUSSHOWDETAILS>(GetConfig()))
	{
		// data that can be changed by a thread
		GetDlgItem(IDC_OPERATION_STATIC)->SetWindowText(strStatusText);	// operation

		if(td.m_strFullFilePath.IsEmpty())
			GetDlgItem(IDC_SOURCE_STATIC)->SetWindowText(GetResManager().LoadString(IDS_NONEINPUTFILE_STRING));
		else
			GetDlgItem(IDC_SOURCE_STATIC)->SetWindowText(td.m_strFullFilePath);	// src object
		
		// count of processed data/overall count of data
		_sntprintf(m_szData, _MAX_PATH, _T("%d/%d ("), td.m_stIndex, td.m_stSize);
		strTemp=CString(m_szData);
		strTemp+=GetSizeString(td.m_ullProcessedSize, m_szData, _MAX_PATH)+CString(_T("/"));
		strTemp+=GetSizeString(td.m_ullSizeAll, m_szData, _MAX_PATH)+CString(_T(")"));
		GetDlgItem(IDC_PROGRESS_STATIC)->SetWindowText(strTemp);
		
		// transfer
		if (m_i64LastProcessed == 0)	// if first time - show average
			strTemp=GetSizeString( td.m_timeElapsed ? td.m_ullProcessedSize/td.m_timeElapsed : 0, m_szData, _MAX_PATH);	// last avg
		else
			if ( (dwCurrentTime-m_dwLastUpdate) != 0)
				strTemp=GetSizeString( (static_cast<double>(td.m_ullProcessedSize) - static_cast<double>(m_i64LastProcessed))/(static_cast<double>(dwCurrentTime-m_dwLastUpdate)/1000.0), m_szData, _MAX_PATH);
			else
				strTemp=GetSizeString( 0ULL, m_szData, _MAX_PATH);

		// avg transfer
		GetDlgItem(IDC_TRANSFER_STATIC)->SetWindowText(strTemp+_T("/s (")+CString(GetResManager().LoadString(IDS_AVERAGEWORD_STRING))
			+CString(GetSizeString(td.m_timeElapsed ? td.m_ullProcessedSize/td.m_timeElapsed : 0, m_szData, _MAX_PATH))+_T("/s )")
			);
		
		// elapsed time / estimated total time (estimated time left)
		FormatTime(td.m_timeElapsed, m_szTimeBuffer1, 40);
		time_t timeTotal = (td.m_ullProcessedSize == 0) ? 0 : (long)(td.m_ullSizeAll * td.m_timeElapsed / td.m_ullProcessedSize);
		FormatTime(timeTotal, m_szTimeBuffer2, 40);
		FormatTime(std::max((time_t)0l, timeTotal - td.m_timeElapsed), m_szTimeBuffer3, 40);

		_sntprintf(m_szData, _MAX_PATH, _T("%s / %s (%s)"), m_szTimeBuffer1, m_szTimeBuffer2, m_szTimeBuffer3);
		GetDlgItem(IDC_TIME_STATIC)->SetWindowText(m_szData);

		// remember current processed data (used for calculating transfer)
		m_i64LastProcessed=td.m_ullProcessedSize;

		// set progress
		m_ctlCurrentProgress.SetPos(td.m_nPercent);

		SetBufferSizesString(td.m_iCurrentBufferSize, td.m_iCurrentBufferIndex);

		// data that can be changed only by user from outside the thread
		// refresh only when there are new selected item
//		if (spTask != m_spLastSelected)
		{
			GetDlgItem(IDC_DESTINATION_STATIC)->SetWindowText(td.m_pathDstPath.ToString());
			GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(td.m_nPriority)));
			GetDlgItem(IDC_ASSOCIATEDFILES__STATIC)->SetWindowText(td.m_strUniqueName);
		}

		// refresh m_spLastSelected
		m_spLastSelected = spTask;
	}
}

void CStatusDlg::OnSetBuffersizeButton()
{
	CTaskPtr spTask = GetSelectedItemPointer();
	if(!spTask)
		return;

	CBufferSizeDlg dlg;
	spTask->GetBufferSizes(dlg.m_bsSizes);
	dlg.m_iActiveIndex = spTask->GetCurrentBufferIndex();
	if(dlg.DoModal() == IDOK)
		spTask->SetBufferSizes(dlg.m_bsSizes);
}

CTaskPtr CStatusDlg::GetSelectedItemPointer()
{
	// returns ptr to a CTask for a given element in listview
	if(m_ctlStatusList.GetSelectedCount() == 1)
	{
		POSITION pos = m_ctlStatusList.GetFirstSelectedItemPosition();
		int nPos = m_ctlStatusList.GetNextSelectedItem(pos);
		return m_pTasks->GetTaskBySessionUniqueID(m_ctlStatusList.GetItemData(nPos));
	}

	return CTaskPtr();
}

void CStatusDlg::OnRollUnrollButton() 
{
	// change settings in config dialog
	SetPropValue<PP_STATUSSHOWDETAILS>(GetConfig(), !GetPropValue<PP_STATUSSHOWDETAILS>(GetConfig()));

	ApplyDisplayDetails();
}

void CStatusDlg::ApplyDisplayDetails(bool bInitial)
{
	// get coord of screen and window
	CRect rcScreen, rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
	GetWindowRect(&rect);

	bool bDetails=GetPropValue<PP_STATUSSHOWDETAILS>(GetConfig());

	// stick cause
	if (rect.right == rcScreen.right && rect.bottom == rcScreen.bottom)
		bInitial=true;

	GetDlgItem(IDC_ROLL_UNROLL_BUTTON)->SetWindowText(bDetails ? _T("<<") : _T(">>"));
	
	CRect list, progress;
	m_ctlProgressAll.GetWindowRect(&progress);
	ScreenToClient(&progress);
	m_ctlStatusList.GetWindowRect(&list);
	ScreenToClient(&list);

	// set dialog size
	CRect destRect;
	if (!bInitial)
	{
		destRect.left=0;
		destRect.top=0;
		destRect.right=bDetails ? progress.right+list.left+3*GetSystemMetrics(SM_CXBORDER) : list.right+list.left+3*GetSystemMetrics(SM_CXBORDER);
		destRect.bottom=rect.Height();
		SetWindowPos(NULL, destRect.left, destRect.top, destRect.right, destRect.bottom, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);
	}
	else
	{
		SetWindowPos(NULL, rcScreen.right-(bDetails ? progress.right+list.left+3*GetSystemMetrics(SM_CXBORDER) : list.right+list.left+3*GetSystemMetrics(SM_CXBORDER)),
			rcScreen.bottom-rect.Height(), (bDetails ? progress.right+list.left+3*GetSystemMetrics(SM_CXBORDER) : list.right+list.left+3*GetSystemMetrics(SM_CXBORDER)),
			rect.Height(), SWP_NOOWNERZORDER | SWP_NOZORDER);
	}
}

void CStatusDlg::ApplyButtonsState()
{
	// remember ptr to CTask
	m_spSelectedItem=GetSelectedItemPointer();

	// set status of buttons pause/resume/cancel
	if (m_spSelectedItem != NULL)
	{
		GetDlgItem(IDC_RESTART_BUTTON)->EnableWindow(true);
		GetDlgItem(IDC_SHOW_LOG_BUTTON)->EnableWindow(true);
		GetDlgItem(IDC_DELETE_BUTTON)->EnableWindow(true);
		
		if (m_spSelectedItem->GetTaskState() == eTaskState_Finished || m_spSelectedItem->GetTaskState() == eTaskState_Cancelled)
		{
			GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
		}	
		else
		{
			// pause/resume
			if (m_spSelectedItem->GetTaskState() == eTaskState_Paused)
			{
				GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
				GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(true);
			}
			else
			{
				GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(true);
				if (m_spSelectedItem->GetTaskState() == eTaskState_Waiting)
					GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(true);
				else
					GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
			}
			
			GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(true);
		}
	}
	else
	{
		GetDlgItem(IDC_SHOW_LOG_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_RESTART_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(false);
		GetDlgItem(IDC_DELETE_BUTTON)->EnableWindow(false);
	}
}

void CStatusDlg::OnSetPriorityButton() 
{
	CMenu menu;
	HMENU hMenu=GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_PRIORITY_MENU));
	if (!menu.Attach(hMenu))
	{
		DestroyMenu(hMenu);
		return;
	}
	
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	if(pPopup)
	{
		// set point in which to set menu
		CRect rect;
		GetDlgItem(IDC_SET_PRIORITY_BUTTON)->GetWindowRect(&rect);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.right+1, rect.top, this);
	}
}

BOOL CStatusDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD(wParam) == 0)
	{
		if (LOWORD(wParam) >= ID_POPUP_TIME_CRITICAL && LOWORD(wParam) <= ID_POPUP_IDLE)
		{
			// processing priority
			if ( (m_spSelectedItem=GetSelectedItemPointer()) == NULL )
				return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);
			
			switch (LOWORD(wParam))
			{
			case ID_POPUP_TIME_CRITICAL:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_TIME_CRITICAL);
				GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_TIME_CRITICAL)));
				break;
			case ID_POPUP_HIGHEST:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_HIGHEST);
				GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_HIGHEST)));
				break;
			case ID_POPUP_ABOVE_NORMAL:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
				GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_ABOVE_NORMAL)));
				break;
			case ID_POPUP_NORMAL:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_NORMAL);
				GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_NORMAL)));
				break;
			case ID_POPUP_BELOW_NORMAL:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
				GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_BELOW_NORMAL)));
				break;
			case ID_POPUP_LOWEST:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_LOWEST);
				GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_LOWEST)));
				break;
			case ID_POPUP_IDLE:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_IDLE);
				GetDlgItem(IDC_PRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_IDLE)));
				break;
			}
		}
	}
	return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);
}

void CStatusDlg::OnPauseButton() 
{
	CTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		TRACE("PauseProcessing call...\n");
		spTask->PauseProcessing();

		RefreshStatus();
	}
}

void CStatusDlg::OnResumeButton() 
{
	CTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		if(spTask->GetTaskState() == eTaskState_Waiting)
			spTask->SetForceFlag();
		else
			spTask->ResumeProcessing();

		RefreshStatus();
	}
}

void CStatusDlg::OnCancelButton() 
{
	CTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		spTask->CancelProcessing();
		RefreshStatus();
	}
}

void CStatusDlg::OnRestartButton() 
{
	CTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		spTask->RestartProcessing();
		RefreshStatus();
	}
}

void CStatusDlg::OnDeleteButton() 
{
	CTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		ETaskCurrentState eTaskState = spTask->GetTaskState();
		if(eTaskState != eTaskState_Finished && eTaskState != eTaskState_Cancelled)
		{
			// ask if cancel
			if(MsgBox(IDS_CONFIRMCANCEL_STRING, MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
			{
				// cancel
				spTask->CancelProcessing();
			}
			else
				return;
		}

		m_pTasks->RemoveFinished(spTask);
		RefreshStatus();
	}
}

void CStatusDlg::OnPauseAllButton() 
{
	TRACE("Pause All...\n");
	m_pTasks->TasksPauseProcessing();
	RefreshStatus();
}

void CStatusDlg::OnStartAllButton() 
{
	TRACE("Resume Processing...\n");
	m_pTasks->TasksResumeProcessing();
	RefreshStatus();
}

void CStatusDlg::OnRestartAllButton() 
{
	TRACE("Restart Processing...\n");
	m_pTasks->TasksRestartProcessing();	
	RefreshStatus();
}

void CStatusDlg::OnCancelAllButton() 
{
	TRACE("Cancel Processing...\n");
	m_pTasks->TasksCancelProcessing();	
	RefreshStatus();
}

void CStatusDlg::OnRemoveFinishedButton() 
{
	m_pTasks->RemoveAllFinished();
	RefreshStatus();
}

void CStatusDlg::OnKeydownStatusList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	switch (pLVKeyDow->wVKey)
	{
	case VK_DELETE:
		OnDeleteButton();
		break;
	case VK_SPACE:
		{
			CTaskPtr spTask = GetSelectedItemPointer();
			if (!spTask)
				return;
		
			if(spTask->GetTaskState() == eTaskState_Paused)
				OnResumeButton();
			else
				OnPauseButton();
			break;
		}
	}

	*pResult = 0;
}

int CStatusDlg::GetImageFromStatus(ETaskCurrentState eState)
{
	switch(eState)
	{
	case eTaskState_Cancelled:
		return 4;
	case eTaskState_Finished:
		return 3;
	case eTaskState_Waiting:
		return 5;
	case eTaskState_Paused:
		return 2;
	case eTaskState_Error:
		return 1;
	default:
		return 0;
	}
}

LPTSTR CStatusDlg::FormatTime(time_t timeSeconds, LPTSTR lpszBuffer, size_t stMaxBufferSize)
{
	long lDays = boost::numeric_cast<long>(timeSeconds/86400);
	timeSeconds %= 86400;
	long lHours = boost::numeric_cast<long>(timeSeconds/3600);
	timeSeconds %= 3600;
	long lMinutes = boost::numeric_cast<long>(timeSeconds/60);
	timeSeconds %= 60;

	if(lDays != 0)
		_sntprintf(lpszBuffer, stMaxBufferSize, _T("%02d:%02d:%02d:%02d"), lDays, lHours, lMinutes, timeSeconds);
	else
	{
		if (lHours != 0)
			_sntprintf(lpszBuffer, stMaxBufferSize, _T("%02d:%02d:%02d"), lHours, lMinutes, timeSeconds);
		else
			_sntprintf(lpszBuffer, stMaxBufferSize, _T("%02d:%02d"), lMinutes, timeSeconds);
	}

	return lpszBuffer;
}

void CStatusDlg::RefreshStatus()
{
	// remember address of a current selection
	m_spSelectedItem=GetSelectedItemPointer();

	// current time
	DWORD dwCurrentTime=GetTickCount();

	// get rid of item after the current part
	m_ctlStatusList.LimitItems(boost::numeric_cast<int>(m_pTasks->GetSize()));

	// add task info
	for(size_t stIndex = 0; stIndex < m_pTasks->GetSize(); ++stIndex)
	{
		AddTaskInfo(boost::numeric_cast<int>(stIndex), m_pTasks->GetAt(stIndex), dwCurrentTime);
	}

	// percent
	int nPercent=m_pTasks->GetPercent();

	// set title
	if (m_pTasks->GetSize() != 0)
		_sntprintf(m_szData, _MAX_PATH, _T("%s [%d %%]"), GetResManager().LoadString(IDS_STATUSTITLE_STRING), m_pTasks->GetPercent());
	else
		_sntprintf(m_szData, _MAX_PATH, _T("%s"), GetResManager().LoadString(IDS_STATUSTITLE_STRING));
	
	// if changed
	CString strTemp;
	GetWindowText(strTemp);
	if (strTemp != CString(m_szData)) 
		SetWindowText(m_szData);
	
	// refresh overall progress
	if (GetPropValue<PP_STATUSSHOWDETAILS>(GetConfig()))
	{
		m_ctlProgressAll.SetPos(nPercent);
		
		// progress - count of processed data/count of data
		strTemp=GetSizeString(m_pTasks->GetPosition(), m_szData, _MAX_PATH)+CString(_T("/"));
		strTemp+=GetSizeString(m_pTasks->GetRange(), m_szData, _MAX_PATH);
		GetDlgItem(IDC_OVERALL_PROGRESS_STATIC)->SetWindowText(strTemp);
		
		// transfer
		if (m_i64LastAllTasksProcessed == 0)
			m_i64LastAllTasksProcessed=m_pTasks->GetPosition();
		
		if (dwCurrentTime-m_dwLastUpdate != 0)
			strTemp=GetSizeString( (static_cast<double>(m_pTasks->GetPosition()) - static_cast<double>(m_i64LastAllTasksProcessed))/static_cast<double>(static_cast<double>(dwCurrentTime-m_dwLastUpdate)/1000.0), m_szData, _MAX_PATH);
		else
			strTemp=GetSizeString( 0ULL, m_szData, _MAX_PATH);
		
		GetDlgItem(IDC_OVERALL_TRANSFER_STATIC)->SetWindowText(strTemp+_T("/s"));
		m_i64LastAllTasksProcessed=m_pTasks->GetPosition();
		m_dwLastUpdate=dwCurrentTime;
	}

	// if selection's missing - hide controls
	if (m_ctlStatusList.GetSelectedCount() == 0)
	{
		EnableControls(false);
		m_spLastSelected.reset();
		m_i64LastProcessed=0;
	}
	else
		EnableControls();		// enable controls
	
	// apply state of the resume, cancel, ... buttons
	ApplyButtonsState();
}

void CStatusDlg::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	TRACE("Received LVN_CHANGEDSELECTION\n");
	RefreshStatus();
}

void CStatusDlg::OnCancel() 
{
	PostCloseMessage();
	CLanguageDialog::OnCancel();
}

void CStatusDlg::OnShowLogButton() 
{
	// show log
	CTaskPtr spTask = GetSelectedItemPointer();
	if (!spTask)
		return;

	unsigned long lResult = (unsigned long)(ShellExecute(this->m_hWnd, _T("open"), _T("notepad.exe"), spTask->GetRelatedPath(CTask::ePathType_TaskLogFile).ToString(), NULL, SW_SHOWNORMAL));
	if(lResult < 32)
	{
		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_SHELLEXECUTEERROR_STRING));
		fmt.SetParam(_t("%errno"), lResult);
		fmt.SetParam(_t("%path"), spTask->GetRelatedPath(CTask::ePathType_TaskLogFile).ToString());
		AfxMessageBox(fmt);
	}
}

LRESULT CStatusDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_UPDATESTATUS)
	{
		TRACE("Received WM_UPDATESTATUS\n");
		RefreshStatus();
	}
	return ictranslate::CLanguageDialog::WindowProc(message, wParam, lParam);
}

void CStatusDlg::OnStickButton() 
{
	ApplyDisplayDetails(true);
}

void CStatusDlg::SetBufferSizesString(UINT uiValue, int iIndex)
{
	TCHAR szData[1024];
	switch(iIndex)
	{
	case BI_DEFAULT:
		GetResManager().LoadStringCopy(IDS_BSDEFAULT_STRING, szData, 256);
		break;
	case BI_ONEDISK:
		GetResManager().LoadStringCopy(IDS_BSONEDISK_STRING, szData, 256);
		break;
	case BI_TWODISKS:
		GetResManager().LoadStringCopy(IDS_BSTWODISKS_STRING, szData, 256);
		break;
	case BI_CD:
		GetResManager().LoadStringCopy(IDS_BSCD_STRING, szData, 256);
		break;
	case BI_LAN:
		GetResManager().LoadStringCopy(IDS_BSLAN_STRING, szData, 256);
		break;
	default:
		_ASSERTE(false);
		szData[0] = _T('\0');
	}

	_tcscat(szData, GetSizeString((ull_t)uiValue, m_szData, _MAX_PATH));

	GetDlgItem(IDC_BUFFERSIZE_STATIC)->SetWindowText(szData);
}

void CStatusDlg::PostCloseMessage()
{
	GetParent()->PostMessage(WM_STATUSCLOSING);
}

void CStatusDlg::OnLanguageChanged()
{
	// remove all columns
	int iCnt=m_ctlStatusList.GetHeaderCtrl()->GetItemCount();

	// Delete all of the columns.
	for (int i=0;i<iCnt;i++)
		m_ctlStatusList.DeleteColumn(0);

	// get size of list ctrl
	CRect rcList;
	m_ctlStatusList.GetWindowRect(&rcList);
	int iWidth=rcList.Width();

	// refresh the header in a list
	LVCOLUMN lvc;
	lvc.mask=LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvc.fmt=LVCFMT_LEFT;

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNSTATUS_STRING); /*_T("Status")*/;
	lvc.cchTextMax = lstrlen(lvc.pszText); 
	lvc.cx = static_cast<int>(0.27*iWidth);
	lvc.iSubItem=-1;
	m_ctlStatusList.InsertColumn(1, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNSOURCE_STRING);/*_T("File");*/
	lvc.cchTextMax = lstrlen(lvc.pszText); 
	lvc.cx = static_cast<int>(0.3*iWidth);
	lvc.iSubItem=0;
	m_ctlStatusList.InsertColumn(2, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNDESTINATION_STRING);/*_T("To:");*/
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.27*iWidth);
	lvc.iSubItem=1;
	m_ctlStatusList.InsertColumn(3, &lvc);

	lvc.pszText=(PTSTR)GetResManager().LoadString(IDS_COLUMNPROGRESS_STRING);/*_T("Progress");*/
	lvc.cchTextMax = lstrlen(lvc.pszText);
	lvc.cx = static_cast<int>(0.15*iWidth);
	lvc.iSubItem=2;
	m_ctlStatusList.InsertColumn(4, &lvc);

	RefreshStatus();
}

// ============================================================================
/// CStatusDlg::PrepareResizableControls
/// @date 2009/04/18
///
/// @brief     Prepares the resizable controls.
// ============================================================================
void CStatusDlg::PrepareResizableControls()
{
	ClearResizableControls();

	AddResizableControl(IDC_001_STATIC, 0, 0, 0.5, 0.0);
	AddResizableControl(IDC_STATUS_LIST, 0, 0, 0.5, 1.0);
	AddResizableControl(IDC_ROLL_UNROLL_BUTTON, 0.5, 0, 0, 0);

	AddResizableControl(IDC_PAUSE_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_RESTART_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_RESUME_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_CANCEL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_DELETE_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_PAUSE_ALL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_START_ALL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_CANCEL_ALL_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_REMOVE_FINISHED_BUTTON, 0, 1.0, 0, 0);
	AddResizableControl(IDC_RESTART_ALL_BUTTON, 0, 1.0, 0, 0);

	AddResizableControl(IDC_STICK_BUTTON, 1.0, 1.0, 0, 0);

	// sections separators
	AddResizableControl(IDC_014_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_015_STATIC, 0.5, 0.0, 0.0, 0);

	AddResizableControl(IDC_018_STATIC, 0.5, 0.0, 0.25, 0);
	AddResizableControl(IDC_019_STATIC, 0.5, 0.0, 0.25, 0);
	AddResizableControl(IDC_016_STATIC, 0.75, 0.0, 0.25, 0);
	AddResizableControl(IDC_017_STATIC, 0.75, 0.0, 0.25, 0);

	// left part of right column
	AddResizableControl(IDC_002_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_003_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_004_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_005_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_006_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_007_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_009_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_010_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_011_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_012_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_013_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_020_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_021_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SHOW_LOG_BUTTON, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_SHOW_LOG_BUTTON, 0.5, 0.0, 0.0, 0.0);

	// full length right column
	AddResizableControl(IDC_ALL_PROGRESS, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_TASK_PROGRESS, 0.5, 0.0, 0.5, 0);

	// right part of right column
	AddResizableControl(IDC_ASSOCIATEDFILES__STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_OPERATION_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_SOURCE_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_DESTINATION_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_PROGRESS_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_TIME_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_TRANSFER_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_OVERALL_PROGRESS_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_OVERALL_TRANSFER_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_BUFFERSIZE_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_PRIORITY_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SET_BUFFERSIZE_BUTTON, 1.0, 0.0, 0.0, 0.0);
	AddResizableControl(IDC_SET_PRIORITY_BUTTON, 1.0, 0.0, 0.0, 0.0);

	InitializeResizableControls();
}

CString CStatusDlg::GetStatusString(const TASK_DISPLAY_DATA& rTaskDisplayData)
{
	CString strStatusText;
	// status string
	// first
	switch(rTaskDisplayData.m_eTaskState)
	{
	case eTaskState_Error:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_ERROR_STRING);
			strStatusText += _T("/");
			break;
		}
	case eTaskState_Paused:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_PAUSED_STRING);
			strStatusText += _T("/");
			break;
		}
	case eTaskState_Finished:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_FINISHED_STRING);
			strStatusText += _T("/");
			break;
		}
	case eTaskState_Waiting:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_WAITING_STRING);
			strStatusText += _T("/");
			break;
		}
	case eTaskState_Cancelled:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_CANCELLED_STRING);
			strStatusText += _T("/");
			break;
		}
	case eTaskState_None:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_INITIALIZING_STRING);
			strStatusText += _T("/");
			break;
		}
	case eTaskState_Processing:
		break;
	default:
		BOOST_ASSERT(false);		// not implemented state
	}

	// second part
	
	if(rTaskDisplayData.m_eSubOperationType == chcore::eSubOperation_Deleting)
		strStatusText += GetResManager().LoadString(IDS_STATUS_DELETING_STRING);
	else if(rTaskDisplayData.m_eSubOperationType == chcore::eSubOperation_Scanning)
		strStatusText += GetResManager().LoadString(IDS_STATUS_SEARCHING_STRING);
	else if(rTaskDisplayData.m_eOperationType == chcore::eOperation_Copy)
		strStatusText += GetResManager().LoadString(IDS_STATUS_COPYING_STRING);
	else if(rTaskDisplayData.m_eOperationType == chcore::eOperation_Move)
		strStatusText += GetResManager().LoadString(IDS_STATUS_MOVING_STRING);
	else
		strStatusText += GetResManager().LoadString(IDS_STATUS_UNKNOWN_STRING);

	if(rTaskDisplayData.m_pafFilters && !rTaskDisplayData.m_pafFilters->IsEmpty())
		strStatusText += GetResManager().LoadString(IDS_FILTERING_STRING);

	// third part
	if(rTaskDisplayData.m_bIgnoreDirectories)
	{
		strStatusText += _T("/");
		strStatusText += GetResManager().LoadString(IDS_STATUS_ONLY_FILES_STRING);
	}
	if(rTaskDisplayData.m_bCreateEmptyFiles)
	{
		strStatusText += _T("/");
		strStatusText += GetResManager().LoadString(IDS_STATUS_WITHOUT_CONTENTS_STRING);
	}

	return strStatusText;
}
