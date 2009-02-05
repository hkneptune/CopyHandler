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
#include "ch.h"

#include "MainWnd.h"
#include "OptionsDlg.h"

#include "shlobj.h"
#include "tchar.h"
#include "structs.h"
#include "dialogs.h"

#include "FolderDialog.h"

#include "CustomCopyDlg.h"
#include "btnIDs.h"
#include "..\Common\FileSupport.h"
#include "AboutDlg.h"
#include "register.h"
#include "ShutdownDlg.h"
#include "StringHelpers.h"
#include "..\common\ipcstructs.h"
#include <assert.h>
#include "af_defs.h"
#include "UpdateChecker.h"
#include "UpdaterDlg.h"
#include <boost/assert.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	WM_ICON_NOTIFY			WM_USER+4
#define WM_SHOWMINIVIEW			WM_USER+3
#define WM_IDENTIFY				WM_USER+11

#define TM_AUTOREMOVE			1000
#define TM_AUTORESUME			1000
#define TM_ACCEPTING			100

extern CSharedConfigStruct* g_pscsShared;

extern int iCount;
extern unsigned short msg[];

extern int iOffCount;
extern unsigned char off[];
extern unsigned short _hash[];


/////////////////////////////////////////////////////////////////////////////
// CMainWnd
// registers main window class
ATOM CMainWnd::RegisterClass()
{
	WNDCLASS wc;

	wc.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC)::DefWindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= AfxGetInstanceHandle();
	wc.hIcon			= ::LoadIcon(NULL, MAKEINTRESOURCE(AFX_IDI_STD_FRAME));
	wc.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= _T("Copy Handler Wnd Class");

	return ::RegisterClass(&wc);
}

// creates this window
BOOL CMainWnd::Create()
{
	ATOM at=RegisterClass();

	return CreateEx(WS_EX_TOOLWINDOW, (LPCTSTR)at, _T("Copy Handler"), WS_OVERLAPPED, 10, 10, 10, 10, NULL, (HMENU)NULL, NULL);
}

int CMainWnd::ShowTrayIcon()
{
	// create system tray icon
	HICON hIcon=(HICON)GetResManager().LoadImage(MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_VGACOLOR);
	PCTSTR pszAppVer = GetApp().GetAppNameVer();
	bool bRes=m_ctlTray.CreateIcon(m_hWnd, WM_TRAYNOTIFY, pszAppVer, hIcon, 0);
	if (!bRes)
	{
//		GetLog()->Log(_T("[CMainWnd] ... creating tray icon failed."));
		return -1;
	}

/*	if (!m_ctlTray.ShowIcon())
		GetLog()->Log(_T("[CMainWnd] ... showing tray icon failed."));
	else
		GetLog()->Log(_T("[CMainWnd] ... showing tray icon succeeded."));
*/
	return 0;
}

IMPLEMENT_DYNCREATE(CMainWnd, CWnd)

BEGIN_MESSAGE_MAP(CMainWnd, CWnd)
	//{{AFX_MSG_MAP(CMainWnd)
	ON_COMMAND(ID_POPUP_SHOW_STATUS, OnPopupShowStatus)
	ON_COMMAND(ID_POPUP_OPTIONS, OnPopupShowOptions)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_COPYDATA()
	ON_WM_CREATE()
	ON_COMMAND(ID_SHOW_MINI_VIEW, OnShowMiniView)
	ON_COMMAND(ID_POPUP_CUSTOM_COPY, OnPopupCustomCopy)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_POPUP_MONITORING, OnPopupMonitoring)
	ON_COMMAND(ID_POPUP_SHUTAFTERFINISHED, OnPopupShutafterfinished)
	ON_COMMAND(ID_POPUP_REGISTERDLL, OnPopupRegisterdll)
	ON_COMMAND(ID_POPUP_UNREGISTERDLL, OnPopupUnregisterdll)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_POPUP_HELP, OnPopupHelp)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
	ON_COMMAND(ID_POPUP_CHECKFORUPDATES, &CMainWnd::OnPopupCheckForUpdates)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainWnd construction/destruction

CMainWnd::CMainWnd() :
	m_pFeedbackFactory(CFeedbackHandlerFactory::CreateFactory())
{
	m_pdlgStatus=NULL;
	m_pdlgMiniView=NULL;
	m_dwLastTime=0;
}

CMainWnd::~CMainWnd()
{
	if(m_pFeedbackFactory)
		m_pFeedbackFactory->Delete();
}

