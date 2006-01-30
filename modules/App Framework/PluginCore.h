/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2003 Ixen Gerthannes (ixen@interia.pl)

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
#ifndef __PLUGINCORE_H__
#define __PLUGINCORE_H__

typedef LRESULT(*PFNEXTINFOPROC)(ULONGLONG dwModuleID, UINT uMsg, WPARAM wParam, LPARAM lParam);

// plugin structures
struct _PLUGININFO
{
	ULONG ulSize;					// size of this structure
	ULARGE_INTEGER uliSignature;	// 64-bit unique ID of this plugin

	char szProgID[32];				// program string identificator for which this plugin was written - current "1.x"
	UINT uiMask;					// what this dll can do (type of this plugin - PT_... value)

	char szPluginName[128];			// full name of this plugin (preferred english description)
	struct _VERSION
	{
		WORD wMajor;
		WORD wMinor;
		WORD wRelease;
		WORD wBuild;
	} vVersion;							// plugin version
	char szAuthor[128];					// name of the author (or company name or both)
	char szPluginDescription[256];		// plugin info - what it can do, ... (preferred english)

	// update info
	char szUpdateInfo[256];			// internet address of a file with new plugin info (address, version, ...). Currently unused, but may be in future.
};

#endif