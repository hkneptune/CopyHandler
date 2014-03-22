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
#include "../libchcore/TTaskManager.h"
#include "../libchcore/TTask.h"
#include "CfgProperties.h"
#include "FolderDialog.h"
#include "ShutdownDlg.h"

using namespace chcore;

CClipboardMonitor CClipboardMonitor::S_ClipboardMonitor;

CClipboardMonitor::CClipboardMonitor()
{
}

CClipboardMonitor::~CClipboardMonitor()
{
	Stop();
}

void CClipboardMonitor::StartMonitor(chcore::TTaskManager* pTasks)
{
	CClipboardMonitor::S_ClipboardMonitor.Start(pTasks);
}

void CClipboardMonitor::StopMonitor()
{
	return CClipboardMonitor::S_ClipboardMonitor.Stop();
}

void CClipboardMonitor::Start(chcore::TTaskManager* pTasks)
{
	m_pTasks = pTasks;

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

	chcore::TConfig& rConfig = GetConfig();
	for(;;)
	{
		if (uiCounter == 0 && GetPropValue<PP_PCLIPBOARDMONITORING>(rConfig) && IsClipboardFormatAvailable(CF_HDROP))
		{
			// get data from clipboard
			OpenClipboard(NULL);
			HANDLE handle=GetClipboardData(CF_HDROP);

			UINT nCount=DragQueryFile(static_cast<HDROP>(handle), 0xffffffff, NULL, 0);

			chcore::TTaskDefinition tTaskDefinition;

			// list of files
			for(UINT stIndex = 0; stIndex < nCount; stIndex++)
			{
				DragQueryFile(static_cast<HDROP>(handle), stIndex, path, _MAX_PATH);

				tTaskDefinition.AddSourcePath(chcore::PathFromString(path));
			}

			// operation type
			chcore::EOperationType eOperation = chcore::eOperation_Copy;

			if(IsClipboardFormatAvailable(nFormat))
			{
				handle=GetClipboardData(nFormat);
				LPVOID addr=GlobalLock(handle);

				DWORD dwData=((DWORD*)addr)[0];
				if(dwData & DROPEFFECT_COPY)
					eOperation = chcore::eOperation_Copy;	// copy
				else if(dwData & DROPEFFECT_MOVE)
					eOperation = chcore::eOperation_Move;	// move

				GlobalUnlock(handle);
			}
			else
				eOperation = chcore::eOperation_Copy;	// default - copy

			tTaskDefinition.SetOperationType(eOperation);	// copy

			// set the default options for task
			GetConfig().ExtractSubConfig(BRANCH_TASK_SETTINGS, tTaskDefinition.GetConfiguration());

			EmptyClipboard();
			CloseClipboard();

			// get dest folder
			CFolderDialog dlg;

			GetPropValue<PP_SHORTCUTS>(rConfig, dlg.m_bdData.cvShortcuts);
			GetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_bdData.cvRecent);

			dlg.m_bdData.bExtended=GetPropValue<PP_FDEXTENDEDVIEW>(rConfig);
			dlg.m_bdData.cx=GetPropValue<PP_FDWIDTH>(rConfig);
			dlg.m_bdData.cy=GetPropValue<PP_FDHEIGHT>(rConfig);
			dlg.m_bdData.iView=GetPropValue<PP_FDSHORTCUTLISTSTYLE>(rConfig);
			dlg.m_bdData.bIgnoreDialogs=GetPropValue<PP_FDIGNORESHELLDIALOGS>(rConfig);

			dlg.m_bdData.strInitialDir=(dlg.m_bdData.cvRecent.size() > 0) ? dlg.m_bdData.cvRecent.at(0) : _T("");

			if(eOperation == chcore::eOperation_Copy)
				dlg.m_bdData.strCaption = GetResManager().LoadString(IDS_TITLECOPY_STRING);
			else if(eOperation == chcore::eOperation_Move)
				dlg.m_bdData.strCaption = GetResManager().LoadString(IDS_TITLEMOVE_STRING);
			else
				dlg.m_bdData.strCaption = GetResManager().LoadString(IDS_TITLEUNKNOWNOPERATION_STRING);
			dlg.m_bdData.strText = GetResManager().LoadString(IDS_MAINBROWSETEXT_STRING);

			// set count of data to display
			size_t stClipboardSize = tTaskDefinition.GetSourcePathCount();
			size_t stEntries = (stClipboardSize > 3) ? 2 : stClipboardSize;
			for(size_t stIndex = 0; stIndex < stEntries; stIndex++)
			{
				dlg.m_bdData.strText += tTaskDefinition.GetSourcePathAt(stIndex).ToString();
				dlg.m_bdData.strText += _T("\n");
			}

			// add ...
			if (stEntries < stClipboardSize)
				dlg.m_bdData.strText+=_T("...");

			// show window
			INT_PTR iResult = dlg.DoModal();

			// set data to config
			SetPropValue<PP_SHORTCUTS>(rConfig, dlg.m_bdData.cvShortcuts);
			SetPropValue<PP_RECENTPATHS>(rConfig, dlg.m_bdData.cvRecent);

			SetPropValue<PP_FDEXTENDEDVIEW>(rConfig, dlg.m_bdData.bExtended);
			SetPropValue<PP_FDWIDTH>(rConfig, dlg.m_bdData.cx);
			SetPropValue<PP_FDHEIGHT>(rConfig, dlg.m_bdData.cy);
			SetPropValue<PP_FDSHORTCUTLISTSTYLE>(rConfig, dlg.m_bdData.iView);
			SetPropValue<PP_FDIGNORESHELLDIALOGS>(rConfig, dlg.m_bdData.bIgnoreDialogs);
			rConfig.Write();

			if(iResult == IDOK)
			{
				// get dest path
				CString strData;
				dlg.GetPath(strData);
				tTaskDefinition.SetDestinationPath(chcore::PathFromString(strData));

				// load resource strings
				chcore::SetTaskPropValue<chcore::eTO_AlternateFilenameFormatString_First>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_FIRSTCOPY_STRING));
				chcore::SetTaskPropValue<chcore::eTO_AlternateFilenameFormatString_AfterFirst>(tTaskDefinition.GetConfiguration(), GetResManager().LoadString(IDS_NEXTCOPY_STRING));

				CString strMessage;
				try
				{
					chcore::TTaskPtr spTask = pData->m_pTasks->CreateTask(tTaskDefinition);

					// write spTask to a file
					spTask->Store();

					// start processing
					spTask->BeginProcessing();
				}
				catch(const std::exception& e)
				{
					strMessage = e.what();
				}

				if(!strMessage.IsEmpty())
				{
					ictranslate::CFormat fmt;

					fmt.SetFormat(GetResManager().LoadString(IDS_TASK_CREATE_FAILED));
					fmt.SetParam(_T("%reason"), strMessage);
					AfxMessageBox(fmt, MB_OK | MB_ICONERROR);
				}
			}
		}

		// do we need to check for turning computer off
		if(uiShutCounter == 0 && GetPropValue<PP_PSHUTDOWNAFTREFINISHED>(GetConfig()))
		{
			if(pData->m_pTasks->AreAllFinished())
			{
				TRACE("Shut down windows\n");
				bool bShutdown=true;
				if (GetPropValue<PP_PTIMEBEFORESHUTDOWN>(GetConfig()) != 0)
				{
					CShutdownDlg dlg;
					dlg.m_iOverallTime = GetPropValue<PP_PTIMEBEFORESHUTDOWN>(GetConfig());
					if (dlg.m_iOverallTime < 0)
						dlg.m_iOverallTime=-dlg.m_iOverallTime;
					bShutdown=(dlg.DoModal() != IDCANCEL);
				}

				SetPropValue<PP_PSHUTDOWNAFTREFINISHED>(GetConfig(), false);
				GetConfig().Write();
				if(bShutdown)
				{
					// adjust token privileges for NT
					HANDLE hToken=NULL;
					TOKEN_PRIVILEGES tp;
					if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)
						&& LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tp.Privileges[0].Luid))
					{
						tp.PrivilegeCount=1;
						tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;

						AdjustTokenPrivileges(hToken, FALSE, &tp, NULL, NULL, NULL);
					}

					BOOL bExit=ExitWindowsEx(EWX_POWEROFF | EWX_SHUTDOWN | (GetPropValue<PP_PFORCESHUTDOWN>(GetConfig()) ? EWX_FORCE : 0), 0);
					if (bExit)
						return 1;
					else
					{
						// some kind of error
						ictranslate::CFormat fmt(GetResManager().LoadString(IDS_SHUTDOWNERROR_STRING));
						fmt.SetParam(_t("%errno"), GetLastError());
						AfxMessageBox(fmt, MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
					}
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
