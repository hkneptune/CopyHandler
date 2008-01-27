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
#include "ch.h"

#include "MainWnd.h"
#include "OptionsDlg.h"

#include "shlobj.h"
#include "tchar.h"
#include "structs.h"
#include "dialogs.h"

#pragma warning (disable : 4201)
#include "mmsystem.h"
#pragma warning (default : 4201)

#include "FolderDialog.h"

#include "CustomCopyDlg.h"
#include "ReplaceFilesDlg.h"
#include "btnIDs.h"
#include "SmallReplaceFilesDlg.h"
#include "ReplaceOnlyDlg.h"
#include "DstFileErrorDlg.h"
#include "..\Common\FileSupport.h"
#include "AboutDlg.h"
#include "NotEnoughRoomDlg.h"
#include "register.h"
#include "ShutdownDlg.h"
#include "StringHelpers.h"
#include "..\common\ipcstructs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	WM_ICON_NOTIFY			WM_USER+4
#define WM_SHOWMINIVIEW			WM_USER+3
#define WM_IDENTIFY				WM_USER+11

#define TM_AUTOREMOVE			1000
#define TM_AUTORESUME			1000
#define TM_ACCEPTING			100

// assume max sectors of 4kB (for rounding)
#define MAXSECTORSIZE			4096

extern CSharedConfigStruct* g_pscsShared;

extern int iCount;
extern unsigned short msg[];

extern int iOffCount;
extern unsigned char off[];
extern unsigned short _hash[];


/////////////////////////////////////////////////////////////////////////////
// CMainWnd
// registers main window class
ATOM CMainWnd::RegisterClass()
{
	WNDCLASS wc;

	wc.style			= CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC)::DefWindowProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= AfxGetInstanceHandle();
	wc.hIcon			= ::LoadIcon(NULL, MAKEINTRESOURCE(AFX_IDI_STD_FRAME));
	wc.hCursor			= ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= _T("Copy Handler Wnd Class");

	return ::RegisterClass(&wc);
}

// creates this window
BOOL CMainWnd::Create()
{
	ATOM at=RegisterClass();

	return CreateEx(WS_EX_TOOLWINDOW, (LPCTSTR)at, _T("Copy Handler"), WS_OVERLAPPED, 10, 10, 10, 10, NULL, (HMENU)NULL, NULL);
}

int CMainWnd::ShowTrayIcon()
{
	// create system tray icon
	HICON hIcon=(HICON)GetResManager()->LoadImage(MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_VGACOLOR);
	bool bRes=m_ctlTray.CreateIcon(m_hWnd, WM_TRAYNOTIFY, GetApp()->GetAppNameVer(), hIcon, 0);
	if (!bRes)
	{
//		GetLog()->Log(_T("[CMainWnd] ... creating tray icon failed."));
		return -1;
	}

/*	if (!m_ctlTray.ShowIcon())
		GetLog()->Log(_T("[CMainWnd] ... showing tray icon failed."));
	else
		GetLog()->Log(_T("[CMainWnd] ... showing tray icon succeeded."));
*/
	return 0;
}

IMPLEMENT_DYNCREATE(CMainWnd, CWnd)

BEGIN_MESSAGE_MAP(CMainWnd, CWnd)
	//{{AFX_MSG_MAP(CMainWnd)
	ON_COMMAND(ID_POPUP_SHOW_STATUS, OnPopupShowStatus)
	ON_COMMAND(ID_POPUP_OPTIONS, OnPopupShowOptions)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_WM_COPYDATA()
	ON_WM_CREATE()
	ON_COMMAND(ID_SHOW_MINI_VIEW, OnShowMiniView)
	ON_COMMAND(ID_POPUP_CUSTOM_COPY, OnPopupCustomCopy)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_POPUP_MONITORING, OnPopupMonitoring)
	ON_COMMAND(ID_POPUP_SHUTAFTERFINISHED, OnPopupShutafterfinished)
	ON_COMMAND(ID_POPUP_REGISTERDLL, OnPopupRegisterdll)
	ON_COMMAND(ID_POPUP_UNREGISTERDLL, OnPopupUnregisterdll)
	ON_COMMAND(ID_APP_EXIT, OnAppExit)
	ON_COMMAND(ID_POPUP_HELP, OnPopupHelp)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_ICON_NOTIFY, OnTrayNotification)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainWnd construction/destruction

CMainWnd::CMainWnd()
{
	m_pdlgStatus=NULL;
	m_pdlgMiniView=NULL;
	m_dwLastTime=0;
}

CMainWnd::~CMainWnd()
{
}

// case insensitive replacement
inline void ReplaceNoCase(CString& rString, CString strOld, CString strNew)
{
	if (rString.Left(strOld.GetLength()).CompareNoCase(strOld) == 0)
		rString=strNew+rString.Right(rString.GetLength()-strOld.GetLength());
}

bool TimeToFileTime(const COleDateTime& time, LPFILETIME pFileTime)
{
	SYSTEMTIME sysTime;
	sysTime.wYear = (WORD)time.GetYear();
	sysTime.wMonth = (WORD)time.GetMonth();
	sysTime.wDay = (WORD)time.GetDay();
	sysTime.wHour = (WORD)time.GetHour();
	sysTime.wMinute = (WORD)time.GetMinute();
	sysTime.wSecond = (WORD)time.GetSecond();
	sysTime.wMilliseconds = 0;

	// convert system time to local file time
	FILETIME localTime;
	if (!SystemTimeToFileTime((LPSYSTEMTIME)&sysTime, &localTime))
		return false;

	// convert local file time to UTC file time
	if (!LocalFileTimeToFileTime(&localTime, pFileTime))
		return false;

	return true;
}

bool SetFileDirectoryTime(LPCTSTR lpszName, CFileInfo* pSrcInfo)
{
	FILETIME creation, lastAccess, lastWrite;

	if (!TimeToFileTime(pSrcInfo->GetCreationTime(), &creation)
		|| !TimeToFileTime(pSrcInfo->GetLastAccessTime(), &lastAccess)
		|| !TimeToFileTime(pSrcInfo->GetLastWriteTime(), &lastWrite) )
		return false;

	HANDLE handle=CreateFile(lpszName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		return false;

	if (!SetFileTime(handle, &creation, &lastAccess, &lastWrite))
	{
		CloseHandle(handle);
		return false;
	}

	if (!CloseHandle(handle))
		return false;

	return true;
}

// searching for files
inline void RecurseDirectories(CTask* pTask)
{
	TRACE("Searching for files...\n");

	// log
	pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFSEARCHINGFORFILES_STRING));
	
	// update status
	pTask->SetStatus(ST_SEARCHING, ST_STEP_MASK);

	// delete the content of m_files
	pTask->FilesRemoveAll();

	// enter some data to m_files
	int nSize=pTask->GetClipboardDataSize();	// size of m_clipboard
	const CFiltersArray* pFilters=pTask->GetFilters();
	int iDestDrvNumber=pTask->GetDestDriveNumber();
	bool bIgnoreDirs=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_DIRS) != 0;
	bool bForceDirectories=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_FORCE_DIRS) != 0;
	bool bMove=pTask->GetStatus(ST_OPERATION_MASK) == ST_MOVE;
	CFileInfo fi;
	fi.SetClipboard(pTask->GetClipboard());

	// add everything
	for (int i=0;i<nSize;i++)
	{
		// read attributes of src file/folder
		if (!fi.Create(pTask->GetClipboardData(i)->GetPath(), i))
		{
			// log
			pTask->m_log.logw(GetResManager()->LoadString(IDS_OTFMISSINGCLIPBOARDINPUT_STRING), (PCTSTR)pTask->GetClipboardData(i)->GetPath());
			continue;
		}
		else
		{
			// log
			pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFADDINGCLIPBOARDFILE_STRING), (PCTSTR)pTask->GetClipboardData(i)->GetPath());
		}

		// found file/folder - check if the dest name has been generated
		if (pTask->GetClipboardData(i)->m_astrDstPaths.GetSize() == 0)
		{
			// generate something - if dest folder == src folder - search for copy
			if (pTask->GetDestPath().GetPath() == fi.GetFileRoot())
			{
				CString strSubst;
				FindFreeSubstituteName(fi.GetFullFilePath(), pTask->GetDestPath().GetPath(), &strSubst);
				pTask->GetClipboardData(i)->m_astrDstPaths.Add(strSubst);
			}
			else
				pTask->GetClipboardData(i)->m_astrDstPaths.Add(fi.GetFileName());
		}
		
		// add if needed
		if (fi.IsDirectory())
		{
			// add if folder's aren't ignored
			if (!bIgnoreDirs && !bForceDirectories)
			{
				pTask->FilesAdd(fi);

				// log
				pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFADDEDFOLDER_STRING), (PCTSTR)fi.GetFullFilePath());
			}

			// don't add folder contents when moving inside one disk boundary
			if (bIgnoreDirs || !bMove || pTask->GetCopies() > 1 || iDestDrvNumber == -1
				|| iDestDrvNumber != fi.GetDriveNumber() || CFileInfo::Exist(fi.GetDestinationPath(pTask->GetDestPath().GetPath(), 0, ((int)bForceDirectories) << 1)) )
			{
				// log
				pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFRECURSINGFOLDER_STRING), (PCTSTR)fi.GetFullFilePath());
				
				// no movefile possibility - use CustomCopyFile
				pTask->GetClipboardData(i)->SetMove(false);

				pTask->FilesAddDir(fi.GetFullFilePath(), pFilters, i, true, !bIgnoreDirs || bForceDirectories);
			}

			// check for kill need
			if (pTask->GetKillFlag())
			{
				// log
				pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFADDINGKILLREQEST_STRING));
				throw new CProcessingException(E_KILL_REQUEST, pTask);
			}
		}
		else
		{
			if (bMove && pTask->GetCopies() == 1 && iDestDrvNumber != -1 && iDestDrvNumber == fi.GetDriveNumber() &&
				!CFileInfo::Exist(fi.GetDestinationPath(pTask->GetDestPath().GetPath(), 0, ((int)bForceDirectories) << 1)) )
			{
				// if moving within one partition boundary set the file size to 0 so the overall size will
				// be ok
				fi.SetLength64(0);
			}
			else
				pTask->GetClipboardData(i)->SetMove(false);	// no MoveFile
			
			pTask->FilesAdd(fi);		// file - add

			// log
			pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFADDEDFILE_STRING), (PCTSTR)fi.GetFullFilePath());
		}
	}
	
	// calc size of all files
	pTask->CalcAllSize();
	
	// update *m_pnTasksAll;
	pTask->IncreaseAllTasksSize(pTask->GetAllSize());
	
	// change state to ST_COPYING - finished searching for files
	pTask->SetStatus(ST_COPYING, ST_STEP_MASK);
	
	// save task status
	TCHAR szPath[_MAX_PATH];
	GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
	GetApp()->ExpandPath(szPath);
	pTask->Store(szPath, true);
	pTask->Store(szPath, false);

	// log
	pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFSEARCHINGFINISHED_STRING));
}

// delete files - after copying
void DeleteFiles(CTask* pTask)
{
	// log
	pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFDELETINGFILES_STRING));

	// current processed path
	BOOL bSuccess;
	CFileInfo fi;

	// index points to 0 or next item to process
	for (int i=pTask->GetCurrentIndex();i<pTask->FilesGetSize();i++)
	{
		// set index in pTask to currently deleted element
		pTask->SetCurrentIndex(i);
		
		// check for kill flag
		if (pTask->GetKillFlag())
		{
			// log
			pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFDELETINGKILLREQUEST_STRING));
			throw new CProcessingException(E_KILL_REQUEST, pTask);
		}
		
		// current processed element
		fi=pTask->FilesGetAt(pTask->FilesGetSize()-i-1);
		
		// delete data
		if (fi.IsDirectory())
		{
			if (!GetConfig()->GetBoolValue(PP_CMPROTECTROFILES))
				SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY);
			bSuccess=RemoveDirectory(fi.GetFullFilePath());
		}
		else
		{
			// set files attributes to normal - it'd slow processing a bit, but it's better.
			if (!GetConfig()->GetBoolValue(PP_CMPROTECTROFILES))
				SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
			bSuccess=DeleteFile(fi.GetFullFilePath());
		}
		
		// operation failed
		DWORD dwLastError=GetLastError();
		if (!bSuccess && dwLastError != ERROR_PATH_NOT_FOUND && dwLastError != ERROR_FILE_NOT_FOUND)
		{
			// log
			pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFDELETINGERROR_STRING), dwLastError, (PCTSTR)fi.GetFullFilePath());
			throw new CProcessingException(E_ERROR, pTask, IDS_CPEDELETINGERROR_STRING, dwLastError, fi.GetFullFilePath());
		}
	}//for

	// change status to finished
	pTask->SetStatus(ST_FINISHED, ST_STEP_MASK);

	// add 1 to current index - looks better
	pTask->IncreaseCurrentIndex();

	// log
	pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFDELETINGFINISHED_STRING));
}

void CustomCopyFile(PCUSTOM_COPY_PARAMS pData)
{
	HANDLE hSrc=INVALID_HANDLE_VALUE, hDst=INVALID_HANDLE_VALUE;
	try
	{
		// do we copy rest or recopy ?
		bool bCopyRest=GetConfig()->GetBoolValue(PP_CMUSEAUTOCOMPLETEFILES);
		UINT uiNotificationType=GetConfig()->GetIntValue(PP_CMSHOWVISUALFEEDBACK);

		// Data regarding dest file
		CFileInfo fiDest;
		bool bExist=fiDest.Create(pData->strDstFile, -1);

		int iDlgCode=-1;
		int *piLastDlgDesc=NULL;		// ptr to int describing last used dialog
		
		// don't ask for copy rest
		bool bDontAsk=(pData->pTask->GetCurrentIndex() == pData->pTask->GetLastProcessedIndex());
		pData->pTask->SetLastProcessedIndex(-1);

		// if dest file size >0 - we can do somethng more than usual
		if ( bExist )
		{
			// src and dst files are the same
			if (fiDest == *pData->pfiSrcFile)
			{
				// copy automatically or ask
				if (uiNotificationType > 1 && !bDontAsk)
				{
					if (pData->pTask->m_iIdentical == -1)
					{
						// show feedback
						CSmallReplaceFilesDlg dlg;
						dlg.m_pfiSource=pData->pfiSrcFile;
						dlg.m_pfiDest=&fiDest;
						dlg.m_bEnableTimer=GetConfig()->GetBoolValue(PP_CMUSETIMEDFEEDBACK);
						dlg.m_iTime=GetConfig()->GetIntValue(PP_CMFEEDBACKTIME);
						dlg.m_iDefaultOption=ID_IGNORE;
						iDlgCode=dlg.DoModal();

						piLastDlgDesc=&pData->pTask->m_iIdentical;
					}
					else
						iDlgCode=pData->pTask->m_iIdentical;
				}
				else
				{
					// increase progress
					pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
					pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());

					return;	// don't continue if NC==0 or 1
				}
			}
			else // iDst != *pData->pfiSrcFile
			{
				// src and dst are different - check sizes
				if (fiDest.GetLength64() < pData->pfiSrcFile->GetLength64())
				{
					// we can copy rest
					if (uiNotificationType > 0 && !bDontAsk)
					{
						if (pData->pTask->m_iDestinationLess == -1)
						{
							// show dialog
							CReplaceFilesDlg dlg;
							dlg.m_pfiSource=pData->pfiSrcFile;
							dlg.m_pfiDest=&fiDest;
							dlg.m_bEnableTimer=GetConfig()->GetBoolValue(PP_CMUSETIMEDFEEDBACK);
							dlg.m_iTime=GetConfig()->GetIntValue(PP_CMFEEDBACKTIME);
							dlg.m_iDefaultOption=ID_COPYREST;
							
							iDlgCode=dlg.DoModal();

							piLastDlgDesc=&pData->pTask->m_iDestinationLess;
						}
						else
							iDlgCode=pData->pTask->m_iDestinationLess;
					}
					// else do nothing - bCopyRest has been initialized
				}
				else
				{
					// dst >= src size
					if (uiNotificationType > 1 && !bDontAsk)
					{
						if (pData->pTask->m_iDestinationGreater == -1)
						{
							CSmallReplaceFilesDlg dlg;
							dlg.m_pfiSource=pData->pfiSrcFile;
							dlg.m_pfiDest=&fiDest;
							dlg.m_bEnableTimer=GetConfig()->GetBoolValue(PP_CMUSETIMEDFEEDBACK);
							dlg.m_iTime=GetConfig()->GetIntValue(PP_CMFEEDBACKTIME);
							dlg.m_iDefaultOption=ID_RECOPY;
							iDlgCode=dlg.DoModal();	// wyœwietl
							
							piLastDlgDesc=&pData->pTask->m_iDestinationGreater;
						}
						else
							iDlgCode=pData->pTask->m_iDestinationGreater;
					}
					else
						bCopyRest=false;	// this case - recopy
				}
			} // iDst == *pData->pfiSrcFile
		}	// bExist

		// check for dialog result
		switch (iDlgCode)
		{
		case -1:
			break;
		case ID_IGNOREALL:
			if (piLastDlgDesc != NULL)
				*piLastDlgDesc=ID_IGNOREALL;
		case ID_IGNORE:
			pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
			pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
			return;
			break;
		case ID_COPYRESTALL:
			if (piLastDlgDesc != NULL)
				*piLastDlgDesc=ID_COPYRESTALL;
		case ID_COPYREST:
			bCopyRest=true;
			break;
		case IDCANCEL:
			// log
			if (GetConfig()->GetBoolValue(PP_CMCREATELOG))
				pData->pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFPRECHECKCANCELREQUEST_STRING), (PCTSTR)pData->pfiSrcFile->GetFullFilePath());
			throw new CProcessingException(E_CANCEL, pData->pTask);
			break;
		case ID_RECOPYALL:
			if (piLastDlgDesc != NULL)
				*piLastDlgDesc=ID_RECOPYALL;
		case ID_RECOPY:
			bCopyRest=false;
			break;
		}

		// change attributes of a dest file
		if (!GetConfig()->GetBoolValue(PP_CMPROTECTROFILES))
			SetFileAttributes(pData->strDstFile, FILE_ATTRIBUTE_NORMAL);

		// first or second pass ? only for FFNB
		bool bFirstPass=true;

		// check size of src file to know whether use flag FILE_FLAG_NOBUFFERING
l_start:
		bool bNoBuffer=(bFirstPass && GetConfig()->GetBoolValue(PP_BFUSENOBUFFERING) && pData->pfiSrcFile->GetLength64() >= GetConfig()->GetIntValue(PP_BFBOUNDARYLIMIT));

		// refresh data about file
		if (!bFirstPass)
			bExist=fiDest.Create(pData->strDstFile, -1);

		// open src
l_openingsrc:
		hSrc=CreateFile(pData->pfiSrcFile->GetFullFilePath(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffer ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
		if (hSrc == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError=GetLastError();
			if (uiNotificationType < 1)
			{
				// log
				pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFOPENINGERROR_STRING), dwLastError, (PCTSTR)pData->pfiSrcFile->GetFullFilePath());
				throw new CProcessingException(E_ERROR, pData->pTask, IDS_CPEOPENINGERROR_STRING, dwLastError, pData->pfiSrcFile->GetFullFilePath());
			}
			else
			{
				if (pData->pTask->m_iMissingInput == -1)
				{
					// no source file - feedback
					CFileInfo fiRealSrc;
					fiRealSrc.Create(pData->pfiSrcFile->GetFullFilePath(), -1);
					
					CReplaceOnlyDlg dlg;
					dlg.m_pfiSource=pData->pfiSrcFile;
					dlg.m_pfiDest=&fiRealSrc;
					dlg.m_bEnableTimer=GetConfig()->GetBoolValue(PP_CMUSETIMEDFEEDBACK);
					dlg.m_iTime=GetConfig()->GetIntValue(PP_CMFEEDBACKTIME);
					dlg.m_iDefaultOption=ID_WAIT;
					
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, dlg.m_strMessage.GetBuffer(_MAX_PATH), _MAX_PATH, NULL);
					dlg.m_strMessage.ReleaseBuffer();
					
					iDlgCode=dlg.DoModal();
				}
				else
					iDlgCode=pData->pTask->m_iMissingInput;
	
				switch (iDlgCode)
				{
				case ID_IGNOREALL:
					pData->pTask->m_iMissingInput=ID_IGNOREALL;
				case ID_IGNORE:
					pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
					pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
					return;
					break;
				case IDCANCEL:
					// log
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFOPENINGCANCELREQUEST_STRING), dwLastError, pData->pfiSrcFile->GetFullFilePath());
					throw new CProcessingException(E_CANCEL, pData->pTask);
					break;
				case ID_WAIT:
					// log
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFOPENINGWAITREQUEST_STRING), dwLastError, pData->pfiSrcFile->GetFullFilePath());
					throw new CProcessingException(E_ERROR, pData->pTask, IDS_CPEOPENINGERROR_STRING, dwLastError, pData->pfiSrcFile->GetFullFilePath());
					break;
				case ID_RETRY:
					// log
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFOPENINGRETRY_STRING), dwLastError, pData->pfiSrcFile->GetFullFilePath());
					goto l_openingsrc;
					break;
				}
			}
		}

		// open dest
