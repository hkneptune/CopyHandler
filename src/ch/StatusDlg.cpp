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
	: ictranslate::CLanguageDialog(CStatusDlg::IDD, pParent, &m_bLock),
	m_spTaskMgrStats(new chcore::TTaskManagerStatsSnapshot),
	m_pTasks(pTasks)
{
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
		m_ctlTaskSizeProgress.SetPos(0);
		m_ctlCurrentObjectProgress.SetPos(0);
		m_ctlSubTaskCountProgress.SetPos(0);
		m_ctlSubTaskSizeProgress.SetPos(0);
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

void CStatusDlg::OnSetBuffersizeButton()
{
	chcore::TTaskPtr spTask = GetSelectedItemPointer();
	if(!spTask)
		return;

	int iCurrentBufferIndex = 0;
	chcore::TTaskStatsSnapshotPtr spTaskStats = m_spTaskMgrStats->GetTaskStatsForTaskID(GetSelectedItemSessionUniqueID());
	if(spTaskStats)
	{
		chcore::TSubTaskStatsSnapshotPtr spSubTaskStats = spTaskStats->GetSubTasksStats().GetCurrentSubTaskSnapshot();
		if(spSubTaskStats)
			iCurrentBufferIndex = spSubTaskStats->GetCurrentBufferIndex();
	}

	CBufferSizeDlg dlg;
	spTask->GetBufferSizes(dlg.m_bsSizes);
	dlg.m_iActiveIndex = iCurrentBufferIndex;
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
		return m_pTasks->GetTaskByTaskID(m_ctlStatusList.GetItemData(nPos));
	}

	return chcore::TTaskPtr();
}

size_t CStatusDlg::GetSelectedItemSessionUniqueID()
{
	// returns ptr to a TTask for a given element in listview
	if(m_ctlStatusList.GetSelectedCount() == 1)
	{
		POSITION pos = m_ctlStatusList.GetFirstSelectedItemPosition();
		int nPos = m_ctlStatusList.GetNextSelectedItem(pos);
		return m_ctlStatusList.GetItemData(nPos);
	}

	return std::numeric_limits<size_t>::max();
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
	chcore::TTaskPtr spSelectedTask = GetSelectedItemPointer();

	// set status of buttons pause/resume/cancel
	if (spSelectedTask != NULL)
	{
		GetDlgItem(IDC_RESTART_BUTTON)->EnableWindow(true);
		GetDlgItem(IDC_SHOW_LOG_BUTTON)->EnableWindow(true);
		GetDlgItem(IDC_DELETE_BUTTON)->EnableWindow(true);
		
		if (spSelectedTask->GetTaskState() == chcore::eTaskState_Finished || spSelectedTask->GetTaskState() == chcore::eTaskState_Cancelled)
		{
			GetDlgItem(IDC_CANCEL_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
			GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(false);
		}	
		else
		{
			// pause/resume
			if (spSelectedTask->GetTaskState() == chcore::eTaskState_Paused)
			{
				GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(false);
				GetDlgItem(IDC_RESUME_BUTTON)->EnableWindow(true);
			}
			else
			{
				GetDlgItem(IDC_PAUSE_BUTTON)->EnableWindow(true);
				if (spSelectedTask->GetTaskState() == chcore::eTaskState_Waiting)
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
			chcore::TTaskPtr spSelectedTask = GetSelectedItemPointer();

			if(spSelectedTask == NULL)
				return ictranslate::CLanguageDialog::OnCommand(wParam, lParam);
			
			switch (LOWORD(wParam))
			{
			case ID_POPUP_TIME_CRITICAL:
				spSelectedTask->SetPriority(THREAD_PRIORITY_TIME_CRITICAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_TIME_CRITICAL)));
				break;
			case ID_POPUP_HIGHEST:
				spSelectedTask->SetPriority(THREAD_PRIORITY_HIGHEST);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_HIGHEST)));
				break;
			case ID_POPUP_ABOVE_NORMAL:
				spSelectedTask->SetPriority(THREAD_PRIORITY_ABOVE_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_ABOVE_NORMAL)));
				break;
			case ID_POPUP_NORMAL:
				spSelectedTask->SetPriority(THREAD_PRIORITY_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_NORMAL)));
				break;
			case ID_POPUP_BELOW_NORMAL:
				spSelectedTask->SetPriority(THREAD_PRIORITY_BELOW_NORMAL);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_BELOW_NORMAL)));
				break;
			case ID_POPUP_LOWEST:
				spSelectedTask->SetPriority(THREAD_PRIORITY_LOWEST);
				GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING+PriorityToIndex(THREAD_PRIORITY_LOWEST)));
				break;
			case ID_POPUP_IDLE:
				spSelectedTask->SetPriority(THREAD_PRIORITY_IDLE);
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

