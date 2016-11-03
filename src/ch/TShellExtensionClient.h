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
#ifndef __TSHELLEXTENSIONCLIENT_H__
#define __TSHELLEXTENSIONCLIENT_H__

#include "../chext/chext.h"
#include "../liblogger/TLogger.h"
#include "../common/ERegistrationResult.h"

class TShellExtensionClient
{
public:
	TShellExtensionClient();
	~TShellExtensionClient();

	// registers or unregisters shell extension
	ERegistrationResult RegisterShellExtDll(long lClientVersion, long& rlExtensionVersion, CString& rstrExtensionStringVersion);
	ERegistrationResult UnRegisterShellExtDll();

	// enables the extension if compatible with the client (CH) version
	// returns S_OK if enabled, S_FALSE if not
	HRESULT EnableExtensionIfCompatible(long lClientVersion, long& rlExtensionVersion, CString& rstrExtensionStringVersion);

	void Close();

private:
	HRESULT RetrieveControlInterface();
	void FreeControlInterface();
	HRESULT InitializeCOM();
	void UninitializeCOM();
	bool DetectRegExe();

	logger::TLoggerPtr& GetLogger();

private:
	logger::TLoggerPtr m_spLog;
	bool m_bInitialized;
	IShellExtControl* m_piShellExtControl;
	std::wstring m_strRegExe;
};

#endif