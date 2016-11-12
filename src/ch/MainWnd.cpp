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
#include "..\common\ipcstructs.h"
#include "UpdateChecker.h"
#include "UpdaterDlg.h"
#include "MiniviewDlg.h"
#include "StatusDlg.h"
#include "ClipboardMonitor.h"
#include <boost/shared_array.hpp>
#include "../common/TShellExtMenuConfig.h"
#include "../libchcore/TConfig.h"
#include "../libchcore/TCoreException.h"
#include "../libchcore/TTaskManagerStatsSnapshot.h"
#include "../libchcore/TSQLiteSerializerFactory.h"
#include "TRecentPathsTools.h"
#include "DirectoryChooser.h"
#include "FeedbackHandlerFactory.h"
#include "../libchcore/TTask.h"
#include "TTaskManagerWrapper.h"
#include "CfgProperties.h"
#include "resource.h"

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

enum ETimers
{
	eTimer_Autosave = 1023,
	eTimer_AutoremoveFinished = 3245,
	eTimer_ResumeWaitingTasks = 8743
};

/////////////////////////////////////////////////////////////////////////////
// CMainWnd
/////////////////////////////////////////////////////////////////////////////
// CMainWnd construction/destruction
CMainWnd::CMainWnd() :
	m_spTaskMgrStats(new chcore::TTaskManagerStatsSnapshot),
	m_spLog(logger::MakeLogger(GetLogFileData(), L"MainWnd"))
{
}

CMainWnd::~CMainWnd()
{
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
	wc.hIcon			= ::LoadIcon(nullptr, MAKEINTRESOURCE(AFX_IDI_STD_FRAME));
	wc.hCursor			= ::LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName		= nullptr;
	wc.lpszClassName	= CH_WNDCLASS_NAME;

	return ::AfxRegisterClass(&wc);
}

// creates this window
BOOL CMainWnd::Create()
{
	BOOL bReg = RegisterClass();
	if(!bReg)
		return FALSE;

	return CreateEx(WS_EX_TOOLWINDOW, CH_WNDCLASS_NAME, _T("Copy Handler"), WS_OVERLAPPED, 10, 10, 10, 10, nullptr, (HMENU)nullptr, nullptr);
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
		LOG_ERROR(m_spLog) << L"Failed to create tray icon";
		return -1;
	}

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

		if(!LoadTaskManager())
		{
			LOG_ERROR(m_spLog) << _T("Couldn't load task manager data. User did not allow re-creation of the database.");
			return -1;
		}

		// import tasks specified at command line (before loading current tasks)
		const TCommandLineParser& cmdLine = GetApp().GetCommandLine();
		ProcessCommandLine(cmdLine);

		// start processing of the tasks loaded above and added by a command line
		m_spTasks->TasksRetryProcessing();

		// start clipboard monitoring
		LOG_INFO(m_spLog) << _T("Starting clipboard monitor...");
		CClipboardMonitor::StartMonitor(m_spTasks);

		CheckForUpdates();

		SetupTimers();

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
		LOG_ERROR(m_spLog) << szErrInfo.get();
		return -1;
	}
	return 0;
}

bool CMainWnd::LoadTaskManager()
{
	using namespace chcore;

	CString strError;
	CString strTasksDir = GetTasksDirectory();
	TSQLiteSerializerFactoryPtr spSerializerFactory(new TSQLiteSerializerFactory(PathFromString(strTasksDir)));
	IFeedbackHandlerFactoryPtr spFeedbackFactory(new CFeedbackHandlerFactory);

	try
	{
		m_spTasks.reset(new chcore::TTaskManager(spSerializerFactory, spFeedbackFactory, PathFromString(strTasksDir), GetApp().GetEngineLoggerConfig()));
	}
	catch(const std::exception& e)
	{
		strError = e.what();
	}

	if(!strError.IsEmpty())
	{
		if(MsgBox(IDS_TASKMANAGER_LOAD_FAILED, MB_ICONERROR | MB_OKCANCEL) == IDOK)
		{
			m_spTasks.reset(new chcore::TTaskManager(spSerializerFactory, spFeedbackFactory, PathFromString(strTasksDir), GetApp().GetEngineLoggerConfig(), true));
		}
		else
			return false;
	}

	// load last state
	LOG_INFO(m_spLog) << _T("Loading existing tasks...");

	// load tasks
	m_spTasks->Load();

	return true;
}