LPTSTR CStatusDlg::FormatTime(unsigned long long timeSeconds, LPTSTR lpszBuffer, size_t stMaxBufferSize)
{
	if(timeSeconds > 30*24*3600)	// more than 30 days
	{
		// we need those pragmas to disable lv4 warning "warning C4428: universal-character-name encountered in source"
		// which incorrectly warns about the infinity char embedded in a string.
#pragma warning(push)
#pragma warning(disable: 4428)
		_tcscpy_s(lpszBuffer, stMaxBufferSize, L"\u221E");
#pragma warning(pop)
		return lpszBuffer;
	}

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
	unsigned long long timeSeconds = timeMiliSeconds / 1000;
	return FormatTime(timeSeconds, lpszBuffer, stMaxBufferSize);
}

void CStatusDlg::RefreshStatus()
{
	// remember address of a current selection
	size_t stSelectedTaskID = GetSelectedItemSessionUniqueID();

	// get all the stats needed
	m_pTasks->GetStatsSnapshot(m_spTaskMgrStats);

	// get rid of item after the current part
	m_ctlStatusList.LimitItems(boost::numeric_cast<int>(m_spTaskMgrStats->GetTaskStatsCount()));

	// add task info
	for(size_t stIndex = 0; stIndex < m_spTaskMgrStats->GetTaskStatsCount(); ++stIndex)
	{
		chcore::TTaskStatsSnapshotPtr spTaskStats = m_spTaskMgrStats->GetTaskStatsAt(stIndex);
		// set (update/add new) entry in the task list (on the left)
		SetTaskListEntry(stIndex, spTaskStats);

		// right side update
		if(spTaskStats->GetTaskID() == stSelectedTaskID)
			UpdateTaskStatsDetails(spTaskStats);
	}

	// set title
	SetWindowTitle(GetProgressWindowTitleText());

	// refresh overall progress
	m_ctlProgressAll.SetRange(0, 100);
	m_ctlProgressAll.SetPos(boost::numeric_cast<int>(m_spTaskMgrStats->GetCombinedProgress() * 100.0));
	
	// progress - count of processed data/count of data
	CString strTemp;
	strTemp=GetSizeString(m_spTaskMgrStats->GetProcessedSize(), m_szData, _MAX_PATH)+CString(_T("/"));
	strTemp+=GetSizeString(m_spTaskMgrStats->GetTotalSize(), m_szData, _MAX_PATH);
	GetDlgItem(IDC_GLOBALPROCESSED_STATIC)->SetWindowText(strTemp);
	
	// transfer
	CString strSpeed = GetSpeedString(m_spTaskMgrStats->GetSizeSpeed(), m_spTaskMgrStats->GetAvgSizeSpeed(), m_spTaskMgrStats->GetCountSpeed(), m_spTaskMgrStats->GetAvgCountSpeed());
	GetDlgItem(IDC_GLOBALTRANSFER_STATIC)->SetWindowText(strSpeed);

	// if selection's missing - hide controls
	if (m_ctlStatusList.GetSelectedCount() == 0)
		EnableControls(false);
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
	if(!spTask)
		return;

	unsigned long lResult = (unsigned long)(ShellExecute(this->m_hWnd, _T("open"), _T("notepad.exe"),
		spTask->GetLogPath().ToString(), NULL, SW_SHOWNORMAL));
	if(lResult < 32)
	{
		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_SHELLEXECUTEERROR_STRING));
		fmt.SetParam(_t("%errno"), lResult);
		fmt.SetParam(_t("%path"), spTask->GetLogPath().ToString());
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

void CStatusDlg::SetBufferSizesString(unsigned long long ullValue, int iIndex)
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

	_tcscat(szData, GetSizeString(ullValue, m_szData, _MAX_PATH));

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

