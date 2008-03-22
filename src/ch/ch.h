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

#ifndef __COPYHANDLER_H__
#define __COPYHANDLER_H__

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "AppHelper.h"
#include "CfgProperties.h"
//#include "LogFile.h"
#include "../libicpf/log.h"
#include "../libicpf/cfg.h"
#include "../libictranslate/ResourceManager.h"
using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CCopyHandlerApp:
// See CopyHandler.cpp for the implementation of this class
//

class CCopyHandlerApp : public CWinApp, public CAppHelper
{
public:
//	BOOL RegisterShellExt();
	CCopyHandlerApp();
	~CCopyHandlerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCopyHandlerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd);

	PCTSTR GetHelpPath() const { return m_pszHelpFilePath; };

	friend LRESULT MainRouter(ULONGLONG ullDst, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	friend int MsgBox(UINT uiID, UINT nType=MB_OK, UINT nIDHelp=0);
	friend CCopyHandlerApp* GetApp();
	friend ictranslate::CResourceManager* GetResManager();
	friend icpf::config* GetConfig();
//	friend CLogFile* GetLog();

	void OnConfigNotify(uint_t uiPropID);
	void OnResManNotify(UINT uiType);
protected:
	bool UpdateHelpPaths();
	HWND HHelp(HWND hwndCaller, LPCTSTR pszFile, UINT uCommand, DWORD dwData);

public:
	ictranslate::CResourceManager m_resManager;
//	CConfigManager m_cfgManager;
	icpf::config m_cfgSettings;
	icpf::log_file m_lfLog;

	CWnd *m_pMainWindow;
	// currently opened dialogs
//	list<CWnd*> m_lhDialogs;

protected:
// Implementation
	HANDLE m_hMapObject;
	//TCHAR m_szHelpPath[_MAX_PATH];	// full file path to the help file
//	CString m_strCrashInfo;			// crash info text

	//{{AFX_MSG(CCopyHandlerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