l_openingdst:
		hDst=CreateFile(pData->strDstFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffer ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
		if (hDst == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError=GetLastError();
			if (uiNotificationType < 1)
			{
				// log
				pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFDESTOPENINGERROR_STRING), dwLastError, pData->strDstFile);
				throw new CProcessingException(E_ERROR, pData->pTask, IDS_CPEDESTOPENINGERROR_STRING, dwLastError, pData->strDstFile);
			}
			else
			{
				if (pData->pTask->m_iOutputError == -1)
				{
					CDstFileErrorDlg dlg;
					dlg.m_strFilename=pData->strDstFile;
					FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwLastError, 0, dlg.m_strMessage.GetBuffer(_MAX_PATH), _MAX_PATH, NULL);
					dlg.m_strMessage.ReleaseBuffer();
					
					dlg.m_bEnableTimer=GetConfig()->GetBoolValue(PP_CMUSETIMEDFEEDBACK);
					dlg.m_iTime=GetConfig()->GetIntValue(PP_CMFEEDBACKTIME);
					dlg.m_iDefaultOption=ID_WAIT;
					iDlgCode=dlg.DoModal();
				}
				else
					iDlgCode=pData->pTask->m_iOutputError;

				switch (iDlgCode)
				{
				case ID_RETRY:
					// change attributes
					if (!GetConfig()->GetBoolValue(PP_CMPROTECTROFILES))
						SetFileAttributes(pData->strDstFile, FILE_ATTRIBUTE_NORMAL);

					// log
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFDESTOPENINGRETRY_STRING), dwLastError, pData->strDstFile);
					goto l_openingdst;
					break;
				case IDCANCEL:
					// log
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFDESTOPENINGCANCELREQUEST_STRING), dwLastError, pData->strDstFile);
					throw new CProcessingException(E_CANCEL, pData->pTask);
					break;
				case ID_IGNOREALL:
					pData->pTask->m_iOutputError=ID_IGNOREALL;
				case ID_IGNORE:
					pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
					pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
					return;
					break;
				case ID_WAIT:
					// log
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFDESTOPENINGWAITREQUEST_STRING), dwLastError, pData->strDstFile);
					throw new CProcessingException(E_ERROR, pData->pTask, IDS_CPEDESTOPENINGERROR_STRING, dwLastError, pData->strDstFile);
					break;
				}
			}
		}

		// seeking
		DWORD dwLastError=0;
		if (!pData->bOnlyCreate)
		{
			if ( bCopyRest )	// if copy rest
			{
				if (!bFirstPass || (bExist && fiDest.GetLength64() > 0))
				{
					// try to move file pointers to the end
					ULONGLONG ullMove=(bNoBuffer ? ROUNDDOWN(fiDest.GetLength64(), MAXSECTORSIZE) : fiDest.GetLength64());
					if (SetFilePointer64(hSrc, ullMove, FILE_BEGIN) == -1 || SetFilePointer64(hDst, ullMove, FILE_BEGIN) == -1)
					{
						// log
						pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFMOVINGPOINTERSERROR_STRING), GetLastError(), pData->pfiSrcFile->GetFullFilePath(), pData->strDstFile, ullMove);

						// seek failed - seek to begin
						if (SetFilePointer64(hSrc, 0, FILE_BEGIN) == -1 || SetFilePointer64(hDst, 0, FILE_BEGIN) == -1)
						{
							// log
							dwLastError=GetLastError();
							pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFRESTORINGPOINTERSERROR_STRING), dwLastError, pData->pfiSrcFile->GetFullFilePath(), pData->strDstFile);
							throw new CProcessingException(E_ERROR, pData->pTask, IDS_CPERESTORINGPOINTERSERROR_STRING, dwLastError, pData->pfiSrcFile->GetFullFilePath(), pData->strDstFile);
						}
						else
						{
							// file pointers restored - if second pass subtract what's needed
							if (!bFirstPass)
							{
								pData->pTask->IncreaseProcessedSize(-static_cast<__int64>(ullMove));
								pData->pTask->IncreaseProcessedTasksSize(-static_cast<__int64>(ullMove));
							}
						}
					}
					else
					{
						// file pointers moved - so we have skipped some work - update positions
						if (bFirstPass)	// przy drugim obiegu jest ju¿ uwzglêdnione
						{
							pData->pTask->IncreaseProcessedSize(ullMove);
							pData->pTask->IncreaseProcessedTasksSize(ullMove);
						}
					}
				}
			}
			else
				if (!SetEndOfFile(hDst))	// if recopying - reset the dest file
				{
					// log
					dwLastError=GetLastError();
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFSETTINGZEROSIZEERROR_STRING), dwLastError, pData->strDstFile);
					throw new CProcessingException(E_ERROR, pData->pTask, IDS_CPESETTINGZEROSIZEERROR_STRING, dwLastError, pData->strDstFile);
				}
				
			// copying
			unsigned long tord, rd, wr;
			int iBufferIndex;
			do
			{
				// kill flag checks
				if (pData->pTask->GetKillFlag())
				{
					// log
					pData->pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFCOPYINGKILLREQUEST_STRING), pData->pfiSrcFile->GetFullFilePath(), pData->strDstFile);
					throw new CProcessingException(E_KILL_REQUEST, pData->pTask);
				}
				
				// recreate buffer if needed
				if (!(*pData->dbBuffer.GetSizes() == *pData->pTask->GetBufferSizes()))
				{
					// log
					const BUFFERSIZES *pbs1=pData->dbBuffer.GetSizes(), *pbs2=pData->pTask->GetBufferSizes();

					pData->pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFCHANGINGBUFFERSIZE_STRING), 
						pbs1->m_uiDefaultSize, pbs1->m_uiOneDiskSize, pbs1->m_uiTwoDisksSize, pbs1->m_uiCDSize, pbs1->m_uiLANSize,
						pbs2->m_uiDefaultSize, pbs2->m_uiOneDiskSize, pbs2->m_uiTwoDisksSize, pbs2->m_uiCDSize, pbs2->m_uiLANSize,
						pData->pfiSrcFile->GetFullFilePath(), pData->strDstFile);
					pData->pTask->SetBufferSizes(pData->dbBuffer.Create(pData->pTask->GetBufferSizes()));
				}
				
				// establish count of data to read
				iBufferIndex=pData->pTask->GetBufferSizes()->m_bOnlyDefault ? 0 : pData->pfiSrcFile->GetBufferIndex();
				tord=bNoBuffer ? ROUNDUP(pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex], MAXSECTORSIZE) : pData->dbBuffer.GetSizes()->m_auiSizes[iBufferIndex];

				// read
				if (!ReadFile(hSrc, pData->dbBuffer, tord, &rd, NULL))
				{
					// log
					dwLastError=GetLastError();
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFREADINGERROR_STRING), dwLastError, tord, pData->pfiSrcFile->GetFullFilePath());
					throw new CProcessingException(E_ERROR, pData->pTask, IDS_CPEREADINGERROR_STRING, dwLastError, tord, pData->pfiSrcFile->GetFullFilePath());
				}
				
				// change count of stored data
				if (bNoBuffer && (ROUNDUP(rd, MAXSECTORSIZE)) != rd)
				{
					// we need to copy rest so do the second pass
					// close files
					CloseHandle(hSrc);
					CloseHandle(hDst);

					// second pass
					bFirstPass=false;
					bCopyRest=true;		// nedd to copy rest

					goto l_start;
				}

				// zapisz
				if (!WriteFile(hDst, pData->dbBuffer, rd, &wr, NULL) || wr != rd)
				{
					// log
					dwLastError=GetLastError();
					pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFWRITINGERROR_STRING), dwLastError, rd, pData->strDstFile);
					throw new CProcessingException(E_ERROR, pData->pTask, IDS_CPEWRITINGERROR_STRING, dwLastError, rd, pData->strDstFile);
				}
				
				// increase count of processed data
				pData->pTask->IncreaseProcessedSize(rd);
				pData->pTask->IncreaseProcessedTasksSize(rd);
//				TRACE("Read: %d, Written: %d\n", rd, wr);
			}
			while ( rd != 0 );
		}
		else
		{
			// we don't copy contents, but need to increase processed size
			pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
			pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
		}

		// close files
		CloseHandle(hSrc);
		CloseHandle(hDst);
	}
	catch(...)
	{
		// log
		pData->pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFCAUGHTEXCEPTIONCCF_STRING), GetLastError());

		// close handles
		if (hSrc != INVALID_HANDLE_VALUE)
			CloseHandle(hSrc);
		if (hDst != INVALID_HANDLE_VALUE)
			CloseHandle(hDst);
		
		throw;
	}
}

// function processes files/folders
void ProcessFiles(CTask* pTask)
{
	// log
	pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFPROCESSINGFILES_STRING));

	// count how much has been done (updates also a member in CTaskArray)
	pTask->CalcProcessedSize();
	
	// create a buffer of size pTask->m_nBufferSize
	CUSTOM_COPY_PARAMS ccp;
	ccp.pTask=pTask;
	ccp.bOnlyCreate=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_CONTENT) != 0;
	ccp.dbBuffer.Create(pTask->GetBufferSizes());
	
	// helpers
	CFileInfo fi;	// for currently processed element
	DWORD dwLastError;
	
	// begin at index which wasn't processed previously
	int nSize=pTask->FilesGetSize();	// wielkoœæ tablicy
	int iCopiesCount=pTask->GetCopies();	// iloœæ kopii
	bool bIgnoreFolders=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_DIRS) != 0;
	bool bForceDirectories=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_FORCE_DIRS) != 0;
	const CDestPath& dpDestPath=pTask->GetDestPath();

	// log
	const BUFFERSIZES* pbs=ccp.dbBuffer.GetSizes();
	pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFPROCESSINGFILESDATA_STRING), ccp.bOnlyCreate,
		pbs->m_uiDefaultSize, pbs->m_uiOneDiskSize, pbs->m_uiTwoDisksSize, pbs->m_uiCDSize, pbs->m_uiLANSize,
		nSize, iCopiesCount, bIgnoreFolders, dpDestPath.GetPath(), pTask->GetCurrentCopy(), pTask->GetCurrentIndex());

	for (unsigned char j=pTask->GetCurrentCopy();j<iCopiesCount;j++)
	{
		pTask->SetCurrentCopy(j);
		for (int i=pTask->GetCurrentIndex();i<nSize;i++)
		{
			// update m_nCurrentIndex, getting current CFileInfo
			pTask->SetCurrentIndex(i);
			fi=pTask->FilesGetAtCurrentIndex();
			
			// should we kill ?
			if (pTask->GetKillFlag())
			{
				// log
				pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFPROCESSINGKILLREQUEST_STRING));
				throw new CProcessingException(E_KILL_REQUEST, pTask);
			}
			
			// set dest path with filename
			ccp.strDstFile=fi.GetDestinationPath(dpDestPath.GetPath(), j, ((int)bForceDirectories) << 1 | (int)bIgnoreFolders);
			
			// are the files/folders lie on the same partition ?
			bool bMove=pTask->GetStatus(ST_OPERATION_MASK) == ST_MOVE;
			if (bMove && dpDestPath.GetDriveNumber() != -1 && dpDestPath.GetDriveNumber() == fi.GetDriveNumber() && iCopiesCount == 1 && fi.GetMove())
			{
				if (!MoveFile(fi.GetFullFilePath(), ccp.strDstFile))
				{
					dwLastError=GetLastError();
					//log
					pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFMOVEFILEERROR_STRING), dwLastError, fi.GetFullFilePath(), ccp.strDstFile);
					throw new CProcessingException(E_ERROR, pTask, IDS_CPEMOVEFILEERROR_STRING, dwLastError, fi.GetFullFilePath(), ccp.strDstFile);
				}
			}
			else
			{
				// if folder - create it
				if ( fi.IsDirectory() )
				{
					if (!CreateDirectory(ccp.strDstFile, NULL) && (dwLastError=GetLastError()) != ERROR_ALREADY_EXISTS )
					{
						// log
						pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFCREATEDIRECTORYERROR_STRING), dwLastError, ccp.strDstFile);
						throw new CProcessingException(E_ERROR, pTask, IDS_CPECREATEDIRECTORYERROR_STRING, dwLastError, ccp.strDstFile);
					}
					
					pTask->IncreaseProcessedSize(fi.GetLength64());
					pTask->IncreaseProcessedTasksSize(fi.GetLength64());
				}
				else
				{
					// start copying/moving file
					ccp.pfiSrcFile=&fi;
					
					// kopiuj dane
					CustomCopyFile(&ccp);
					
					// if moving - delete file (only if config flag is set)
					if (bMove && !GetConfig()->GetBoolValue(PP_CMDELETEAFTERFINISHED) && j == iCopiesCount-1)
					{
						if (!GetConfig()->GetBoolValue(PP_CMPROTECTROFILES))
							SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
						DeleteFile(fi.GetFullFilePath());	// there will be another try later, so I don't check
															// if succeeded
					}
				}
				
				// set a time
				if (GetConfig()->GetBoolValue(PP_CMSETDESTDATE))
					SetFileDirectoryTime(ccp.strDstFile, &fi); // no error check - ma³o istotne
				
				// attributes
				if (GetConfig()->GetBoolValue(PP_CMSETDESTATTRIBUTES))
					SetFileAttributes(ccp.strDstFile, fi.GetAttributes());	// j.w.
			}
		}

		// current copy finished - change what's needed
		pTask->SetCurrentIndex(0);
	}

	// delete buffer - it's not needed
	ccp.dbBuffer.Delete();
	
	// change status
	if (pTask->GetStatus(ST_OPERATION_MASK) == ST_MOVE)
	{
		pTask->SetStatus(ST_DELETING, ST_STEP_MASK);
		// set the index to 0 before deleting
		pTask->SetCurrentIndex(0);
	}
	else
	{
		pTask->SetStatus(ST_FINISHED, ST_STEP_MASK);

		// to look better - increase current index by 1
		pTask->SetCurrentIndex(nSize);
	}
	// log
	pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFPROCESSINGFINISHED_STRING));
}

void CheckForWaitState(CTask* pTask)
{
	// limiting operation count
	pTask->SetStatus(ST_WAITING, ST_WAITING_MASK);
	bool bContinue=false;
	while (!bContinue)
	{
		if (pTask->CanBegin())
		{
			TRACE("CAN BEGIN ALLOWED TO CONTINUE...\n");
			pTask->SetStatus(0, ST_WAITING);
			bContinue=true;
			
			pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFWAITINGFINISHED_STRING));

//			return; // skips sleep and kill flag checking
		}
		
		Sleep(50);	// not to make it too hard for processor
		
		if (pTask->GetKillFlag())
		{
			// log
			pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFWAITINGKILLREQUEST_STRING));
			throw new CProcessingException(E_KILL_REQUEST, pTask);
		}
	}
}

UINT ThrdProc(LPVOID pParam)
{
	TRACE("\n\nENTERING ThrdProc (new task started)...\n");
	CTask* pTask=static_cast<CTask*>(pParam);
	TCHAR szPath[_MAX_PATH];
	GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
	GetApp()->ExpandPath(szPath);
	_tcscat(szPath, pTask->GetUniqueName()+_T(".log"));
	pTask->m_log.init(szPath, 262144, icpf::log_file::level_debug, false, false);

	// set thread boost
	HANDLE hThread=GetCurrentThread();
	::SetThreadPriorityBoost(hThread, GetConfig()->GetBoolValue(PP_CMDISABLEPRIORITYBOOST));

	CTime tm=CTime::GetCurrentTime();
	pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFTHREADSTART_STRING), tm.GetDay(), tm.GetMonth(), tm.GetYear(), tm.GetHour(), tm.GetMinute(), tm.GetSecond());

	try
	{
		// to make the value stable
		bool bReadTasksSize=GetConfig()->GetBoolValue(PP_CMREADSIZEBEFOREBLOCKING);

		if (!bReadTasksSize)
			CheckForWaitState(pTask);	// operation limiting

		// set what's needed
		pTask->m_lLastTime=time(NULL);	// last time (start counting)

		// search for files if needed
		if ((pTask->GetStatus(ST_STEP_MASK) == ST_NULL_STATUS
			|| pTask->GetStatus(ST_STEP_MASK) == ST_SEARCHING))
		{
			// get rid of info about processed sizes
			pTask->DecreaseProcessedTasksSize(pTask->GetProcessedSize());
			pTask->SetProcessedSize(0);
			pTask->DecreaseAllTasksSize(pTask->GetAllSize());
			pTask->SetAllSize(0);

			// start searching
			RecurseDirectories(pTask);
		}

		// check for free space
		__int64 i64Needed, i64Available;
l_showfeedback:
		pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFCHECKINGSPACE_STRING));

		if (!pTask->GetRequiredFreeSpace(&i64Needed, &i64Available))
		{
			pTask->m_log.logw(GetResManager()->LoadString(IDS_OTFNOTENOUGHFREESPACE_STRING), i64Needed, i64Available);
			
			// default
			int iResult=ID_IGNORE;

			if (GetConfig()->GetIntValue(PP_CMSHOWVISUALFEEDBACK) > 0)
			{
				// make user know that some place is missing
				CNotEnoughRoomDlg dlg;
				dlg.m_bEnableTimer=GetConfig()->GetBoolValue(PP_CMUSETIMEDFEEDBACK);
				dlg.m_iTime=GetConfig()->GetIntValue(PP_CMFEEDBACKTIME);
				
				dlg.m_llRequired=i64Needed;
				
				for (int i=0;i<pTask->GetClipboardDataSize();i++)
					dlg.m_strFiles.Add(pTask->GetClipboardData(i)->GetPath());
				
				dlg.m_strDisk=pTask->GetDestPath().GetPath();

				// show
				iResult=dlg.DoModal();
			}

			switch (iResult)
			{
			case IDCANCEL:
				{
					pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFFREESPACECANCELREQUEST_STRING));
					throw new CProcessingException(E_CANCEL, pTask);
					break;
				}
			case ID_RETRY:
				pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFFREESPACERETRYING_STRING));
				goto l_showfeedback;
				break;
			case ID_IGNORE:
				pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFFREESPACEIGNORE_STRING));
				break;
			}
		}

		if (bReadTasksSize)
		{
			pTask->UpdateTime();
			pTask->m_lLastTime=-1;

			CheckForWaitState(pTask);

			pTask->m_lLastTime=time(NULL);
		}
		
		// Phase II - copying/moving
		if (pTask->GetStatus(ST_STEP_MASK) == ST_COPYING)
		{
			// decrease processed in ctaskarray - the rest will be done in ProcessFiles
			pTask->DecreaseProcessedTasksSize(pTask->GetProcessedSize());
			ProcessFiles(pTask);
		}
		
		// deleting data - III phase
		if (pTask->GetStatus(ST_STEP_MASK) == ST_DELETING)
			DeleteFiles(pTask);
		
		// refresh time
		pTask->UpdateTime();

		// save progress before killed
		TCHAR szPath[_MAX_PATH];
		GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
		GetApp()->ExpandPath(szPath);
		pTask->Store(szPath, false);
		
		// we are ending
		pTask->DecreaseOperationsPending();

		// play sound
		if (GetConfig()->GetBoolValue(PP_SNDPLAYSOUNDS))
		{
			GetConfig()->GetStringValue(PP_SNDFINISHEDSOUNDPATH, szPath, _MAX_PATH);
			GetApp()->ExpandPath(szPath);
			PlaySound(szPath, NULL, SND_FILENAME | SND_ASYNC);
		}

		CTime tm=CTime::GetCurrentTime();
		pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFTHREADFINISHED_STRING), tm.GetDay(), tm.GetMonth(), tm.GetYear(), tm.GetHour(), tm.GetMinute(), tm.GetSecond());

		// we have been killed - the last operation
		InterlockedIncrement(pTask->m_plFinished);
		pTask->CleanupAfterKill();
		pTask->SetKilledFlag();
	}
	catch(CProcessingException* e)
	{
		// increment count of beginnings
		InterlockedIncrement(pTask->m_plFinished);
		
		// refresh time
		pTask->UpdateTime();
		
		// log
		pTask->m_log.logerr(GetResManager()->LoadString(IDS_OTFCAUGHTEXCEPTIONMAIN_STRING), e->m_dwError, e->m_iType);

		if (e->m_iType == E_ERROR && GetConfig()->GetBoolValue(PP_SNDPLAYSOUNDS))
		{
			TCHAR szPath[_MAX_PATH];
			GetConfig()->GetStringValue(PP_SNDERRORSOUNDPATH, szPath, _MAX_PATH);
			GetApp()->ExpandPath(szPath);
			PlaySound(szPath, NULL, SND_FILENAME | SND_ASYNC);
		}

		// cleanup changes flags and calls cleanup for a task
		e->Cleanup();
		delete e;

		if (pTask->GetStatus(ST_WAITING_MASK) & ST_WAITING)
			pTask->SetStatus(0, ST_WAITING);

		pTask->DecreaseOperationsPending();
		pTask->SetContinueFlag(false);
		pTask->SetForceFlag(false);

		return 0xffffffff;	// almost like -1
	}

	TRACE("TASK FINISHED - exiting ThrdProc.\n");
	return 0;
}

UINT ClipboardMonitorProc(LPVOID pParam)
{
	volatile CLIPBOARDMONITORDATA* pData=static_cast<CLIPBOARDMONITORDATA *>(pParam);
	ASSERT(pData->m_hwnd);

	// bufor
	TCHAR path[_MAX_PATH];
	UINT i;	// counter
	CTask *pTask;	// ptr to a task
	CClipboardEntry* pEntry=NULL;

	// register clipboard format
	UINT nFormat=RegisterClipboardFormat(_T("Preferred DropEffect"));
	UINT uiCounter=0, uiShutCounter=0;;
	LONG lFinished=0;
	bool bEnd=false;

	while (!pData->bKill)
	{
		if (uiCounter == 0 && GetConfig()->GetBoolValue(PP_PCLIPBOARDMONITORING) && IsClipboardFormatAvailable(CF_HDROP))
		{
			// get data from clipboard
			OpenClipboard(pData->m_hwnd);
			HANDLE handle=GetClipboardData(CF_HDROP);

			UINT nCount=DragQueryFile(static_cast<HDROP>(handle), 0xffffffff, NULL, 0);

			pTask=new CTask(&pData->m_pTasks->m_tcd);

			for (i=0;i<nCount;i++)
			{
				DragQueryFile(static_cast<HDROP>(handle), i, path, _MAX_PATH);
				pEntry=new CClipboardEntry;
				pEntry->SetPath(path);
				pTask->AddClipboardData(pEntry);
			}
			
			if (IsClipboardFormatAvailable(nFormat))
			{
				HANDLE handle=GetClipboardData(nFormat);
				LPVOID addr=GlobalLock(handle);
				
				DWORD dwData=((DWORD*)addr)[0];
				if (dwData & DROPEFFECT_COPY)
					pTask->SetStatus(ST_COPY, ST_OPERATION_MASK);	// copy
				else if (dwData & DROPEFFECT_MOVE)
					pTask->SetStatus(ST_MOVE, ST_OPERATION_MASK);	// move

				GlobalUnlock(handle);
			}
			else
				pTask->SetStatus(ST_COPY, ST_OPERATION_MASK);	// default - copy

			EmptyClipboard();
			CloseClipboard();
			
			BUFFERSIZES bs;
			bs.m_bOnlyDefault=GetConfig()->GetBoolValue(PP_BFUSEONLYDEFAULT);
			bs.m_uiDefaultSize=GetConfig()->GetIntValue(PP_BFDEFAULT);
			bs.m_uiOneDiskSize=GetConfig()->GetIntValue(PP_BFONEDISK);
			bs.m_uiTwoDisksSize=GetConfig()->GetIntValue(PP_BFTWODISKS);
			bs.m_uiCDSize=GetConfig()->GetIntValue(PP_BFCD);
			bs.m_uiLANSize=GetConfig()->GetIntValue(PP_BFLAN);

			pTask->SetBufferSizes(&bs);
			pTask->SetPriority(GetConfig()->GetIntValue(PP_CMDEFAULTPRIORITY));

			// get dest folder
			CFolderDialog dlg;
			GetConfig()->GetStringArrayValue(PP_SHORTCUTS, &dlg.m_bdData.cvShortcuts);
			GetConfig()->GetStringArrayValue(PP_RECENTPATHS, &dlg.m_bdData.cvRecent);
			dlg.m_bdData.bExtended=GetConfig()->GetBoolValue(PP_FDEXTENDEDVIEW);
			dlg.m_bdData.cx=GetConfig()->GetIntValue(PP_FDWIDTH);
			dlg.m_bdData.cy=GetConfig()->GetIntValue(PP_FDHEIGHT);
			dlg.m_bdData.iView=GetConfig()->GetIntValue(PP_FDSHORTCUTLISTSTYLE);
			dlg.m_bdData.bIgnoreDialogs=GetConfig()->GetBoolValue(PP_FDIGNORESHELLDIALOGS);

			dlg.m_bdData.strInitialDir=(dlg.m_bdData.cvRecent.size() > 0) ? dlg.m_bdData.cvRecent.at(0) : _T("");

			int iStatus=pTask->GetStatus(ST_OPERATION_MASK);
			if (iStatus == ST_COPY)
				dlg.m_bdData.strCaption=GetResManager()->LoadString(IDS_TITLECOPY_STRING);
			else if (iStatus == ST_MOVE)
				dlg.m_bdData.strCaption=GetResManager()->LoadString(IDS_TITLEMOVE_STRING);
			else
				dlg.m_bdData.strCaption=GetResManager()->LoadString(IDS_TITLEUNKNOWNOPERATION_STRING);
			dlg.m_bdData.strText=GetResManager()->LoadString(IDS_MAINBROWSETEXT_STRING);
			
			// set count of data to display
			int iClipboardSize=pTask->GetClipboardDataSize();
			int iEntries=(iClipboardSize > 3) ? 2 : iClipboardSize;
			for (int i=0;i<iEntries;i++)
				dlg.m_bdData.strText+=pTask->GetClipboardData(i)->GetPath()+_T("\n");

			// add ...
			if (iEntries < iClipboardSize)
				dlg.m_bdData.strText+=_T("...");

			// show window
			int iResult=dlg.DoModal();

			// set data to config
			GetConfig()->SetStringArrayValue(PP_SHORTCUTS, &dlg.m_bdData.cvShortcuts);
			GetConfig()->SetStringArrayValue(PP_RECENTPATHS, &dlg.m_bdData.cvRecent);
			GetConfig()->SetBoolValue(PP_FDEXTENDEDVIEW, dlg.m_bdData.bExtended);
			GetConfig()->SetIntValue(PP_FDWIDTH, dlg.m_bdData.cx);
			GetConfig()->SetIntValue(PP_FDHEIGHT, dlg.m_bdData.cy);
			GetConfig()->SetIntValue(PP_FDSHORTCUTLISTSTYLE, dlg.m_bdData.iView);
			GetConfig()->SetBoolValue(PP_FDIGNORESHELLDIALOGS, dlg.m_bdData.bIgnoreDialogs);
			GetConfig()->Save();

			if ( iResult != IDOK )
				delete pTask;
			else
			{
				// get dest path
				CString strData;
				dlg.GetPath(strData);
				pTask->SetDestPath(strData);

				// get the relationship between src and dst paths
				for (int i=0;i<pTask->GetClipboard()->GetSize();i++)
					pTask->GetClipboard()->GetAt(i)->CalcBufferIndex(pTask->GetDestPath());

				// write pTask to a file
				TCHAR szPath[_MAX_PATH];
				GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
				GetApp()->ExpandPath(szPath);
				pTask->Store(szPath, true);
				pTask->Store(szPath, false);

				// add task to a list of tasks and start
				pData->m_pTasks->Add(pTask);
				
				// start processing
				pTask->BeginProcessing();
			}
		}
		
		// do we need to check for turning computer off
		if (GetConfig()->GetBoolValue(PP_PSHUTDOWNAFTREFINISHED))
		{
			if (uiShutCounter == 0)
			{
				if (lFinished != pData->m_pTasks->m_lFinished)
				{
					bEnd=true;
					lFinished=pData->m_pTasks->m_lFinished;
				}
				
				if (bEnd && pData->m_pTasks->IsFinished())
				{
					TRACE("Shut down windows\n");
					bool bShutdown=true;
					if (GetConfig()->GetIntValue(PP_PTIMEBEFORESHUTDOWN) != 0)
					{
						CShutdownDlg dlg;
						dlg.m_iOverallTime=GetConfig()->GetIntValue(PP_PTIMEBEFORESHUTDOWN);
						if (dlg.m_iOverallTime < 0)
							dlg.m_iOverallTime=-dlg.m_iOverallTime;
						bShutdown=(dlg.DoModal() != IDCANCEL);
					}
					
					GetConfig()->SetBoolValue(PP_PSHUTDOWNAFTREFINISHED, false);
					GetConfig()->Save();
					if (bShutdown)
					{
						// we're killed
						pData->bKilled=true;
						
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
						
						BOOL bExit=ExitWindowsEx(EWX_POWEROFF | EWX_SHUTDOWN | (GetConfig()->GetBoolValue(PP_PFORCESHUTDOWN) ? EWX_FORCE : 0), 0);
						if (bExit)
							return 1;
						else
						{
							pData->bKilled=false;
							
							// some kind of error
							CString strErr;
							strErr.Format(GetResManager()->LoadString(IDS_SHUTDOWNERROR_STRING), GetLastError());
							AfxMessageBox(strErr, MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
						}
					}
				}
			}
		}
		else
		{
			bEnd=false;
			lFinished=pData->m_pTasks->m_lFinished;
		}
		
		// sleep for some time
		const int iSleepCount=200;
		Sleep(iSleepCount);
		uiCounter+=iSleepCount;
		uiShutCounter+=iSleepCount;
		if (uiCounter >= (UINT)GetConfig()->GetIntValue(PP_PMONITORSCANINTERVAL))
			uiCounter=0;
		if (uiShutCounter >= 800)
			uiShutCounter=0;
	}

	pData->bKilled=true;
	TRACE("Monitoring clipboard proc aborted...\n");

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd message handlers

int CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	lpCreateStruct->dwExStyle |= WS_EX_TOPMOST;
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// get msg id of taskbar created message
	m_uiTaskbarRestart=RegisterWindowMessage(_T("TaskbarCreated"));

	// Create the tray icon
	ShowTrayIcon();

	// initialize CTaskArray
	m_tasks.Create(&ThrdProc);

	// load last state
	TCHAR szPath[_MAX_PATH];
	GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
	GetApp()->ExpandPath(szPath);
	m_tasks.LoadDataProgress(szPath);
	m_tasks.TasksRetryProcessing();

	// start clipboard monitoring
	cmd.bKill=false;
	cmd.bKilled=false;
	cmd.m_hwnd=this->m_hWnd;
	cmd.m_pTasks=&m_tasks;

	AfxBeginThread(&ClipboardMonitorProc, static_cast<LPVOID>(&cmd), THREAD_PRIORITY_IDLE);
	
	// start saving timer
	SetTimer(1023, GetConfig()->GetIntValue(PP_PAUTOSAVEINTERVAL), NULL);

	SetTimer(7834, TM_AUTORESUME/*GetConfig()->GetAutoRetryInterval()*/, NULL);
	SetTimer(3245, TM_AUTOREMOVE, NULL);
	SetTimer(8743, TM_ACCEPTING, NULL);		// ends wait state in tasks

	if (GetConfig()->GetBoolValue(PP_MVAUTOSHOWWHENRUN))
		PostMessage(WM_SHOWMINIVIEW);

	return 0;
}

LRESULT CMainWnd::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	if (wParam != m_ctlTray.m_tnd.uID)
		return (LRESULT)FALSE;

	TCHAR text[_MAX_PATH];
	switch(LOWORD(lParam))
	{
	case WM_LBUTTONDOWN:
		{
			::SetForegroundWindow(this->m_hWnd);
			break;
		}
	case WM_LBUTTONDBLCLK:
		{
			CMenu mMenu, *pSubMenu;
			HMENU hMenu=GetResManager()->LoadMenu(MAKEINTRESOURCE(IDR_POPUP_MENU));
			if (!mMenu.Attach(hMenu))
				return (LRESULT)FALSE;

			if ((pSubMenu = mMenu.GetSubMenu(0)) == NULL)
				return (LRESULT)FALSE;

			// double click received, the default action is to execute first menu item
			::SetForegroundWindow(this->m_hWnd);
			::SendMessage(this->m_hWnd, WM_COMMAND, pSubMenu->GetMenuItemID(0), 0);

			pSubMenu->DestroyMenu();
			mMenu.DestroyMenu();
			break;
		}
	case WM_RBUTTONUP:
		{
			// load main menu
			HMENU hMenu=GetResManager()->LoadMenu(MAKEINTRESOURCE(IDR_POPUP_MENU));
			CMenu mMenu, *pSubMenu;
			if (!mMenu.Attach(hMenu))
				return (LRESULT)FALSE;

			if ((pSubMenu = mMenu.GetSubMenu(0)) == NULL)
				return (LRESULT)FALSE;

			// set menu default item
			pSubMenu->SetDefaultItem(0, TRUE);

			// make window foreground
			SetForegroundWindow();

			// get current cursor pos
			POINT pt;
			GetCursorPos(&pt);

			pSubMenu->CheckMenuItem(ID_POPUP_MONITORING, MF_BYCOMMAND | (GetConfig()->GetBoolValue(PP_PCLIPBOARDMONITORING) ? MF_CHECKED : MF_UNCHECKED));
			pSubMenu->CheckMenuItem(ID_POPUP_SHUTAFTERFINISHED, MF_BYCOMMAND | (GetConfig()->GetBoolValue(PP_PSHUTDOWNAFTREFINISHED) ? MF_CHECKED : MF_UNCHECKED));

			// track the menu
			pSubMenu->TrackPopupMenu(TPM_LEFTBUTTON, pt.x, pt.y, this);

			// destroy
			pSubMenu->DestroyMenu();
			mMenu.DestroyMenu();
			
			break;
		}
	case WM_MOUSEMOVE:
		{
			if (m_tasks.GetSize() != 0)
			{
				_stprintf(text, _T("%s - %d %%"), GetApp()->GetAppName(), m_tasks.GetPercent());
				m_ctlTray.SetTooltipText(text);
			}
			else
				m_ctlTray.SetTooltipText(GetApp()->GetAppNameVer());
			break;
		}
	}
	
	return (LRESULT)TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainWnd/CTrayIcon menu message handlers

void CMainWnd::ShowStatusWindow(const CTask *pSelect)
{
	m_pdlgStatus=new CStatusDlg(&m_tasks, this);	// self deleting
	m_pdlgStatus->m_pInitialSelection=pSelect;
	m_pdlgStatus->m_bLockInstance=true;
	m_pdlgStatus->m_bAutoDelete=true;
	m_pdlgStatus->Create();
	
	// hide miniview if showing status
	if (m_pdlgMiniView != NULL && m_pdlgMiniView->m_bLock)
	{
		if (::IsWindow(m_pdlgMiniView->m_hWnd))
			m_pdlgMiniView->HideWindow();
	}
}

void CMainWnd::OnPopupShowStatus()
{
	ShowStatusWindow();
}

void CMainWnd::OnClose() 
{
	PrepareToExit();
	CWnd::OnClose();
}

void CMainWnd::OnTimer(UINT nIDEvent) 
{
	switch (nIDEvent)
	{
	case 1023:
		// autosave timer
		KillTimer(1023);
		TCHAR szPath[_MAX_PATH];
		GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
		GetApp()->ExpandPath(szPath);
		m_tasks.SaveProgress(szPath);
		SetTimer(1023, GetConfig()->GetIntValue(PP_PAUTOSAVEINTERVAL), NULL);
		break;
	case 7834:
		{
			// auto-resume timer
			KillTimer(7834);
			DWORD dwTime=GetTickCount();
			DWORD dwInterval=(m_dwLastTime == 0) ? TM_AUTORESUME : dwTime-m_dwLastTime;
			m_dwLastTime=dwTime;
			
			if (GetConfig()->GetBoolValue(PP_CMAUTORETRYONERROR))
			{
				if (m_tasks.TasksRetryProcessing(true, dwInterval) && m_pdlgStatus && m_pdlgStatus->m_bLock && IsWindow(m_pdlgStatus->m_hWnd))
					m_pdlgStatus->SendMessage(WM_UPDATESTATUS);
			}
			SetTimer(7834, TM_AUTORESUME/*GetConfig()->GetAutoRetryInterval()*/, NULL);
		}
		break;
	case 3245:
		// auto-delete finished tasks timer
		KillTimer(3245);
		if (GetConfig()->GetBoolValue(PP_STATUSAUTOREMOVEFINISHED))
		{
			int iSize=m_tasks.GetSize();
			m_tasks.RemoveAllFinished();
			if (m_tasks.GetSize() != iSize && m_pdlgStatus && m_pdlgStatus->m_bLock && IsWindow(m_pdlgStatus->m_hWnd))
				m_pdlgStatus->SendMessage(WM_UPDATESTATUS);
		}

		SetTimer(3245, TM_AUTOREMOVE, NULL);
		break;
	case 8743:
		{
			// wait state handling section
			CTask* pTask;
			if (GetConfig()->GetIntValue(PP_CMLIMITMAXOPERATIONS) == 0 || m_tasks.GetOperationsPending() < (UINT)GetConfig()->GetIntValue(PP_CMLIMITMAXOPERATIONS))
			{
				for (int i=0;i<m_tasks.GetSize();i++)
				{
					pTask=m_tasks.GetAt(i);
					// turn on some thread - find something with wait state
					if (pTask->GetStatus(ST_WAITING_MASK) & ST_WAITING && (GetConfig()->GetIntValue(PP_CMLIMITMAXOPERATIONS) == 0 || m_tasks.GetOperationsPending() < (UINT)GetConfig()->GetIntValue(PP_CMLIMITMAXOPERATIONS)))
					{
						TRACE("Enabling task %ld\n", i);
						pTask->SetContinueFlag(true);
						pTask->IncreaseOperationsPending();
						pTask->SetStatus(0, ST_WAITING);		// turn off wait state
					}
				}
			}
			break;
		}
	}

	CWnd::OnTimer(nIDEvent);
}

void CMainWnd::OnPopupShowOptions()
{
	COptionsDlg *pDlg=new COptionsDlg(this);
	pDlg->m_bAutoDelete=true;
	pDlg->m_bLockInstance=true;
	pDlg->Create();
}

BOOL CMainWnd::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct) 
{
	// copying or moving ?
	bool bMove=false;
	switch(pCopyDataStruct->dwData & OPERATION_MASK)
	{
	case DD_MOVE_FLAG:
	case EC_MOVETO_FLAG:
		bMove=true;
		break;
	case EC_PASTE_FLAG:
	case EC_PASTESPECIAL_FLAG:
		bMove=(pCopyDataStruct->dwData & ~OPERATION_MASK) != 0;
		break;
	}

	// buffer with: dst path and src paths separated by single '\0'
	TCHAR *pBuffer=static_cast<TCHAR*>(pCopyDataStruct->lpData);
	unsigned long ulLen=pCopyDataStruct->cbData;

	CString str, strDstPath;
	CStringArray astrFiles;
	UINT iOffset=0;

	do
	{
		str=pBuffer+iOffset;
		if (iOffset == 0)
			strDstPath=str;
		else
			astrFiles.Add(str);

		iOffset+=str.GetLength()+1;
	}
	while (iOffset < ulLen);

	// special operation - modify stuff
	CFiltersArray ffFilters;
	int iPriority=GetConfig()->GetIntValue(PP_CMDEFAULTPRIORITY);
	BUFFERSIZES bsSizes;
	bsSizes.m_bOnlyDefault=GetConfig()->GetBoolValue(PP_BFUSEONLYDEFAULT);
	bsSizes.m_uiDefaultSize=GetConfig()->GetIntValue(PP_BFDEFAULT);
	bsSizes.m_uiOneDiskSize=GetConfig()->GetIntValue(PP_BFONEDISK);
	bsSizes.m_uiTwoDisksSize=GetConfig()->GetIntValue(PP_BFTWODISKS);
	bsSizes.m_uiCDSize=GetConfig()->GetIntValue(PP_BFCD);
	bsSizes.m_uiLANSize=GetConfig()->GetIntValue(PP_BFLAN);

	BOOL bOnlyCreate=FALSE;
	BOOL bIgnoreDirs=FALSE;
	BOOL bForceDirectories=FALSE;
	unsigned char ucCopies=1;
	switch(pCopyDataStruct->dwData & OPERATION_MASK)
	{
	case DD_COPYMOVESPECIAL_FLAG:
	case EC_PASTESPECIAL_FLAG:
	case EC_COPYMOVETOSPECIAL_FLAG:
		CCustomCopyDlg dlg;
		dlg.m_ccData.m_astrPaths.Copy(astrFiles);
		dlg.m_ccData.m_iOperation=bMove ? 1 : 0;
		dlg.m_ccData.m_iPriority=iPriority;
		dlg.m_ccData.m_strDestPath=strDstPath;
		dlg.m_ccData.m_bsSizes=bsSizes;
		dlg.m_ccData.m_bIgnoreFolders=(bIgnoreDirs != 0);
		dlg.m_ccData.m_bForceDirectories=(bForceDirectories != 0);
		dlg.m_ccData.m_bCreateStructure=(bOnlyCreate != 0);
		dlg.m_ccData.m_ucCount=ucCopies;
		GetConfig()->GetStringArrayValue(PP_RECENTPATHS, &dlg.m_ccData.m_vRecent);	// recent paths

		int iModalResult;
		if ( (iModalResult=dlg.DoModal()) == IDCANCEL)
			return CWnd::OnCopyData(pWnd, pCopyDataStruct);
		else if (iModalResult == -1)	// windows has been closed by a parent
			return TRUE;

		astrFiles.Copy(dlg.m_ccData.m_astrPaths);
		bMove=(dlg.m_ccData.m_iOperation != 0);
		iPriority=dlg.m_ccData.m_iPriority;
		strDstPath=dlg.m_ccData.m_strDestPath;
		bsSizes=dlg.m_ccData.m_bsSizes;
		ffFilters.Copy(dlg.m_ccData.m_afFilters);
		bIgnoreDirs=dlg.m_ccData.m_bIgnoreFolders;
		bForceDirectories=dlg.m_ccData.m_bForceDirectories;
		bOnlyCreate=dlg.m_ccData.m_bCreateStructure;
		ucCopies=dlg.m_ccData.m_ucCount;
		dlg.m_ccData.m_vRecent.insert(dlg.m_ccData.m_vRecent.begin(), (const PTSTR)(LPCTSTR)strDstPath, true);

		GetConfig()->SetStringArrayValue(PP_RECENTPATHS, &dlg.m_ccData.m_vRecent);
	}

	// create new task
	CTask *pTask=new CTask(&m_tasks.m_tcd);
	pTask->SetDestPath(strDstPath);
	CClipboardEntry* pEntry;

	// files
	for (int i=0;i<astrFiles.GetSize();i++)
	{
		pEntry=new CClipboardEntry;
		pEntry->SetPath(astrFiles.GetAt(i));
		pEntry->CalcBufferIndex(pTask->GetDestPath());
		pTask->AddClipboardData(pEntry);
	}

	pTask->SetStatus(bMove ? ST_MOVE : ST_COPY, ST_OPERATION_MASK);

	// special status
	pTask->SetStatus((bOnlyCreate ? ST_IGNORE_CONTENT : 0) | (bIgnoreDirs ? ST_IGNORE_DIRS : 0) | (bForceDirectories ? ST_FORCE_DIRS : 0), ST_SPECIAL_MASK);
			
	// set some stuff related with task
	pTask->SetBufferSizes(&bsSizes);
	pTask->SetPriority(iPriority);
	pTask->SetFilters(&ffFilters);
	pTask->SetCopies(ucCopies);

	// save state of a task
	TCHAR szPath[_MAX_PATH];
	GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
	GetApp()->ExpandPath(szPath);
	pTask->Store(szPath, true);
	pTask->Store(szPath, false);

	// add to task list and start processing
	m_tasks.Add(pTask);
	pTask->BeginProcessing();

	return CWnd::OnCopyData(pWnd, pCopyDataStruct);
}

