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

#ifndef __MAINFRM_H__
#define __MAINFRM_H__

#include "TrayIcon.h"
#include "structs.h"
#include "MiniviewDlg.h"
#include "DataBuffer.h"
#include "StatusDlg.h"

typedef struct _CUSTOM_COPY_PARAMS
{
	CTask* pTask;			// ptr to CTask object on which we do the operation

	CFileInfo* pfiSrcFile;	// CFileInfo - src file
	CString strDstFile;		// dest path with filename

	CDataBuffer dbBuffer;	// buffer handling
	bool bOnlyCreate;		// flag from configuration - skips real copying - only create
} CUSTOM_COPY_PARAMS, *PCUSTOM_COPY_PARAMS;

class CMainWnd : public CWnd
{
public:
	CMainWnd();
	DECLARE_DYNCREATE(CMainWnd)

	BOOL Create();

// Attributes
public:
	CTrayIcon m_ctlTray;
	CLIPBOARDMONITORDATA cmd;
	
	CTaskArray m_tasks;

	CMiniViewDlg* m_pdlgMiniView;
	CStatusDlg* m_pdlgStatus;

	DWORD m_dwLastTime;
	UINT m_uiTaskbarRestart;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainWnd();

// Generated message map functions
protected:
	ATOM RegisterClass();
	int ShowTrayIcon();
	void ShowStatusWindow(const CTask* pSelect=NULL);
	void PrepareToExit();
	//{{AFX_MSG(CMainWnd)
	afx_msg void OnPopupShowStatus();
	afx_msg void OnPopupShowOptions();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT nIDEvent);
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
	//}}AFX_MSG
	afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif