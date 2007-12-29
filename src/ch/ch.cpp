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

#include "stdafx.h"
#include "ch.h"

#include "CfgProperties.h"
#include "MainWnd.h"
#include "..\common\ipcstructs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCopyHandlerApp

BEGIN_MESSAGE_MAP(CCopyHandlerApp, CWinApp)
	//{{AFX_MSG_MAP(CCopyHandlerApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CSharedConfigStruct* g_pscsShared;

int iCount=98;
unsigned short msg[]={	0x40d1, 0x4dcd, 0x8327, 0x6cdf, 0xb912, 0x017b, 0xac78, 0x1e04, 0x5637,
						0x1822, 0x0a69, 0x1b40, 0x4169, 0x504d, 0x80ff, 0x6c2f, 0xa612, 0x017e,
						0xac84, 0x1c8c, 0x552b, 0x16e2, 0x0a4b, 0x1dc0, 0x4179, 0x4d0d, 0x8337,
						0x6c4f, 0x6512, 0x0169, 0xac46, 0x1db4, 0x55cf, 0x1652, 0x0a0b, 0x1480,
						0x40fd, 0x470d, 0x822f, 0x6b8f, 0x6512, 0x013a, 0xac5a, 0x1d24, 0x5627,
						0x1762, 0x0a27, 0x1240, 0x40f5, 0x3f8d, 0x8187, 0x690f, 0x6e12, 0x011c,
						0xabc0, 0x1cc4, 0x567f, 0x1952, 0x0a51, 0x1cc0, 0x4175, 0x3ccd, 0x8377,
						0x6c5f, 0x6512, 0x0186, 0xac7c, 0x1e04, 0x5677, 0x1412, 0x0a61, 0x1d80,
						0x4169, 0x4e8d, 0x838f, 0x6c0f, 0xb212, 0x0132, 0xac7e, 0x1e54, 0x5593,
						0x1412, 0x0a15, 0x3dc0, 0x4195, 0x4e0d, 0x832f, 0x67ff, 0x9812, 0x0186,
						0xac6e, 0x1e4c, 0x5667, 0x1942, 0x0a47, 0x1f80, 0x4191, 0x4f8d };

int iOffCount=12;
unsigned char off[]={ 2, 6, 3, 4, 8, 0, 1, 3, 2, 4, 1, 6 };
unsigned short _hash[]={ 0x3fad, 0x34cd, 0x7fff, 0x65ff, 0x4512, 0x0112, 0xabac, 0x1abc, 0x54ab, 0x1212, 0x0981, 0x0100 };

/////////////////////////////////////////////////////////////////////////////
// The one and only CCopyHandlerApp object

CCopyHandlerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCopyHandlerApp construction

// main routing function - routes any message that comes from modules
LRESULT MainRouter(ULONGLONG ullDst, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	TRACE("Main routing func received ullDst=%I64u, uiMsg=%lu, wParam=%lu, lParam=%lu\n", ullDst, uiMsg, wParam, lParam);
	ULONGLONG ullOperation=ullDst & 0xff00000000000000;
	ullDst &= 0x00ffffffffffffff;			// get rid of operation

	switch (ullOperation)
	{
	case ROT_EVERYWHERE:
		{
			// TODO: send it to a registered modules (external plugins ?)

			// now additional processing
			switch (uiMsg)
			{
			case WM_CFGNOTIFY:
				theApp.OnConfigNotify((UINT)wParam, lParam);
				break;
			case WM_RMNOTIFY:
				theApp.OnResManNotify((UINT)wParam, lParam);
				break;
			}

			break;
		}

	case ROT_REGISTERED:
		// TODO: send a message to the registered modules
		break;

	case ROT_EXACT:
		{
			switch(ullDst)
			{
			case IMID_CONFIGMANAGER:
				return theApp.m_cfgManager.MsgRouter(uiMsg, wParam, lParam);
			}
		
			// TODO: send a msg to a registered module/program internal module with a given ID
		}
		break;
	}

	return (LRESULT)TRUE;
}

CCopyHandlerApp::CCopyHandlerApp() :
	m_lfLog()
{
	m_pMainWindow=NULL;
	m_szHelpPath[0]=_T('\0');

	// this is the one-instance application
	InitProtection();
}

CCopyHandlerApp::~CCopyHandlerApp()
{
	// Unmap shared memory from the process's address space.
	UnmapViewOfFile((LPVOID)g_pscsShared); 
	
	// Close the process's handle to the file-mapping object.
	CloseHandle(m_hMapObject);

	if (m_pMainWindow)
	{
		((CMainWnd*)m_pMainWindow)->DestroyWindow();
		delete m_pMainWindow;
		m_pMainWnd=NULL;
	}
}

CCopyHandlerApp* GetApp()
{
	return &theApp;
}

CResourceManager* GetResManager()
{
	return &theApp.m_resManager;
}

CConfigManager* GetConfig()
{
	return &theApp.m_cfgManager;
}
/*
CLogFile* GetLog()
{
	return &theApp.m_lfLog;
}*/

int MsgBox(UINT uiID, UINT nType, UINT nIDHelp)
{
	return AfxMessageBox(GetResManager()->LoadString(uiID), nType, nIDHelp);
}

bool CCopyHandlerApp::UpdateHelpPaths()
{
	bool bChanged=false;		// flag that'll be returned - if the paths has changed

	// generate the current filename - uses language from config
	TCHAR szBuffer[_MAX_PATH];
	GetConfig()->GetStringValue(PP_PHELPDIR, szBuffer, _MAX_PATH);
	ExpandPath(szBuffer);
	_tcscat(szBuffer, GetResManager()->m_ld.GetHelpName());
	if (_tcscmp(szBuffer, m_szHelpPath) != 0)
	{
		bChanged=true;
		_tcscpy(m_szHelpPath, szBuffer);
	}

	return bChanged;
}

/////////////////////////////////////////////////////////////////////////////
// CCopyHandlerApp initialization
#include "charvect.h"

BOOL CCopyHandlerApp::InitInstance()
{
	CWinApp::InitInstance();

	m_hMapObject = CreateFileMapping((HANDLE)0xFFFFFFFF, NULL, PAGE_READWRITE, 0, sizeof(CSharedConfigStruct), _T("CHLMFile"));
	if (m_hMapObject == NULL)
		return FALSE; 
	
	// Get a pointer to the file-mapped shared memory.
	g_pscsShared=(CSharedConfigStruct*)MapViewOfFile(m_hMapObject, FILE_MAP_WRITE, 0, 0, 0);
	if (g_pscsShared == NULL) 
		return FALSE; 
	
	// load configuration
	m_cfgManager.SetCallback((PFNNOTIFYCALLBACK)MainRouter);
	TCHAR szPath[_MAX_PATH];
	_tcscpy(szPath, GetProgramPath());
	_tcscat(szPath, _T("\\ch.ini"));
	m_cfgManager.Open(szPath);

	// register all properties
	RegisterProperties(&m_cfgManager);

	// set this process class
	HANDLE hProcess=GetCurrentProcess();
	::SetPriorityClass(hProcess, m_cfgManager.GetIntValue(PP_PPROCESSPRIORITYCLASS));

	// set current language
	m_resManager.Init(AfxGetInstanceHandle());
	m_resManager.SetCallback((PFNNOTIFYCALLBACK)MainRouter);
	m_cfgManager.GetStringValue(PP_PLANGUAGE, szPath, _MAX_PATH);
	TRACE("Help path=%s\n", szPath);
	if (!m_resManager.SetLanguage(ExpandPath(szPath)))
	{
		TCHAR szData[2048];
		_stprintf(szData, _T("Couldn't find the language file specified in configuration file:\n%s\nPlease correct this path to point the language file to use.\nProgram will now exit."), szPath);
		AfxMessageBox(szData, MB_ICONSTOP | MB_OK);
		return FALSE;
	}

	// for dialogs
	CLanguageDialog::SetResManager(&m_resManager);

	// initialize log file
	m_cfgManager.GetStringValue(PP_LOGPATH, szPath, _MAX_PATH);
	m_lfLog.init(ExpandPath(szPath), m_cfgManager.GetIntValue(PP_LOGMAXLIMIT), icpf::log_file::level_debug, false, false);

	// TODO: remove unused properties from configuration
/*	m_lfLog.EnableLogging(m_cfgManager.GetBoolValue(PP_LOGENABLELOGGING));
	m_lfLog.SetPreciseLimiting(m_cfgManager.GetBoolValue(PP_LOGPRECISELIMITING));
	m_lfLog.SetSizeLimit(m_cfgManager.GetBoolValue(PP_LOGLIMITATION), m_cfgManager.GetIntValue(PP_LOGMAXLIMIT));
	m_lfLog.SetTruncateBufferSize(m_cfgManager.GetIntValue(PP_LOGTRUNCBUFFERSIZE));
	m_lfLog.Init(ExpandPath(szPath), GetResManager());*/

#ifndef _DEBUG		// for easier writing the program - doesn't collide with std CH
	// set "run with system" registry settings
	SetAutorun(m_cfgManager.GetBoolValue(PP_PRELOADAFTERRESTART));
#endif

	// check instance - return false if it's the second one
	if (!IsFirstInstance())
	{
		MsgBox(IDS_ONECOPY_STRING);
		return FALSE;
	}

	m_pMainWindow=new CMainWnd;
	if (!((CMainWnd*)m_pMainWindow)->Create())
		return FALSE;				// will be deleted at destructor

	m_pMainWnd = m_pMainWindow;

	return TRUE;
}

