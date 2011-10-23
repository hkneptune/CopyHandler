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
#include "../common/TShellExtMenuConfig.h"
#include "../libchcore/TConfig.h"
#include "FileSupport.h"
#include "StringHelpers.h"
#include "../libchcore/TCoreException.h"
#include "../libicpf/exception.h"

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
	if(GetApp().IsInPortableMode())
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

	bool bCaughtError = false;
	const size_t stMaxErrInfo = 1024;
	boost::shared_array<wchar_t> szErrInfo(new wchar_t[stMaxErrInfo]);
	
	try
	{
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
		m_tasks.SetTasksDir(chcore::PathFromString(strPath));

		// load tasks
		m_tasks.LoadDataProgress();

		// import tasks specified at command line (before loading current tasks)
		const TCommandLineParser& cmdLine = GetApp().GetCommandLine();
		ProcessCommandLine(cmdLine);

		// start processing of the tasks loaded above and added by a command line
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
	}
	catch(chcore::TCoreException& e)
	{
		bCaughtError = true;
		e.GetErrorInfo(szErrInfo.get(), stMaxErrInfo);
	}
	catch(std::exception& e)
	{
		bCaughtError = true;
		_snwprintf_s(szErrInfo.get(), stMaxErrInfo, _TRUNCATE, _T("%S"), e.what());
		szErrInfo.get()[stMaxErrInfo - 1] = _T('\0');
	}
	catch(...)
	{
		bCaughtError = true;
		_snwprintf_s(szErrInfo.get(), stMaxErrInfo, _TRUNCATE, _T("Caught an unknown exception"));
	}

	if(bCaughtError)
	{
		LOG_ERROR(szErrInfo.get());
		return -1;
	}
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
				if(GetApp().IsInPortableMode())
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
	if(!pCopyDataStruct)
		return CWnd::OnCopyData(pWnd, pCopyDataStruct);

	switch(pCopyDataStruct->dwData)
	{
	case eCDType_TaskDefinitionContentSpecial:
	case eCDType_TaskDefinitionContent:
		{
			// load task from buffer
			wchar_t* pszBuffer = static_cast<wchar_t*>(pCopyDataStruct->lpData);
			unsigned long ulLen = pCopyDataStruct->cbData / sizeof(wchar_t);

			// check if the string ends with '\0', so we can safely use it without length checks
			if(!pszBuffer || ulLen == 0 || pszBuffer[ulLen - 1] != L'\0')
				return FALSE;

			chcore::TString wstrData(pszBuffer);

			chcore::TTaskDefinition tTaskDefinition;
			tTaskDefinition.LoadFromString(wstrData);

			// apply current options from global config; in the future we might want to merge the incoming options with global ones instead of overwriting...
			chcore::TConfig& rConfig = GetConfig();
			rConfig.ExtractSubConfig(BRANCH_TASK_SETTINGS, tTaskDefinition.GetConfiguration());

			// special operation - modify stuff
			if(pCopyDataStruct->dwData == eCDType_TaskDefinitionContentSpecial)
			{
				CCustomCopyDlg dlg(tTaskDefinition);

				GetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_vRecent);

				INT_PTR iModalResult;
				if((iModalResult = dlg.DoModal()) == IDCANCEL)
					return CWnd::OnCopyData(pWnd, pCopyDataStruct);
				else if(iModalResult == -1)	// windows has been closed by a parent
					return TRUE;

				dlg.m_vRecent.push_back(dlg.m_tTaskDefinition.GetDestinationPath().ToString());

				SetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_vRecent);

				tTaskDefinition = dlg.m_tTaskDefinition;
			}

			// load resource strings
			SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_FIRSTCOPY_STRING));
			SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_NEXTCOPY_STRING));

			// create task with the above definition
			CTaskPtr spTask = m_tasks.CreateTask(tTaskDefinition);

			// add to task list and start processing
			spTask->BeginProcessing();

			break;
		}
	case eCDType_CommandLineArguments:
		{
			// load task from buffer
			wchar_t* pszBuffer = static_cast<wchar_t*>(pCopyDataStruct->lpData);
			unsigned long ulLen = pCopyDataStruct->cbData / sizeof(wchar_t);

			// check if the string ends with '\0', so we can safely use it without length checks
			if(!pszBuffer || ulLen == 0 || pszBuffer[ulLen - 1] != L'\0')
				return FALSE;

			TCommandLineParser cmdLineParser;
			cmdLineParser.ParseCommandLine(pszBuffer);

			ProcessCommandLine(cmdLineParser);
			m_tasks.TasksRetryProcessing();

			return TRUE;
		}
	}

	return CWnd::OnCopyData(pWnd, pCopyDataStruct);
}

