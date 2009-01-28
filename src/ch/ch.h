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
#ifndef __COPYHANDLER_H__
#define __COPYHANDLER_H__

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "AppHelper.h"
#include "CfgProperties.h"
#include "../chext/chext.h"
//#include "LogFile.h"
#include "../libicpf/log.h"
#include "../libchcore/EngineCfg.h"
#include "../libictranslate/ResourceManager.h"

using namespace std;

/////////////////////////////////////////////////////////////////////////////
// CCopyHandlerApp:
// See CopyHandler.cpp for the implementation of this class
//

class CCopyHandlerApp : public CWinApp, public CAppHelper
{
public:
	CCopyHandlerApp();
	~CCopyHandlerApp();

// Overrides
public:
	virtual BOOL InitInstance();


	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd);

	PCTSTR GetHelpPath() const { return m_pszHelpFilePath; };

	friend LRESULT MainRouter(ULONGLONG ullDst, UINT uiMsg, WPARAM wParam, LPARAM lParam);
	friend int MsgBox(UINT uiID, UINT nType=MB_OK, UINT nIDHelp=0);
	friend CCopyHandlerApp* GetApp();
	friend ictranslate::CResourceManager* GetResManager();
	friend chcore::engine_config* GetConfig();
//	friend CLogFile* GetLog();

	bool IsShellExtEnabled() const;

	void OnConfigNotify(uint_t uiPropID);
	void OnResManNotify(UINT uiType);
protected:
	bool UpdateHelpPaths();
	HWND HHelp(HWND hwndCaller, LPCTSTR pszFile, UINT uCommand, DWORD dwData);

public:
	ictranslate::CResourceManager m_resManager;
//	CConfigManager m_cfgManager;
	chcore::engine_config m_cfgSettings;
	icpf::log_file m_lfLog;

	IShellExtControl* m_piShellExtControl;

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
public:
	virtual int ExitInstance();
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif
