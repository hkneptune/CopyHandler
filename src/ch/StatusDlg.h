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

namespace chcore
{
	class TTaskManager;
	class TTask;
	typedef boost::shared_ptr<TTask> TTaskPtr;
}

#define WM_UPDATESTATUS WM_USER+6
#define WM_STATUSCLOSING WM_USER+12

/////////////////////////////////////////////////////////////////////////////
// CStatusDlg dialog
class CStatusDlg : public ictranslate::CLanguageDialog
{
	enum { IDD = IDD_STATUS_DIALOG };

// Construction
public:
	CStatusDlg(chcore::TTaskManager* pTasks, CWnd* pParent = NULL);   // standard constructor
	virtual ~CStatusDlg();

	void PostCloseMessage();
	void SetBufferSizesString(unsigned long long ullValue, int iIndex);
	void RefreshStatus();

	int GetImageFromStatus(chcore::ETaskCurrentState eState);

	void ApplyButtonsState();
	chcore::TTaskPtr GetSelectedItemPointer();
	size_t GetSelectedItemSessionUniqueID();

	void EnableControls(bool bEnable=true);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	virtual void OnLanguageChanged();

	void PrepareResizableControls();
	CString GetStatusString(const chcore::TTaskStatsSnapshotPtr& spTaskStats);

	void StickDialogToScreenEdge();

	LPTSTR FormatTime(unsigned long long timeSeconds, LPTSTR lpszBuffer, size_t stMaxBufferSize);
	LPTSTR FormatTimeMiliseconds(unsigned long long timeMiliSeconds, LPTSTR lpszBuffer, size_t stMaxBufferSize);

	CString GetProcessedText(unsigned long long ullProcessedCount, unsigned long long ullTotalCount, unsigned long long ullProcessedSize, unsigned long long ullTotalSize);
	void UpdateTaskStatsDetails(const chcore::TTaskStatsSnapshotPtr& spTaskStats);
	void SetTaskListEntry(size_t stPos, const chcore::TTaskStatsSnapshotPtr& spTaskStats);
	CString GetSubtaskName(chcore::ESubOperationType eSubtask) const;

	CString GetProgressWindowTitleText() const;
	CString GetSpeedString(double dSizeSpeed, double dAvgSizeSpeed, double dCountSpeed, double dAvgCountSpeed) const;
	void SetWindowTitle(PCTSTR pszText);

	virtual BOOL OnInitDialog();
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
	virtual void OnCancel();
	afx_msg void OnShowLogButton();
	afx_msg void OnStickButton();
	afx_msg void OnResumeButton();

	DECLARE_MESSAGE_MAP()

public:
	chcore::TTaskPtr m_spInitialSelection;

	static bool m_bLock;				// locker

protected:
	chcore::TTaskManager* m_pTasks;

	TCHAR m_szData[_MAX_PATH];
	TCHAR m_szTimeBuffer1[40];
	TCHAR m_szTimeBuffer2[40];
	TCHAR m_szTimeBuffer3[40];

	CImageList m_images;
	CFFListCtrl m_ctlStatusList;

private:
	TProgressCtrlEx	m_ctlTaskCountProgress;
	TProgressCtrlEx	m_ctlTaskSizeProgress;
	TProgressCtrlEx m_ctlCurrentObjectProgress;
	TProgressCtrlEx m_ctlSubTaskCountProgress;
	TProgressCtrlEx m_ctlSubTaskSizeProgress;
	TProgressCtrlEx	m_ctlProgressAll;

	chcore::TTaskManagerStatsSnapshotPtr m_spTaskMgrStats;
};

#endif
