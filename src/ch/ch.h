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

#include "AppHelper.h"
#include "../libictranslate/ResourceManager.h"
#include "../libchcore/TConfig.h"
#include "TShellExtensionClient.h"
#include "TCommandLineParser.h"
#include "../common/TLogger.h"
#include "../common/TLoggerInitializer.h"
#include "../libchcore/TCoreEngine.h"
#include "../common/TLoggerFactory.h"

class CCopyHandlerApp : public CWinApp, public CAppHelper
{
public:
	CCopyHandlerApp();
	~CCopyHandlerApp();

	virtual BOOL InitInstance() override;
	virtual int ExitInstance() override;

	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd) override;

	PCTSTR GetHelpPath() const { return m_pszHelpFilePath; };

	friend int MsgBox(UINT uiID, UINT nType=MB_OK, UINT nIDHelp=0);

	friend CCopyHandlerApp& GetApplication();
	static ictranslate::CResourceManager& GetResManager();
	static chcore::TConfig& GetConfig();

	chcore::TLoggerFactoryPtr GetLogFactory();

	void RegisterShellExtension();
	void UnregisterShellExtension();

	void OnConfigNotify(const chcore::TStringSet& setPropNames);
	void OnResManNotify(UINT uiType);

	const TCommandLineParser& GetCommandLine() const { return m_cmdLineParser; }

protected:
	bool UpdateHelpPaths();
	HWND HHelp(HWND hwndCaller, LPCTSTR pszFile, UINT uCommand, DWORD_PTR dwData);

	void InitShellExtension();
	bool ParseCommandLine();

protected:
	TShellExtensionClient m_tShellExtClient;
	TCommandLineParser m_cmdLineParser;

	chcore::TCoreEngine m_chEngine;
	TLoggerInitializer m_logInitializer;
	chcore::TLoggerFactoryPtr m_spLogFactory;
	chcore::TLoggerPtr m_spLog;

	CWnd *m_pMainWindow;

	DECLARE_MESSAGE_MAP()
};

inline CCopyHandlerApp& GetApp()
{
	return GetApplication();
}

inline chcore::TLoggerFactoryPtr GetLogFactory()
{
	return GetApp().GetLogFactory();
}

inline ictranslate::CResourceManager& GetResManager()
{
	return CCopyHandlerApp::GetResManager();
}

inline chcore::TConfig& GetConfig()
{
	return CCopyHandlerApp::GetConfig();
}

#endif
