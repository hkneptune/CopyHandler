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
#pragma once

/////////////////////////////////////////////////////////////////////////////
// CMiniViewDlg dialog
#include "ProgressListBox.h"
#include <array>
#include "TaskContextMenu.h"

namespace chengine {
	class TTaskManager;
}

namespace chcore
{
	class TTaskManager;
}

#define WM_MINIVIEWDBLCLK		WM_USER+14

class CMiniViewDlg : public ictranslate::CLanguageDialog
{
public:
	CMiniViewDlg(chengine::TTaskManager* pArray, bool* pbHide, CWnd* pParent = nullptr);   // standard constructor

	void ShowWindow();
	void HideWindow();
	void ResizeDialog();

	void RefreshStatus();
	void RecalcSize(int nHeight, bool bInitial);

	UINT GetLanguageUpdateOptions() override { return LDF_NODIALOGSIZE; }
	void OnLanguageChanged() override;

protected:
	void DoDataExchange(CDataExchange* pDX) override;    // DDX/DDV support
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam) override;

	BOOL OnInitDialog() override;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnSelchangeProgressList();
	afx_msg void OnSetfocusProgressList();
	afx_msg void OnSelcancelProgressList();
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg void OnDblclkProgressList();
	afx_msg LRESULT OnTaskRClick(WPARAM wParam, LPARAM lParam);

	void ExecTaskCommand(int idCmd);
	void OnEditUserFeedback();

	DECLARE_MESSAGE_MAP()

public:
	static bool m_bLock;

private:
	// from CMainWnd
	chengine::TTaskManager *m_pTasks;

	int m_iLastHeight;
	bool m_bShown;

	// cache
	bool m_bActive;

	// lock
	bool *m_pbHide;		// is the big status dialog visible ?

	// in onmousemove points to last pressed button
	int m_iIndex;

	CProgressListBox m_ctlStatus;
	TaskContextMenu m_menuContext;
	chengine::taskid_t m_currentTaskId = chengine::NoTaskID;
};
