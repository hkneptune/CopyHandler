/***************************************************************************
*   Copyright (C) 2001-2011 by Jozef Starosczyk                           *
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
#include "FolderDialog.h"
#include "CustomCopyDlg.h"
#include "AboutDlg.h"
#include "ShutdownDlg.h"
#include "..\common\ipcstructs.h"
#include "UpdateChecker.h"
#include "UpdaterDlg.h"
#include "FeedbackHandler.h"
#include "MiniviewDlg.h"
#include "StatusDlg.h"
#include "ClipboardMonitor.h"
#include <boost/make_shared.hpp>
#include <boost/shared_array.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CH_WNDCLASS_NAME   _T("Copy Handler Wnd Class")

#define WM_TRAYNOTIFY			(WM_USER+0)

#define	WM_ICON_NOTIFY			WM_USER+4
#define WM_SHOWMINIVIEW			WM_USER+3
#define WM_IDENTIFY				WM_USER+11

#define TM_AUTOREMOVE			1000
#define TM_ACCEPTING			100

extern CSharedConfigStruct* g_pscsShared;

extern int iCount;
extern unsigned short msg[];

extern int iOffCount;
extern unsigned char off[];
extern unsigned short _hash[];


/////////////////////////////////////////////////////////////////////////////
// CMainWnd
/////////////////////////////////////////////////////////////////////////////
// CMainWnd construction/destruction
CMainWnd::CMainWnd() :
	m_pFeedbackFactory(CFeedbackHandlerFactory::CreateFactory()),
	m_pdlgStatus(NULL),
	m_pdlgMiniView(NULL),
	m_dwLastTime(0)
{
}

CMainWnd::~CMainWnd()
{
	if(m_pFeedbackFactory)
		m_pFeedbackFactory->Delete();
}

// registers main window class
BOOL CMainWnd::RegisterClass()
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
	wc.lpszClassName	= CH_WNDCLASS_NAME;

	return ::AfxRegisterClass(&wc);
}

// creates this window
BOOL CMainWnd::Create()
{
	BOOL bReg = RegisterClass();
	if(!bReg)
		return FALSE;

	return CreateEx(WS_EX_TOOLWINDOW, CH_WNDCLASS_NAME, _T("Copy Handler"), WS_OVERLAPPED, 10, 10, 10, 10, NULL, (HMENU)NULL, NULL);
}

int CMainWnd::ShowTrayIcon()
{
	// create system tray icon
	HICON hIcon = (HICON)GetResManager().LoadImage(MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_VGACOLOR);

	CString strText = GetApp().GetAppNameVer();
	strText += GetResManager().LoadString(IDS_CH_PORTABLE_STRING);

	bool bRes=m_ctlTray.CreateIcon(m_hWnd, WM_TRAYNOTIFY, strText, hIcon, 0);
	if(!bRes)
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
// CMainWnd message handlers

int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->dwExStyle |= WS_EX_TOPMOST;
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// get msg id of taskbar created message
	m_uiTaskbarRestart = RegisterWindowMessage(_T("TaskbarCreated"));

	// Create the tray icon
	ShowTrayIcon();

	// initialize CTaskArray
	m_tasks.Create(m_pFeedbackFactory);

	// load last state
	LOG_INFO(_T("Loading existing tasks..."));
	CString strPath;
	GetApp().GetProgramDataPath(strPath);
	strPath += _T("\\Tasks\\");
	m_tasks.SetTasksDir(strPath);

	// load tasks
	m_tasks.LoadDataProgress();

	// import tasks specified at command line (before loading current tasks)
	const TCommandLineParser& cmdLine = GetApp().GetCommandLine();
	if(cmdLine.HasTaskDefinitionPath())
	{
		std::vector<CString> vTaskPaths;
		cmdLine.GetTaskDefinitionPaths(vTaskPaths);

		const size_t stBufferSize = 4096;
		boost::shared_array<wchar_t> szBuffer(new wchar_t[stBufferSize]);

		BOOST_FOREACH(const CString& strPath, vTaskPaths)
		{
			bool bImported = false;

			try
			{
				CTaskPtr spTask = m_tasks.ImportTask(strPath);
				if(spTask)
					spTask->Store();
				bImported = true;
			}
			catch(icpf::exception& e)
			{
				bImported = false;
				e.get_info(szBuffer.get(), stBufferSize);
			}
			catch(...)
			{
				bImported = false;
				szBuffer.get()[0] = _T('\0');
			}

			if(!bImported)
			{
				ictranslate::CFormat fmt;
				fmt.SetFormat(_T("Error encountered while importing task from path '%path'. Error: %err."));
				fmt.SetParam(_T("%path"), strPath);
				fmt.SetParam(_T("%error"), szBuffer.get());

				LOG_ERROR(fmt);

				fmt.SetFormat(GetResManager().LoadString(IDS_TASK_IMPORT_FAILED));
				fmt.SetParam(_T("%path"), strPath);
				AfxMessageBox(fmt, MB_OK | MB_ICONERROR);
			}
		}
	}

	// resume tasks
	m_tasks.TasksRetryProcessing();

	// start clipboard monitoring
	LOG_INFO(_T("Starting clipboard monitor..."));
	CClipboardMonitor::StartMonitor(&m_tasks);

	EUpdatesFrequency eFrequency = (EUpdatesFrequency)GetPropValue<PP_PCHECK_FOR_UPDATES_FREQUENCY>(GetConfig());
	if(eFrequency != eFreq_Never)
	{
		unsigned long long ullMinInterval = 0;
		switch(eFrequency)
		{
		case eFreq_Daily:
			ullMinInterval = 1*24*60*60;
			break;
		case eFreq_Weekly:
			ullMinInterval = 7*24*60*60;
			break;
		case eFreq_OnceEvery2Weeks:
			ullMinInterval = 14*24*60*60;
			break;
		case eFreq_Monthly:
			ullMinInterval = 30*24*60*60;	// we don't really care if it is a day less or more
			break;
		case eFreq_Quarterly:
			ullMinInterval = 90*24*60*60;
			break;
		case eFreq_EveryStartup:
		default:
			ullMinInterval = 0;
		}

		// get last check time stored in configuration
		unsigned long long ullCurrentStamp = _time64(NULL);
		unsigned long long ullTimestamp = GetPropValue<PP_LAST_UPDATE_TIMESTAMP>(GetConfig());

		// perform checking for updates only when the minimal interval has passed
		if(ullCurrentStamp - ullTimestamp >= ullMinInterval)
		{
			LOG_INFO(_T("Checking for updates..."));

			CUpdaterDlg* pDlg = new CUpdaterDlg(true);
			pDlg->m_bAutoDelete = true;

			pDlg->Create();
			chcore::TConfig& rConfig = GetConfig();
			try
			{
				SetPropValue<PP_LAST_UPDATE_TIMESTAMP>(rConfig, _time64(NULL));
				rConfig.Write();
			}
			catch(icpf::exception& /*e*/)
			{
				LOG_ERROR(_T("Storing last update check timestamp in configuration failed"));
			}
		}
	}

	// start saving timer
	SetTimer(1023, GetPropValue<PP_PAUTOSAVEINTERVAL>(GetConfig()), NULL);

	SetTimer(3245, TM_AUTOREMOVE, NULL);
	SetTimer(8743, TM_ACCEPTING, NULL);		// ends wait state in tasks

	if (GetPropValue<PP_MVAUTOSHOWWHENRUN>(GetConfig()))
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

			pSubMenu->CheckMenuItem(ID_POPUP_MONITORING, MF_BYCOMMAND | (GetPropValue<PP_PCLIPBOARDMONITORING>(GetConfig()) ? MF_CHECKED : MF_UNCHECKED));
			pSubMenu->CheckMenuItem(ID_POPUP_SHUTAFTERFINISHED, MF_BYCOMMAND | (GetPropValue<PP_PSHUTDOWNAFTREFINISHED>(GetConfig()) ? MF_CHECKED : MF_UNCHECKED));

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
			{
				CString strText = GetApp().GetAppNameVer();
				strText += GetResManager().LoadString(IDS_CH_PORTABLE_STRING);
				m_ctlTray.SetTooltipText(strText);
			}
			break;
		}
	}
	
	return (LRESULT)TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd/CTrayIcon menu message handlers

