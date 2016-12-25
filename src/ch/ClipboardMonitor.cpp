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
/// @file ClipboardMonitor.cpp
/// @brief Contains the implementation of clipboard monitor package.
//******************************************************************************
#include "stdafx.h"
#include "../libchcore/TWorkerThreadController.h"
#include "ClipboardMonitor.h"
#include "ch.h"
#include "CfgProperties.h"
#include "FolderDialog.h"
#include "ShutdownDlg.h"
#include "DirectoryChooser.h"
#include "TTaskManagerWrapper.h"
#include "resource.h"

using namespace chcore;

CClipboardMonitor CClipboardMonitor::S_ClipboardMonitor;

CClipboardMonitor::CClipboardMonitor()
{
}

CClipboardMonitor::~CClipboardMonitor()
{
	Stop();
}

void CClipboardMonitor::StartMonitor(chengine::TTaskManagerPtr spTasks)
{
	CClipboardMonitor::S_ClipboardMonitor.Start(spTasks);
}

void CClipboardMonitor::StopMonitor()
{
	return CClipboardMonitor::S_ClipboardMonitor.Stop();
}

void CClipboardMonitor::Start(chengine::TTaskManagerPtr spTasks)
{
	m_spTasks = spTasks;

	m_threadWorker.StartThread(&CClipboardMonitor::ClipboardMonitorProc, this);
}

void CClipboardMonitor::Stop()
{
	m_threadWorker.StopThread();
}

DWORD WINAPI CClipboardMonitor::ClipboardMonitorProc(LPVOID pParam)
{
	CClipboardMonitor* pData = (CClipboardMonitor*)pParam;

	// bufor
	TCHAR path[_MAX_PATH];

	// register clipboard format
	UINT nFormat=RegisterClipboardFormat(_T("Preferred DropEffect"));
	UINT uiCounter=0, uiShutCounter=0;

	chengine::TConfig& rConfig = GetConfig();
	for(;;)
	{
		if (uiCounter == 0 && GetPropValue<PP_PCLIPBOARDMONITORING>(rConfig) && IsClipboardFormatAvailable(CF_HDROP))
		{
			// get data from clipboard
			OpenClipboard(nullptr);
			HANDLE handle=GetClipboardData(CF_HDROP);

			UINT nCount=DragQueryFile(static_cast<HDROP>(handle), 0xffffffff, nullptr, 0);

			chengine::TTaskDefinition tTaskDefinition;

			// list of files
			for(UINT stIndex = 0; stIndex < nCount; stIndex++)
			{
				DragQueryFile(static_cast<HDROP>(handle), stIndex, path, _MAX_PATH);

				tTaskDefinition.AddSourcePath(chcore::PathFromString(path));
			}

			// operation type
			chengine::EOperationType eOperation = chengine::eOperation_Copy;

			if(IsClipboardFormatAvailable(nFormat))
			{
				handle=GetClipboardData(nFormat);
				LPVOID addr=GlobalLock(handle);

				DWORD dwData=((DWORD*)addr)[0];
				if(dwData & DROPEFFECT_COPY)
					eOperation = chengine::eOperation_Copy;	// copy
				else if(dwData & DROPEFFECT_MOVE)
					eOperation = chengine::eOperation_Move;	// move

				GlobalUnlock(handle);
			}
			else
				eOperation = chengine::eOperation_Copy;	// default - copy

			tTaskDefinition.SetOperationType(eOperation);	// copy

			// set the default options for task
			GetConfig().ExtractSubConfig(BRANCH_TASK_SETTINGS, tTaskDefinition.GetConfiguration());

			EmptyClipboard();
			CloseClipboard();

			TSmartPath pathSelected;
			INT_PTR iResult = DirectoryChooser::ChooseDirectory(eOperation, tTaskDefinition.GetSourcePaths(), pathSelected);
			if(iResult == IDOK)
			{
				// get dest path
				tTaskDefinition.SetDestinationPath(pathSelected);

				TTaskManagerWrapper tTaskManager(pData->m_spTasks);

				tTaskManager.CreateTask(tTaskDefinition);
			}
		}

		// do we need to check for turning computer off
		if(uiShutCounter == 0 && GetPropValue<PP_PSHUTDOWNAFTREFINISHED>(GetConfig()))
		{
			if(pData->m_spTasks->AreAllFinished())
			{
				TRACE("Shut down windows\n");
				bool bShutdown=true;
				if (GetPropValue<PP_PTIMEBEFORESHUTDOWN>(GetConfig()) != 0)
				{
					CShutdownDlg dlg;
					dlg.SetOverallTime(GetPropValue<PP_PTIMEBEFORESHUTDOWN>(GetConfig()));
					bShutdown=(dlg.DoModal() != IDCANCEL);
				}

				SetPropValue<PP_PSHUTDOWNAFTREFINISHED>(GetConfig(), false);
				GetConfig().Write();
				if(bShutdown)
				{
					// adjust token privileges for NT
					HANDLE hToken = nullptr;
					TOKEN_PRIVILEGES tp = { 0 };
					if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)
						&& LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid))
					{
						tp.PrivilegeCount=1;
						tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

						AdjustTokenPrivileges(hToken, FALSE, &tp, 0, nullptr, nullptr);

					}

					if(hToken)
						CloseHandle(hToken);

					BOOL bExit=ExitWindowsEx(EWX_POWEROFF | EWX_SHUTDOWN | (GetPropValue<PP_PFORCESHUTDOWN>(GetConfig()) ? EWX_FORCE : 0), 0);
					if (bExit)
						return 1;

					// some kind of error
					ictranslate::CFormat fmt(GetResManager().LoadString(IDS_SHUTDOWNERROR_STRING));
					fmt.SetParam(_T("%errno"), GetLastError());
					AfxMessageBox(fmt.ToString(), MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
				}
			}
		}

		// sleep for some time
		const int iSleepCount=200;
		
		if(pData->m_threadWorker.KillRequested(iSleepCount))
			break;
		
		uiCounter+=iSleepCount;
		uiShutCounter+=iSleepCount;
		if(uiCounter >= GetPropValue<PP_PMONITORSCANINTERVAL>(GetConfig()))
			uiCounter=0;
		if(uiShutCounter >= 800)
			uiShutCounter=0;
	}

	TRACE("Monitoring clipboard proc aborted...\n");

	return 0;
}
