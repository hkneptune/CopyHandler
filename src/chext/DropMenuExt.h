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
#ifndef __DROPMENUEXT_H_
#define __DROPMENUEXT_H_

#include "resource.h"       // main symbols
#include "TContextMenuHandler.h"
#include "../common/TShellExtMenuConfig.h"
#include "TShellExtData.h"
#include "../liblogger/TLogger.h"
#include "ShellExtControl.h"

/////////////////////////////////////////////////////////////////////////////
// CDropMenuExt
class CDropMenuExt :
	public IShellExtInit,
	public IContextMenu3
{
public:
	CDropMenuExt();
	~CDropMenuExt();

public:
	STDMETHODIMP QueryInterface(REFIID, LPVOID FAR *) override;
	STDMETHODIMP_(ULONG) AddRef() override;
	STDMETHODIMP_(ULONG) Release() override;

	STDMETHOD(InvokeCommand) (LPCMINVOKECOMMANDINFO lpici);
	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject* piDataObject, HKEY /*hkeyProgID*/);
	STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax);
	STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT uFlags);

	STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);
	STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

protected:
	volatile ULONG m_ulRefCnt = 0;

	IShellExtControl* m_piShellExtControl;

	TShellExtData m_tShellExtData;
	TShellExtMenuConfig m_tShellExtMenuConfig;

	TContextMenuHandler m_tContextMenuHandler;

	chcore::TPathContainer m_vPaths;
	chcore::TSmartPath m_pathPidl;

	logger::TLoggerPtr m_spLog;
};

#endif //__DROPMENUEXT_H_