void CMainWnd::ShowStatusWindow(const CTaskPtr& spSelect)
{
	m_pdlgStatus=new CStatusDlg(&m_tasks, this);	// self deleting
	m_pdlgStatus->m_spInitialSelection = spSelect;
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
		m_tasks.SaveData();
		SetTimer(1023, GetPropValue<PP_PAUTOSAVEINTERVAL>(GetConfig()), NULL);
		break;
	case 3245:
		// auto-delete finished tasks timer
		KillTimer(3245);
		if (GetPropValue<PP_STATUSAUTOREMOVEFINISHED>(GetConfig()))
		{
			size_t stSize = m_tasks.GetSize();
			m_tasks.RemoveAllFinished();
			if(m_tasks.GetSize() != stSize && m_pdlgStatus && m_pdlgStatus->m_bLock && IsWindow(m_pdlgStatus->m_hWnd))
				m_pdlgStatus->SendMessage(WM_UPDATESTATUS);
		}

		SetTimer(3245, TM_AUTOREMOVE, NULL);
		break;
	case 8743:
		{
			// wait state handling section
			m_tasks.ResumeWaitingTasks((size_t)GetPropValue<PP_CMLIMITMAXOPERATIONS>(GetConfig()));
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
	// copying or moving ?
	bool bMove=false;
	switch(pCopyDataStruct->dwData & CSharedConfigStruct::OPERATION_MASK)
	{
	case CSharedConfigStruct::DD_MOVE_FLAG:
	case CSharedConfigStruct::EC_MOVETO_FLAG:
		bMove=true;
		break;
	case CSharedConfigStruct::EC_PASTE_FLAG:
	case CSharedConfigStruct::EC_PASTESPECIAL_FLAG:
		bMove=(pCopyDataStruct->dwData & ~CSharedConfigStruct::OPERATION_MASK) != 0;
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

	chcore::TConfig& rConfig = GetConfig();

	// special operation - modify stuff
	CFiltersArray ffFilters;
	int iPriority = boost::numeric_cast<int>(GetPropValue<PP_CMDEFAULTPRIORITY>(GetConfig()));
	BUFFERSIZES bsSizes;
	bsSizes.m_bOnlyDefault=GetPropValue<PP_BFUSEONLYDEFAULT>(GetConfig());
	bsSizes.m_uiDefaultSize=GetPropValue<PP_BFDEFAULT>(GetConfig());
	bsSizes.m_uiOneDiskSize=GetPropValue<PP_BFONEDISK>(GetConfig());
	bsSizes.m_uiTwoDisksSize=GetPropValue<PP_BFTWODISKS>(GetConfig());
	bsSizes.m_uiCDSize=GetPropValue<PP_BFCD>(GetConfig());
	bsSizes.m_uiLANSize=GetPropValue<PP_BFLAN>(GetConfig());

	BOOL bOnlyCreate=FALSE;
	BOOL bIgnoreDirs=FALSE;
	BOOL bForceDirectories=FALSE;
	switch(pCopyDataStruct->dwData & CSharedConfigStruct::OPERATION_MASK)
	{
	case CSharedConfigStruct::DD_COPYMOVESPECIAL_FLAG:
	case CSharedConfigStruct::EC_PASTESPECIAL_FLAG:
	case CSharedConfigStruct::EC_COPYMOVETOSPECIAL_FLAG:
		CCustomCopyDlg dlg;
		dlg.m_ccData.m_astrPaths.Copy(astrFiles);
		dlg.m_ccData.m_iOperation=bMove ? 1 : 0;
		dlg.m_ccData.m_iPriority=iPriority;
		dlg.m_ccData.m_strDestPath=strDstPath;
		dlg.m_ccData.m_bsSizes=bsSizes;
		dlg.m_ccData.m_bIgnoreFolders=(bIgnoreDirs != 0);
		dlg.m_ccData.m_bForceDirectories=(bForceDirectories != 0);
		dlg.m_ccData.m_bCreateStructure=(bOnlyCreate != 0);

		dlg.m_ccData.m_vRecent.clear();

		GetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_ccData.m_vRecent);

		INT_PTR iModalResult;
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
		dlg.m_ccData.m_vRecent.insert(dlg.m_ccData.m_vRecent.begin(), strDstPath);

		SetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_ccData.m_vRecent);
	}

	// create new task
	TTaskDefinition tTaskDefinition;
	tTaskDefinition.SetDestinationPath(chcore::PathFromString(strDstPath));

	// files
	for(int i = 0; i < astrFiles.GetSize(); i++)
	{
		tTaskDefinition.AddSourcePath(chcore::PathFromString(astrFiles.GetAt(i)));
	}

	tTaskDefinition.SetOperationType(bMove ? eOperation_Move : eOperation_Copy);

	// set the default options for task
	GetConfig().ExtractSubConfig(BRANCH_TASK_SETTINGS, tTaskDefinition.GetConfiguration());

	// and override them with manual settings
	SetTaskPropValue<eTO_CreateEmptyFiles>(tTaskDefinition.GetConfiguration(), bOnlyCreate != FALSE);
	SetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(tTaskDefinition.GetConfiguration(), bForceDirectories != FALSE);
	SetTaskPropValue<eTO_IgnoreDirectories>(tTaskDefinition.GetConfiguration(), bIgnoreDirs != FALSE);

	// buffer sizes
	SetTaskPropValue<eTO_DefaultBufferSize>(tTaskDefinition.GetConfiguration(), bsSizes.m_uiDefaultSize);
	SetTaskPropValue<eTO_OneDiskBufferSize>(tTaskDefinition.GetConfiguration(), bsSizes.m_uiOneDiskSize);
	SetTaskPropValue<eTO_TwoDisksBufferSize>(tTaskDefinition.GetConfiguration(), bsSizes.m_uiTwoDisksSize);
	SetTaskPropValue<eTO_CDBufferSize>(tTaskDefinition.GetConfiguration(), bsSizes.m_uiCDSize);
	SetTaskPropValue<eTO_LANBufferSize>(tTaskDefinition.GetConfiguration(), bsSizes.m_uiLANSize);
	SetTaskPropValue<eTO_UseOnlyDefaultBuffer>(tTaskDefinition.GetConfiguration(), bsSizes.m_bOnlyDefault);

	// Task priority
	SetTaskPropValue<eTO_ThreadPriority>(tTaskDefinition.GetConfiguration(), iPriority);

	// load resource strings
	SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_FIRSTCOPY_STRING));
	SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_NEXTCOPY_STRING));

	SetTaskPropValue<eTO_Filters>(tTaskDefinition.GetConfiguration(), ffFilters);

	// create task with the above definition
	CTaskPtr spTask = m_tasks.CreateTask(tTaskDefinition);

	// add to task list and start processing
	spTask->BeginProcessing();

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
	chcore::TConfig& rConfig = GetConfig();

	CCustomCopyDlg dlg;
	dlg.m_ccData.m_iOperation=0;
	dlg.m_ccData.m_iPriority = boost::numeric_cast<int>(GetPropValue<PP_CMDEFAULTPRIORITY>(rConfig));
	dlg.m_ccData.m_bsSizes.m_bOnlyDefault=GetPropValue<PP_BFUSEONLYDEFAULT>(rConfig);
	dlg.m_ccData.m_bsSizes.m_uiDefaultSize=GetPropValue<PP_BFDEFAULT>(rConfig);
	dlg.m_ccData.m_bsSizes.m_uiOneDiskSize=GetPropValue<PP_BFONEDISK>(rConfig);
	dlg.m_ccData.m_bsSizes.m_uiTwoDisksSize=GetPropValue<PP_BFTWODISKS>(rConfig);
	dlg.m_ccData.m_bsSizes.m_uiCDSize=GetPropValue<PP_BFCD>(rConfig);
	dlg.m_ccData.m_bsSizes.m_uiLANSize=GetPropValue<PP_BFLAN>(rConfig);

	dlg.m_ccData.m_bCreateStructure=false;
	dlg.m_ccData.m_bForceDirectories=false;
	dlg.m_ccData.m_bIgnoreFolders=false;

	dlg.m_ccData.m_vRecent.clear();

	GetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_ccData.m_vRecent);

	if (dlg.DoModal() == IDOK)
	{
		SetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_ccData.m_vRecent);

		// save recent paths
		dlg.m_ccData.m_vRecent.push_back((PCTSTR)dlg.m_ccData.m_strDestPath);

		TTaskDefinition tTaskDefinition;

		for (int iIndex = 0; iIndex < dlg.m_ccData.m_astrPaths.GetSize(); iIndex++)
		{
			tTaskDefinition.AddSourcePath(chcore::PathFromString(dlg.m_ccData.m_astrPaths.GetAt(iIndex)));
		}

		tTaskDefinition.SetDestinationPath(chcore::PathFromString(dlg.m_ccData.m_strDestPath));

		tTaskDefinition.SetOperationType((dlg.m_ccData.m_iOperation == 1) ? eOperation_Move : eOperation_Copy);

		// set the default options for task
		GetConfig().ExtractSubConfig(BRANCH_TASK_SETTINGS, tTaskDefinition.GetConfiguration());

		// and override them with manual settings
		SetTaskPropValue<eTO_CreateEmptyFiles>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bCreateStructure);
		SetTaskPropValue<eTO_CreateDirectoriesRelativeToRoot>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bForceDirectories);
		SetTaskPropValue<eTO_IgnoreDirectories>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bIgnoreFolders);

		// Buffer settings
		SetTaskPropValue<eTO_DefaultBufferSize>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bsSizes.m_uiDefaultSize);
		SetTaskPropValue<eTO_OneDiskBufferSize>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bsSizes.m_uiOneDiskSize);
		SetTaskPropValue<eTO_TwoDisksBufferSize>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bsSizes.m_uiTwoDisksSize);
		SetTaskPropValue<eTO_CDBufferSize>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bsSizes.m_uiCDSize);
		SetTaskPropValue<eTO_LANBufferSize>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bsSizes.m_uiLANSize);
		SetTaskPropValue<eTO_UseOnlyDefaultBuffer>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_bsSizes.m_bOnlyDefault);

		// Task priority
		SetTaskPropValue<eTO_ThreadPriority>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_iPriority);

		// load resource strings
		SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_FIRSTCOPY_STRING));
		SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_NEXTCOPY_STRING));

		SetTaskPropValue<eTO_Filters>(tTaskDefinition.GetConfiguration(), dlg.m_ccData.m_afFilters);

		// new task
		CTaskPtr spTask = m_tasks.CreateTask(tTaskDefinition);

		// start
		spTask->BeginProcessing();
	}
}

