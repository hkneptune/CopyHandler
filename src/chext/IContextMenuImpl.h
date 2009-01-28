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