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
#include "../libchcore/TTaskManager.h"
#include "../libchcore/TTask.h"
#include "resource.h"
#include "StatusDlg.h"
#include "BufferSizeDlg.h"
#include "StringHelpers.h"
#include "StaticEx.h"
#include "Structs.h"
#include "../libchcore/TTaskStatsSnapshot.h"
#include "../libchcore/TTaskManagerStatsSnapshot.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool CStatusDlg::m_bLock=false;

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog

CStatusDlg::CStatusDlg(chcore::TTaskManager* pTasks, CWnd* pParent /*=NULL*/)
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
	DDX_Control(pDX, IDC_TASKCOUNT_PROGRESS, m_ctlTaskCountProgress);
	DDX_Control(pDX, IDC_TASKSIZE_PROGRESS, m_ctlTaskSizeProgress);
	DDX_Control(pDX, IDC_CURRENTOBJECT_PROGRESS, m_ctlCurrentObjectProgress);
	DDX_Control(pDX, IDC_SUBTASKCOUNT_PROGRESS, m_ctlSubTaskCountProgress);
	DDX_Control(pDX, IDC_SUBTASKSIZE_PROGRESS, m_ctlSubTaskSizeProgress);
	DDX_Control(pDX, IDC_GLOBAL_PROGRESS, m_ctlProgressAll);
	DDX_Control(pDX, IDC_STATUS_LIST, m_ctlStatusList);
}

BEGIN_MESSAGE_MAP(CStatusDlg,ictranslate::CLanguageDialog)
	//{{AFX_MSG_MAP(CStatusDlg)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PAUSE_BUTTON, OnPauseButton)
	ON_BN_CLICKED(IDC_CANCEL_BUTTON, OnCancelButton)
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
	m_ctlTaskCountProgress.SetRange32(0, 100);
	m_ctlProgressAll.SetRange32(0, 100);

	// change the size of a dialog
	StickDialogToScreenEdge();
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
		GetDlgItem(IDC_TASKID_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTASKID_STRING));

		GetDlgItem(IDC_OPERATION_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYOPERATIONTEXT_STRING));
		GetDlgItem(IDC_SOURCEOBJECT_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYSOURCETEXT_STRING));
		GetDlgItem(IDC_DESTINATIONOBJECT_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYDESTINATIONTEXT_STRING));
		GetDlgItem(IDC_BUFFERSIZE_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYBUFFERSIZETEXT_STRING));
		GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPRIORITYTEXT_STRING));
		
		// subtask
		GetDlgItem(IDC_SUBTASKNAME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYSUBTASKNAME_STRING));
		GetDlgItem(IDC_SUBTASKPROCESSED_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPROCESSEDTEXT_STRING));
		GetDlgItem(IDC_SUBTASKTIME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTIMETEXT_STRING));
		GetDlgItem(IDC_SUBTASKTRANSFER_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTRANSFERTEXT_STRING));

		GetDlgItem(IDC_TASKPROCESSED_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPROCESSEDTEXT_STRING));
		GetDlgItem(IDC_TASKTRANSFER_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTRANSFERTEXT_STRING));
		GetDlgItem(IDC_TASKTIME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTIMETEXT_STRING));

		m_ctlTaskCountProgress.SetPos(0);
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

void CStatusDlg::AddTaskInfo(int nPos, const chcore::TTaskPtr& spTask, DWORD dwCurrentTime)
{
	_ASSERTE(spTask != NULL);
	if(spTask == NULL)
		return;

	// get data snapshot from task
	chcore::TASK_DISPLAY_DATA td;
	spTask->GetSnapshot(&td);

	// set (update/add new) entry in the task list (on the left)
	SetTaskListEntry(td, nPos, spTask);

	// right side update
	if(spTask == m_spSelectedItem)
		UpdateTaskStatsDetails(td, dwCurrentTime);
}

void CStatusDlg::OnSetBuffersizeButton()
{
	chcore::TTaskPtr spTask = GetSelectedItemPointer();
	if(!spTask)
		return;

	CBufferSizeDlg dlg;
	chcore::TTaskStatsSnapshot tTaskStats;
	spTask->GetTaskStats(tTaskStats);

	spTask->GetBufferSizes(dlg.m_bsSizes);
	dlg.m_iActiveIndex = tTaskStats.GetCurrentSubTaskStats().GetCurrentBufferIndex();
	if(dlg.DoModal() == IDOK)
		spTask->SetBufferSizes(dlg.m_bsSizes);
}

chcore::TTaskPtr CStatusDlg::GetSelectedItemPointer()
{
	// returns ptr to a TTask for a given element in listview
	if(m_ctlStatusList.GetSelectedCount() == 1)
	{
		POSITION pos = m_ctlStatusList.GetFirstSelectedItemPosition();
		int nPos = m_ctlStatusList.GetNextSelectedItem(pos);
		return m_pTasks->GetTaskBySessionUniqueID(m_ctlStatusList.GetItemData(nPos));
	}

	return chcore::TTaskPtr();
}

void CStatusDlg::StickDialogToScreenEdge()
{
	// get coord of screen and window
	CRect rcScreen, rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);
	GetWindowRect(&rect);

	SetWindowPos(NULL, rcScreen.right-rect.Width(),
		rcScreen.bottom-rect.Height(), rect.Width(), rect.Height(),
		SWP_NOOWNERZORDER | SWP_NOZORDER);
}

void CStatusDlg::ApplyButtonsState()
{
	// remember ptr to TTask
	m_spSelectedItem=GetSelectedItemPointer();

	// set status of buttons pause/resume/cancel
	if (m_spSelectedItem != NULL)
	{
		GetDlgItem(IDC_RESTART_BUTTON)->EnableWindow(true);
		GetDlgItem(IDC_SHOW_LOG_BUTTON)->EnableWindow(true);
		GetDlgItem(IDC_DELETE_BUTTON)->EnableWindow(true);
		
		if (m_spSelectedItem->GetTaskState() == chcore::eTaskState_Finished || m_spSelectedItem->GetTaskState() == chcore::eTaskState_Cancelled)
		{
			GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
		}	
		else
		{
			// pause/resume
			if (m_spSelectedItem->GetTaskState() == chcore::eTaskState_Paused)
			{
				GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
				GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(true);
			}
			else
			{
				GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(true);
				if (m_spSelectedItem->GetTaskState() == chcore::eTaskState_Waiting)
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
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_TIME_CRITICAL)));
				break;
			case ID_POPUP_HIGHEST:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_HIGHEST);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_HIGHEST)));
				break;
			case ID_POPUP_ABOVE_NORMAL:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_ABOVE_NORMAL)));
				break;
			case ID_POPUP_NORMAL:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_NORMAL)));
				break;
			case ID_POPUP_BELOW_NORMAL:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_BELOW_NORMAL)));
				break;
			case ID_POPUP_LOWEST:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_LOWEST);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_LOWEST)));
				break;
			case ID_POPUP_IDLE:
				m_spSelectedItem->SetPriority(THREAD_PRIORITY_IDLE);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_IDLE)));
				break;
			}
		}
	}
	return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);
}

void CStatusDlg::OnPauseButton() 
{
	chcore::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		TRACE("PauseProcessing call...\n");
		spTask->PauseProcessing();

		RefreshStatus();
	}
}

void CStatusDlg::OnResumeButton() 
{
	chcore::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		if(spTask->GetTaskState() == chcore::eTaskState_Waiting)
			spTask->SetForceFlag();
		else
			spTask->ResumeProcessing();

		RefreshStatus();
	}
}

void CStatusDlg::OnCancelButton() 
{
	chcore::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		spTask->CancelProcessing();
		RefreshStatus();
	}
}

void CStatusDlg::OnRestartButton() 
{
	chcore::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		spTask->RestartProcessing();
		RefreshStatus();
	}
}

void CStatusDlg::OnDeleteButton() 
{
	chcore::TTaskPtr spTask = GetSelectedItemPointer();
	if(spTask)
	{
		chcore::ETaskCurrentState eTaskState = spTask->GetTaskState();
		if(eTaskState != chcore::eTaskState_Finished && eTaskState != chcore::eTaskState_Cancelled)
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
			chcore::TTaskPtr spTask = GetSelectedItemPointer();
			if (!spTask)
				return;
		
			if(spTask->GetTaskState() == chcore::eTaskState_Paused)
				OnResumeButton();
			else
				OnPauseButton();
			break;
		}
	}

	*pResult = 0;
}