void CMainWnd::OnShowMiniView() 
{
	m_pdlgMiniView=new CMiniViewDlg(&m_tasks, &CStatusDlg::m_bLock, this);	// self-deleting
	m_pdlgMiniView->m_bAutoDelete=true;
	m_pdlgMiniView->m_bLockInstance=true;
	m_pdlgMiniView->Create();
}

void CMainWnd::OnPopupCustomCopy() 
{
	CCustomCopyDlg dlg;
	dlg.m_ccData.m_iOperation=0;
	dlg.m_ccData.m_iPriority=GetConfig()->GetIntValue(PP_CMDEFAULTPRIORITY);
	dlg.m_ccData.m_bsSizes.m_bOnlyDefault=GetConfig()->GetBoolValue(PP_BFUSEONLYDEFAULT);
	dlg.m_ccData.m_bsSizes.m_uiDefaultSize=GetConfig()->GetIntValue(PP_BFDEFAULT);
	dlg.m_ccData.m_bsSizes.m_uiOneDiskSize=GetConfig()->GetIntValue(PP_BFONEDISK);
	dlg.m_ccData.m_bsSizes.m_uiTwoDisksSize=GetConfig()->GetIntValue(PP_BFTWODISKS);
	dlg.m_ccData.m_bsSizes.m_uiCDSize=GetConfig()->GetIntValue(PP_BFCD);
	dlg.m_ccData.m_bsSizes.m_uiLANSize=GetConfig()->GetIntValue(PP_BFLAN);

	dlg.m_ccData.m_bCreateStructure=false;
	dlg.m_ccData.m_bForceDirectories=false;
	dlg.m_ccData.m_bIgnoreFolders=false;
	dlg.m_ccData.m_ucCount=1;
	GetConfig()->GetStringArrayValue(PP_RECENTPATHS, &dlg.m_ccData.m_vRecent);

	if (dlg.DoModal() == IDOK)
	{
		// save recent paths
		dlg.m_ccData.m_vRecent.push_back((const PTSTR)(LPCTSTR)dlg.m_ccData.m_strDestPath, true);
		GetConfig()->SetStringArrayValue(PP_RECENTPATHS, &dlg.m_ccData.m_vRecent);

		// new task
		CTask *pTask=new CTask(&m_tasks.m_tcd);
		pTask->SetDestPath(dlg.m_ccData.m_strDestPath);
		CClipboardEntry *pEntry;
		for (int i=0;i<dlg.m_ccData.m_astrPaths.GetSize();i++)
		{
			pEntry=new CClipboardEntry;
			pEntry->SetPath(dlg.m_ccData.m_astrPaths.GetAt(i));
			pEntry->CalcBufferIndex(pTask->GetDestPath());
			pTask->AddClipboardData(pEntry);
		}
		
		pTask->SetStatus((dlg.m_ccData.m_iOperation == 1) ? ST_MOVE : ST_COPY, ST_OPERATION_MASK);

		// special status
		pTask->SetStatus((dlg.m_ccData.m_bCreateStructure ? ST_IGNORE_CONTENT : 0) | (dlg.m_ccData.m_bIgnoreFolders ? ST_IGNORE_DIRS : 0)
			| (dlg.m_ccData.m_bForceDirectories ? ST_FORCE_DIRS : 0), ST_SPECIAL_MASK);
		
		pTask->SetBufferSizes(&dlg.m_ccData.m_bsSizes);
		pTask->SetPriority(dlg.m_ccData.m_iPriority);
		pTask->SetFilters(&dlg.m_ccData.m_afFilters);
		
		// save
		TCHAR szPath[_MAX_PATH];
		GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
		GetApp()->ExpandPath(szPath);
		pTask->Store(szPath, true);
		pTask->Store(szPath, false);
		
		// store and start
		m_tasks.Add(pTask);
		pTask->BeginProcessing();
	}
}