UINT ClipboardMonitorProc(LPVOID pParam)
{
	volatile CLIPBOARDMONITORDATA* pData=static_cast<CLIPBOARDMONITORDATA *>(pParam);
	ASSERT(pData->m_hwnd);

	// bufor
	TCHAR path[_MAX_PATH];
//	UINT i;	// counter
	CTask *pTask;	// ptr to a task
	CClipboardEntry* pEntry=NULL;

	// register clipboard format
	UINT nFormat=RegisterClipboardFormat(_T("Preferred DropEffect"));
	UINT uiCounter=0, uiShutCounter=0;;
	LONG lFinished=0;
	bool bEnd=false;

	icpf::config& rConfig = GetConfig();
	while (!pData->bKill)
	{
		if (uiCounter == 0 && rConfig.get_bool(PP_PCLIPBOARDMONITORING) && IsClipboardFormatAvailable(CF_HDROP))
		{
			// get data from clipboard
			OpenClipboard(pData->m_hwnd);
			HANDLE handle=GetClipboardData(CF_HDROP);

			UINT nCount=DragQueryFile(static_cast<HDROP>(handle), 0xffffffff, NULL, 0);

			pTask = pData->m_pTasks->CreateTask();

			for (UINT i=0;i<nCount;i++)
			{
				DragQueryFile(static_cast<HDROP>(handle), i, path, _MAX_PATH);
				pEntry=new CClipboardEntry;
				pEntry->SetPath(path);
				pTask->AddClipboardData(pEntry);
			}
			
			if (IsClipboardFormatAvailable(nFormat))
			{
				handle=GetClipboardData(nFormat);
				LPVOID addr=GlobalLock(handle);
				
				DWORD dwData=((DWORD*)addr)[0];
				if (dwData & DROPEFFECT_COPY)
					pTask->SetStatus(ST_COPY, ST_OPERATION_MASK);	// copy
				else if (dwData & DROPEFFECT_MOVE)
					pTask->SetStatus(ST_MOVE, ST_OPERATION_MASK);	// move

				GlobalUnlock(handle);
			}
			else
				pTask->SetStatus(ST_COPY, ST_OPERATION_MASK);	// default - copy

			EmptyClipboard();
			CloseClipboard();
			
			BUFFERSIZES bs;
			bs.m_bOnlyDefault=rConfig.get_bool(PP_BFUSEONLYDEFAULT);
			bs.m_uiDefaultSize=(UINT)rConfig.get_signed_num(PP_BFDEFAULT);
			bs.m_uiOneDiskSize=(UINT)rConfig.get_signed_num(PP_BFONEDISK);
			bs.m_uiTwoDisksSize=(UINT)rConfig.get_signed_num(PP_BFTWODISKS);
			bs.m_uiCDSize=(UINT)rConfig.get_signed_num(PP_BFCD);
			bs.m_uiLANSize=(UINT)rConfig.get_signed_num(PP_BFLAN);

			pTask->SetBufferSizes(&bs);
			pTask->SetPriority((int)rConfig.get_signed_num(PP_CMDEFAULTPRIORITY));

			// get dest folder
			CFolderDialog dlg;

			const tchar_t* pszPath = NULL;
			dlg.m_bdData.cvShortcuts.clear(true);
			size_t stCount = rConfig.get_value_count(PP_SHORTCUTS);
			for(size_t stIndex = 0; stIndex < stCount; stIndex++)
			{
				pszPath = rConfig.get_string(PP_SHORTCUTS, stIndex);
				dlg.m_bdData.cvShortcuts.push_back(pszPath);
			}

			dlg.m_bdData.cvRecent.clear(true);
			stCount = rConfig.get_value_count(PP_RECENTPATHS);
			for(size_t stIndex = 0; stIndex < stCount; stIndex++)
			{
				pszPath = rConfig.get_string(PP_RECENTPATHS, stIndex);
					dlg.m_bdData.cvRecent.push_back(pszPath);
			}

			dlg.m_bdData.bExtended=rConfig.get_bool(PP_FDEXTENDEDVIEW);
			dlg.m_bdData.cx=(int)rConfig.get_signed_num(PP_FDWIDTH);
			dlg.m_bdData.cy=(int)rConfig.get_signed_num(PP_FDHEIGHT);
			dlg.m_bdData.iView=(int)rConfig.get_signed_num(PP_FDSHORTCUTLISTSTYLE);
			dlg.m_bdData.bIgnoreDialogs=rConfig.get_bool(PP_FDIGNORESHELLDIALOGS);

			dlg.m_bdData.strInitialDir=(dlg.m_bdData.cvRecent.size() > 0) ? dlg.m_bdData.cvRecent.at(0) : _T("");

			int iStatus=pTask->GetStatus(ST_OPERATION_MASK);
			if (iStatus == ST_COPY)
				dlg.m_bdData.strCaption=GetResManager().LoadString(IDS_TITLECOPY_STRING);
			else if (iStatus == ST_MOVE)
				dlg.m_bdData.strCaption=GetResManager().LoadString(IDS_TITLEMOVE_STRING);
			else
				dlg.m_bdData.strCaption=GetResManager().LoadString(IDS_TITLEUNKNOWNOPERATION_STRING);
			dlg.m_bdData.strText=GetResManager().LoadString(IDS_MAINBROWSETEXT_STRING);
			
			// set count of data to display
			int iClipboardSize=pTask->GetClipboardDataSize();
			int iEntries=(iClipboardSize > 3) ? 2 : iClipboardSize;
			for (int i=0;i<iEntries;i++)
				dlg.m_bdData.strText+=pTask->GetClipboardData(i)->GetPath()+_T("\n");

			// add ...
			if (iEntries < iClipboardSize)
				dlg.m_bdData.strText+=_T("...");

			// show window
			int iResult=dlg.DoModal();

			// set data to config
			rConfig.clear_array_values(PP_SHORTCUTS);
			for(char_vector::iterator it = dlg.m_bdData.cvShortcuts.begin(); it != dlg.m_bdData.cvShortcuts.end(); it++)
			{
				rConfig.set_string(PP_SHORTCUTS, (*it), icpf::property::action_add);
			}

			rConfig.clear_array_values(PP_RECENTPATHS);
			for(char_vector::iterator it = dlg.m_bdData.cvRecent.begin(); it != dlg.m_bdData.cvRecent.end(); it++)
			{
				rConfig.set_string(PP_RECENTPATHS, (*it), icpf::property::action_add);
			}

			rConfig.set_bool(PP_FDEXTENDEDVIEW, dlg.m_bdData.bExtended);
			rConfig.set_signed_num(PP_FDWIDTH, dlg.m_bdData.cx);
			rConfig.set_signed_num(PP_FDHEIGHT, dlg.m_bdData.cy);
			rConfig.set_signed_num(PP_FDSHORTCUTLISTSTYLE, dlg.m_bdData.iView);
			rConfig.set_bool(PP_FDIGNORESHELLDIALOGS, dlg.m_bdData.bIgnoreDialogs);
			rConfig.write(NULL);

			if ( iResult != IDOK )
				delete pTask;
			else
			{
				// get dest path
				CString strData;
				dlg.GetPath(strData);
				pTask->SetDestPath(strData);

				// get the relationship between src and dst paths
				for (int i=0;i<pTask->GetClipboard()->GetSize();i++)
					pTask->GetClipboard()->GetAt(i)->CalcBufferIndex(pTask->GetDestPath());

				// add task to a list of tasks and start
				pData->m_pTasks->Add(pTask);

				// write pTask to a file
				pTask->Store(true);
				pTask->Store(false);
				
				// start processing
				pTask->BeginProcessing();
			}
		}
		
		// do we need to check for turning computer off
		if (GetConfig().get_bool(PP_PSHUTDOWNAFTREFINISHED))
		{
			if (uiShutCounter == 0)
			{
				if (lFinished != pData->m_pTasks->m_lFinished)
				{
					bEnd=true;
					lFinished=pData->m_pTasks->m_lFinished;
				}
				
				if (bEnd && pData->m_pTasks->IsFinished())
				{
					TRACE("Shut down windows\n");
					bool bShutdown=true;
					if (GetConfig().get_signed_num(PP_PTIMEBEFORESHUTDOWN) != 0)
					{
						CShutdownDlg dlg;
						dlg.m_iOverallTime=(int)GetConfig().get_signed_num(PP_PTIMEBEFORESHUTDOWN);
						if (dlg.m_iOverallTime < 0)
							dlg.m_iOverallTime=-dlg.m_iOverallTime;
						bShutdown=(dlg.DoModal() != IDCANCEL);
					}
					
					GetConfig().set_bool(PP_PSHUTDOWNAFTREFINISHED, false);
					GetConfig().write(NULL);
					if (bShutdown)
					{
						// we're killed
						pData->bKilled=true;
						
						// adjust token privileges for NT
						HANDLE hToken=NULL;
						TOKEN_PRIVILEGES tp;
						if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)
							&& LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid))
						{
							tp.PrivilegeCount=1;
							tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
							
							AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, NULL, NULL);
						}
						
						BOOL bExit=ExitWindowsEx(EWX_POWEROFF | EWX_SHUTDOWN | (GetConfig().get_bool(PP_PFORCESHUTDOWN) ? EWX_FORCE : 0), 0);
						if (bExit)
							return 1;
						else
						{
							pData->bKilled=false;
							
							// some kind of error
							ictranslate::CFormat fmt(GetResManager().LoadString(IDS_SHUTDOWNERROR_STRING));
							fmt.SetParam(_t("%errno"), GetLastError());
							AfxMessageBox(fmt, MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
						}
					}
				}
			}
		}
		else
		{
			bEnd=false;
			lFinished=pData->m_pTasks->m_lFinished;
		}
		
		// sleep for some time
		const int iSleepCount=200;
		Sleep(iSleepCount);
		uiCounter+=iSleepCount;
		uiShutCounter+=iSleepCount;
		if (uiCounter >= (UINT)GetConfig().get_signed_num(PP_PMONITORSCANINTERVAL))
			uiCounter=0;
		if (uiShutCounter >= 800)
			uiShutCounter=0;
	}

	pData->bKilled=true;
	TRACE("Monitoring clipboard proc aborted...\n");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd message handlers

int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->dwExStyle |= WS_EX_TOPMOST;
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// get msg id of taskbar created message
	m_uiTaskbarRestart=RegisterWindowMessage(_T("TaskbarCreated"));

	// Create the tray icon
	ShowTrayIcon();

	// initialize CTaskArray
	m_tasks.Create(m_pFeedbackFactory);

	// load last state
	CString strPath;
	GetApp().GetProgramDataPath(strPath);
	strPath += _T("\\tasks");
	m_tasks.SetTasksDir(strPath);
	m_tasks.LoadDataProgress();
	m_tasks.TasksRetryProcessing();

	// start clipboard monitoring
	cmd.bKill=false;
	cmd.bKilled=false;
	cmd.m_hwnd=this->m_hWnd;
	cmd.m_pTasks=&m_tasks;

	AfxBeginThread(&ClipboardMonitorProc, static_cast<LPVOID>(&cmd), THREAD_PRIORITY_IDLE);
	
	// start saving timer
	SetTimer(1023, (UINT)GetConfig().get_signed_num(PP_PAUTOSAVEINTERVAL), NULL);

	SetTimer(7834, TM_AUTORESUME, NULL);
	SetTimer(3245, TM_AUTOREMOVE, NULL);
	SetTimer(8743, TM_ACCEPTING, NULL);		// ends wait state in tasks

	if (GetConfig().get_bool(PP_MVAUTOSHOWWHENRUN))
		PostMessage(WM_SHOWMINIVIEW);

	return 0;
}

LRESULT CMainWnd::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	if (wParam != m_ctlTray.m_tnd.uID)
		return (LRESULT)FALSE;

	TCHAR text[_MAX_PATH];
	switch(LOWORD(lParam))
	{
	case WM_LBUTTONDOWN:
		{
			::SetForegroundWindow(this->m_hWnd);
			break;
		}
	case WM_LBUTTONDBLCLK:
		{
			CMenu mMenu, *pSubMenu;
			HMENU hMenu=GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_POPUP_MENU));
			if (!mMenu.Attach(hMenu))
				return (LRESULT)FALSE;

			if ((pSubMenu = mMenu.GetSubMenu(0)) == NULL)
				return (LRESULT)FALSE;

			// double click received, the default action is to execute first menu item
			::SetForegroundWindow(this->m_hWnd);
			::SendMessage(this->m_hWnd, WM_COMMAND, pSubMenu->GetMenuItemID(0), 0);

			pSubMenu->DestroyMenu();
			mMenu.DestroyMenu();
			break;
		}
	case WM_RBUTTONUP:
		{
			// load main menu
			HMENU hMenu=GetResManager().LoadMenu(MAKEINTRESOURCE(IDR_POPUP_MENU));
			CMenu mMenu, *pSubMenu;
			if (!mMenu.Attach(hMenu))
				return (LRESULT)FALSE;

			if ((pSubMenu = mMenu.GetSubMenu(0)) == NULL)
				return (LRESULT)FALSE;

			// set menu default item
			pSubMenu->SetDefaultItem(0, TRUE);

			// make window foreground
			SetForegroundWindow();

			// get current cursor pos
			POINT pt;
			GetCursorPos(&pt);

			pSubMenu->CheckMenuItem(ID_POPUP_MONITORING, MF_BYCOMMAND | (GetConfig().get_bool(PP_PCLIPBOARDMONITORING) ? MF_CHECKED : MF_UNCHECKED));
			pSubMenu->CheckMenuItem(ID_POPUP_SHUTAFTERFINISHED, MF_BYCOMMAND | (GetConfig().get_bool(PP_PSHUTDOWNAFTREFINISHED) ? MF_CHECKED : MF_UNCHECKED));

			// track the menu
			pSubMenu->TrackPopupMenu(TPM_LEFTBUTTON, pt.x, pt.y, this);

			// destroy
			pSubMenu->DestroyMenu();
			mMenu.DestroyMenu();
			
			break;
		}
	case WM_MOUSEMOVE:
		{
			if (m_tasks.GetSize() != 0)
			{
				_sntprintf(text, _MAX_PATH, _T("%s - %d %%"), GetApp().GetAppName(), m_tasks.GetPercent());
				m_ctlTray.SetTooltipText(text);
			}
			else
				m_ctlTray.SetTooltipText(GetApp().GetAppNameVer());
			break;
		}
	}
	
	return (LRESULT)TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd/CTrayIcon menu message handlers

void CMainWnd::ShowStatusWindow(const CTask *pSelect)
{
	m_pdlgStatus=new CStatusDlg(&m_tasks, this);	// self deleting
	m_pdlgStatus->m_pInitialSelection=pSelect;
	m_pdlgStatus->m_bLockInstance=true;
	m_pdlgStatus->m_bAutoDelete=true;
	m_pdlgStatus->Create();
	
	// hide miniview if showing status
	if (m_pdlgMiniView != NULL && m_pdlgMiniView->m_bLock)
	{
		if (::IsWindow(m_pdlgMiniView->m_hWnd))
			m_pdlgMiniView->HideWindow();
	}
}

void CMainWnd::OnPopupShowStatus()
{
	ShowStatusWindow();
}

void CMainWnd::OnClose() 
{
	PrepareToExit();
	CWnd::OnClose();
}

void CMainWnd::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case 1023:
		// autosave timer
		KillTimer(1023);
		m_tasks.SaveProgress();
		SetTimer(1023, (UINT)GetConfig().get_signed_num(PP_PAUTOSAVEINTERVAL), NULL);
		break;
	case 7834:
		{
			// auto-resume timer
			KillTimer(7834);
			DWORD dwTime=GetTickCount();
			DWORD dwInterval=(m_dwLastTime == 0) ? TM_AUTORESUME : dwTime-m_dwLastTime;
			m_dwLastTime=dwTime;
			
			if (GetConfig().get_bool(PP_CMAUTORETRYONERROR))
			{
				if (m_tasks.TasksRetryProcessing(true, dwInterval) && m_pdlgStatus && m_pdlgStatus->m_bLock && IsWindow(m_pdlgStatus->m_hWnd))
					m_pdlgStatus->SendMessage(WM_UPDATESTATUS);
			}
			SetTimer(7834, TM_AUTORESUME, NULL);
		}
		break;
	case 3245:
		// auto-delete finished tasks timer
		KillTimer(3245);
		if (GetConfig().get_bool(PP_STATUSAUTOREMOVEFINISHED))
		{
			int iSize=m_tasks.GetSize();
			m_tasks.RemoveAllFinished();
			if (m_tasks.GetSize() != iSize && m_pdlgStatus && m_pdlgStatus->m_bLock && IsWindow(m_pdlgStatus->m_hWnd))
				m_pdlgStatus->SendMessage(WM_UPDATESTATUS);
		}

		SetTimer(3245, TM_AUTOREMOVE, NULL);
		break;
	case 8743:
		{
			// wait state handling section
			CTask* pTask;
			if (GetConfig().get_signed_num(PP_CMLIMITMAXOPERATIONS) == 0 || m_tasks.GetOperationsPending() < (UINT)GetConfig().get_signed_num(PP_CMLIMITMAXOPERATIONS))
			{
				for (int i=0;i<m_tasks.GetSize();i++)
				{
					pTask=m_tasks.GetAt(i);
					// turn on some thread - find something with wait state
					if (pTask->GetStatus(ST_WAITING_MASK) & ST_WAITING && (GetConfig().get_signed_num(PP_CMLIMITMAXOPERATIONS) == 0 || m_tasks.GetOperationsPending() < (UINT)GetConfig().get_signed_num(PP_CMLIMITMAXOPERATIONS)))
					{
						TRACE("Enabling task %ld\n", i);
						pTask->SetContinueFlag(true);
						pTask->IncreaseOperationsPending();
						pTask->SetStatus(0, ST_WAITING);		// turn off wait state
					}
				}
			}
			break;
		}
	}

	CWnd::OnTimer(nIDEvent);
}