int CStatusDlg::GetImageFromStatus(chcore::ETaskCurrentState eState)
{
	switch(eState)
	{
	case chcore::eTaskState_Cancelled:
		return 4;
	case chcore::eTaskState_Finished:
		return 3;
	case chcore::eTaskState_Waiting:
		return 5;
	case chcore::eTaskState_Paused:
		return 2;
	case chcore::eTaskState_Error:
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

LPTSTR CStatusDlg::FormatTimeMiliseconds(unsigned long long timeMiliSeconds, LPTSTR lpszBuffer, size_t stMaxBufferSize)
{
	time_t timeSeconds = timeMiliSeconds / 1000;
	return FormatTime(timeSeconds, lpszBuffer, stMaxBufferSize);
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
	chcore::TTaskManagerStatsSnapshot tTMStats;
	m_pTasks->GetStatsSnapshot(tTMStats);

	// set title
	if (m_pTasks->GetSize() != 0)
		_sntprintf(m_szData, _MAX_PATH, _T("%s [%.0f %%]"), GetResManager().LoadString(IDS_STATUSTITLE_STRING), tTMStats.GetGlobalProgressInPercent());
	else
		_sntprintf(m_szData, _MAX_PATH, _T("%s"), GetResManager().LoadString(IDS_STATUSTITLE_STRING));
	
	// if changed
	CString strTemp;
	GetWindowText(strTemp);
	if (strTemp != CString(m_szData)) 
		SetWindowText(m_szData);
	
	// refresh overall progress
	m_ctlProgressAll.SetPos(boost::numeric_cast<int>(tTMStats.GetGlobalProgressInPercent()));
	
	// progress - count of processed data/count of data
	strTemp=GetSizeString(tTMStats.GetProcessedSize(), m_szData, _MAX_PATH)+CString(_T("/"));
	strTemp+=GetSizeString(tTMStats.GetTotalSize(), m_szData, _MAX_PATH);
	GetDlgItem(IDC_GLOBALPROCESSED_STATIC)->SetWindowText(strTemp);
	
	// transfer
	if (m_i64LastAllTasksProcessed == 0)
		m_i64LastAllTasksProcessed=tTMStats.GetProcessedSize();
	
	if (dwCurrentTime-m_dwLastUpdate != 0)
		strTemp=GetSizeString( (static_cast<double>(tTMStats.GetProcessedSize()) - static_cast<double>(m_i64LastAllTasksProcessed))/static_cast<double>(static_cast<double>(dwCurrentTime-m_dwLastUpdate)/1000.0), m_szData, _MAX_PATH);
	else
		strTemp=GetSizeString( 0ULL, m_szData, _MAX_PATH);
	
	GetDlgItem(IDC_GLOBALTRANSFER_STATIC)->SetWindowText(strTemp+_T("/s"));
	m_i64LastAllTasksProcessed=tTMStats.GetProcessedSize();
	m_dwLastUpdate=dwCurrentTime;

	// if selection's missing - hide controls
	if (m_ctlStatusList.GetSelectedCount() == 0)
	{
		EnableControls(false);
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
	chcore::TTaskPtr spTask = GetSelectedItemPointer();
	if (!spTask)
		return;

	unsigned long lResult = (unsigned long)(ShellExecute(this->m_hWnd, _T("open"), _T("notepad.exe"), spTask->GetRelatedPath(chcore::TTask::ePathType_TaskLogFile).ToString(), NULL, SW_SHOWNORMAL));
	if(lResult < 32)
	{
		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_SHELLEXECUTEERROR_STRING));
		fmt.SetParam(_t("%errno"), lResult);
		fmt.SetParam(_t("%path"), spTask->GetRelatedPath(chcore::TTask::ePathType_TaskLogFile).ToString());
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
	StickDialogToScreenEdge();
}

void CStatusDlg::SetBufferSizesString(UINT uiValue, int iIndex)
{
	TCHAR szData[1024];
	switch(iIndex)
	{
	case chcore::TBufferSizes::eBuffer_Default:
		GetResManager().LoadStringCopy(IDS_BSDEFAULT_STRING, szData, 256);
		break;
	case chcore::TBufferSizes::eBuffer_OneDisk:
		GetResManager().LoadStringCopy(IDS_BSONEDISK_STRING, szData, 256);
		break;
	case chcore::TBufferSizes::eBuffer_TwoDisks:
		GetResManager().LoadStringCopy(IDS_BSTWODISKS_STRING, szData, 256);
		break;
	case chcore::TBufferSizes::eBuffer_CD:
		GetResManager().LoadStringCopy(IDS_BSCD_STRING, szData, 256);
		break;
	case chcore::TBufferSizes::eBuffer_LAN:
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

	// left part of dialog (task list)
	AddResizableControl(IDC_TASKLIST_LABEL_STATIC, 0, 0, 0.5, 0.0);
	AddResizableControl(IDC_STATUS_LIST, 0, 0, 0.5, 1.0);

	// left part of dialog (buttons under the task list)
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

	// left part of dialog (global stats)
	AddResizableControl(IDC_GLOBAL_GROUP_STATIC, 0.0, 1.0, 0.5, 0);

	AddResizableControl(IDC_GLOBALPROCESSED_LABEL_STATIC, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_GLOBALPROCESSED_STATIC, 0.0, 1.0, 0.5, 0);
	AddResizableControl(IDC_GLOBALTRANSFER_LABEL_STATIC, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_GLOBALTRANSFER_STATIC, 0.0, 1.0, 0.5, 0);
	AddResizableControl(IDC_GLOBALPROGRESS_LABEL_STATIC, 0.0, 1.0, 0.0, 0.0);
	AddResizableControl(IDC_GLOBAL_PROGRESS, 0.0, 1.0, 0.5, 0.0);

	// right part of dialog  (task info)
	AddResizableControl(IDC_TASKINFORMATION_GROUP_STATIC, 0.5, 0.0, 0.5, 0);

	// right part of dialog (subsequent entries)
	AddResizableControl(IDC_TASKID_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKID_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_OPERATION_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_OPERATION_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SOURCEOBJECT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SOURCEOBJECT_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_DESTINATIONOBJECT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_DESTINATIONOBJECT_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_BUFFERSIZE_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_BUFFERSIZE_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_SET_BUFFERSIZE_BUTTON, 1.0, 0.0, 0.0, 0.0);

	AddResizableControl(IDC_THREADPRIORITY_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_THREADPRIORITY_STATIC, 0.5, 0.0, 0.5, 0);
	AddResizableControl(IDC_SET_PRIORITY_BUTTON, 1.0, 0.0, 0.0, 0.0);

	// right part of the dialog (subtask stats)
	AddResizableControl(IDC_CURRENTPHASE_GROUP_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKNAME_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKNAME_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKPROCESSED_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKPROCESSED_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKTIME_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKTIME_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKTRANSFER_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKTRANSFER_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_CURRENTOBJECT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_CURRENTOBJECT_PROGRESS, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKCOUNT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKCOUNT_PROGRESS, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SUBTASKSIZE_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_SUBTASKSIZE_PROGRESS, 0.5, 0.0, 0.5, 0);

	// right part of the dialog (task stats)
	AddResizableControl(IDC_ENTIRETASK_GROUP_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKPROCESSED_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKPROCESSED_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKTIME_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKTIME_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKTRANSFER_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKTRANSFER_STATIC, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKCOUNT_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKCOUNT_PROGRESS, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_TASKSIZE_LABEL_STATIC, 0.5, 0.0, 0.0, 0);
	AddResizableControl(IDC_TASKSIZE_PROGRESS, 0.5, 0.0, 0.5, 0);

	AddResizableControl(IDC_SHOW_LOG_BUTTON, 1.0, 0.0, 0.0, 0);
	AddResizableControl(IDC_STICK_BUTTON, 1.0, 1.0, 0, 0);

	InitializeResizableControls();
}

CString CStatusDlg::GetStatusString(const chcore::TASK_DISPLAY_DATA& rTaskDisplayData)
{
	CString strStatusText;
	// status string
	// first
	switch(rTaskDisplayData.m_eTaskState)
	{
	case chcore::eTaskState_Error:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_ERROR_STRING);
			strStatusText += _T("/");
			break;
		}
	case chcore::eTaskState_Paused:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_PAUSED_STRING);
			strStatusText += _T("/");
			break;
		}
	case chcore::eTaskState_Finished:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_FINISHED_STRING);
			strStatusText += _T("/");
			break;
		}
	case chcore::eTaskState_Waiting:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_WAITING_STRING);
			strStatusText += _T("/");
			break;
		}
	case chcore::eTaskState_Cancelled:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_CANCELLED_STRING);
			strStatusText += _T("/");
			break;
		}
	case chcore::eTaskState_None:
		{
			strStatusText = GetResManager().LoadString(IDS_STATUS_INITIALIZING_STRING);
			strStatusText += _T("/");
			break;
		}
	case chcore::eTaskState_Processing:
		break;
	default:
		BOOST_ASSERT(false);		// not implemented state
	}

	// second part
	
	if(rTaskDisplayData.m_eSubOperationType == chcore::eSubOperation_Deleting)
		strStatusText += GetResManager().LoadString(IDS_STATUS_DELETING_STRING);
	else if(rTaskDisplayData.m_eSubOperationType == chcore::eSubOperation_Scanning)
		strStatusText += GetResManager().LoadString(IDS_STATUS_SEARCHING_STRING);
	else if(rTaskDisplayData.m_eSubOperationType == chcore::eSubOperation_FastMove)
		strStatusText += GetResManager().LoadString(IDS_STATUS_FASTMOVE_STRING);
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

