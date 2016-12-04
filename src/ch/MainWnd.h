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

#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#include "TrayIcon.h"
#include "../libchcore/TTaskManager.h"
#include "../libchcore/TSharedMemory.h"

class TShellExtMenuConfig;
class CMiniViewDlg;
class CStatusDlg;

class CMainWnd : public CWnd
{
public:
	CMainWnd();
	virtual ~CMainWnd();

	DECLARE_DYNCREATE(CMainWnd)

	BOOL Create();

protected:
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

	void PrepareDragAndDropMenuItems(TShellExtMenuConfig &cfgShellExt) const;
	void PrepareNormalMenuItems(TShellExtMenuConfig &cfgShellExt) const;

	BOOL RegisterClass();
	int ShowTrayIcon();
	void ShowStatusWindow(const chcore::TTaskPtr& spSelect = chcore::TTaskPtr());
	void PrepareToExit();

	void ProcessCommandLine(const TCommandLineParser& rCommandLine);

	CString GetTasksDirectory() const;

	void SetupTimers();
	void CheckForUpdates();
	bool LoadTaskManager();

	afx_msg void OnPopupShowStatus();
	afx_msg void OnPopupShowOptions();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowMiniView();
	afx_msg void OnPopupCustomCopy();
	afx_msg void OnAppAbout();
	afx_msg void OnPopupMonitoring();
	afx_msg void OnPopupShutafterfinished();
	afx_msg void OnPopupRegisterdll();
	afx_msg void OnPopupUnregisterdll();
	afx_msg void OnAppExit();
	afx_msg void OnPopupHelp();
	afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPopupCheckForUpdates();

	DECLARE_MESSAGE_MAP()

private:
	chcore::TTaskManagerStatsSnapshotPtr m_spTaskMgrStats;

	CTrayIcon m_ctlTray;

	chcore::TTaskManagerPtr m_spTasks;
	chcore::TSharedMemory m_tCHExtharedMemory;

	CMiniViewDlg* m_pdlgMiniView = nullptr;
	CStatusDlg* m_pdlgStatus = nullptr;

	DWORD m_dwLastTime = 0;
	UINT m_uiTaskbarRestart = 0;

	logger::TLoggerPtr m_spLog;
};

#endif