LRESULT CMainWnd::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	if (wParam != m_ctlTray.m_tnd.uID)
		return (LRESULT)FALSE;

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

			if ((pSubMenu = mMenu.GetSubMenu(0)) == nullptr)
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

			if ((pSubMenu = mMenu.GetSubMenu(0)) == nullptr)
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
			if (m_spTasks->GetSize() != 0)
			{
				m_spTasks->GetStatsSnapshot(m_spTaskMgrStats);

				TCHAR text[ _MAX_PATH ];
				_sntprintf(text, _MAX_PATH, _T("%s - %.0f %%"), GetApp().GetAppName(), m_spTaskMgrStats->GetCombinedProgress() * 100.0);
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

void CMainWnd::ShowStatusWindow(const chcore::TTaskPtr& spSelect)
{
	m_pdlgStatus=new CStatusDlg(m_spTasks.get(), this);	// self deleting
	m_pdlgStatus->m_spInitialSelection = spSelect;
	m_pdlgStatus->m_bLockInstance=true;
	m_pdlgStatus->m_bAutoDelete=true;
	m_pdlgStatus->Create();
	
	// hide miniview if showing status
	if (m_pdlgMiniView != nullptr && m_pdlgMiniView->m_bLock)
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
	CString strMessage;
	try
	{
		PrepareToExit();
	}
	catch(const chcore::TBaseException& e)
	{
		const size_t stMaxError = 1024;
		wchar_t szError[ stMaxError ];
		e.GetErrorInfo(szError, stMaxError);

		strMessage = szError;
	}
	catch(const std::exception& e)
	{
		strMessage = e.what();
	}

	if(!strMessage.IsEmpty())
	{
		LOG_ERROR(m_spLog) << L"Failed to finalize tasks before exiting Copy Handler. Error: " + strMessage;

		ictranslate::CFormat fmt;

		fmt.SetFormat(GetResManager().LoadString(IDS_FINALIZE_CH_ERROR));
		fmt.SetParam(_T("%reason"), strMessage);
		AfxMessageBox(fmt, MB_OK | MB_ICONERROR);
	}

	CWnd::OnClose();
}

void CMainWnd::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case eTimer_Autosave:
		{
			// autosave timer
			KillTimer(eTimer_Autosave);
			try
			{
				m_spTasks->Store(false);
			}
			catch(const std::exception& e)
			{
				CString strError = e.what();

				ictranslate::CFormat fmt;
				fmt.SetFormat(_T("Failed to autosave task. Error: %err."));
				fmt.SetParam(_T("%err"), (PCTSTR)strError);

				LOG_ERROR(m_spLog) << fmt;
			}

			SetTimer(eTimer_Autosave, GetPropValue<PP_PAUTOSAVEINTERVAL>(GetConfig()), nullptr);
			break;
		}
	case eTimer_AutoremoveFinished:
		// auto-delete finished tasks timer
		KillTimer(eTimer_AutoremoveFinished);
		if (GetPropValue<PP_STATUSAUTOREMOVEFINISHED>(GetConfig()))
		{
			size_t stSize = m_spTasks->GetSize();
			m_spTasks->RemoveAllFinished();
			if(m_spTasks->GetSize() != stSize && m_pdlgStatus && m_pdlgStatus->m_bLock && IsWindow(m_pdlgStatus->m_hWnd))
				m_pdlgStatus->SendMessage(WM_UPDATESTATUS);
		}

		SetTimer(eTimer_AutoremoveFinished, TM_AUTOREMOVE, nullptr);
		break;

	case eTimer_ResumeWaitingTasks:
		{
			// wait state handling section
			m_spTasks->ResumeWaitingTasks((size_t)GetPropValue<PP_CMLIMITMAXOPERATIONS>(GetConfig()));
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

			LOG_DEBUG(m_spLog) << L"Received task definition to process: " << wstrData;

			chcore::TTaskDefinition tTaskDefinition;
			CString strError;
			try
			{
				tTaskDefinition.LoadFromString(wstrData, true);
			}
			catch(const chcore::TCoreException& e)
			{
				strError.Format(_T("Error code: %d"), e.GetErrorCode());
			}
			catch(const std::exception& e)
			{
				strError.Format(_T("Error message: %S"), e.what());
			}

			if(!strError.IsEmpty())
			{
				ictranslate::CFormat fmt;
				fmt.SetFormat(_T("Cannot import shell extension xml in WM_COPYDATA. Xml: '%xml'. Error: %err."));
				fmt.SetParam(_T("%xml"), wstrData.c_str());
				fmt.SetParam(_T("%err"), (PCTSTR)strError);

				LOG_ERROR(m_spLog) << fmt;

				fmt.SetFormat(GetResManager().LoadString(IDS_SHELLEXT_XML_IMPORT_FAILED));
				fmt.SetParam(_T("%err"), (PCTSTR)strError);
				AfxMessageBox(fmt, MB_OK | MB_ICONERROR);

				break;
			}

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

				TRecentPathsTools::AddNewPath(dlg.m_vRecent, dlg.m_tTaskDefinition.GetDestinationPath().ToString());

				SetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_vRecent);

				tTaskDefinition = dlg.m_tTaskDefinition;
			}
			else if(tTaskDefinition.GetDestinationPath().IsEmpty())
			{
				chcore::TSmartPath pathSelected;
				if(DirectoryChooser::ChooseDirectory(tTaskDefinition.GetOperationType(), tTaskDefinition.GetSourcePaths(), pathSelected) == IDOK)
					tTaskDefinition.SetDestinationPath(pathSelected);
				else
					break;
			}

			TTaskManagerWrapper tTaskManager(m_spTasks);
			tTaskManager.CreateTask(tTaskDefinition);

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
			m_spTasks->TasksRetryProcessing();

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
				chcore::TTaskPtr spTask = m_spTasks->ImportTask(strPath);
				if(spTask)
					spTask->Store(true);
				bImported = true;
			}
			catch(const chcore::TBaseException& e)
			{
				bImported = false;
				e.GetDetailedErrorInfo(szBuffer.get(), stBufferSize);
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
				fmt.SetParam(_T("%err"), szBuffer.get());

				LOG_ERROR(m_spLog) << fmt;

				fmt.SetFormat(GetResManager().LoadString(IDS_TASK_IMPORT_FAILED));
				fmt.SetParam(_T("%path"), strPath.ToString());
				AfxMessageBox(fmt, MB_OK | MB_ICONERROR);
			}
		}
	}
}

