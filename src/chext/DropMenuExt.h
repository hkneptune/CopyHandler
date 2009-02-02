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
#include "IShellExtInitImpl.h"
#include "IContextMenuImpl.h"
#include "comdef.h"
#include "..\Common\ipcstructs.h"

/////////////////////////////////////////////////////////////////////////////
// CDropMenuExt
class ATL_NO_VTABLE CDropMenuExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CDropMenuExt, &CLSID_DropMenuExt>,
	public IObjectWithSiteImpl<CDropMenuExt>,
	public IDispatchImpl<IDropMenuExt, &IID_IDropMenuExt, &LIBID_CHEXTLib>,
	public IShellExtInit,
	public IContextMenu3
{
public:
	CDropMenuExt();
	~CDropMenuExt();

DECLARE_REGISTRY_RESOURCEID(IDR_DROPMENUEXT)
DECLARE_NOT_AGGREGATABLE(CDropMenuExt)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CDropMenuExt)
	COM_INTERFACE_ENTRY(IDropMenuExt)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IContextMenu)
	COM_INTERFACE_ENTRY(IObjectWithSite)
END_COM_MAP()

// IDropMenuExt
public:
	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO lpici);
	STDMETHOD(Initialize)(LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY /*hkeyProgID*/);
	STDMETHOD(GetCommandString)(UINT_PTR idCmd, UINT uFlags, UINT* /*pwReserved*/, LPSTR pszName, UINT cchMax);
	STDMETHOD(QueryContextMenu)(HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT /*idCmdLast*/, UINT uFlags);

	STDMETHOD(HandleMenuMsg)(UINT, WPARAM, LPARAM)
	{
		return S_FALSE;
	}

	STDMETHOD(HandleMenuMsg2)(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, LRESULT* /*plResult*/)
	{
		return S_FALSE;
	}

protected:
	class CBuffer
	{
	public:
		CBuffer() { m_pszFiles=NULL; m_iDataSize=0; };
		void Destroy() { delete [] m_pszFiles; m_pszFiles=NULL; m_iDataSize=0; };
		~CBuffer() { Destroy(); };

	public:
		TCHAR *m_pszFiles;
		UINT m_iDataSize;
	} m_bBuffer;

	TCHAR m_szDstPath[_MAX_PATH];
	UINT m_uiDropEffect;
	bool m_bExplorer;			// if the operation has been retrieved from explorer or from the program

	IShellExtControl* m_piShellExtControl;
};

#endif //__DROPMENUEXT_H_