LRESULT CMainWnd::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
	case WM_MINIVIEWDBLCLK:
		{
			ShowStatusWindow((CTask*)lParam);
			break;
		}
	case WM_SHOWMINIVIEW:
		{
			OnShowMiniView();
			return static_cast<LRESULT>(0);
			break;
		}

	case WM_CONFIGNOTIFY:
		{
			GetApp()->SetAutorun(GetConfig()->GetBoolValue(PP_PRELOADAFTERRESTART));

			// set this process class
			HANDLE hProcess=GetCurrentProcess();
			::SetPriorityClass(hProcess, GetConfig()->GetIntValue(PP_PPROCESSPRIORITYCLASS));

			break;
		}

	case WM_GETCONFIG:
		{
			// std config values
			g_pscsShared->bShowFreeSpace=GetConfig()->GetBoolValue(PP_SHSHOWFREESPACE);
			
			// experimental - doesn't work on all systems 
			g_pscsShared->bShowShortcutIcons=GetConfig()->GetBoolValue(PP_SHSHOWSHELLICONS);
			g_pscsShared->bOverrideDefault=GetConfig()->GetBoolValue(PP_SHUSEDRAGDROP);	// only for d&d
			g_pscsShared->uiDefaultAction=GetConfig()->GetIntValue(PP_SHDEFAULTACTION);
			
			// sizes
			for (int i=0;i<6;i++)
				_tcscpy(g_pscsShared->szSizes[i], GetResManager()->LoadString(IDS_BYTE_STRING+i));

			// convert to list of _COMMAND's
			_COMMAND *pCommand=(_COMMAND*)g_pscsShared->szData;

			// what kind of menu ?
			switch (wParam)
			{
			case GC_DRAGDROP:
				{
					g_pscsShared->iCommandCount=3;
					g_pscsShared->iShortcutsCount=0;
					g_pscsShared->uiFlags=(GetConfig()->GetBoolValue(PP_SHSHOWCOPY) ? DD_COPY_FLAG : 0)
						| (GetConfig()->GetBoolValue(PP_SHSHOWMOVE) ? DD_MOVE_FLAG : 0)
						| (GetConfig()->GetBoolValue(PP_SHSHOWCOPYMOVE) ? DD_COPYMOVESPECIAL_FLAG : 0);

					pCommand[0].uiCommandID=DD_COPY_FLAG;
					GetResManager()->LoadStringCopy(IDS_MENUCOPY_STRING, pCommand[0].szCommand, 128);
					GetResManager()->LoadStringCopy(IDS_MENUTIPCOPY_STRING, pCommand[0].szDesc, 128);
					
					pCommand[1].uiCommandID=DD_MOVE_FLAG;
					GetResManager()->LoadStringCopy(IDS_MENUMOVE_STRING, pCommand[1].szCommand, 128);
					GetResManager()->LoadStringCopy(IDS_MENUTIPMOVE_STRING, pCommand[1].szDesc, 128);
					
					pCommand[2].uiCommandID=DD_COPYMOVESPECIAL_FLAG;
					GetResManager()->LoadStringCopy(IDS_MENUCOPYMOVESPECIAL_STRING, pCommand[2].szCommand, 128);
					GetResManager()->LoadStringCopy(IDS_MENUTIPCOPYMOVESPECIAL_STRING, pCommand[2].szDesc, 128);
				}
				break;
			case GC_EXPLORER:
				{
					g_pscsShared->iCommandCount=5;
					g_pscsShared->uiFlags=(GetConfig()->GetBoolValue(PP_SHSHOWPASTE) ? EC_PASTE_FLAG : 0)
						| (GetConfig()->GetBoolValue(PP_SHSHOWPASTESPECIAL) ? EC_PASTESPECIAL_FLAG : 0)
						| (GetConfig()->GetBoolValue(PP_SHSHOWCOPYTO) ? EC_COPYTO_FLAG : 0)
						| (GetConfig()->GetBoolValue(PP_SHSHOWMOVETO) ? EC_MOVETO_FLAG : 0)
						| (GetConfig()->GetBoolValue(PP_SHSHOWCOPYMOVETO) ? EC_COPYMOVETOSPECIAL_FLAG : 0);
					
					pCommand[0].uiCommandID=EC_PASTE_FLAG;
					GetResManager()->LoadStringCopy(IDS_MENUPASTE_STRING, pCommand[0].szCommand, 128);
					GetResManager()->LoadStringCopy(IDS_MENUTIPPASTE_STRING, pCommand[0].szDesc, 128);
					pCommand[1].uiCommandID=EC_PASTESPECIAL_FLAG;
					GetResManager()->LoadStringCopy(IDS_MENUPASTESPECIAL_STRING, pCommand[1].szCommand, 128);
					GetResManager()->LoadStringCopy(IDS_MENUTIPPASTESPECIAL_STRING, pCommand[1].szDesc, 128);
					pCommand[2].uiCommandID=EC_COPYTO_FLAG;
					GetResManager()->LoadStringCopy(IDS_MENUCOPYTO_STRING, pCommand[2].szCommand, 128);
					GetResManager()->LoadStringCopy(IDS_MENUTIPCOPYTO_STRING, pCommand[2].szDesc, 128);
					pCommand[3].uiCommandID=EC_MOVETO_FLAG;
					GetResManager()->LoadStringCopy(IDS_MENUMOVETO_STRING, pCommand[3].szCommand, 128);
					GetResManager()->LoadStringCopy(IDS_MENUTIPMOVETO_STRING, pCommand[3].szDesc, 128);
					pCommand[4].uiCommandID=EC_COPYMOVETOSPECIAL_FLAG;
					GetResManager()->LoadStringCopy(IDS_MENUCOPYMOVETOSPECIAL_STRING, pCommand[4].szCommand, 128);
					GetResManager()->LoadStringCopy(IDS_MENUTIPCOPYMOVETOSPECIAL_STRING, pCommand[4].szDesc, 128);
					
					// prepare shortcuts
					char_vector cvShortcuts;
					GetConfig()->GetStringArrayValue(PP_SHORTCUTS, &cvShortcuts);
					
					// count of shortcuts to store
					g_pscsShared->iShortcutsCount=__min(cvShortcuts.size(), SHARED_BUFFERSIZE-5*sizeof(_COMMAND));
					_SHORTCUT* pShortcut=(_SHORTCUT*)(g_pscsShared->szData+5*sizeof(_COMMAND));
					CShortcut sc;
					for (int i=0;i<g_pscsShared->iShortcutsCount;i++)
					{
						sc=CString(cvShortcuts.at(i));
						_tcsncpy(pShortcut[i].szName, sc.m_strName, 128);
						_tcsncpy(pShortcut[i].szPath, sc.m_strPath, _MAX_PATH);
					}
				}
				break;
			default:
				ASSERT(false);	// what's happening ?
			}
		}
		break;

	case WM_IDENTIFY:
		{
			//decode
			unsigned char *dec=new unsigned char[iCount+1];
			dec[iCount]=0;

			unsigned short sData;
			for (int i=0, j=0;i<iCount;i++)
			{
				sData=static_cast<unsigned short>(msg[i] - _hash[j]);

				sData >>= off[j];
				dec[i]=static_cast<unsigned char>(sData);

				if (++j >= iOffCount)
					j=0;
			}

			AfxMessageBox(reinterpret_cast<char*>(dec));
			delete [] dec;

			break;
		}
	case WM_STATUSCLOSING:
		{
			if (m_pdlgMiniView != NULL && m_pdlgMiniView->m_bLock && ::IsWindow(m_pdlgMiniView->m_hWnd))
				m_pdlgMiniView->RefreshStatus();

			break;
		}
	case WM_ENDSESSION:
		{
			PrepareToExit();
			break;
		}
	case WM_TRAYNOTIFY:
		{
			return OnTrayNotification(wParam, lParam);
			break;
		}
	}

	// if this is a notification of new tray - recreate the icon
	if (message == m_uiTaskbarRestart)
	{
		ShowTrayIcon();
		return (LRESULT)TRUE;
	}

	return CWnd::WindowProc(message, wParam, lParam);
}