void CMainWnd::OnShowMiniView() 
{
	m_pdlgMiniView=new CMiniViewDlg(m_spTasks.get(), &CStatusDlg::m_bLock, this);	// self-deleting
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
		TRecentPathsTools::AddNewPath(dlg.m_vRecent, dlg.m_tTaskDefinition.GetDestinationPath().ToString());

		SetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_vRecent);

		chcore::TTaskDefinition tTaskDefinition = dlg.m_tTaskDefinition;

		TTaskManagerWrapper tTaskManager(m_spTasks);
		tTaskManager.CreateTask(tTaskDefinition);
	}
}

LRESULT CMainWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_MINIVIEWDBLCLK:
		{
			chcore::TTaskPtr spTask = m_spTasks->GetTaskByTaskID(boost::numeric_cast<chcore::taskid_t>(lParam));
			ShowStatusWindow(spTask);
			break;
		}
	case WM_SHOWMINIVIEW:
		{
			OnShowMiniView();
			return static_cast<LRESULT>(0);

		}

	case WM_CONFIGNOTIFY:
		{
			GetApp().SetAutorun(GetPropValue<PP_PRELOADAFTERRESTART>(GetConfig()));

			// set this process class
			HANDLE hProcess=GetCurrentProcess();
			::SetPriorityClass(hProcess, (DWORD)GetPropValue<PP_PPROCESSPRIORITYCLASS>(GetConfig()));

			break;
		}

	case WM_IDENTIFY:
		{
			//decode
			unsigned char *dec=new unsigned char[iCount+1];
			dec[iCount]=0;

			for (int i=0, j=0;i<iCount;i++)
			{
				unsigned short sData=static_cast<unsigned short>(msg[i] - _hash[j]);

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
			if (m_pdlgMiniView != nullptr && m_pdlgMiniView->m_bLock && ::IsWindow(m_pdlgMiniView->m_hWnd))
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

	m_spTasks->StopAllTasks();

	// delete all tasks
	m_spTasks->ClearBeforeExit();
}

void CMainWnd::OnAppExit()
{
	PostMessage(WM_CLOSE);
}

void CMainWnd::OnPopupHelp() 
{
	GetApp().HtmlHelp(HH_DISPLAY_TOPIC, 0);
}

void CMainWnd::OnPopupCheckForUpdates()
{
	CUpdaterDlg* pDlg = new CUpdaterDlg(false);
	pDlg->m_bAutoDelete = true;
	
	pDlg->Create();
}

CString CMainWnd::GetTasksDirectory() const
{
	CString strPath;
	GetApp().GetProgramDataPath(strPath);
	strPath += _T("\\Tasks\\");
	return strPath;
}

void CMainWnd::CheckForUpdates()
{
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
		unsigned long long ullCurrentStamp = _time64(nullptr);
		unsigned long long ullTimestamp = GetPropValue<PP_LAST_UPDATE_TIMESTAMP>(GetConfig());

		// perform checking for updates only when the minimal interval has passed
		if(ullCurrentStamp - ullTimestamp >= ullMinInterval)
		{
			LOG_INFO(m_spLog) << _T("Checking for updates...");

			CUpdaterDlg* pDlg = new CUpdaterDlg(true);
			pDlg->m_bAutoDelete = true;

			pDlg->Create();
			chcore::TConfig& rConfig = GetConfig();
			try
			{
				SetPropValue<PP_LAST_UPDATE_TIMESTAMP>(rConfig, _time64(nullptr));
				rConfig.Write();
			}
			catch(const std::exception& /*e*/)
			{
				LOG_ERROR(m_spLog) << _T("Storing last update check timestamp in configuration failed");
			}
		}
	}
}

void CMainWnd::SetupTimers()
{
	// start saving timer
	SetTimer(eTimer_Autosave, GetPropValue<PP_PAUTOSAVEINTERVAL>(GetConfig()), nullptr);

	SetTimer(eTimer_AutoremoveFinished, TM_AUTOREMOVE, nullptr);
	SetTimer(eTimer_ResumeWaitingTasks, TM_ACCEPTING, nullptr);		// ends wait state in tasks
}