CString CStatusDlg::GetStatusString(const chcore::TTaskStatsSnapshotPtr& spTaskStats)
{
	CString strStatusText;
	// status string
	// first
	switch(spTaskStats->GetTaskState())
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
	chcore::ESubOperationType eSubOperationType = chcore::eSubOperation_None;
	chcore::TSubTaskStatsSnapshotPtr spSubtaskStats = spTaskStats->GetSubTasksStats().GetCurrentSubTaskSnapshot();
	if(spSubtaskStats)
		eSubOperationType = spSubtaskStats->GetSubOperationType();

	if(eSubOperationType == chcore::eSubOperation_Deleting)
		strStatusText += GetResManager().LoadString(IDS_STATUS_DELETING_STRING);
	else if(eSubOperationType == chcore::eSubOperation_Scanning)
		strStatusText += GetResManager().LoadString(IDS_STATUS_SEARCHING_STRING);
	else if(eSubOperationType == chcore::eSubOperation_FastMove)
		strStatusText += GetResManager().LoadString(IDS_STATUS_FASTMOVE_STRING);
	else if(spTaskStats->GetOperationType() == chcore::eOperation_Copy)
		strStatusText += GetResManager().LoadString(IDS_STATUS_COPYING_STRING);
	else if(spTaskStats->GetOperationType() == chcore::eOperation_Move)
		strStatusText += GetResManager().LoadString(IDS_STATUS_MOVING_STRING);
	else
		strStatusText += GetResManager().LoadString(IDS_STATUS_UNKNOWN_STRING);

	if(!spTaskStats->GetFilters().IsEmpty())
		strStatusText += GetResManager().LoadString(IDS_FILTERING_STRING);

	// third part
	if(spTaskStats->GetIgnoreDirectories())
	{
		strStatusText += _T("/");
		strStatusText += GetResManager().LoadString(IDS_STATUS_ONLY_FILES_STRING);
	}
	if(spTaskStats->GetCreateEmptyFiles())
	{
		strStatusText += _T("/");
		strStatusText += GetResManager().LoadString(IDS_STATUS_WITHOUT_CONTENTS_STRING);
	}

	return strStatusText;
}

CString CStatusDlg::GetSubtaskName(chcore::ESubOperationType eSubtask) const
{
	if(eSubtask == chcore::eSubOperation_Deleting)
		return GetResManager().LoadString(IDS_STATUS_DELETING_STRING);
	else if(eSubtask == chcore::eSubOperation_Scanning)
		return GetResManager().LoadString(IDS_STATUS_SEARCHING_STRING);
	else if(eSubtask == chcore::eSubOperation_FastMove)
		return GetResManager().LoadString(IDS_STATUS_FASTMOVE_STRING);
	else if(eSubtask == chcore::eSubOperation_Copying)
		return GetResManager().LoadString(IDS_STATUS_COPYING_STRING);
	else
		return GetResManager().LoadString(IDS_STATUS_UNKNOWN_STRING);
}

void CStatusDlg::SetTaskListEntry(size_t stPos, const chcore::TTaskStatsSnapshotPtr& spTaskStats)
{
	// index subitem
	CString strStatusText = GetStatusString(spTaskStats);
	LVITEM lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem = boost::numeric_cast<int>(stPos);
	lvi.iSubItem = 0;
	lvi.pszText = (PTSTR)(PCTSTR)strStatusText;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	lvi.lParam = spTaskStats->GetTaskID();
	lvi.iImage = GetImageFromStatus(spTaskStats->GetTaskState());
	if(boost::numeric_cast<int>(stPos) < m_ctlStatusList.GetItemCount())
		m_ctlStatusList.SetItem(&lvi);
	else
		m_ctlStatusList.InsertItem(&lvi);

	chcore::TSubTaskStatsSnapshotPtr spSubTaskStats = spTaskStats->GetSubTasksStats().GetCurrentSubTaskSnapshot();
	chcore::TString strCurrentPath;
	if(spSubTaskStats)
	{
		chcore::TSmartPath path;
		path.FromString(spSubTaskStats->GetCurrentPath());
		strCurrentPath = path.GetFileName().ToString();
	}

	// input file
	lvi.mask=LVIF_TEXT;
	lvi.iSubItem = 1;
	if(strCurrentPath.IsEmpty())
		strCurrentPath = GetResManager().LoadString(IDS_NONEINPUTFILE_STRING);

	lvi.pszText = (PTSTR)(PCTSTR)strCurrentPath;;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);

	// destination path
	lvi.iSubItem = 2;
	chcore::TString strDestinationPath = spTaskStats->GetDestinationPath();
	lvi.pszText = (PTSTR)(PCTSTR)strDestinationPath;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);

	// insert dest subitem
	lvi.iSubItem=3;

	CString strFmt;
	strFmt.Format(_T("%.0f %%"), spTaskStats->GetCombinedProgress() * 100.0);

	lvi.pszText = (PTSTR)(PCTSTR)strFmt;
	lvi.cchTextMax = lstrlen(lvi.pszText);
	m_ctlStatusList.SetItem(&lvi);
}

