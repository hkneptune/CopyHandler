//******************************************************************************
//   Copyright (C) 2001-2008 by Jozef Starosczyk
//   ixen@copyhandler.com
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Library General Public License
//   (version 2) as published by the Free Software Foundation;
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU Library General Public
//   License along with this program; if not, write to the
//   Free Software Foundation, Inc.,
//   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
/// @file ClipboardMonitor.h
/// @brief Contains clipboard monitoring package.
//******************************************************************************
#ifndef __CLIPBOARDMONITOR_H__
#define __CLIPBOARDMONITOR_H__

#include "..\libchcore\TWorkerThreadController.h"
#include "..\libchcore\TTaskManager.h"

namespace chcore
{
	class TTaskManager;
}

class CClipboardMonitor
{
public:
	static void StartMonitor(chcore::TTaskManagerPtr spTasks);
	static void StopMonitor();

	void Start(chcore::TTaskManagerPtr spTasks);
	void Stop();

protected:
	CClipboardMonitor();
	~CClipboardMonitor();

	static DWORD WINAPI ClipboardMonitorProc(LPVOID pParam);

protected:
	static CClipboardMonitor S_ClipboardMonitor;

	chcore::TTaskManagerPtr m_spTasks;

	// thread control
	chcore::TWorkerThreadController m_threadWorker;
};

#endif
