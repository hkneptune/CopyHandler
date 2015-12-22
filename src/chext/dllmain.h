// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#ifndef __DLLMAIN_H__
#define __DLLMAIN_H__

class CCHExtModule : public CAtlDllModuleT<CCHExtModule>
{
public :
	DECLARE_LIBID(LIBID_CHEXTLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_CHEXT, "{9D4C4C5F-EE90-4a6b-9245-244C369E4FAE}")
};

extern class CCHExtModule _AtlModule;

#endif