CString CStatusDlg::GetProcessedText(unsigned long long ullProcessedCount, unsigned long long ullTotalCount, unsigned long long ullProcessedSize, unsigned long long ullTotalSize)
{
	CString strTemp;
	_sntprintf(m_szData, _MAX_PATH, _T("%I64u/%I64u ("), ullProcessedCount, ullTotalCount);
	strTemp = CString(m_szData);
	strTemp += GetSizeString(ullProcessedSize, m_szData, _MAX_PATH) + CString(_T("/"));
	strTemp += GetSizeString(ullTotalSize, m_szData, _MAX_PATH) + CString(_T(")"));
	return strTemp;
}

CString CStatusDlg::GetSpeedString(double dSizeSpeed, double dAvgSizeSpeed, double dCountSpeed, double dAvgCountSpeed) const
{
	TCHAR szData[_MAX_PATH];
	CString strSpeedText = GetSizeString(dSizeSpeed, szData, _MAX_PATH);	// last avg
	CString strAvgSpeedText = GetSizeString(dAvgSizeSpeed, szData, _MAX_PATH);	// last avg

	CString strAvgWord = GetResManager().LoadString(IDS_AVERAGEWORD_STRING);

	// avg transfer
	CString strFmt;
	strFmt.Format(_T("%s/s (%s%s/s); %.0f/s (%s%.0f/s)"), strSpeedText, strAvgWord, strAvgSpeedText,
		dCountSpeed, strAvgWord, dAvgCountSpeed);

	return strFmt;
}

