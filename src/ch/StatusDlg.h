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
#ifndef __STATUSDLG_H__
#define __STATUSDLG_H__

#include "FFListCtrl.h"
#include "TProgressCtrlEx.h"
#include "TExplorerTaskBarProgress.h"
#include "../libchengine/TTask.h"
#include "../libchengine/TTaskManager.h"
#include "TaskContextMenu.h"

namespace chengine {
	class TTaskManager;
}

#define WM_UPDATESTATUS WM_USER+6
#define WM_STATUSCLOSING WM_USER+12

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog
class CStatusDlg : public ictranslate::CLanguageDialog
{
// Construction
public:
	explicit CStatusDlg(chengine::TTaskManager* pTasks, CWnd* pParent = nullptr);   // standard constructor
	virtual ~CStatusDlg();

	void PostCloseMessage();
	void SetBufferSizesString(unsigned long long ullValue, int iIndex);
	void RefreshStatus();

	int GetImageFromStatus(chengine::ETaskCurrentState eState);

	void ApplyButtonsState();
	chengine::TTaskPtr GetSelectedItemPointer();
	size_t GetSelectedItemSessionUniqueID();

	void EnableControls(bool bEnable=true);

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	BOOL OnCommand(WPARAM wParam, LPARAM lParam) override;

	void OnResetUserFeedback();

	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

	void OnLanguageChanged() override;

	void PrepareResizableControls();
	CString GetStatusString(const chengine::TTaskStatsSnapshotPtr& spTaskStats);

	void StickDialogToScreenEdge();

	CString FormatTime(unsigned long long timeSeconds);
	CString FormatTimeMiliseconds(unsigned long long timeMiliSeconds);

	CString GetProcessedText(unsigned long long ullProcessedCount, unsigned long long ullTotalCount, unsigned long long ullProcessedSize, unsigned long long ullTotalSize);
	void UpdateTaskStatsDetails(const chengine::TTaskStatsSnapshotPtr& spTaskStats);
	void SetTaskListEntry(size_t stPos, const chengine::TTaskStatsSnapshotPtr& spTaskStats);
	CString GetSubtaskName(chengine::EOperationType eOperation, chengine::ESubOperationType eSubtask) const;

	CString GetProgressWindowTitleText() const;
	void UpdateTaskBarProgress() const;

	CString GetSpeedString(double dSizeSpeed, double dAvgSizeSpeed, double dCountSpeed, double dAvgCountSpeed) const;
	void SetWindowTitle(PCTSTR pszText);

	BOOL OnInitDialog() override;

	void SelectInitialTask();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnPauseButton();
	afx_msg void OnCancelButton();
	afx_msg void OnSetPriorityButton();
	afx_msg void OnSetBuffersizeButton();
	afx_msg void OnStartAllButton();
	afx_msg void OnRestartButton();
	afx_msg void OnDeleteButton();
	afx_msg void OnPauseAllButton();
	afx_msg void OnRestartAllButton();
	afx_msg void OnCancelAllButton();
	afx_msg void OnRemoveFinishedButton();
	afx_msg void OnKeydownStatusList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/);
	afx_msg void OnStatusListRClick(NMHDR* pNMHDR, LRESULT* pResult);
	void OnCancel() override;
	afx_msg void OnShowLogButton();
	afx_msg void OnStickButton();
	afx_msg void OnResumeButton();

	DECLARE_MESSAGE_MAP()

public:
	chengine::TTaskPtr m_spInitialSelection;

	static bool m_bLock;				// locker

protected:
	chengine::TTaskManager* m_pTasks;

	CImageList m_images;
	CFFListCtrl m_ctlStatusList;

	TProgressCtrlEx	m_ctlTaskCountProgress;
	TProgressCtrlEx	m_ctlTaskSizeProgress;
	TProgressCtrlEx m_ctlCurrentObjectProgress;
	TProgressCtrlEx m_ctlSubTaskCountProgress;
	TProgressCtrlEx m_ctlSubTaskSizeProgress;
	TProgressCtrlEx	m_ctlProgressAll;

	TaskContextMenu m_menuContext;
	chengine::taskid_t m_currentTaskId = chengine::NoTaskID;

	chengine::TTaskManagerStatsSnapshotPtr m_spTaskMgrStats;
	TExplorerTaskBarProgress m_taskBarProgress;
};

#endif