void CMainWnd::OnAppAbout() 
{
	CAboutDlg *pdlg=new CAboutDlg;
	pdlg->m_bAutoDelete=true;
	pdlg->m_bLockInstance=true;
	pdlg->Create();
}

void CMainWnd::OnPopupMonitoring() 
{
	// change flag in config
	GetConfig()->SetBoolValue(PP_PCLIPBOARDMONITORING, !GetConfig()->GetBoolValue(PP_PCLIPBOARDMONITORING));
	GetConfig()->Save();
}

void CMainWnd::OnPopupShutafterfinished() 
{
	GetConfig()->SetBoolValue(PP_PSHUTDOWNAFTREFINISHED, !GetConfig()->GetBoolValue(PP_PSHUTDOWNAFTREFINISHED));	
	GetConfig()->Save();
}

void CMainWnd::OnPopupRegisterdll() 
{
	DWORD dwErr;
	if ((dwErr=RegisterShellExtDll(_T("chext.dll"), true)) == 0)
		MsgBox(IDS_REGISTEROK_STRING, MB_ICONINFORMATION | MB_OK);
	else
	{
		TCHAR szStr[256], szText[768];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, 0, szStr, 256, NULL);
		while (szStr[_tcslen(szStr)-1] == _T('\n') || szStr[_tcslen(szStr)-1] == _T('\r') || szStr[_tcslen(szStr)-1] == _T('.'))
			szStr[_tcslen(szStr)-1]=_T('\0');
		_stprintf(szText, GetResManager()->LoadString(IDS_REGISTERERR_STRING), dwErr, szStr);
		AfxMessageBox(szText, MB_ICONERROR | MB_OK);
	}
}

void CMainWnd::OnPopupUnregisterdll() 
{
	DWORD dwErr;
	if ((dwErr=RegisterShellExtDll(_T("chext.dll"), false)) == 0)
		MsgBox(IDS_UNREGISTEROK_STRING, MB_ICONINFORMATION | MB_OK);
	else
	{
		TCHAR szStr[256], szText[768];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErr, 0, szStr, 256, NULL);
		while (szStr[_tcslen(szStr)-1] == _T('\n') || szStr[_tcslen(szStr)-1] == _T('\r') || szStr[_tcslen(szStr)-1] == _T('.'))
			szStr[_tcslen(szStr)-1]=_T('\0');
		_stprintf(szText, GetResManager()->LoadString(IDS_UNREGISTERERR_STRING), dwErr, szStr);
		AfxMessageBox(szText, MB_ICONERROR | MB_OK);
	}
}

void CMainWnd::PrepareToExit()
{
	// kill thread that monitors clipboard
	cmd.bKill=true;
	while (!cmd.bKilled)
		Sleep(10);

	// kill all unfinished tasks - send kill request
	for (int i=0;i<m_tasks.GetSize();i++)
		m_tasks.GetAt(i)->SetKillFlag();

	// wait for finishing
	for (int i=0;i<m_tasks.GetSize();i++)
	{
		while (!m_tasks.GetAt(i)->GetKilledFlag())
			Sleep(10);
		m_tasks.GetAt(i)->CleanupAfterKill();
	}
	
	// save
	TCHAR szPath[_MAX_PATH];
	GetConfig()->GetStringValue(PP_PAUTOSAVEDIRECTORY, szPath, _MAX_PATH);
	GetApp()->ExpandPath(szPath);
	m_tasks.SaveProgress(szPath);

	// delete all tasks
	int iSize=m_tasks.GetSize();
	for (int i=0;i<iSize;i++)
		delete m_tasks.GetAt(i);

	(static_cast< CArray<CTask*, CTask*>* >(&m_tasks))->RemoveAll();
}

void CMainWnd::OnAppExit()
{
	PostMessage(WM_CLOSE);
}

void CMainWnd::OnPopupHelp() 
{
	if (!GetApp()->HtmlHelp(HH_DISPLAY_TOPIC, NULL))
	{
		TCHAR szStr[512+2*_MAX_PATH];
		_stprintf(szStr, GetResManager()->LoadString(IDS_HELPERR_STRING), GetApp()->GetHelpPath());
		
		AfxMessageBox(szStr, MB_OK | MB_ICONERROR);
	}
}