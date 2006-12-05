/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
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
#ifndef __SHAREDDATA_H__
#define __SHAREDDATA_H__

// drag&drop flags
#define OPERATION_MASK				0x00ffffff
#define DD_COPY_FLAG				0x00000001
#define DD_MOVE_FLAG				0x00000002
#define DD_COPYMOVESPECIAL_FLAG		0x00000004

#define EC_PASTE_FLAG				0x00000010
#define EC_PASTESPECIAL_FLAG		0x00000020
#define EC_COPYTO_FLAG				0x00000040
#define EC_MOVETO_FLAG				0x00000080
#define EC_COPYMOVETOSPECIAL_FLAG	0x00000100

// messages used
#define WM_GETCONFIG	WM_USER+20

// config type to get from program
#define GC_DRAGDROP		0x00
#define GC_EXPLORER		0x01

// command properties (used in menu displaying)
#pragma pack(push, 1)
struct _COMMAND
{
	UINT uiCommandID;		// command ID - would be send be
	TCHAR szCommand[128];	// command name
	TCHAR szDesc[128];		// and it's description
};
#pragma pack(pop)

#pragma pack(push, 1)
struct _SHORTCUT
{
	TCHAR szName[128];
	TCHAR szPath[_MAX_PATH];
};
#pragma pack(pop)

// shared memory size in bytes
#define SHARED_BUFFERSIZE	65536

// structure used for passing data from program to DLL
// the rest is a dynamic texts
class CSharedConfigStruct
{
public:
	UINT uiFlags;				// what items and how to display in drag&drop ctx menu & explorer.ctx.menu

	bool bShowFreeSpace;		// showthe free space by the shortcuts ?
	TCHAR szSizes[6][64];		// names of the kB, GB, ...
	bool bShowShortcutIcons;	// show shell icons with shortcuts ?
	bool bOverrideDefault;		// only for d&d - want to change menu default item to the one from ch ?
	UINT uiDefaultAction;		// default action for drag&drop when using above option
	int iCommandCount;			// count of commands stored at the beginning of a buffer
	int iShortcutsCount;		// count of shortcuts to display in submenus
	
	TCHAR szData[SHARED_BUFFERSIZE];		// buffer for texts and other stuff
};

#endif