void CMainWnd::ProcessCommandLine(const TCommandLineParser& rCommandLine)
{
	if(rCommandLine.HasTaskDefinitionPath())
	{
		chcore::TPathContainer vTaskPaths;
		rCommandLine.GetTaskDefinitionPaths(vTaskPaths);

		const size_t stBufferSize = 4096;
		boost::shared_array<wchar_t> szBuffer(new wchar_t[stBufferSize]);

		for(size_t stIndex = 0; stIndex < vTaskPaths.GetCount(); ++stIndex)
		{
			const chcore::TSmartPath& strPath = vTaskPaths.GetAt(stIndex);

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
				fmt.SetParam(_T("%path"), strPath.ToString());
				fmt.SetParam(_T("%error"), szBuffer.get());

				LOG_ERROR(fmt);

				fmt.SetFormat(GetResManager().LoadString(IDS_TASK_IMPORT_FAILED));
				fmt.SetParam(_T("%path"), strPath.ToString());
				AfxMessageBox(fmt, MB_OK | MB_ICONERROR);
			}
		}
	}
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

	GetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_vRecent);

	if(dlg.DoModal() == IDOK)
	{
		dlg.m_vRecent.push_back(dlg.m_tTaskDefinition.GetDestinationPath().ToString());

		SetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_vRecent);

		chcore::TTaskDefinition tTaskDefinition = dlg.m_tTaskDefinition;

		// load resource strings
		SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_FIRSTCOPY_STRING));
		SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_NEXTCOPY_STRING));

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

			TShellExtMenuConfig cfgShellExt;

			// experimental - doesn't work on all systems 
			cfgShellExt.SetShowShortcutIcons(GetPropValue<PP_SHSHOWSHELLICONS>(rConfig));

			cfgShellExt.SetInterceptDragAndDrop(GetPropValue<PP_SHINTERCEPTDRAGDROP>(rConfig));
			cfgShellExt.SetInterceptKeyboardActions(GetPropValue<PP_SHINTERCEPTKEYACTIONS>(rConfig));
			cfgShellExt.SetInterceptCtxMenuActions(GetPropValue<PP_SHINTERCEPTCTXMENUACTIONS>(rConfig));

			TShellMenuItemPtr spRootItem = cfgShellExt.GetCommandRoot();

			// what kind of menu ?
			switch (wParam)
			{
			case eLocation_DragAndDropMenu:
				{
					bool bAddedAnyOption = false;
					if(GetPropValue<PP_SHSHOWCOPY>(rConfig))
					{
						spRootItem->AddChild(boost::make_shared<TShellMenuItem>(GetResManager().LoadString(IDS_MENUCOPY_STRING), GetResManager().LoadString(IDS_MENUTIPCOPY_STRING),
							TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Copy),
							TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeIDataObject),
							TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializePidlFolder, chcore::TSmartPath()), false, chcore::eOperation_Copy));
						bAddedAnyOption = true;
					}

					if(GetPropValue<PP_SHSHOWMOVE>(rConfig))
					{
						spRootItem->AddChild(boost::make_shared<TShellMenuItem>(GetResManager().LoadString(IDS_MENUMOVE_STRING), GetResManager().LoadString(IDS_MENUTIPMOVE_STRING),
							TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Move),
							TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeIDataObject),
							TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializePidlFolder, chcore::TSmartPath()), false, chcore::eOperation_Move));
						bAddedAnyOption = true;
					}

					if(GetPropValue<PP_SHSHOWCOPYMOVE>(rConfig))
					{
						spRootItem->AddChild(boost::make_shared<TShellMenuItem>(GetResManager().LoadString(IDS_MENUCOPYMOVESPECIAL_STRING), GetResManager().LoadString(IDS_MENUTIPCOPYMOVESPECIAL_STRING),
							TOperationTypeInfo(TOperationTypeInfo::eOpType_Autodetect, chcore::eOperation_Copy),
							TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeIDataObject),
							TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializePidlFolder, chcore::TSmartPath()), true));
						bAddedAnyOption = true;
					}

					if(bAddedAnyOption)
					{
						// insert separator as an addition to other items
						spRootItem->AddChild(boost::make_shared<TShellMenuItem>());
					}
				}
				break;
			case eLocation_ContextMenu:
				{
					if(GetPropValue<PP_SHSHOWPASTE>(rConfig))
					{
						spRootItem->AddChild(boost::make_shared<TShellMenuItem>(GetResManager().LoadString(IDS_MENUPASTE_STRING), GetResManager().LoadString(IDS_MENUTIPPASTE_STRING),
							TOperationTypeInfo(TOperationTypeInfo::eOpType_Autodetect, chcore::eOperation_Copy),
							TSourcePathsInfo(TSourcePathsInfo::eSrcType_Clipboard),
							TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializeAuto, chcore::TSmartPath()), false));
					}

					if(GetPropValue<PP_SHSHOWPASTESPECIAL>(rConfig))
					{
						spRootItem->AddChild(boost::make_shared<TShellMenuItem>(GetResManager().LoadString(IDS_MENUPASTESPECIAL_STRING), GetResManager().LoadString(IDS_MENUTIPPASTESPECIAL_STRING),
							TOperationTypeInfo(TOperationTypeInfo::eOpType_Autodetect, chcore::eOperation_Copy),
							TSourcePathsInfo(TSourcePathsInfo::eSrcType_Clipboard),
							TDestinationPathInfo(TDestinationPathInfo::eDstType_InitializeAuto, chcore::TSmartPath()), false));
					}

					if(GetPropValue<PP_SHSHOWCOPYTO>(rConfig) || GetPropValue<PP_SHSHOWMOVETO>(rConfig) || GetPropValue<PP_SHSHOWCOPYMOVETO>(rConfig))
					{
						// prepare shortcuts for all menu options
						std::vector<CString> vShortcutStrings;
						GetPropValue<PP_SHORTCUTS>(rConfig, vShortcutStrings);

						bool bRetrieveFreeSpace = GetPropValue<PP_SHSHOWFREESPACE>(rConfig);

						std::vector<CShortcut> vShortcuts;
						const size_t stSizeBufferSize = 64;
						boost::shared_array<wchar_t> spSizeBuffer(new wchar_t[stSizeBufferSize]);

						BOOST_FOREACH(const CString& strShortcutString, vShortcutStrings)
						{
							CShortcut tShortcut;
							if(tShortcut.FromString(strShortcutString))
							{
								unsigned long long ullSize = 0;

								// retrieving free space might fail, but it's not critical - we just won't show the free space
								if(bRetrieveFreeSpace && GetDynamicFreeSpace(tShortcut.m_strPath, &ullSize, NULL))
								{
									CString strNameFormat;
									strNameFormat.Format(_T("%s (%s)"), tShortcut.m_strName, GetSizeString(ullSize, spSizeBuffer.get(), stSizeBufferSize));

									tShortcut.m_strName = strNameFormat;
								}

								vShortcuts.push_back(tShortcut);
							}
							else
								BOOST_ASSERT(false);	// non-critical, but not very nice
						}

						if(GetPropValue<PP_SHSHOWCOPYTO>(rConfig))
						{
							boost::shared_ptr<TShellMenuItem> menuItem(boost::make_shared<TShellMenuItem>(GetResManager().LoadString(IDS_MENUCOPYTO_STRING), GetResManager().LoadString(IDS_MENUTIPCOPYTO_STRING)));
							BOOST_FOREACH(const CShortcut& tShortcut, vShortcuts)
							{
								menuItem->AddChild(boost::make_shared<TShellMenuItem>((PCTSTR)tShortcut.m_strName, (PCTSTR)tShortcut.m_strPath,
									TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Copy),
									TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
									TDestinationPathInfo(TDestinationPathInfo::eDstType_Specified, chcore::PathFromString((PCTSTR)tShortcut.m_strPath)), false));
							}

							spRootItem->AddChild(menuItem);
						}

						if(GetPropValue<PP_SHSHOWMOVETO>(rConfig))
						{
							boost::shared_ptr<TShellMenuItem> menuItem(boost::make_shared<TShellMenuItem>(GetResManager().LoadString(IDS_MENUMOVETO_STRING), GetResManager().LoadString(IDS_MENUTIPMOVETO_STRING)));
							BOOST_FOREACH(const CShortcut& tShortcut, vShortcuts)
							{
								menuItem->AddChild(boost::make_shared<TShellMenuItem>((PCTSTR)tShortcut.m_strName, (PCTSTR)tShortcut.m_strPath,
									TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Move),
									TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
									TDestinationPathInfo(TDestinationPathInfo::eDstType_Specified, chcore::PathFromString((PCTSTR)tShortcut.m_strPath)), false));
							}

							spRootItem->AddChild(menuItem);
						}

						if(GetPropValue<PP_SHSHOWCOPYMOVETO>(rConfig))
						{
							boost::shared_ptr<TShellMenuItem> menuItem(boost::make_shared<TShellMenuItem>(GetResManager().LoadString(IDS_MENUCOPYMOVETOSPECIAL_STRING), GetResManager().LoadString(IDS_MENUTIPCOPYMOVETOSPECIAL_STRING)));
							BOOST_FOREACH(const CShortcut& tShortcut, vShortcuts)
							{
								menuItem->AddChild(boost::make_shared<TShellMenuItem>((PCTSTR)tShortcut.m_strName, (PCTSTR)tShortcut.m_strPath,
									TOperationTypeInfo(TOperationTypeInfo::eOpType_Specified, chcore::eOperation_Copy),
									TSourcePathsInfo(TSourcePathsInfo::eSrcType_InitializeAuto),
									TDestinationPathInfo(TDestinationPathInfo::eDstType_Specified, chcore::PathFromString((PCTSTR)tShortcut.m_strPath)), true));
							}

							spRootItem->AddChild(menuItem);
						}
					}
				}

				break;
			default:
				ASSERT(false);	// unhandled case
			}

			chcore::TConfig cfgStorage;
			chcore::TString wstrData;

			cfgShellExt.StoreInConfig(cfgStorage, _T("ShellExtCfg"));
			cfgStorage.WriteToString(wstrData);

			std::wstring strSHMName = IPCSupport::GenerateSHMName((unsigned long)lParam);

			m_tCHExtharedMemory.Create(strSHMName.c_str(), wstrData);
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
