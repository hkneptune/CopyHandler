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
#ifndef __ISHELLEXTINITIMPL_H__
#define __ISHELLEXTINITIMPL_H__

// IShellExtInitImpl.h
//
//////////////////////////////////////////////////////////////////////
#include <AtlCom.h>
#include <ShlObj.h>


class ATL_NO_VTABLE IShellExtInitImpl : public IShellExtInit
{
public:

	// IUnknown
	//
	STDMETHOD(QueryInterface)(REFIID riid, void** ppvObject) = 0;
	_ATL_DEBUG_ADDREF_RELEASE_IMPL( IShellExtInitImpl )


	// IShellExtInit
	//
	STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT, HKEY)
	{
		return S_FALSE;
	};

};

#endif