void CMainWnd::OnPopupShowOptions()
{
	COptionsDlg *pDlg=new COptionsDlg(this);
	pDlg->m_bAutoDelete=true;
	pDlg->m_bLockInstance=true;
	pDlg->Create();
}

BOOL CMainWnd::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	if(!GetApp().IsShellExtEnabled())
		return FALSE;

	// copying or moving ?
	bool bMove=false;
	switch(pCopyDataStruct->dwData & OPERATION_MASK)
	{
	case DD_MOVE_FLAG:
	case EC_MOVETO_FLAG:
		bMove=true;
		break;
	case EC_PASTE_FLAG:
	case EC_PASTESPECIAL_FLAG:
		bMove=(pCopyDataStruct->dwData & ~OPERATION_MASK) != 0;
		break;
	}

	// buffer with: dst path and src paths separated by single '\0'
	TCHAR *pBuffer=static_cast<TCHAR*>(pCopyDataStruct->lpData);
	unsigned long ulLen=pCopyDataStruct->cbData / sizeof(TCHAR);

	CString str, strDstPath;
	CStringArray astrFiles;
	UINT iOffset=0;

	do
	{
		str=pBuffer+iOffset;
		if (iOffset == 0)
			strDstPath=str;
		else
			astrFiles.Add(str);

		iOffset+=str.GetLength()+1;
	}
	while (iOffset < ulLen);

	icpf::config& rConfig = GetConfig();

	// special operation - modify stuff
	CFiltersArray ffFilters;
	int iPriority=(int)GetConfig().get_signed_num(PP_CMDEFAULTPRIORITY);
	BUFFERSIZES bsSizes;
	bsSizes.m_bOnlyDefault=GetConfig().get_bool(PP_BFUSEONLYDEFAULT);
	bsSizes.m_uiDefaultSize=(UINT)GetConfig().get_signed_num(PP_BFDEFAULT);
	bsSizes.m_uiOneDiskSize=(UINT)GetConfig().get_signed_num(PP_BFONEDISK);
	bsSizes.m_uiTwoDisksSize=(UINT)GetConfig().get_signed_num(PP_BFTWODISKS);
	bsSizes.m_uiCDSize=(UINT)GetConfig().get_signed_num(PP_BFCD);
	bsSizes.m_uiLANSize=(UINT)GetConfig().get_signed_num(PP_BFLAN);

	BOOL bOnlyCreate=FALSE;
	BOOL bIgnoreDirs=FALSE;
	BOOL bForceDirectories=FALSE;
	unsigned char ucCopies=1;
	switch(pCopyDataStruct->dwData & OPERATION_MASK)
	{
	case DD_COPYMOVESPECIAL_FLAG:
	case EC_PASTESPECIAL_FLAG:
	case EC_COPYMOVETOSPECIAL_FLAG:
		CCustomCopyDlg dlg;
		dlg.m_ccData.m_astrPaths.Copy(astrFiles);
		dlg.m_ccData.m_iOperation=bMove ? 1 : 0;
		dlg.m_ccData.m_iPriority=iPriority;
		dlg.m_ccData.m_strDestPath=strDstPath;
		dlg.m_ccData.m_bsSizes=bsSizes;
		dlg.m_ccData.m_bIgnoreFolders=(bIgnoreDirs != 0);
		dlg.m_ccData.m_bForceDirectories=(bForceDirectories != 0);
		dlg.m_ccData.m_bCreateStructure=(bOnlyCreate != 0);
		dlg.m_ccData.m_ucCount=ucCopies;

		dlg.m_ccData.m_vRecent.clear(true);
		const tchar_t* pszPath = NULL;
		size_t stCount = rConfig.get_value_count(PP_RECENTPATHS);
		for(size_t stIndex = 0; stIndex < stCount; stIndex++)
		{
			pszPath = rConfig.get_string(PP_RECENTPATHS, stIndex);
			if(pszPath)
				dlg.m_ccData.m_vRecent.push_back(pszPath);
		}

		int iModalResult;
		if ( (iModalResult=dlg.DoModal()) == IDCANCEL)
			return CWnd::OnCopyData(pWnd, pCopyDataStruct);
		else if (iModalResult == -1)	// windows has been closed by a parent
			return TRUE;

		astrFiles.Copy(dlg.m_ccData.m_astrPaths);
		bMove=(dlg.m_ccData.m_iOperation != 0);
		iPriority=dlg.m_ccData.m_iPriority;
		strDstPath=dlg.m_ccData.m_strDestPath;
		bsSizes=dlg.m_ccData.m_bsSizes;
		ffFilters = dlg.m_ccData.m_afFilters;
		bIgnoreDirs=dlg.m_ccData.m_bIgnoreFolders;
		bForceDirectories=dlg.m_ccData.m_bForceDirectories;
		bOnlyCreate=dlg.m_ccData.m_bCreateStructure;
		ucCopies=dlg.m_ccData.m_ucCount;
		dlg.m_ccData.m_vRecent.insert(dlg.m_ccData.m_vRecent.begin(), (const PTSTR)(LPCTSTR)strDstPath, true);

		rConfig.clear_array_values(PP_RECENTPATHS);
		for(char_vector::iterator it = dlg.m_ccData.m_vRecent.begin(); it != dlg.m_ccData.m_vRecent.end(); it++)
		{
			rConfig.set_string(PP_RECENTPATHS, (*it), icpf::property::action_add);
		}
	}

	// create new task
	CTask *pTask = m_tasks.CreateTask();
	pTask->SetDestPath(strDstPath);
	CClipboardEntry* pEntry;

	// files
	for (int i=0;i<astrFiles.GetSize();i++)
	{
		pEntry=new CClipboardEntry;
		pEntry->SetPath(astrFiles.GetAt(i));
		pEntry->CalcBufferIndex(pTask->GetDestPath());
		pTask->AddClipboardData(pEntry);
	}

	pTask->SetStatus(bMove ? ST_MOVE : ST_COPY, ST_OPERATION_MASK);

	// special status
	pTask->SetStatus((bOnlyCreate ? ST_IGNORE_CONTENT : 0) | (bIgnoreDirs ? ST_IGNORE_DIRS : 0) | (bForceDirectories ? ST_FORCE_DIRS : 0), ST_SPECIAL_MASK);
			
	// set some stuff related with task
	pTask->SetBufferSizes(&bsSizes);
	pTask->SetPriority(iPriority);
	pTask->SetFilters(&ffFilters);
	pTask->SetCopies(ucCopies);

	m_tasks.Add(pTask);

	// save state of a task
	pTask->Store(true);
	pTask->Store(false);

	// add to task list and start processing
	pTask->BeginProcessing();

	return CWnd::OnCopyData(pWnd, pCopyDataStruct);
}

