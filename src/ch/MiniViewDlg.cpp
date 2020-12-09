// ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#include "stdafx.h"
#include "MiniViewDlg.h"
#include "ch.h"
#include <assert.h>
#include "CfgProperties.h"
#include "resource.h"
#include "../libchengine/TTaskManager.h"
#include "../libchengine/TTask.h"
#include "GuiOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define WM_INITDATA				WM_USER+5

using namespace chengine;

bool CMiniViewDlg::m_bLock=false;

/////////////////////////////////////////////////////////////////////////////
// CMiniViewDlg dialog

CMiniViewDlg::CMiniViewDlg(chengine::TTaskManager* pTaskManager, bool *pbHide, CWnd* pParent /*=nullptr*/)
	:ictranslate::CLanguageDialog(IDD_MINIVIEW_DIALOG, pParent, &m_bLock),
	m_pTasks(pTaskManager),
	m_iLastHeight(0),
	m_bShown(false),
	m_bActive(false),
	m_pbHide(pbHide),
	m_iIndex(-1)
{
}

void CMiniViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CLanguageDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_LIST, m_ctlStatus);
}

BEGIN_MESSAGE_MAP(CMiniViewDlg,ictranslate::CLanguageDialog)
	ON_WM_TIMER()
	ON_LBN_SELCHANGE(IDC_PROGRESS_LIST, OnSelchangeProgressList)
	ON_LBN_SETFOCUS(IDC_PROGRESS_LIST, OnSetfocusProgressList)
	ON_LBN_SELCANCEL(IDC_PROGRESS_LIST, OnSelcancelProgressList)
	ON_WM_SETTINGCHANGE()
	ON_LBN_DBLCLK(IDC_PROGRESS_LIST, OnDblclkProgressList)
	ON_MESSAGE(WM_TASK_RCLICK, OnTaskRClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMiniViewDlg message handlers

BOOL CMiniViewDlg::OnInitDialog() 
{
	CLanguageDialog::OnInitDialog();

	m_menuContext.Load();

	ResizeDialog();
	PostMessage(WM_INITDATA);

	return TRUE;
}

void CMiniViewDlg::OnTimer(UINT_PTR nIDEvent) 
{
	if (nIDEvent == 9843)
	{
		KillTimer(9843);

		RefreshStatus();

		SetTimer(9843, GetPropValue<PP_MVREFRESHINTERVAL>(GetConfig()), nullptr);
	}

	CLanguageDialog::OnTimer(nIDEvent);
}

void CMiniViewDlg::RecalcSize(int nHeight, bool bInitial)
{
	CRect rcMargins(0, 0, 5, 3);

	MapDialogRect(&rcMargins);

	const int XMargin = rcMargins.Width();
	const int YMargin = rcMargins.Height();

	// set listbox size
	CRect rcList;
	m_ctlStatus.GetWindowRect(&rcList);

	if (nHeight == 0)
		nHeight = rcList.Height();
	
	// don't do anything if height doesn't changed
	if (nHeight == m_iLastHeight && !bInitial)
		return;

	// remember height
	m_iLastHeight = nHeight;

	CRect rcNewDlgPos(rcList);
	rcNewDlgPos.InflateRect(XMargin, YMargin);

	CRect rcNewDlgPosClient = rcNewDlgPos;

	AdjustWindowRectEx(&rcNewDlgPos, GetStyle(), FALSE, GetWindowLong(GetSafeHwnd(), GWL_EXSTYLE));

	// use dynamic margin
	int iWidth = rcNewDlgPos.Width();
	int iHeight = rcNewDlgPos.Height();

	// place listbox in the best place
	m_ctlStatus.SetWindowPos(nullptr, XMargin, YMargin / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	// size of a dialog and screen
	CRect rcDialog, rcScreen;
	GetWindowRect(&rcDialog);
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, 0);

	bool bIsGluedToScreenEdge = (rcDialog.left == rcScreen.right-rcDialog.Width()
		&& rcDialog.top == rcScreen.bottom-rcDialog.Height());

	if (bInitial || bIsGluedToScreenEdge)
		SetWindowPos(&wndTopMost, rcScreen.right - iWidth, rcScreen.bottom - iHeight, iWidth, iHeight, 0);
	else
		SetWindowPos(&wndTopMost, 0, 0, iWidth, iHeight, SWP_NOMOVE);
}

void CMiniViewDlg::RefreshStatus()
{
	if(!m_pTasks)
		return;

	chengine::TTaskManagerStatsSnapshotPtr spTaskMgrStats(new chengine::TTaskManagerStatsSnapshot);

	m_pTasks->GetStatsSnapshot(spTaskMgrStats);

	int index=0;
	_PROGRESSITEM_* pItem=nullptr;

	if(GetPropValue<PP_MVSHOWSINGLETASKS>(GetConfig()))
	{
		size_t stTasksCount = spTaskMgrStats->GetTaskStatsCount();
		for(size_t stIndex = 0; stIndex < stTasksCount; ++stIndex)
		{
			chengine::TTaskStatsSnapshotPtr spTaskStats = spTaskMgrStats->GetTaskStatsAt(stIndex);
			chengine::ETaskCurrentState eTaskState = spTaskStats->GetTaskState();

			if(eTaskState != chengine::eTaskState_Finished && eTaskState != chengine::eTaskState_Cancelled && eTaskState != chengine::eTaskState_LoadError)
			{
				pItem = m_ctlStatus.GetItemAddress(index++);

				// load
				if(eTaskState == chengine::eTaskState_Error)
					pItem->m_crColor=RGB(255, 0, 0);
				else if(eTaskState == chengine::eTaskState_Paused)
					pItem->m_crColor=RGB(255, 201, 14);
				else if(eTaskState == chengine::eTaskState_Waiting)
					pItem->m_crColor=RGB(0, 0, 255);
				else
					pItem->m_crColor=RGB(0, 255, 0);

				string::TString strPath;
				chengine::TSubTaskStatsSnapshotPtr spSubtaskStats = spTaskStats->GetSubTasksStats().GetCurrentSubTaskSnapshot();
				if(spSubtaskStats)
					strPath = spSubtaskStats->GetCurrentPath();

				if(strPath.IsEmpty())
					strPath = GetResManager().LoadString(IDS_NONEINPUTFILE_STRING);

				pItem->m_strText = strPath.c_str();
				pItem->m_uiPos = boost::numeric_cast<int>(spTaskStats->GetCombinedProgress() * 100.0);
				pItem->m_tTaskID = spTaskStats->GetTaskID();
			}
		}
	}

	// should we show ?
	bool bInitial=false;
	if (index == 0)
	{
		if (m_bShown)
		{
			if (GetPropValue<PP_MVAUTOHIDEWHENEMPTY>(GetConfig()) || *m_pbHide)
				HideWindow();
		}
		else if (!GetPropValue<PP_MVAUTOHIDEWHENEMPTY>(GetConfig()) && !(*m_pbHide))
		{
			// need to be visible
			ShowWindow();
			bInitial=true;
		}
	}
	else
	{
		if (!m_bShown)
		{
			if (!(*m_pbHide))
			{
				ShowWindow();
				bInitial=true;
			}
		}
		else
		{
			if (*m_pbHide)
				HideWindow();
		}
	}

	// add all state
	pItem=m_ctlStatus.GetItemAddress(index++);
	pItem->m_crColor=GetSysColor(COLOR_HIGHLIGHT);
	pItem->m_strText=GetResManager().LoadString(IDS_MINIVIEWALL_STRING);
	pItem->m_uiPos = boost::numeric_cast<int>(spTaskMgrStats->GetCombinedProgress() * 100.0);
	pItem->m_tTaskID = chengine::NoTaskID;

	// get rid of the rest
	m_ctlStatus.SetSmoothProgress(GetPropValue<PP_MVUSESMOOTHPROGRESS>(GetConfig()));
	m_ctlStatus.UpdateItems(index, true);
	
	m_ctlStatus.SetShowCaptions(GetPropValue<PP_MVSHOWFILENAMES>(GetConfig()));

	// calc size
	RecalcSize(0, bInitial);
}

LRESULT CMiniViewDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message == WM_INITDATA)
	{
		// listbox with progress pseudocontrols
		m_ctlStatus.Init();
		
		// refresh
		RefreshStatus();
		
		// set refresh timer
		SetTimer(9843, GetPropValue<PP_MVREFRESHINTERVAL>(GetConfig()), nullptr);

		return static_cast<LRESULT>(0);
	}

	return ictranslate::CLanguageDialog::WindowProc(message, wParam, lParam);
}

