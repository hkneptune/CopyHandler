/************************************************************************
	Copy Handler 1.x - program for copying data	in Microsoft Windows
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
#ifndef __ICONTEXTMENUIMPL_H__
#define __ICONTEXTMENUIMPL_H__

// IContextMenuImpl.h
//
//////////////////////////////////////////////////////////////////////
#include <AtlCom.h>
#include <ShlObj.h>


class ATL_NO_VTABLE IContextMenuImpl : public IContextMenu3
{
public:

	// IUnknown
	//
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) = 0;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL( IContextMenuImpl )


	// IContextMenu
	//
	STDMETHOD(GetCommandString)(UINT, UINT, UINT*, LPSTR, UINT)
	{
		return S_FALSE;
	}

	STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO)
	{
		return S_FALSE;
	}

	STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT , UINT, UINT)
	{
		return S_FALSE;
	}
	
	STDMETHOD(HandleMenuMsg)(UINT, WPARAM, LPARAM)
	{
		return S_FALSE;
	}

	STDMETHOD(HandleMenuMsg2)(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, LRESULT* /*plResult*/)
	{
		return S_FALSE;
	}
};

#endif