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
#ifndef __MENUEXT_H_
#define __MENUEXT_H_

#include "resource.h"       // main symbols
#include "TContextMenuHandler.h"
#include "..\common\TShellExtMenuConfig.h"
#include "TShellExtData.h"
#include "..\liblogger\TLogger.h"

class TShellMenuItem;

/////////////////////////////////////////////////////////////////////////////
// CMenuExt
class ATL_NO_VTABLE CMenuExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMenuExt, &CLSID_MenuExt>,
	public IShellExtInit,
	public IContextMenu3
{
public:
	CMenuExt();
	~CMenuExt();

DECLARE_REGISTRY_RESOURCEID(IDR_MENUEXT)
DECLARE_NOT_AGGREGATABLE(CMenuExt)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CMenuExt)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
	COM_INTERFACE_ENTRY(IContextMenu2)
	COM_INTERFACE_ENTRY(IContextMenu3)
END_COM_MAP()

// IMenuExt
public:
	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, IDataObject* piDataObject, HKEY /*hkeyProgID*/);
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
	STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax);
	STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT /*uFlags*/);
	STDMETHOD(HandleMenuMsg)(UINT uMsg, WPARAM wParam, LPARAM lParam);
	STDMETHOD(HandleMenuMsg2)(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult);

protected:
	HRESULT DrawMenuItem(LPDRAWITEMSTRUCT lpdis);

private:
	TShellExtData m_tShellExtData;

	TShellExtMenuConfig m_tShellExtMenuConfig;
	TContextMenuHandler m_tContextMenuHandler;

	IShellExtControl* m_piShellExtControl;
	logger::TLoggerPtr m_spLog;
};

#endif //__MENUEXT_H_