void CMiniViewDlg::OnSelchangeProgressList() 
{
	RefreshStatus();
	RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME);
//	PostMessage(WM_NCPAINT);
}

void CMiniViewDlg::OnSetfocusProgressList() 
{
	RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME);
//	PostMessage(WM_NCPAINT);	
}

void CMiniViewDlg::OnSelcancelProgressList() 
{
	RefreshStatus();
	RedrawWindow(nullptr, nullptr, RDW_INVALIDATE | RDW_FRAME);
//	PostMessage(WM_NCPAINT);	
}

void CMiniViewDlg::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) 
{
	CLanguageDialog::OnSettingChange(uFlags, lpszSection);

	if (uFlags == SPI_SETNONCLIENTMETRICS)
		ResizeDialog();
}

void CMiniViewDlg::ResizeDialog()
{
	if(!IsWindowVisible())
		return;

	RecalcSize(0, true);
}

void CMiniViewDlg::HideWindow()
{
	static_cast<CLanguageDialog*>(this)->ShowWindow(SW_HIDE);
	m_bShown=false;
}

void CMiniViewDlg::ShowWindow()
{
	static_cast<CLanguageDialog*>(this)->ShowWindow(SW_SHOW);
	m_bShown=true;
}

void CMiniViewDlg::OnDblclkProgressList() 
{
	chengine::taskid_t tTaskID = m_ctlStatus.GetSelectedTaskId();
	GetParent()->PostMessage(WM_MINIVIEWDBLCLK, 0, boost::numeric_cast<LPARAM>(tTaskID));
}

