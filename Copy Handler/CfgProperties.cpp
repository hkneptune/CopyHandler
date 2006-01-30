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
#include "stdafx.h"
#include "CfgProperties.h"

bool RegisterProperties(CConfigManager* pManager)
{
	pManager->RegisterBoolProperty(_T("Program"),	_T("Enabled clipboard monitoring"), false);
	pManager->RegisterIntProperty(_T("Program"),	_T("Monitor scan interval"), 1000);
	pManager->RegisterBoolProperty(_T("Program"),	_T("Reload after restart"), false);
	pManager->RegisterBoolProperty(_T("Program"),	_T("Shutdown system after finished"), false);
	pManager->RegisterIntProperty(_T("Program"),	_T("Time before shutdown"), 10000);
	pManager->RegisterBoolProperty(_T("Program"),	_T("Force shutdown"), false);
	pManager->RegisterIntProperty(_T("Program"),	_T("Autosave interval"), 30000);
	pManager->RegisterIntProperty(_T("Program"),	_T("Process priority class"), NORMAL_PRIORITY_CLASS);
	pManager->RegisterStringProperty(_T("Program"),	_T("Autosave directory"), _T("<TEMP>\\"), RF_PATH);
	pManager->RegisterStringProperty(_T("Program"), _T("Plugins directory"), _T("<PROGRAM>\\Plugins\\"), RF_PATH);
	pManager->RegisterStringProperty(_T("Program"), _T("Help directory"), _T("<PROGRAM>\\Help\\"), RF_PATH);
	pManager->RegisterStringProperty(_T("Program"),	_T("Language"), _T("<PROGRAM>\\Langs\\English.lng"));
	pManager->RegisterStringProperty(_T("Program"), _T("Languages directory"), _T("<PROGRAM>\\Langs\\"), RF_PATH);

	pManager->RegisterIntProperty(_T("Status dialog"), _T("Status refresh interval"), 1000);
	pManager->RegisterBoolProperty(_T("Status dialog"), _T("Show details"), true);
	pManager->RegisterBoolProperty(_T("Status dialog"), _T("Auto remove finished"), false);

	pManager->RegisterIntProperty(_T("Folder dialog"), _T("Dialog width"), -1);
	pManager->RegisterIntProperty(_T("Folder dialog"), _T("Dialog height"), -1);
	pManager->RegisterIntProperty(_T("Folder dialog"), _T("Shortcut list style"), 1);
	pManager->RegisterBoolProperty(_T("Folder dialog"), _T("Extended view"), true);
	pManager->RegisterBoolProperty(_T("Folder dialog"), _T("Ignore shell dialogs"), false);

	pManager->RegisterBoolProperty(_T("Mini view"), _T("Show filenames"), true);
	pManager->RegisterBoolProperty(_T("Mini view"), _T("Show single tasks"), true);
	pManager->RegisterIntProperty(_T("Mini view"), _T("Miniview refresh interval"), 200);
	pManager->RegisterBoolProperty(_T("Mini view"), _T("Autoshow when run"), true);
	pManager->RegisterBoolProperty(_T("Mini view"), _T("Autohide when empty"), true);
	pManager->RegisterBoolProperty(_T("Mini view"), _T("Use smooth progress"), true);

	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Use auto-complete files"), true);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Always set destination attributes"), true);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Always set destination time"), true);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Protect read-only files"), false);
	pManager->RegisterIntProperty(_T("Copying/moving"), _T("Limit max operations"), 1);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Read tasks size before blocking"), true);
	pManager->RegisterIntProperty(_T("Copying/moving"), _T("Show visual feedback"), 2);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Use timed feedback dialogs"), false);
	pManager->RegisterIntProperty(_T("Copying/moving"), _T("Feedback time"), 60000);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Auto retry on error"), true);
	pManager->RegisterIntProperty(_T("Copying/moving"), _T("Auto retry interval"), 10000);
	pManager->RegisterIntProperty(_T("Copying/moving"), _T("Default priority"), THREAD_PRIORITY_NORMAL);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Disable priority boost"), false);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Delete files after finished"), true);
	pManager->RegisterBoolProperty(_T("Copying/moving"), _T("Create log file"), true);

	pManager->RegisterBoolProperty(_T("Shell"), _T("Show 'Copy' command"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show 'Move' command"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show 'Copy/move special' command"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show 'Paste' command"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show 'Paste special' command"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show 'Copy to' command"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show 'Move to' command"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show 'Copy to/Move to special' command"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show free space along with shortcut"), true);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Show shell icons in shortcuts menu"), false);
	pManager->RegisterBoolProperty(_T("Shell"), _T("Use drag&drop default menu item override"), true);
	pManager->RegisterIntProperty(_T("Shell"), _T("Default action when dragging"), 3);

	pManager->RegisterBoolProperty(_T("Buffer"), _T("Use only default buffer"), false);
	pManager->RegisterIntProperty(_T("Buffer"), _T("Default buffer size"), 2097152);
	pManager->RegisterIntProperty(_T("Buffer"), _T("One physical disk buffer size"), 4194304);
	pManager->RegisterIntProperty(_T("Buffer"), _T("Two different hard disks buffer size"), 524288);
	pManager->RegisterIntProperty(_T("Buffer"), _T("CD buffer size"), 262144);
	pManager->RegisterIntProperty(_T("Buffer"), _T("LAN buffer size"), 131072);
	pManager->RegisterBoolProperty(_T("Buffer"), _T("Use no buffering for large files"), true);
	pManager->RegisterIntProperty(_T("Buffer"), _T("Large files lower boundary limit"), 2097152);

	pManager->RegisterStringProperty(_T("Log file"), _T("Path to main log file"), _T("<PROGRAM>\\ch.log"));
	pManager->RegisterBoolProperty(_T("Log file"), _T("Enable logging"), true);
	pManager->RegisterBoolProperty(_T("Log file"), _T("Enable log size limitation"), true);
	pManager->RegisterIntProperty(_T("Log file"), _T("Max log size limit"), 65535);
	pManager->RegisterBoolProperty(_T("Log file"), _T("Precise log size limiting"), false);
	pManager->RegisterIntProperty(_T("Log file"), _T("Truncation buffer size"), 65535, PDL_PARANOID);

	pManager->RegisterBoolProperty(_T("Sounds"), _T("Play sounds"), true);
	pManager->RegisterStringProperty(_T("Sounds"), _T("Error sound path"), _T("<WINDOWS>\\media\\chord.wav"));
	pManager->RegisterStringProperty(_T("Sounds"), _T("Finished sound path"), _T("<WINDOWS>\\media\\ding.wav"));

	pManager->RegisterStringArrayProperty(_T("Shortcuts"));
	pManager->RegisterStringArrayProperty(_T("Recent paths"));

	return true;
}