LRESULT CMainWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_MINIVIEWDBLCLK:
		{
			CTaskPtr spTask = m_tasks.GetTaskBySessionUniqueID(lParam);
			ShowStatusWindow(spTask);
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
			GetApp().SetAutorun(GetPropValue<PP_PRELOADAFTERRESTART>(GetConfig()));

			// set this process class
			HANDLE hProcess=GetCurrentProcess();
			::SetPriorityClass(hProcess, (DWORD)GetPropValue<PP_PPROCESSPRIORITYCLASS>(GetConfig()));

			break;
		}

	case WM_GETCONFIG:
		{
			chcore::TConfig& rConfig = GetConfig();

			// std config values
			g_pscsShared->bShowFreeSpace=GetPropValue<PP_SHSHOWFREESPACE>(rConfig);
			
			// experimental - doesn't work on all systems 
			g_pscsShared->bShowShortcutIcons=GetPropValue<PP_SHSHOWSHELLICONS>(rConfig);
			g_pscsShared->uiFlags = (GetPropValue<PP_SHINTERCEPTDRAGDROP>(rConfig) ? CSharedConfigStruct::eFlag_InterceptDragAndDrop : 0) |
									(GetPropValue<PP_SHINTERCEPTKEYACTIONS>(rConfig) ? CSharedConfigStruct::eFlag_InterceptKeyboardActions : 0) |
									(GetPropValue<PP_SHINTERCEPTCTXMENUACTIONS>(rConfig) ? CSharedConfigStruct::eFlag_InterceptCtxMenuActions : 0);
			
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
					g_pscsShared->uiFlags |= (GetPropValue<PP_SHSHOWCOPY>(rConfig) ? CSharedConfigStruct::DD_COPY_FLAG : 0)
						| (GetPropValue<PP_SHSHOWMOVE>(rConfig) ? CSharedConfigStruct::DD_MOVE_FLAG : 0)
						| (GetPropValue<PP_SHSHOWCOPYMOVE>(rConfig) ? CSharedConfigStruct::DD_COPYMOVESPECIAL_FLAG : 0);

					pCommand[0].uiCommandID=CSharedConfigStruct::DD_COPY_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUCOPY_STRING, pCommand[0].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPCOPY_STRING, pCommand[0].szDesc, 128);
					
					pCommand[1].uiCommandID=CSharedConfigStruct::DD_MOVE_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUMOVE_STRING, pCommand[1].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPMOVE_STRING, pCommand[1].szDesc, 128);
					
					pCommand[2].uiCommandID=CSharedConfigStruct::DD_COPYMOVESPECIAL_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUCOPYMOVESPECIAL_STRING, pCommand[2].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPCOPYMOVESPECIAL_STRING, pCommand[2].szDesc, 128);
				}
				break;
			case GC_EXPLORER:
				{
					g_pscsShared->iCommandCount=5;
					g_pscsShared->uiFlags |= (GetPropValue<PP_SHSHOWPASTE>(rConfig) ? CSharedConfigStruct::EC_PASTE_FLAG : 0)
						| (GetPropValue<PP_SHSHOWPASTESPECIAL>(rConfig) ? CSharedConfigStruct::EC_PASTESPECIAL_FLAG : 0)
						| (GetPropValue<PP_SHSHOWCOPYTO>(rConfig) ? CSharedConfigStruct::EC_COPYTO_FLAG : 0)
						| (GetPropValue<PP_SHSHOWMOVETO>(rConfig) ? CSharedConfigStruct::EC_MOVETO_FLAG : 0)
						| (GetPropValue<PP_SHSHOWCOPYMOVETO>(rConfig) ? CSharedConfigStruct::EC_COPYMOVETOSPECIAL_FLAG : 0);
					
					pCommand[0].uiCommandID=CSharedConfigStruct::EC_PASTE_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUPASTE_STRING, pCommand[0].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPPASTE_STRING, pCommand[0].szDesc, 128);
					pCommand[1].uiCommandID=CSharedConfigStruct::EC_PASTESPECIAL_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUPASTESPECIAL_STRING, pCommand[1].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPPASTESPECIAL_STRING, pCommand[1].szDesc, 128);
					pCommand[2].uiCommandID=CSharedConfigStruct::EC_COPYTO_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUCOPYTO_STRING, pCommand[2].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPCOPYTO_STRING, pCommand[2].szDesc, 128);
					pCommand[3].uiCommandID=CSharedConfigStruct::EC_MOVETO_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUMOVETO_STRING, pCommand[3].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPMOVETO_STRING, pCommand[3].szDesc, 128);
					pCommand[4].uiCommandID=CSharedConfigStruct::EC_COPYMOVETOSPECIAL_FLAG;
					GetResManager().LoadStringCopy(IDS_MENUCOPYMOVETOSPECIAL_STRING, pCommand[4].szCommand, 128);
					GetResManager().LoadStringCopy(IDS_MENUTIPCOPYMOVETOSPECIAL_STRING, pCommand[4].szDesc, 128);
					
					// prepare shortcuts
					std::vector<CString> cvShortcuts;
					GetPropValue<PP_SHORTCUTS>(rConfig, cvShortcuts);

					// count of shortcuts to store
					g_pscsShared->iShortcutsCount = boost::numeric_cast<int>(std::min(cvShortcuts.size(), (SHARED_BUFFERSIZE - 5 * sizeof(_COMMAND)) / sizeof(_SHORTCUT)));
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
	SetPropValue<PP_PCLIPBOARDMONITORING>(GetConfig(), !GetPropValue<PP_PCLIPBOARDMONITORING>(GetConfig()));
	GetConfig().Write();
}

void CMainWnd::OnPopupShutafterfinished() 
{
	SetPropValue<PP_PSHUTDOWNAFTREFINISHED>(GetConfig(), !GetPropValue<PP_PSHUTDOWNAFTREFINISHED>(GetConfig()));	
	GetConfig().Write();
}

void CMainWnd::OnPopupRegisterdll() 
{
	GetApp().RegisterShellExtension();
}

void CMainWnd::OnPopupUnregisterdll() 
{
	GetApp().UnregisterShellExtension();
}

void CMainWnd::PrepareToExit()
{
	// kill thread that monitors clipboard
	CClipboardMonitor::StopMonitor();

	m_tasks.StopAllTasks();

	// save
	m_tasks.SaveData();

	// delete all tasks
	m_tasks.RemoveAll();
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
	CUpdaterDlg* pDlg = new CUpdaterDlg(false);
	pDlg->m_bAutoDelete = true;
	
	pDlg->Create();
}