void CCopyHandlerApp::OnConfigNotify(UINT uiType, LPARAM lParam)
{
	// is this language
	if (uiType == CNFT_PROFILECHANGE || (uiType == CNFT_PROPERTYCHANGE && ((UINT)lParam) == PP_PLANGUAGE))
	{
		// update language in resource manager
		TCHAR szPath[_MAX_PATH];
		m_cfgManager.GetStringValue(PP_PLANGUAGE, szPath, _MAX_PATH);
		m_resManager.SetLanguage(ExpandPath(szPath));
	}
	if (uiType == CNFT_PROFILECHANGE || (uiType == CNFT_PROPERTYCHANGE && ((UINT)lParam) == PP_PHELPDIR))
	{
		if (UpdateHelpPaths())
			HtmlHelp(HH_CLOSE_ALL, NULL);
	}
}

void CCopyHandlerApp::OnResManNotify(UINT uiType, LPARAM lParam)
{
	if (uiType == RMNT_LANGCHANGE)
	{
		// language has been changed - close the current help file
		if (UpdateHelpPaths())
			HtmlHelp(HH_CLOSE_ALL, NULL);
	}
}

HWND CCopyHandlerApp::HHelp(HWND hwndCaller, LPCSTR pszFile, UINT uCommand, DWORD dwData)
{
	PCTSTR pszPath=NULL;
	WIN32_FIND_DATA wfd;
	HANDLE handle=::FindFirstFile(m_szHelpPath, &wfd);
	if (handle != INVALID_HANDLE_VALUE)
	{
		pszPath=m_szHelpPath;
		::FindClose(handle);
	}

	if (pszPath == NULL)
		return NULL;

	if (pszFile != NULL)
	{
		TCHAR szAdd[2*_MAX_PATH];
		_tcscpy(szAdd, pszPath);
		_tcscat(szAdd, pszFile);
		return ::HtmlHelp(hwndCaller, szAdd, uCommand, dwData);
	}
	else
		return ::HtmlHelp(hwndCaller, pszPath, uCommand, dwData);
}

bool CCopyHandlerApp::HtmlHelp(UINT uiCommand, LPARAM lParam)
{
	switch (uiCommand)
	{
	case HH_DISPLAY_TOPIC:
	case HH_HELP_CONTEXT:
		{
			return HHelp(GetDesktopWindow(), NULL, uiCommand, lParam) != NULL;
			break;
		}
	case HH_CLOSE_ALL:
		return ::HtmlHelp(NULL, NULL, HH_CLOSE_ALL, NULL) != NULL;
		break;
	case HH_DISPLAY_TEXT_POPUP:
		{
			HELPINFO* pHelp=(HELPINFO*)lParam;
			if ( pHelp->dwContextId == 0 || pHelp->iCtrlId == 0
				|| ::GetWindowContextHelpId((HWND)pHelp->hItemHandle) == 0)
				return false;

			HH_POPUP hhp;
			hhp.cbStruct=sizeof(HH_POPUP);
			hhp.hinst=NULL;
			hhp.idString=(pHelp->dwContextId & 0xffff);
			hhp.pszText=NULL;
			hhp.pt=pHelp->MousePos;
			hhp.pt.y+=::GetSystemMetrics(SM_CYCURSOR)/2;
			hhp.clrForeground=(COLORREF)-1;
			hhp.clrBackground=(COLORREF)-1;
			hhp.rcMargins.left=-1;
			hhp.rcMargins.right=-1;
			hhp.rcMargins.top=-1;
			hhp.rcMargins.bottom=-1;
			hhp.pszFont=_T("Tahoma, 8, , ");

			TCHAR szPath[_MAX_PATH];
			_stprintf(szPath, _T("::/%lu.txt"), (pHelp->dwContextId >> 16) & 0x7fff);
			return (HHelp(GetDesktopWindow(), szPath, HH_DISPLAY_TEXT_POPUP, (DWORD)&hhp) != NULL);

			break;
		}
	}

	return true;
}