void CStatusDlg::UpdateTaskStatsDetails(const chcore::TTaskStatsSnapshotPtr& spTaskStats)
{
	unsigned long long timeTotalEstimated = 0;
	unsigned long long timeElapsed = 0;
	unsigned long long timeRemaining = 0;

	chcore::TSubTaskStatsSnapshotPtr spSubTaskStats = spTaskStats->GetSubTasksStats().GetCurrentSubTaskSnapshot();
	if(spSubTaskStats)
	{
		// text progress
		CString strProcessedText = GetProcessedText(spSubTaskStats->GetProcessedCount(), spSubTaskStats->GetTotalCount(), spSubTaskStats->GetProcessedSize(), spSubTaskStats->GetTotalSize());
		GetDlgItem(IDC_SUBTASKPROCESSED_STATIC)->SetWindowText(strProcessedText);

		// progress bars
		m_ctlCurrentObjectProgress.SetProgress(spSubTaskStats->GetCurrentItemProcessedSize(), spSubTaskStats->GetCurrentItemTotalSize());
		m_ctlSubTaskCountProgress.SetProgress(spSubTaskStats->GetProcessedCount(), spSubTaskStats->GetTotalCount());
		m_ctlSubTaskSizeProgress.SetProgress(spSubTaskStats->GetProcessedSize(), spSubTaskStats->GetTotalSize());

		// time information
		timeTotalEstimated = spSubTaskStats->GetEstimatedTotalTime();
		timeElapsed = spSubTaskStats->GetTimeElapsed();
		timeRemaining = timeTotalEstimated - timeElapsed;

		FormatTimeMiliseconds(timeElapsed, m_szTimeBuffer1, 40);
		FormatTimeMiliseconds(timeTotalEstimated, m_szTimeBuffer2, 40);
		FormatTimeMiliseconds(timeRemaining, m_szTimeBuffer3, 40);

		_sntprintf(m_szData, _MAX_PATH, _T("%s / %s (%s)"), m_szTimeBuffer1, m_szTimeBuffer2, m_szTimeBuffer3);
		GetDlgItem(IDC_SUBTASKTIME_STATIC)->SetWindowText(m_szData);

		// speed information
		CString strSpeed = GetSpeedString(spSubTaskStats->GetSizeSpeed(), spSubTaskStats->GetAvgSizeSpeed(), spSubTaskStats->GetCountSpeed(), spSubTaskStats->GetAvgCountSpeed());
		GetDlgItem(IDC_SUBTASKTRANSFER_STATIC)->SetWindowText(strSpeed);

		// subtask name
		chcore::ESubOperationType eSubOperationType = spSubTaskStats->GetSubOperationType();
		CString strSubtaskName = GetSubtaskName(eSubOperationType);
		GetDlgItem(IDC_SUBTASKNAME_STATIC)->SetWindowText(strSubtaskName);

		// current path
		chcore::TString strPath = spSubTaskStats->GetCurrentPath();
		if(strPath.IsEmpty())
			strPath = GetResManager().LoadString(IDS_NONEINPUTFILE_STRING);

		GetDlgItem(IDC_SOURCEOBJECT_STATIC)->SetWindowText(strPath);	// src object

		SetBufferSizesString(spTaskStats->GetCurrentBufferSize(), spSubTaskStats->GetCurrentBufferIndex());
	}
	else
	{
		m_ctlCurrentObjectProgress.SetProgress(0, 100);
		m_ctlSubTaskCountProgress.SetProgress(0, 100);
		m_ctlSubTaskSizeProgress.SetProgress(0, 100);

		GetDlgItem(IDC_SUBTASKNAME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYSUBTASKNAME_STRING));
		GetDlgItem(IDC_SUBTASKPROCESSED_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYPROCESSEDTEXT_STRING));
		GetDlgItem(IDC_SUBTASKTIME_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTIMETEXT_STRING));
		GetDlgItem(IDC_SUBTASKTRANSFER_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYTRANSFERTEXT_STRING));
		GetDlgItem(IDC_SOURCEOBJECT_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYSOURCETEXT_STRING));
		GetDlgItem(IDC_BUFFERSIZE_STATIC)->SetWindowText(GetResManager().LoadString(IDS_EMPTYBUFFERSIZETEXT_STRING));
	}

	//////////////////////////////////////////////////////
	// data that can be changed by a thread
	CString strStatusText = GetStatusString(spTaskStats);
	GetDlgItem(IDC_OPERATION_STATIC)->SetWindowText(strStatusText);	// operation

	// count of processed data/overall count of data
	CString strProcessedText = GetProcessedText(spTaskStats->GetProcessedCount(), spTaskStats->GetTotalCount(),
		spTaskStats->GetProcessedSize(), spTaskStats->GetTotalSize());
	GetDlgItem(IDC_TASKPROCESSED_STATIC)->SetWindowText(strProcessedText);

	// transfer
	CString strTaskSpeed = GetSpeedString(spTaskStats->GetSizeSpeed(), spTaskStats->GetAvgSizeSpeed(), spTaskStats->GetCountSpeed(), spTaskStats->GetAvgCountSpeed());
	GetDlgItem(IDC_TASKTRANSFER_STATIC)->SetWindowText(strTaskSpeed);

	// elapsed time / estimated total time (estimated time left)
	timeTotalEstimated = spTaskStats->GetEstimatedTotalTime();
	timeElapsed = spTaskStats->GetTimeElapsed();
	timeRemaining = timeTotalEstimated - timeElapsed;

	FormatTimeMiliseconds(timeElapsed, m_szTimeBuffer1, 40);
	FormatTimeMiliseconds(timeTotalEstimated, m_szTimeBuffer2, 40);
	FormatTimeMiliseconds(timeRemaining, m_szTimeBuffer3, 40);

	_sntprintf(m_szData, _MAX_PATH, _T("%s / %s (%s)"), m_szTimeBuffer1, m_szTimeBuffer2, m_szTimeBuffer3);
	GetDlgItem(IDC_TASKTIME_STATIC)->SetWindowText(m_szData);

	// set progress
	m_ctlTaskCountProgress.SetProgress(spTaskStats->GetProcessedCount(), spTaskStats->GetTotalCount());
	m_ctlTaskSizeProgress.SetProgress(spTaskStats->GetProcessedSize(), spTaskStats->GetTotalSize());

	GetDlgItem(IDC_DESTINATIONOBJECT_STATIC)->SetWindowText(spTaskStats->GetDestinationPath());
	GetDlgItem(IDC_THREADPRIORITY_STATIC)->SetWindowText(GetResManager().LoadString(IDS_PRIORITY0_STRING + PriorityToIndex(spTaskStats->GetThreadPriority())));
	GetDlgItem(IDC_TASKID_STATIC)->SetWindowText(spTaskStats->GetTaskName());
}

void CStatusDlg::SetWindowTitle(PCTSTR pszText)
{
	CString strCurrentTitle;
	GetWindowText(strCurrentTitle);
	if(strCurrentTitle != CString(pszText))
		SetWindowText(pszText);
}

CString CStatusDlg::GetProgressWindowTitleText() const
{
	CString strTitleText;

	if(m_spTaskMgrStats->GetTaskStatsCount() != 0)
		strTitleText.Format(_T("%s [%.0f %%]"), GetResManager().LoadString(IDS_STATUSTITLE_STRING), m_spTaskMgrStats->GetCombinedProgress() * 100.0);
	else
		strTitleText = GetResManager().LoadString(IDS_STATUSTITLE_STRING);

	return strTitleText;
}