LRESULT CMiniViewDlg::OnTaskRClick(WPARAM /*wParam*/, LPARAM lParam)
{
	TASK_CLICK_NOTIFICATION* pNotify = (TASK_CLICK_NOTIFICATION*)lParam;
	m_currentTaskId = pNotify->taskId;

	ETaskCurrentState eState = eTaskState_None;

	TTaskPtr spTask = m_pTasks->GetTaskByTaskID(m_currentTaskId);
	if(spTask)
		eState = spTask->GetTaskState();
	spTask.reset();

	int iMenuItem = m_menuContext.TrackPopupMenu(eState, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, pNotify->point.x, pNotify->point.y, this);
	switch(iMenuItem)
	{
	case ID_TASK_MENU_PAUSE:
	case ID_TASK_MENU_RESUME:
	case ID_TASK_MENU_RESTART:
	case ID_TASK_MENU_CANCEL:
	case ID_TASK_MENU_REMOVE:
	case ID_TASK_MENU_RESET_FEEDBACK:
		ExecTaskCommand(iMenuItem);
		break;

	case ID_TASK_MENU_PAUSE_ALL:
		m_pTasks->TasksPauseProcessing();
		break;
	case ID_TASK_MENU_RESUME_ALL:
		m_pTasks->TasksResumeProcessing();
		break;
	case ID_TASK_MENU_RESTART_ALL:
		m_pTasks->TasksRestartProcessing();
		break;
	case ID_TASK_MENU_CANCEL_ALL:
		m_pTasks->TasksCancelProcessing();
		break;
	case ID_TASK_MENU_REMOVE_INACTIVE:
		m_pTasks->RemoveAllFinished();
		break;
	}

	return 0;
}

void CMiniViewDlg::ExecTaskCommand(int idCmd)
{
	TTaskPtr spTask = m_pTasks->GetTaskByTaskID(m_currentTaskId);
	if(!spTask)
		return;

	ETaskCurrentState eState = spTask->GetTaskState();

	switch(idCmd)
	{
	case ID_TASK_MENU_PAUSE:
		spTask->PauseProcessing();
		break;
	case ID_TASK_MENU_RESUME:
		if(eState == chengine::eTaskState_Waiting)
			spTask->SetForceFlag(true);
		else
			spTask->ResumeProcessing();
		break;
	case ID_TASK_MENU_RESTART:
		spTask->RestartProcessing();
		break;
	case ID_TASK_MENU_CANCEL:
		spTask->CancelProcessing();
		break;
	case ID_TASK_MENU_REMOVE:
	{
		switch(eState)
		{
		case chengine::eTaskState_Finished:
		case chengine::eTaskState_Cancelled:
		case chengine::eTaskState_LoadError:
			break;

		default:
			if(MsgBox(IDS_CONFIRMCANCEL_STRING, MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
				spTask->CancelProcessing();
			else
				return;
		}

		m_pTasks->RemoveFinished(spTask);
		break;
	}
	case ID_TASK_MENU_RESET_FEEDBACK:
		spTask->RestoreFeedbackDefaults();
		break;
	}
}

void CMiniViewDlg::OnLanguageChanged()
{
	ResizeDialog();
}