void CMainWnd::OnShowMiniView() 
{
	m_pdlgMiniView=new CMiniViewDlg(&m_tasks, &CStatusDlg::m_bLock, this);	// self-deleting
	m_pdlgMiniView->m_bAutoDelete=true;
	m_pdlgMiniView->m_bLockInstance=true;
	m_pdlgMiniView->Create();
}

void CMainWnd::OnPopupCustomCopy() 
{
	icpf::config& rConfig = GetConfig();

	CCustomCopyDlg dlg;
	dlg.m_ccData.m_iOperation=0;
	dlg.m_ccData.m_iPriority=(int)rConfig.get_signed_num(PP_CMDEFAULTPRIORITY);
	dlg.m_ccData.m_bsSizes.m_bOnlyDefault=rConfig.get_bool(PP_BFUSEONLYDEFAULT);
	dlg.m_ccData.m_bsSizes.m_uiDefaultSize=(UINT)rConfig.get_signed_num(PP_BFDEFAULT);
	dlg.m_ccData.m_bsSizes.m_uiOneDiskSize=(UINT)rConfig.get_signed_num(PP_BFONEDISK);
	dlg.m_ccData.m_bsSizes.m_uiTwoDisksSize=(UINT)rConfig.get_signed_num(PP_BFTWODISKS);
	dlg.m_ccData.m_bsSizes.m_uiCDSize=(UINT)rConfig.get_signed_num(PP_BFCD);
	dlg.m_ccData.m_bsSizes.m_uiLANSize=(UINT)rConfig.get_signed_num(PP_BFLAN);

	dlg.m_ccData.m_bCreateStructure=false;
	dlg.m_ccData.m_bForceDirectories=false;
	dlg.m_ccData.m_bIgnoreFolders=false;
	dlg.m_ccData.m_ucCount=1;

	dlg.m_ccData.m_vRecent.clear(true);
	const tchar_t* pszPath = NULL;
	size_t stCount = rConfig.get_value_count(PP_RECENTPATHS);
	for(size_t stIndex = 0; stIndex < stCount; stIndex++)
	{
		pszPath = rConfig.get_string(PP_RECENTPATHS, stIndex);
		if(pszPath)
			dlg.m_ccData.m_vRecent.push_back(pszPath);
	}

	if (dlg.DoModal() == IDOK)
	{
		// save recent paths
		dlg.m_ccData.m_vRecent.push_back((PCTSTR)dlg.m_ccData.m_strDestPath);

		rConfig.clear_array_values(PP_RECENTPATHS);
		for(char_vector::iterator it = dlg.m_ccData.m_vRecent.begin(); it != dlg.m_ccData.m_vRecent.end(); it++)
		{
			rConfig.set_string(PP_RECENTPATHS, (*it), icpf::property::action_add);
		}

		// new task
		CTask *pTask = m_tasks.CreateTask();
		pTask->SetDestPath(dlg.m_ccData.m_strDestPath);
		CClipboardEntry *pEntry;
		for (int i=0;i<dlg.m_ccData.m_astrPaths.GetSize();i++)
		{
			pEntry=new CClipboardEntry;
			pEntry->SetPath(dlg.m_ccData.m_astrPaths.GetAt(i));
			pEntry->CalcBufferIndex(pTask->GetDestPath());
			pTask->AddClipboardData(pEntry);
		}
		
		pTask->SetStatus((dlg.m_ccData.m_iOperation == 1) ? ST_MOVE : ST_COPY, ST_OPERATION_MASK);

		// special status
		pTask->SetStatus((dlg.m_ccData.m_bCreateStructure ? ST_IGNORE_CONTENT : 0) | (dlg.m_ccData.m_bIgnoreFolders ? ST_IGNORE_DIRS : 0)
			| (dlg.m_ccData.m_bForceDirectories ? ST_FORCE_DIRS : 0), ST_SPECIAL_MASK);
		
		pTask->SetBufferSizes(&dlg.m_ccData.m_bsSizes);
		pTask->SetPriority(dlg.m_ccData.m_iPriority);
		pTask->SetFilters(&dlg.m_ccData.m_afFilters);
		
		m_tasks.Add(pTask);

		// save
		pTask->Store(true);
		pTask->Store(false);
		
		// store and start
		pTask->BeginProcessing();
	}
}