void CStatusDlg::SetTaskListEntry(const chcore::TASK_DISPLAY_DATA &td, int nPos, const chcore::TTaskPtr& spTask)
{
	// index subitem
	CString strStatusText = GetStatusString(td);
	CString strTemp;
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem = nPos;
	lvi.iSubItem = 0;
	lvi.pszText = (PTSTR)(PCTSTR)strStatusText;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	lvi.lParam = spTask->GetSessionUniqueID();
	lvi.iImage = GetImageFromStatus(td.m_eTaskState);
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
	_itot(boost::numeric_cast<int>(td.m_dPercent), m_szData, 10);
	_tcscat(m_szData, _T(" %"));
	lvi.pszText=m_szData;
	lvi.cchTextMax=lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);
}

CString CStatusDlg::GetProcessedText(unsigned long long ullProcessedCount, unsigned long long ullTotalCount, unsigned long long ullProcessedSize, unsigned long long ullTotalSize)
{
	CString strTemp;
	_sntprintf(m_szData, _MAX_PATH, _T("%ld/%ld ("), ullProcessedCount, ullTotalCount);
	strTemp = CString(m_szData);
	strTemp += GetSizeString(ullProcessedSize, m_szData, _MAX_PATH) + CString(_T("/"));
	strTemp += GetSizeString(ullTotalSize, m_szData, _MAX_PATH) + CString(_T(")"));
	return strTemp;
}

void CStatusDlg::UpdateTaskStatsDetails(chcore::TASK_DISPLAY_DATA &td, DWORD dwCurrentTime)
{
	chcore::TSubTaskStatsSnapshot& tSubTaskStats = td.m_tTaskSnapshot.GetCurrentSubTaskStats();

	// text progress
	CString strProcessedText = GetProcessedText(tSubTaskStats.GetProcessedCount(), tSubTaskStats.GetTotalCount(), tSubTaskStats.GetProcessedSize(), tSubTaskStats.GetTotalSize());
	GetDlgItem(IDC_SUBTASKPROCESSED_STATIC)->SetWindowText(strProcessedText);

	// progress bars
	m_ctlCurrentObjectProgress.SetProgress(tSubTaskStats.GetCurrentItemProcessedSize(), tSubTaskStats.GetCurrentItemTotalSize());
	m_ctlSubTaskCountProgress.SetProgress(tSubTaskStats.GetProcessedCount(), tSubTaskStats.GetTotalCount());
	m_ctlSubTaskSizeProgress.SetProgress(tSubTaskStats.GetProcessedSize(), tSubTaskStats.GetTotalSize());

	// time information
	unsigned long long timeTotalEstimated = tSubTaskStats.GetEstimatedTotalTime();
	unsigned long long timeElapsed = tSubTaskStats.GetTimeElapsed();
	unsigned long long timeRemaining = timeTotalEstimated - timeElapsed;

	FormatTimeMiliseconds(timeElapsed, m_szTimeBuffer1, 40);
	FormatTimeMiliseconds(timeTotalEstimated, m_szTimeBuffer2, 40);
	FormatTimeMiliseconds(timeRemaining, m_szTimeBuffer3, 40);

	_sntprintf(m_szData, _MAX_PATH, _T("%s / %s (%s)"), m_szTimeBuffer1, m_szTimeBuffer2, m_szTimeBuffer3);

	GetDlgItem(IDC_SUBTASKTIME_STATIC)->SetWindowText(m_szData);

	// speed information
	CString strSizeSpeed;
	CString strCountSpeed;

	GetSizeString(tSubTaskStats.GetSizeSpeed(), m_szData, _MAX_PATH);
	strSizeSpeed = m_szData;
	GetSizeString(tSubTaskStats.GetAvgSizeSpeed(), m_szData, _MAX_PATH);
	strSizeSpeed.AppendFormat(_T("/s (a: %s/s)"), m_szData);

	strCountSpeed.Format(_T("%.2f/s (a: %.2f/s)"), tSubTaskStats.GetCountSpeed(), tSubTaskStats.GetAvgCountSpeed());
	GetDlgItem(IDC_SUBTASKTRANSFER_STATIC)->SetWindowText(strSizeSpeed + _T("; ") + strCountSpeed);

	//////////////////////////////////////////////////////
	// data that can be changed by a thread
	CString strStatusText = GetStatusString(td);
	GetDlgItem(IDC_OPERATION_STATIC)->SetWindowText(strStatusText);	// operation

	if(td.m_strFullFilePath.IsEmpty())
		GetDlgItem(IDC_SOURCEOBJECT_STATIC)->SetWindowText(GetResManager().LoadString(IDS_NONEINPUTFILE_STRING));
	else
		GetDlgItem(IDC_SOURCEOBJECT_STATIC)->SetWindowText(td.m_strFullFilePath);	// src object

	// count of processed data/overall count of data
	strProcessedText = GetProcessedText(td.m_stIndex, td.m_stSize, td.m_ullProcessedSize, td.m_ullSizeAll);
	GetDlgItem(IDC_TASKPROCESSED_STATIC)->SetWindowText(strProcessedText);

	// transfer
	CString strSpeedText;
	if (m_i64LastProcessed == 0)	// if first time - show average
		strSpeedText=GetSizeString( td.m_timeElapsed ? td.m_ullProcessedSize/td.m_timeElapsed : 0, m_szData, _MAX_PATH);	// last avg
	else
	{
		if ( (dwCurrentTime-m_dwLastUpdate) != 0)
			strSpeedText=GetSizeString( (static_cast<double>(td.m_ullProcessedSize) - static_cast<double>(m_i64LastProcessed))/(static_cast<double>(dwCurrentTime-m_dwLastUpdate)/1000.0), m_szData, _MAX_PATH);
		else
			strSpeedText=GetSizeString( 0ULL, m_szData, _MAX_PATH);
	}

	// avg transfer
	GetDlgItem(IDC_TASKTRANSFER_STATIC)->SetWindowText(strSpeedText+_T("/s (")+CString(GetResManager().LoadString(IDS_AVERAGEWORD_STRING))
		+CString(GetSizeString(td.m_timeElapsed ? td.m_ullProcessedSize/td.m_timeElapsed : 0, m_szData, _MAX_PATH))+_T("/s )")
		);

	// elapsed time / estimated total time (estimated time left)
	FormatTime(td.m_timeElapsed, m_szTimeBuffer1, 40);
	time_t timeTotal = (td.m_ullProcessedSize == 0) ? 0 : (long)(td.m_ullSizeAll * td.m_timeElapsed / td.m_ullProcessedSize);
	FormatTime(timeTotal, m_szTimeBuffer2, 40);
	FormatTime(std::max((time_t)0l, timeTotal - td.m_timeElapsed), m_szTimeBuffer3, 40);

	_sntprintf(m_szData, _MAX_PATH, _T("%s / %s (%s)"), m_szTimeBuffer1, m_szTimeBuffer2, m_szTimeBuffer3);
	GetDlgItem(IDC_TASKTIME_STATIC)->SetWindowText(m_szData);

	// remember current processed data (used for calculating transfer)
	m_i64LastProcessed=td.m_ullProcessedSize;

	// set progress
	m_ctlTaskCountProgress.SetPos(boost::numeric_cast<int>(td.m_dPercent));

	SetBufferSizesString(td.m_iCurrentBufferSize, td.m_iCurrentBufferIndex);

	GetDlgItem(IDC_DESTINATIONOBJECT_STATIC)->SetWindowText(td.m_pathDstPath.ToString());
	GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(td.m_nPriority)));
	GetDlgItem(IDC_TASKID_STATIC)->SetWindowText(td.m_strUniqueName);
}