LRESULT CMainWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_MINIVIEWDBLCLK:
		{
			ShowStatusWindow((CTask*)lParam);
			break;
		}
	case WM_SHOWMINIVIEW:
		{
			OnShowMiniView();
			return static_cast<LRESULT>(0);
			break;
		}

	case WM_CONFIGNOTIFY:
		{
			GetApp().SetAutorun(GetConfig().get_bool(PP_PRELOADAFTERRESTART));

			// set this process class
			HANDLE hProcess=GetCurrentProcess();
			::SetPriorityClass(hProcess, (DWORD)GetConfig().get_signed_num(PP_PPROCESSPRIORITYCLASS));

			break;
		}

	case WM_GETCONFIG:
		{
			icpf::config& rConfig = GetConfig();

			// std config values
			g_pscsShared->bShowFreeSpace=rConfig.get_bool(PP_SHSHOWFREESPACE);
			
			// experimental - doesn't work on all systems 
			g_pscsShared->bShowShortcutIcons=rConfig.get_bool(PP_SHSHOWSHELLICONS);
			g_pscsShared->bOverrideDefault=rConfig.get_bool(PP_SHUSEDRAGDROP);	// only for d&d
			g_pscsShared->uiDefaultAction=(UINT)rConfig.get_signed_num(PP_SHDEFAULTACTION);
			
			// sizes
			for (int i=0;i<6;i++)
				_tcscpy(g_pscsShared->szSizes[i], GetResManager().LoadString(IDS_BYTE_STRING+i));

			// convert to list of _COMMAND's
			_COMMAND *pCommand = g_pscsShared->GetCommandsPtr();

			// what kind of menu ?
			switch (wParam)
			{
			case GC_DRAGDROP:
				{
					g_pscsShared->iCommandCount=3;
					g_pscsShared->iShortcutsCount=0;
					g_pscsShared->uiFlags=(rConfig.get_bool(PP_SHSHOWCOPY) ? DD_COPY_FLAG : 0)
						| (rConfig.get_bool(PP_SHSHOWMOVE) ? DD_MOVE_FLAG : 0)
						| (rConfig.get_bool(PP_SHSHOWCOPYMOVE) ? DD_COPYMOVESPECIAL_FLAG : 0);

					pCommand[0].uiCommandID=DD_COPY_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUCOPY_STRING, pCommand[0].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPCOPY_STRING, pCommand[0].szDesc, 128);
					
					pCommand[1].uiCommandID=DD_MOVE_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUMOVE_STRING, pCommand[1].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPMOVE_STRING, pCommand[1].szDesc, 128);
					
					pCommand[2].uiCommandID=DD_COPYMOVESPECIAL_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUCOPYMOVESPECIAL_STRING, pCommand[2].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPCOPYMOVESPECIAL_STRING, pCommand[2].szDesc, 128);
				}
				break;
			case GC_EXPLORER:
				{
					g_pscsShared->iCommandCount=5;
					g_pscsShared->uiFlags=(rConfig.get_bool(PP_SHSHOWPASTE) ? EC_PASTE_FLAG : 0)
						| (rConfig.get_bool(PP_SHSHOWPASTESPECIAL) ? EC_PASTESPECIAL_FLAG : 0)
						| (rConfig.get_bool(PP_SHSHOWCOPYTO) ? EC_COPYTO_FLAG : 0)
						| (rConfig.get_bool(PP_SHSHOWMOVETO) ? EC_MOVETO_FLAG : 0)
						| (rConfig.get_bool(PP_SHSHOWCOPYMOVETO) ? EC_COPYMOVETOSPECIAL_FLAG : 0);
					
					pCommand[0].uiCommandID=EC_PASTE_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUPASTE_STRING, pCommand[0].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPPASTE_STRING, pCommand[0].szDesc, 128);
					pCommand[1].uiCommandID=EC_PASTESPECIAL_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUPASTESPECIAL_STRING, pCommand[1].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPPASTESPECIAL_STRING, pCommand[1].szDesc, 128);
					pCommand[2].uiCommandID=EC_COPYTO_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUCOPYTO_STRING, pCommand[2].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPCOPYTO_STRING, pCommand[2].szDesc, 128);
					pCommand[3].uiCommandID=EC_MOVETO_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUMOVETO_STRING, pCommand[3].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPMOVETO_STRING, pCommand[3].szDesc, 128);
					pCommand[4].uiCommandID=EC_COPYMOVETOSPECIAL_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUCOPYMOVETOSPECIAL_STRING, pCommand[4].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPCOPYMOVETOSPECIAL_STRING, pCommand[4].szDesc, 128);
					
					// prepare shortcuts
					char_vector cvShortcuts;
					const tchar_t* pszPath = NULL;
					size_t stCount = rConfig.get_value_count(PP_SHORTCUTS);
					for(size_t stIndex = 0; stIndex < stCount; stIndex++)
					{
						pszPath = rConfig.get_string(PP_SHORTCUTS, stIndex);
						if(pszPath)
							cvShortcuts.push_back(pszPath);
					}
					
					// count of shortcuts to store
					g_pscsShared->iShortcutsCount=__min(cvShortcuts.size(), (SHARED_BUFFERSIZE - 5 * sizeof(_COMMAND)) / sizeof(_SHORTCUT));
					_SHORTCUT* pShortcut = g_pscsShared->GetShortcutsPtr();
					CShortcut sc;
					for (int i=0;i<g_pscsShared->iShortcutsCount;i++)
					{
						sc=CString(cvShortcuts.at(i));
						_tcsncpy(pShortcut[i].szName, sc.m_strName, 128);
						_tcsncpy(pShortcut[i].szPath, sc.m_strPath, _MAX_PATH);
					}
				}
				break;
			default:
				ASSERT(false);	// what's happening ?
			}
		}
		break;

	case WM_IDENTIFY:
		{
			//decode
			unsigned char *dec=new unsigned char[iCount+1];
			dec[iCount]=0;

			unsigned short sData;
			for (int i=0, j=0;i<iCount;i++)
			{
				sData=static_cast<unsigned short>(msg[i] - _hash[j]);

				sData >>= off[j];
				dec[i]=static_cast<unsigned char>(sData);

				if (++j >= iOffCount)
					j=0;
			}

			CA2T ca2t(reinterpret_cast<char*>(dec));
			AfxMessageBox(ca2t);
			delete [] dec;

			break;
		}
	case WM_STATUSCLOSING:
		{
			if (m_pdlgMiniView != NULL && m_pdlgMiniView->m_bLock && ::IsWindow(m_pdlgMiniView->m_hWnd))
				m_pdlgMiniView->RefreshStatus();

			break;
		}
	case WM_ENDSESSION:
		{
			PrepareToExit();
			break;
		}
	case WM_TRAYNOTIFY:
		{
			return OnTrayNotification(wParam, lParam);
			break;
		}
	}

	// if this is a notification of new tray - recreate the icon
	if (message == m_uiTaskbarRestart)
	{
		ShowTrayIcon();
		return (LRESULT)TRUE;
	}

	return CWnd::WindowProc(message, wParam, lParam);
}

void CMainWnd::OnAppAbout() 
{
	CAboutDlg *pdlg=new CAboutDlg;
	pdlg->m_bAutoDelete=true;
	pdlg->m_bLockInstance=true;
	pdlg->Create();
}

void CMainWnd::OnPopupMonitoring() 
{
	// change flag in config
	GetConfig().set_bool(PP_PCLIPBOARDMONITORING, !GetConfig().get_bool(PP_PCLIPBOARDMONITORING));
	GetConfig().write(NULL);
}

void CMainWnd::OnPopupShutafterfinished() 
{
	GetConfig().set_bool(PP_PSHUTDOWNAFTREFINISHED, !GetConfig().get_bool(PP_PSHUTDOWNAFTREFINISHED));	
	GetConfig().write(NULL);
}

void CMainWnd::OnPopupRegisterdll() 
{
	CString strPath;
	CCopyHandlerApp& rApp = GetApp();
	if(rApp)
	{
		strPath = rApp.GetProgramPath();
		strPath += _T("\\");
	}

#ifdef _WIN64
	strPath += _T("chext64.dll");
#else
	strPath += _T("chext.dll");
#endif
	HRESULT hResult = RegisterShellExtDll(strPath, true);
	if(FAILED(hResult))
	{
		TCHAR szStr[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hResult, 0, szStr, 256, NULL);
		while (szStr[_tcslen(szStr)-1] == _T('\n') || szStr[_tcslen(szStr)-1] == _T('\r') || szStr[_tcslen(szStr)-1] == _T('.'))
			szStr[_tcslen(szStr)-1]=_T('\0');

		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_REGISTERERR_STRING));
		fmt.SetParam(_T("%errno"), (ulong_t)hResult);
		fmt.SetParam(_T("%errdesc"), szStr);
		AfxMessageBox(fmt, MB_ICONERROR | MB_OK);
	}
	else if(hResult == S_OK)
		MsgBox(IDS_REGISTEROK_STRING, MB_ICONINFORMATION | MB_OK);
}

void CMainWnd::OnPopupUnregisterdll() 
{
	CString strPath;
	CCopyHandlerApp& rApp = GetApp();
	strPath = rApp.GetProgramPath();
	strPath += _T("\\");

#ifdef _WIN64
	strPath += _T("chext64.dll");
#else
	strPath += _T("chext.dll");
#endif

	HRESULT hResult = RegisterShellExtDll(strPath, false);
	if(FAILED(hResult))
	{
		TCHAR szStr[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hResult, 0, szStr, 256, NULL);
		while (szStr[_tcslen(szStr)-1] == _T('\n') || szStr[_tcslen(szStr)-1] == _T('\r') || szStr[_tcslen(szStr)-1] == _T('.'))
			szStr[_tcslen(szStr)-1]=_T('\0');

		ictranslate::CFormat fmt(GetResManager().LoadString(IDS_UNREGISTERERR_STRING));
		fmt.SetParam(_T("%errno"), (ulong_t)hResult);
		fmt.SetParam(_T("%errdesc"), szStr);

		AfxMessageBox(fmt, MB_ICONERROR | MB_OK);
	}
	else if(hResult == S_OK)
		MsgBox(IDS_UNREGISTEROK_STRING, MB_ICONINFORMATION | MB_OK);
}

void CMainWnd::PrepareToExit()
{
	// kill thread that monitors clipboard
	cmd.bKill=true;
	while (!cmd.bKilled)
		Sleep(10);

	// kill all unfinished tasks - send kill request
	for (int i=0;i<m_tasks.GetSize();i++)
		m_tasks.GetAt(i)->SetKillFlag();

	// wait for finishing
	for (int i=0;i<m_tasks.GetSize();i++)
	{
		while (!m_tasks.GetAt(i)->GetKilledFlag())
			Sleep(10);
		m_tasks.GetAt(i)->CleanupAfterKill();
	}
	
	// save
	m_tasks.SaveProgress();

	// delete all tasks
	int iSize=m_tasks.GetSize();
	for (int i=0;i<iSize;i++)
		delete m_tasks.GetAt(i);

	(static_cast< CArray<CTask*, CTask*>* >(&m_tasks))->RemoveAll();
}

void CMainWnd::OnAppExit()
{
	PostMessage(WM_CLOSE);
}

void CMainWnd::OnPopupHelp() 
{
	GetApp().HtmlHelp(HH_DISPLAY_TOPIC, NULL);
}

void CMainWnd::OnPopupCheckForUpdates()
{
	CUpdaterDlg* pDlg = new CUpdaterDlg;
	pDlg->m_bAutoDelete = true;
	
	pDlg->Create();
}
