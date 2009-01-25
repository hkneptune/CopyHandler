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
#include "btnIDs.h"
#include "..\Common\FileSupport.h"
#include "AboutDlg.h"
#include "register.h"
#include "ShutdownDlg.h"
#include "StringHelpers.h"
#include "..\common\ipcstructs.h"
#include <assert.h>
#include "af_defs.h"
#include "UpdateChecker.h"
#include "UpdaterDlg.h"
#include <boost/assert.hpp>

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
	PCTSTR pszAppVer = GetApp()->GetAppNameVer();
	bool bRes=m_ctlTray.CreateIcon(m_hWnd, WM_TRAYNOTIFY, pszAppVer, hIcon, 0);
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
	ON_COMMAND(ID_POPUP_CHECKFORUPDATES, &CMainWnd::OnPopupCheckForUpdates)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMainWnd construction/destruction

CMainWnd::CMainWnd() :
	m_pFeedbackFactory(CFeedbackHandlerFactory::CreateFactory())
{
	m_pdlgStatus=NULL;
	m_pdlgMiniView=NULL;
	m_dwLastTime=0;
}

CMainWnd::~CMainWnd()
{
	if(m_pFeedbackFactory)
		m_pFeedbackFactory->Delete();
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
	ictranslate::CFormat fmt;
	for (int i=0;i<nSize;i++)
	{
		// read attributes of src file/folder
		if (!fi.Create(pTask->GetClipboardData(i)->GetPath(), i))
		{
			// log
			fmt.SetFormat(GetResManager()->LoadString(IDS_OTFMISSINGCLIPBOARDINPUT_STRING));
			fmt.SetParam(_t("%path"), pTask->GetClipboardData(i)->GetPath());
			pTask->m_log.logw(fmt);
			continue;
		}
		else
		{
			// log
			fmt.SetFormat(GetResManager()->LoadString(IDS_OTFADDINGCLIPBOARDFILE_STRING));
			fmt.SetParam(_t("%path"), pTask->GetClipboardData(i)->GetPath());
			pTask->m_log.logi(fmt);
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
				fmt.SetFormat(GetResManager()->LoadString(IDS_OTFADDEDFOLDER_STRING));
				fmt.SetParam(_t("%path"), fi.GetFullFilePath());
				pTask->m_log.logi(fmt);
			}

			// don't add folder contents when moving inside one disk boundary
			if (bIgnoreDirs || !bMove || pTask->GetCopies() > 1 || iDestDrvNumber == -1
				|| iDestDrvNumber != fi.GetDriveNumber() || CFileInfo::Exist(fi.GetDestinationPath(pTask->GetDestPath().GetPath(), 0, ((int)bForceDirectories) << 1)) )
			{
				// log
				fmt.SetFormat(GetResManager()->LoadString(IDS_OTFRECURSINGFOLDER_STRING));
				fmt.SetParam(_t("%path"), fi.GetFullFilePath());
				pTask->m_log.logi(fmt);
				
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
			fmt.SetFormat(GetResManager()->LoadString(IDS_OTFADDEDFILE_STRING));
			fmt.SetParam(_t("%path"), fi.GetFullFilePath());
			pTask->m_log.logi(fmt);
		}
	}
	
	// calc size of all files
	pTask->CalcAllSize();
	
	// update *m_pnTasksAll;
	pTask->IncreaseAllTasksSize(pTask->GetAllSize());
	
	// change state to ST_COPYING - finished searching for files
	pTask->SetStatus(ST_COPYING, ST_STEP_MASK);
	
	// save task status
	pTask->Store(true);
	pTask->Store(false);

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
	ictranslate::CFormat fmt;

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
		if(!(fi.GetFlags() & FIF_PROCESSED))
			continue;
		
		// delete data
		if (fi.IsDirectory())
		{
			if (!GetConfig()->get_bool(PP_CMPROTECTROFILES))
				SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY);
			bSuccess=RemoveDirectory(fi.GetFullFilePath());
		}
		else
		{
			// set files attributes to normal - it'd slow processing a bit, but it's better.
			if (!GetConfig()->get_bool(PP_CMPROTECTROFILES))
				SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
			bSuccess=DeleteFile(fi.GetFullFilePath());
		}
		
		// operation failed
		DWORD dwLastError=GetLastError();
		if (!bSuccess && dwLastError != ERROR_PATH_NOT_FOUND && dwLastError != ERROR_FILE_NOT_FOUND)
		{
			// log
			fmt.SetFormat(GetResManager()->LoadString(IDS_OTFDELETINGERROR_STRING));
			fmt.SetParam(_t("%errno"), dwLastError);
			fmt.SetParam(_t("%path"), fi.GetFullFilePath());
			pTask->m_log.loge(fmt);
			throw new CProcessingException(E_ERROR, pTask, dwLastError, fmt);
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
	ictranslate::CFormat fmt;
	try
	{
		// do we copy rest or recopy ?
		bool bCopyRest=GetConfig()->get_bool(PP_CMUSEAUTOCOMPLETEFILES);
//		UINT uiNotificationType=(UINT)GetConfig()->get_signed_num(PP_CMSHOWVISUALFEEDBACK);

		// Data regarding dest file
		CFileInfo fiDest;
		bool bExist=fiDest.Create(pData->strDstFile, -1);

		chcore::IFeedbackHandler* piFeedbackHandler = pData->pTask->GetFeedbackHandler();
		BOOST_ASSERT(piFeedbackHandler);

//		int iDlgCode=-1;
//		int *piLastDlgDesc=NULL;		// ptr to int describing last used dialog
		
		// don't ask for copy rest
//		bool bDontAsk=(pData->pTask->GetCurrentIndex() == pData->pTask->GetLastProcessedIndex());
		pData->pTask->SetLastProcessedIndex(-1);

		// if dest file size >0 - we can do somethng more than usual
		if(bExist)
		{
			// src and dst files are the same
			FEEDBACK_ALREADYEXISTS feedStruct = { pData->pfiSrcFile, &fiDest };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileAlreadyExists, &feedStruct);
			// check for dialog result
			switch(frResult)
			{
			case CFeedbackHandler::eResult_Overwrite:
				{
					bCopyRest=false;
					break;
				}
			case CFeedbackHandler::eResult_CopyRest:
				{
					bCopyRest=true;
					break;
				}
			case CFeedbackHandler::eResult_Skip:
				{
					pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
					pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
					pData->bProcessed = false;
					return;
				}
			case CFeedbackHandler::eResult_Cancel:
				{
					// log
					if (GetConfig()->get_bool(PP_CMCREATELOG))
					{
						fmt.SetFormat(GetResManager()->LoadString(IDS_OTFPRECHECKCANCELREQUEST_STRING));
						fmt.SetParam(_t("%path"), pData->pfiSrcFile->GetFullFilePath());
						pData->pTask->m_log.logi(fmt);
					}
					throw new CProcessingException(E_CANCEL, pData->pTask);
					break;
				}
			case CFeedbackHandler::eResult_Pause:
				{
					throw new CProcessingException(E_PAUSE, pData->pTask);
					break;
				}
			default:
				{
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
					break;
				}
			}
		}// bExist

		// change attributes of a dest file
		if (!GetConfig()->get_bool(PP_CMPROTECTROFILES))
			SetFileAttributes(pData->strDstFile, FILE_ATTRIBUTE_NORMAL);

		// first or second pass ? only for FFNB
		bool bFirstPass=true;

		// check size of src file to know whether use flag FILE_FLAG_NOBUFFERING
l_start:
		bool bNoBuffer=(bFirstPass && GetConfig()->get_bool(PP_BFUSENOBUFFERING) && pData->pfiSrcFile->GetLength64() >= (unsigned long long)GetConfig()->get_signed_num(PP_BFBOUNDARYLIMIT));

		// refresh data about file
		if (!bFirstPass)
			bExist=fiDest.Create(pData->strDstFile, -1);

		// open src
l_openingsrc:
		hSrc=CreateFile(pData->pfiSrcFile->GetFullFilePath(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffer ? FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH : 0), NULL);
		if (hSrc == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError=GetLastError();
			CString strFile = pData->pfiSrcFile->GetFullFilePath();
			FEEDBACK_FILEERROR feedStruct = { (PCTSTR)strFile, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);

			switch (frResult)
			{
			case CFeedbackHandler::eResult_Skip:
				pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
				pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
				pData->bProcessed = false;
				return;
				break;
			case CFeedbackHandler::eResult_Cancel:
				// log
				fmt.SetFormat(GetResManager()->LoadString(IDS_OTFOPENINGCANCELREQUEST_STRING));
				fmt.SetParam(_t("%errno"), dwLastError);
				fmt.SetParam(_t("%path"), pData->pfiSrcFile->GetFullFilePath());
				pData->pTask->m_log.loge(fmt);
				throw new CProcessingException(E_CANCEL, pData->pTask);
				break;
			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE, pData->pTask);
				break;
			case CFeedbackHandler::eResult_Retry:
				// log
				fmt.SetFormat(GetResManager()->LoadString(IDS_OTFOPENINGRETRY_STRING));
				fmt.SetParam(_t("%errno"), dwLastError);
				fmt.SetParam(_t("%path"), pData->pfiSrcFile->GetFullFilePath());
				pData->pTask->m_log.loge(fmt);
				goto l_openingsrc;
				break;
			default:
				{
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
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
			CString strFile = pData->strDstFile;
 
			FEEDBACK_FILEERROR feedStruct = { (PCTSTR)strFile, dwLastError };
			CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_FileError, &feedStruct);
			switch (frResult)
			{
			case CFeedbackHandler::eResult_Retry:
				// change attributes
				if (!GetConfig()->get_bool(PP_CMPROTECTROFILES))
					SetFileAttributes(pData->strDstFile, FILE_ATTRIBUTE_NORMAL);

				// log
				fmt.SetFormat(GetResManager()->LoadString(IDS_OTFDESTOPENINGRETRY_STRING));
				fmt.SetParam(_t("%errno"), dwLastError);
				fmt.SetParam(_t("%path"), pData->strDstFile);
				pData->pTask->m_log.loge(fmt);
				goto l_openingdst;
				break;
			case CFeedbackHandler::eResult_Cancel:
				// log
				fmt.SetFormat(GetResManager()->LoadString(IDS_OTFDESTOPENINGCANCELREQUEST_STRING));
				fmt.SetParam(_t("%errno"), dwLastError);
				fmt.SetParam(_t("%path"), pData->strDstFile);
				pData->pTask->m_log.loge(fmt);
				throw new CProcessingException(E_CANCEL, pData->pTask);
				break;
			case CFeedbackHandler::eResult_Skip:
				pData->pTask->IncreaseProcessedSize(pData->pfiSrcFile->GetLength64());
				pData->pTask->IncreaseProcessedTasksSize(pData->pfiSrcFile->GetLength64());
				pData->bProcessed = false;
				return;
				break;
			case CFeedbackHandler::eResult_Pause:
				throw new CProcessingException(E_PAUSE, pData->pTask);
				break;
			default:
				{
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, pData->pTask, 0, _t("Unknown feedback result type"));
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
						fmt.SetFormat(GetResManager()->LoadString(IDS_OTFMOVINGPOINTERSERROR_STRING));
						fmt.SetParam(_t("%errno"), GetLastError());
						fmt.SetParam(_t("%srcpath"), pData->pfiSrcFile->GetFullFilePath());
						fmt.SetParam(_t("%dstpath"), pData->strDstFile);
						fmt.SetParam(_t("%pos"), ullMove);
						pData->pTask->m_log.loge(fmt);

						// seek failed - seek to begin
						if (SetFilePointer64(hSrc, 0, FILE_BEGIN) == -1 || SetFilePointer64(hDst, 0, FILE_BEGIN) == -1)
						{
							// log
							dwLastError=GetLastError();
							fmt.SetFormat(GetResManager()->LoadString(IDS_OTFRESTORINGPOINTERSERROR_STRING));
							fmt.SetParam(_t("%errno"), dwLastError);
							fmt.SetParam(_t("%srcpath"), pData->pfiSrcFile->GetFullFilePath());
							fmt.SetParam(_t("%dstpath"), pData->strDstFile);
							pData->pTask->m_log.loge(fmt);
							throw new CProcessingException(E_ERROR, pData->pTask, dwLastError, fmt);
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
					fmt.SetFormat(GetResManager()->LoadString(IDS_OTFSETTINGZEROSIZEERROR_STRING));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%path"), pData->strDstFile);
					pData->pTask->m_log.loge(fmt);
					throw new CProcessingException(E_ERROR, pData->pTask, dwLastError, fmt);
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
					fmt.SetFormat(GetResManager()->LoadString(IDS_OTFCOPYINGKILLREQUEST_STRING));
					fmt.SetParam(_t("%srcpath"), pData->pfiSrcFile->GetFullFilePath());
					fmt.SetParam(_t("%dstpath"), pData->strDstFile);
					pData->pTask->m_log.logi(fmt);
					throw new CProcessingException(E_KILL_REQUEST, pData->pTask);
				}
				
				// recreate buffer if needed
				if (!(*pData->dbBuffer.GetSizes() == *pData->pTask->GetBufferSizes()))
				{
					// log
					const BUFFERSIZES *pbs1=pData->dbBuffer.GetSizes(), *pbs2=pData->pTask->GetBufferSizes();

					fmt.SetFormat(GetResManager()->LoadString(IDS_OTFCHANGINGBUFFERSIZE_STRING));
					
					fmt.SetParam(_t("%defsize"), pbs1->m_uiDefaultSize);
					fmt.SetParam(_t("%onesize"), pbs1->m_uiOneDiskSize);
					fmt.SetParam(_t("%twosize"), pbs1->m_uiTwoDisksSize);
					fmt.SetParam(_t("%cdsize"), pbs1->m_uiCDSize);
					fmt.SetParam(_t("%lansize"), pbs1->m_uiLANSize);
					fmt.SetParam(_t("%defsize2"), pbs2->m_uiDefaultSize);
					fmt.SetParam(_t("%onesize2"), pbs2->m_uiOneDiskSize);
					fmt.SetParam(_t("%twosize2"), pbs2->m_uiTwoDisksSize);
					fmt.SetParam(_t("%cdsize2"), pbs2->m_uiCDSize);
					fmt.SetParam(_t("%lansize2"), pbs2->m_uiLANSize);
					fmt.SetParam(_t("%srcpath"), pData->pfiSrcFile->GetFullFilePath());
					fmt.SetParam(_t("%dstpath"), pData->strDstFile);

					pData->pTask->m_log.logi(fmt);
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
					fmt.SetFormat(GetResManager()->LoadString(IDS_OTFREADINGERROR_STRING));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%count"), tord);
					fmt.SetParam(_t("%path"), pData->pfiSrcFile->GetFullFilePath());
					pData->pTask->m_log.loge(fmt);
					throw new CProcessingException(E_ERROR, pData->pTask, dwLastError, fmt);
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
					fmt.SetFormat(GetResManager()->LoadString(IDS_OTFWRITINGERROR_STRING));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%count"), rd);
					fmt.SetParam(_t("%path"), pData->strDstFile);
					pData->pTask->m_log.loge(fmt);
					throw new CProcessingException(E_ERROR, pData->pTask, dwLastError, fmt);
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

		pData->bProcessed = true;
	}
	catch(...)
	{
		// log
		fmt.SetFormat(GetResManager()->LoadString(IDS_OTFCAUGHTEXCEPTIONCCF_STRING));
		fmt.SetParam(_t("%errno"), GetLastError());
		fmt.SetParam(_t("%timestamp"), GetTickCount());
		pData->pTask->m_log.loge(fmt);

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
	ccp.bProcessed = false;
	ccp.pTask=pTask;
	ccp.bOnlyCreate=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_CONTENT) != 0;
	ccp.dbBuffer.Create(pTask->GetBufferSizes());
	
	// helpers
	//CFileInfo fi;	// for currently processed element
	DWORD dwLastError;
	
	// begin at index which wasn't processed previously
	int nSize=pTask->FilesGetSize();	// wielkoœæ tablicy
	int iCopiesCount=pTask->GetCopies();	// iloœæ kopii
	bool bIgnoreFolders=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_IGNORE_DIRS) != 0;
	bool bForceDirectories=(pTask->GetStatus(ST_SPECIAL_MASK) & ST_FORCE_DIRS) != 0;
	const CDestPath& dpDestPath=pTask->GetDestPath();

	// log
	const BUFFERSIZES* pbs=ccp.dbBuffer.GetSizes();

	ictranslate::CFormat fmt;
	fmt.SetFormat(GetResManager()->LoadString(IDS_OTFPROCESSINGFILESDATA_STRING));
	fmt.SetParam(_t("%create"), ccp.bOnlyCreate);
	fmt.SetParam(_t("%defsize"), pbs->m_uiDefaultSize);
	fmt.SetParam(_t("%onesize"), pbs->m_uiOneDiskSize);
	fmt.SetParam(_t("%twosize"), pbs->m_uiTwoDisksSize);
	fmt.SetParam(_t("%cdsize"), pbs->m_uiCDSize);
	fmt.SetParam(_t("%lansize"), pbs->m_uiLANSize);
	fmt.SetParam(_t("%filecount"), nSize);
	fmt.SetParam(_t("%copycount"), iCopiesCount);
	fmt.SetParam(_t("%ignorefolders"), bIgnoreFolders);
	fmt.SetParam(_t("%dstpath"), dpDestPath.GetPath());
	fmt.SetParam(_t("%currpass"), pTask->GetCurrentCopy());
	fmt.SetParam(_t("%currindex"), pTask->GetCurrentIndex());

	pTask->m_log.logi(fmt);

	for (unsigned char j=pTask->GetCurrentCopy();j<iCopiesCount;j++)
	{
		pTask->SetCurrentCopy(j);
		for (int i=pTask->GetCurrentIndex();i<nSize;i++)
		{
			// update m_nCurrentIndex, getting current CFileInfo
			pTask->SetCurrentIndex(i);
			CFileInfo& fi=pTask->FilesGetAtCurrentIndex();
			
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
					fmt.SetFormat(GetResManager()->LoadString(IDS_OTFMOVEFILEERROR_STRING));
					fmt.SetParam(_t("%errno"), dwLastError);
					fmt.SetParam(_t("%srcpath"), fi.GetFullFilePath());
					fmt.SetParam(_t("%dstpath"), ccp.strDstFile);
					pTask->m_log.loge(fmt);
					throw new CProcessingException(E_ERROR, pTask, dwLastError, fmt);
				}
				else
					fi.SetFlags(FIF_PROCESSED, FIF_PROCESSED);
			}
			else
			{
				// if folder - create it
				if ( fi.IsDirectory() )
				{
					if (!CreateDirectory(ccp.strDstFile, NULL) && (dwLastError=GetLastError()) != ERROR_ALREADY_EXISTS )
					{
						// log
						fmt.SetFormat(GetResManager()->LoadString(IDS_OTFCREATEDIRECTORYERROR_STRING));
						fmt.SetParam(_t("%errno"), dwLastError);
						fmt.SetParam(_t("%path"), ccp.strDstFile);
						pTask->m_log.loge(fmt);
						throw new CProcessingException(E_ERROR, pTask, dwLastError, fmt);
					}
					
					pTask->IncreaseProcessedSize(fi.GetLength64());
					pTask->IncreaseProcessedTasksSize(fi.GetLength64());
					fi.SetFlags(FIF_PROCESSED, FIF_PROCESSED);
				}
				else
				{
					// start copying/moving file
					ccp.pfiSrcFile=&fi;
					ccp.bProcessed = false;
					
					// kopiuj dane
					CustomCopyFile(&ccp);
					fi.SetFlags(ccp.bProcessed ? FIF_PROCESSED : 0, FIF_PROCESSED);

					// if moving - delete file (only if config flag is set)
					if (bMove && fi.GetFlags() & FIF_PROCESSED && !GetConfig()->get_bool(PP_CMDELETEAFTERFINISHED) && j == iCopiesCount-1)
					{
						if (!GetConfig()->get_bool(PP_CMPROTECTROFILES))
							SetFileAttributes(fi.GetFullFilePath(), FILE_ATTRIBUTE_NORMAL);
						DeleteFile(fi.GetFullFilePath());	// there will be another try later, so I don't check
															// if succeeded
					}
				}
				
				// set a time
				if (GetConfig()->get_bool(PP_CMSETDESTDATE))
					SetFileDirectoryTime(ccp.strDstFile, &fi); // no error check - ma³o istotne
				
				// attributes
				if (GetConfig()->get_bool(PP_CMSETDESTATTRIBUTES))
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

	tstring_t strPath = pTask->GetTaskPath();
	strPath += pTask->GetUniqueName()+_T(".log");

	pTask->m_log.init(strPath.c_str(), 262144, icpf::log_file::level_debug, false, false);

	// set thread boost
	HANDLE hThread=GetCurrentThread();
	::SetThreadPriorityBoost(hThread, GetConfig()->get_bool(PP_CMDISABLEPRIORITYBOOST));

	CTime tm=CTime::GetCurrentTime();

	ictranslate::CFormat fmt;
	fmt.SetFormat(GetResManager()->LoadString(IDS_OTFTHREADSTART_STRING));
	fmt.SetParam(_t("%year"), tm.GetYear());
	fmt.SetParam(_t("%month"), tm.GetMonth());
	fmt.SetParam(_t("%day"), tm.GetDay());
	fmt.SetParam(_t("%hour"), tm.GetHour());
	fmt.SetParam(_t("%minute"), tm.GetMinute());
	fmt.SetParam(_t("%second"), tm.GetSecond());
	pTask->m_log.logi(fmt);

	try
	{
		// to make the value stable
		bool bReadTasksSize=GetConfig()->get_bool(PP_CMREADSIZEBEFOREBLOCKING);

		if (!bReadTasksSize)
			CheckForWaitState(pTask);	// operation limiting

		// set what's needed
		pTask->m_lLastTime=(long)time(NULL);	// last time (start counting)

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
		ull_t ullNeededSize = 0, ullAvailableSize = 0;
l_showfeedback:
		pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFCHECKINGSPACE_STRING));

		if (!pTask->GetRequiredFreeSpace(&ullNeededSize, &ullAvailableSize))
		{
			fmt.SetFormat(GetResManager()->LoadString(IDS_OTFNOTENOUGHFREESPACE_STRING));
			fmt.SetParam(_t("%needsize"), ullNeededSize);
			fmt.SetParam(_t("%availablesize"), ullAvailableSize);
			pTask->m_log.logw(fmt);
			
			chcore::IFeedbackHandler* piFeedbackHandler = pTask->GetFeedbackHandler();
			BOOST_ASSERT(piFeedbackHandler);

			if(pTask->GetClipboardDataSize() > 0)
			{
				CString strSrcPath = pTask->GetClipboardData(0)->GetPath();
				CString strDstPath = pTask->GetDestPath().GetPath();
				FEEDBACK_NOTENOUGHSPACE feedStruct = { ullNeededSize, (PCTSTR)strSrcPath, (PCTSTR)strDstPath };
				CFeedbackHandler::EFeedbackResult frResult = (CFeedbackHandler::EFeedbackResult)piFeedbackHandler->RequestFeedback(CFeedbackHandler::eFT_NotEnoughSpace, &feedStruct);

				// default
				switch (frResult)
				{
				case CFeedbackHandler::eResult_Cancel:
					{
						pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFFREESPACECANCELREQUEST_STRING));
						throw new CProcessingException(E_CANCEL, pTask);
						break;
					}
				case CFeedbackHandler::eResult_Retry:
					pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFFREESPACERETRYING_STRING));
					goto l_showfeedback;
					break;
				case CFeedbackHandler::eResult_Skip:
					pTask->m_log.logi(GetResManager()->LoadString(IDS_OTFFREESPACEIGNORE_STRING));
					break;
				default:
					BOOST_ASSERT(FALSE);		// unknown result
					throw new CProcessingException(E_ERROR, pTask, 0, _t("Unknown feedback result type"));
					break;
				}
			}
		}

		if (bReadTasksSize)
		{
			pTask->UpdateTime();
			pTask->m_lLastTime=-1;

			CheckForWaitState(pTask);

			pTask->m_lLastTime=(long)time(NULL);
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
		pTask->Store(false);
		
		// we are ending
		pTask->DecreaseOperationsPending();

		// play sound
		if (GetConfig()->get_bool(PP_SNDPLAYSOUNDS))
		{
			GetConfig()->get_string(PP_SNDFINISHEDSOUNDPATH, szPath, _MAX_PATH);
			GetApp()->ExpandPath(szPath);
			PlaySound(szPath, NULL, SND_FILENAME | SND_ASYNC);
		}

		tm=CTime::GetCurrentTime();
		fmt.SetFormat(GetResManager()->LoadString(IDS_OTFTHREADFINISHED_STRING));
		fmt.SetParam(_t("%year"), tm.GetYear());
		fmt.SetParam(_t("%month"), tm.GetMonth());
		fmt.SetParam(_t("%day"), tm.GetDay());
		fmt.SetParam(_t("%hour"), tm.GetHour());
		fmt.SetParam(_t("%minute"), tm.GetMinute());
		fmt.SetParam(_t("%second"), tm.GetSecond());
		pTask->m_log.logi(fmt);

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
		fmt.SetFormat(GetResManager()->LoadString(IDS_OTFCAUGHTEXCEPTIONMAIN_STRING));
		fmt.SetParam(_t("%errno"), e->m_dwError);
		fmt.SetParam(_t("%type"), e->m_iType);
		pTask->m_log.loge(fmt);

		if (e->m_iType == E_ERROR && GetConfig()->get_bool(PP_SNDPLAYSOUNDS))
		{
			GetConfig()->get_string(PP_SNDERRORSOUNDPATH, szPath, _MAX_PATH);
			GetApp()->ExpandPath(szPath);
			PlaySound(szPath, NULL, SND_FILENAME | SND_ASYNC);
		}

		// pause task if requested
		if(e->m_iType == E_PAUSE)
			pTask->SetStatus(ST_PAUSED, ST_PAUSED);

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
//	UINT i;	// counter
	CTask *pTask;	// ptr to a task
	CClipboardEntry* pEntry=NULL;

	// register clipboard format
	UINT nFormat=RegisterClipboardFormat(_T("Preferred DropEffect"));
	UINT uiCounter=0, uiShutCounter=0;;
	LONG lFinished=0;
	bool bEnd=false;

	icpf::config* pConfig = GetConfig();
	BOOST_ASSERT(pConfig);
	if(!pConfig)
		return 1;
	while (!pData->bKill)
	{
		if (uiCounter == 0 && pConfig->get_bool(PP_PCLIPBOARDMONITORING) && IsClipboardFormatAvailable(CF_HDROP))
		{
			// get data from clipboard
			OpenClipboard(pData->m_hwnd);
			HANDLE handle=GetClipboardData(CF_HDROP);

			UINT nCount=DragQueryFile(static_cast<HDROP>(handle), 0xffffffff, NULL, 0);

			pTask = pData->m_pTasks->CreateTask();

			for (UINT i=0;i<nCount;i++)
			{
				DragQueryFile(static_cast<HDROP>(handle), i, path, _MAX_PATH);
				pEntry=new CClipboardEntry;
				pEntry->SetPath(path);
				pTask->AddClipboardData(pEntry);
			}
			
			if (IsClipboardFormatAvailable(nFormat))
			{
				handle=GetClipboardData(nFormat);
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
			bs.m_bOnlyDefault=pConfig->get_bool(PP_BFUSEONLYDEFAULT);
			bs.m_uiDefaultSize=(UINT)pConfig->get_signed_num(PP_BFDEFAULT);
			bs.m_uiOneDiskSize=(UINT)pConfig->get_signed_num(PP_BFONEDISK);
			bs.m_uiTwoDisksSize=(UINT)pConfig->get_signed_num(PP_BFTWODISKS);
			bs.m_uiCDSize=(UINT)pConfig->get_signed_num(PP_BFCD);
			bs.m_uiLANSize=(UINT)pConfig->get_signed_num(PP_BFLAN);

			pTask->SetBufferSizes(&bs);
			pTask->SetPriority((int)pConfig->get_signed_num(PP_CMDEFAULTPRIORITY));

			// get dest folder
			CFolderDialog dlg;

			const tchar_t* pszPath = NULL;
			dlg.m_bdData.cvShortcuts.clear(true);
			size_t stCount = pConfig->get_value_count(PP_SHORTCUTS);
			for(size_t stIndex = 0; stIndex < stCount; stIndex++)
			{
				pszPath = pConfig->get_string(PP_SHORTCUTS, stIndex);
				dlg.m_bdData.cvShortcuts.push_back(pszPath);
			}

			dlg.m_bdData.cvRecent.clear(true);
			stCount = pConfig->get_value_count(PP_RECENTPATHS);
			for(size_t stIndex = 0; stIndex < stCount; stIndex++)
			{
				pszPath = pConfig->get_string(PP_RECENTPATHS, stIndex);
					dlg.m_bdData.cvRecent.push_back(pszPath);
			}

			dlg.m_bdData.bExtended=pConfig->get_bool(PP_FDEXTENDEDVIEW);
			dlg.m_bdData.cx=(int)pConfig->get_signed_num(PP_FDWIDTH);
			dlg.m_bdData.cy=(int)pConfig->get_signed_num(PP_FDHEIGHT);
			dlg.m_bdData.iView=(int)pConfig->get_signed_num(PP_FDSHORTCUTLISTSTYLE);
			dlg.m_bdData.bIgnoreDialogs=pConfig->get_bool(PP_FDIGNORESHELLDIALOGS);

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
			pConfig->clear_array_values(PP_SHORTCUTS);
			for(char_vector::iterator it = dlg.m_bdData.cvShortcuts.begin(); it != dlg.m_bdData.cvShortcuts.end(); it++)
			{
				pConfig->set_string(PP_SHORTCUTS, (*it), icpf::property::action_add);
			}

			pConfig->clear_array_values(PP_RECENTPATHS);
			for(char_vector::iterator it = dlg.m_bdData.cvRecent.begin(); it != dlg.m_bdData.cvRecent.end(); it++)
			{
				pConfig->set_string(PP_RECENTPATHS, (*it), icpf::property::action_add);
			}

			pConfig->set_bool(PP_FDEXTENDEDVIEW, dlg.m_bdData.bExtended);
			pConfig->set_signed_num(PP_FDWIDTH, dlg.m_bdData.cx);
			pConfig->set_signed_num(PP_FDHEIGHT, dlg.m_bdData.cy);
			pConfig->set_signed_num(PP_FDSHORTCUTLISTSTYLE, dlg.m_bdData.iView);
			pConfig->set_bool(PP_FDIGNORESHELLDIALOGS, dlg.m_bdData.bIgnoreDialogs);
			pConfig->write(NULL);

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

				// add task to a list of tasks and start
				pData->m_pTasks->Add(pTask);

				// write pTask to a file
				pTask->Store(true);
				pTask->Store(false);
				
				// start processing
				pTask->BeginProcessing();
			}
		}
		
		// do we need to check for turning computer off
		if (GetConfig()->get_bool(PP_PSHUTDOWNAFTREFINISHED))
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
					if (GetConfig()->get_signed_num(PP_PTIMEBEFORESHUTDOWN) != 0)
					{
						CShutdownDlg dlg;
						dlg.m_iOverallTime=(int)GetConfig()->get_signed_num(PP_PTIMEBEFORESHUTDOWN);
						if (dlg.m_iOverallTime < 0)
							dlg.m_iOverallTime=-dlg.m_iOverallTime;
						bShutdown=(dlg.DoModal() != IDCANCEL);
					}
					
					GetConfig()->set_bool(PP_PSHUTDOWNAFTREFINISHED, false);
					GetConfig()->write(NULL);
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
						
						BOOL bExit=ExitWindowsEx(EWX_POWEROFF | EWX_SHUTDOWN | (GetConfig()->get_bool(PP_PFORCESHUTDOWN) ? EWX_FORCE : 0), 0);
						if (bExit)
							return 1;
						else
						{
							pData->bKilled=false;
							
							// some kind of error
							ictranslate::CFormat fmt(GetResManager()->LoadString(IDS_SHUTDOWNERROR_STRING));
							fmt.SetParam(_t("%errno"), GetLastError());
							AfxMessageBox(fmt, MB_ICONERROR | MB_OK | MB_SYSTEMMODAL);
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
		if (uiCounter >= (UINT)GetConfig()->get_signed_num(PP_PMONITORSCANINTERVAL))
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
	m_tasks.Create(m_pFeedbackFactory, &ThrdProc);

	// load last state
	CString strPath;
	GetApp()->GetProgramDataPath(strPath);
	strPath += _T("\\tasks");
	m_tasks.SetTasksDir(strPath);
	m_tasks.LoadDataProgress();
	m_tasks.TasksRetryProcessing();

	// start clipboard monitoring
	cmd.bKill=false;
	cmd.bKilled=false;
	cmd.m_hwnd=this->m_hWnd;
	cmd.m_pTasks=&m_tasks;

	AfxBeginThread(&ClipboardMonitorProc, static_cast<LPVOID>(&cmd), THREAD_PRIORITY_IDLE);
	
	// start saving timer
	SetTimer(1023, (UINT)GetConfig()->get_signed_num(PP_PAUTOSAVEINTERVAL), NULL);

	SetTimer(7834, TM_AUTORESUME/*GetConfig()->GetAutoRetryInterval()*/, NULL);
	SetTimer(3245, TM_AUTOREMOVE, NULL);
	SetTimer(8743, TM_ACCEPTING, NULL);		// ends wait state in tasks

	if (GetConfig()->get_bool(PP_MVAUTOSHOWWHENRUN))
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

			pSubMenu->CheckMenuItem(ID_POPUP_MONITORING, MF_BYCOMMAND | (GetConfig()->get_bool(PP_PCLIPBOARDMONITORING) ? MF_CHECKED : MF_UNCHECKED));
			pSubMenu->CheckMenuItem(ID_POPUP_SHUTAFTERFINISHED, MF_BYCOMMAND | (GetConfig()->get_bool(PP_PSHUTDOWNAFTREFINISHED) ? MF_CHECKED : MF_UNCHECKED));

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
				_sntprintf(text, _MAX_PATH, _T("%s - %d %%"), GetApp()->GetAppName(), m_tasks.GetPercent());
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

void CMainWnd::OnTimer(UINT_PTR nIDEvent) 
{
	switch (nIDEvent)
	{
	case 1023:
		// autosave timer
		KillTimer(1023);
		m_tasks.SaveProgress();
		SetTimer(1023, (UINT)GetConfig()->get_signed_num(PP_PAUTOSAVEINTERVAL), NULL);
		break;
	case 7834:
		{
			// auto-resume timer
			KillTimer(7834);
			DWORD dwTime=GetTickCount();
			DWORD dwInterval=(m_dwLastTime == 0) ? TM_AUTORESUME : dwTime-m_dwLastTime;
			m_dwLastTime=dwTime;
			
			if (GetConfig()->get_bool(PP_CMAUTORETRYONERROR))
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
		if (GetConfig()->get_bool(PP_STATUSAUTOREMOVEFINISHED))
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
			if (GetConfig()->get_signed_num(PP_CMLIMITMAXOPERATIONS) == 0 || m_tasks.GetOperationsPending() < (UINT)GetConfig()->get_signed_num(PP_CMLIMITMAXOPERATIONS))
			{
				for (int i=0;i<m_tasks.GetSize();i++)
				{
					pTask=m_tasks.GetAt(i);
					// turn on some thread - find something with wait state
					if (pTask->GetStatus(ST_WAITING_MASK) & ST_WAITING && (GetConfig()->get_signed_num(PP_CMLIMITMAXOPERATIONS) == 0 || m_tasks.GetOperationsPending() < (UINT)GetConfig()->get_signed_num(PP_CMLIMITMAXOPERATIONS)))
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
	if(!GetApp()->IsShellExtEnabled())
		return FALSE;

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
	unsigned long ulLen=pCopyDataStruct->cbData / sizeof(TCHAR);

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

	icpf::config* pConfig = GetConfig();
	assert(pConfig);

	// special operation - modify stuff
	CFiltersArray ffFilters;
	int iPriority=(int)GetConfig()->get_signed_num(PP_CMDEFAULTPRIORITY);
	BUFFERSIZES bsSizes;
	bsSizes.m_bOnlyDefault=GetConfig()->get_bool(PP_BFUSEONLYDEFAULT);
	bsSizes.m_uiDefaultSize=(UINT)GetConfig()->get_signed_num(PP_BFDEFAULT);
	bsSizes.m_uiOneDiskSize=(UINT)GetConfig()->get_signed_num(PP_BFONEDISK);
	bsSizes.m_uiTwoDisksSize=(UINT)GetConfig()->get_signed_num(PP_BFTWODISKS);
	bsSizes.m_uiCDSize=(UINT)GetConfig()->get_signed_num(PP_BFCD);
	bsSizes.m_uiLANSize=(UINT)GetConfig()->get_signed_num(PP_BFLAN);

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

		dlg.m_ccData.m_vRecent.clear(true);
		const tchar_t* pszPath = NULL;
		size_t stCount = pConfig->get_value_count(PP_RECENTPATHS);
		for(size_t stIndex = 0; stIndex < stCount; stIndex++)
		{
			pszPath = pConfig->get_string(PP_RECENTPATHS, stIndex);
			if(pszPath)
				dlg.m_ccData.m_vRecent.push_back(pszPath);
		}

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
		ffFilters = dlg.m_ccData.m_afFilters;
		bIgnoreDirs=dlg.m_ccData.m_bIgnoreFolders;
		bForceDirectories=dlg.m_ccData.m_bForceDirectories;
		bOnlyCreate=dlg.m_ccData.m_bCreateStructure;
		ucCopies=dlg.m_ccData.m_ucCount;
		dlg.m_ccData.m_vRecent.insert(dlg.m_ccData.m_vRecent.begin(), (const PTSTR)(LPCTSTR)strDstPath, true);

		pConfig->clear_array_values(PP_RECENTPATHS);
		for(char_vector::iterator it = dlg.m_ccData.m_vRecent.begin(); it != dlg.m_ccData.m_vRecent.end(); it++)
		{
			pConfig->set_string(PP_RECENTPATHS, (*it), icpf::property::action_add);
		}
	}

	// create new task
	CTask *pTask = m_tasks.CreateTask();
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

	m_tasks.Add(pTask);

	// save state of a task
	pTask->Store(true);
	pTask->Store(false);

	// add to task list and start processing
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
	icpf::config* pConfig = GetConfig();
	assert(pConfig);
	if(!pConfig)
		return;

	CCustomCopyDlg dlg;
	dlg.m_ccData.m_iOperation=0;
	dlg.m_ccData.m_iPriority=(int)pConfig->get_signed_num(PP_CMDEFAULTPRIORITY);
	dlg.m_ccData.m_bsSizes.m_bOnlyDefault=pConfig->get_bool(PP_BFUSEONLYDEFAULT);
	dlg.m_ccData.m_bsSizes.m_uiDefaultSize=(UINT)pConfig->get_signed_num(PP_BFDEFAULT);
	dlg.m_ccData.m_bsSizes.m_uiOneDiskSize=(UINT)pConfig->get_signed_num(PP_BFONEDISK);
	dlg.m_ccData.m_bsSizes.m_uiTwoDisksSize=(UINT)pConfig->get_signed_num(PP_BFTWODISKS);
	dlg.m_ccData.m_bsSizes.m_uiCDSize=(UINT)pConfig->get_signed_num(PP_BFCD);
	dlg.m_ccData.m_bsSizes.m_uiLANSize=(UINT)pConfig->get_signed_num(PP_BFLAN);

	dlg.m_ccData.m_bCreateStructure=false;
	dlg.m_ccData.m_bForceDirectories=false;
	dlg.m_ccData.m_bIgnoreFolders=false;
	dlg.m_ccData.m_ucCount=1;

	dlg.m_ccData.m_vRecent.clear(true);
	const tchar_t* pszPath = NULL;
	size_t stCount = pConfig->get_value_count(PP_RECENTPATHS);
	for(size_t stIndex = 0; stIndex < stCount; stIndex++)
	{
		pszPath = pConfig->get_string(PP_RECENTPATHS, stIndex);
		if(pszPath)
			dlg.m_ccData.m_vRecent.push_back(pszPath);
	}

	if (dlg.DoModal() == IDOK)
	{
		// save recent paths
		dlg.m_ccData.m_vRecent.push_back((PCTSTR)dlg.m_ccData.m_strDestPath);

		pConfig->clear_array_values(PP_RECENTPATHS);
		for(char_vector::iterator it = dlg.m_ccData.m_vRecent.begin(); it != dlg.m_ccData.m_vRecent.end(); it++)
		{
			pConfig->set_string(PP_RECENTPATHS, (*it), icpf::property::action_add);
		}

		// new task
		CTask *pTask = m_tasks.CreateTask();
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
		
		m_tasks.Add(pTask);

		// save
		pTask->Store(true);
		pTask->Store(false);
		
		// store and start
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
			GetApp()->SetAutorun(GetConfig()->get_bool(PP_PRELOADAFTERRESTART));

			// set this process class
			HANDLE hProcess=GetCurrentProcess();
			::SetPriorityClass(hProcess, (DWORD)GetConfig()->get_signed_num(PP_PPROCESSPRIORITYCLASS));

			break;
		}

	case WM_GETCONFIG:
		{
			icpf::config* pConfig = GetConfig();
			assert(pConfig);

			// std config values
			g_pscsShared->bShowFreeSpace=pConfig->get_bool(PP_SHSHOWFREESPACE);
			
			// experimental - doesn't work on all systems 
			g_pscsShared->bShowShortcutIcons=pConfig->get_bool(PP_SHSHOWSHELLICONS);
			g_pscsShared->bOverrideDefault=pConfig->get_bool(PP_SHUSEDRAGDROP);	// only for d&d
			g_pscsShared->uiDefaultAction=(UINT)pConfig->get_signed_num(PP_SHDEFAULTACTION);
			
			// sizes
			for (int i=0;i<6;i++)
				_tcscpy(g_pscsShared->szSizes[i], GetResManager()->LoadString(IDS_BYTE_STRING+i));

			// convert to list of _COMMAND's
			_COMMAND *pCommand = g_pscsShared->GetCommandsPtr();

			// what kind of menu ?
			switch (wParam)
			{
			case GC_DRAGDROP:
				{
					g_pscsShared->iCommandCount=3;
					g_pscsShared->iShortcutsCount=0;
					g_pscsShared->uiFlags=(pConfig->get_bool(PP_SHSHOWCOPY) ? DD_COPY_FLAG : 0)
						| (pConfig->get_bool(PP_SHSHOWMOVE) ? DD_MOVE_FLAG : 0)
						| (pConfig->get_bool(PP_SHSHOWCOPYMOVE) ? DD_COPYMOVESPECIAL_FLAG : 0);

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
					g_pscsShared->uiFlags=(pConfig->get_bool(PP_SHSHOWPASTE) ? EC_PASTE_FLAG : 0)
						| (pConfig->get_bool(PP_SHSHOWPASTESPECIAL) ? EC_PASTESPECIAL_FLAG : 0)
						| (pConfig->get_bool(PP_SHSHOWCOPYTO) ? EC_COPYTO_FLAG : 0)
						| (pConfig->get_bool(PP_SHSHOWMOVETO) ? EC_MOVETO_FLAG : 0)
						| (pConfig->get_bool(PP_SHSHOWCOPYMOVETO) ? EC_COPYMOVETOSPECIAL_FLAG : 0);
					
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
					const tchar_t* pszPath = NULL;
					size_t stCount = pConfig->get_value_count(PP_SHORTCUTS);
					for(size_t stIndex = 0; stIndex < stCount; stIndex++)
					{
						pszPath = pConfig->get_string(PP_SHORTCUTS, stIndex);
						if(pszPath)
							cvShortcuts.push_back(pszPath);
					}
					
					// count of shortcuts to store
					g_pscsShared->iShortcutsCount=__min(cvShortcuts.size(), (SHARED_BUFFERSIZE - 5 * sizeof(_COMMAND)) / sizeof(_SHORTCUT));
					_SHORTCUT* pShortcut = g_pscsShared->GetShortcutsPtr();
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

			CA2T ca2t(reinterpret_cast<char*>(dec));
			AfxMessageBox(ca2t);
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
	GetConfig()->set_bool(PP_PCLIPBOARDMONITORING, !GetConfig()->get_bool(PP_PCLIPBOARDMONITORING));
	GetConfig()->write(NULL);
}

void CMainWnd::OnPopupShutafterfinished() 
{
	GetConfig()->set_bool(PP_PSHUTDOWNAFTREFINISHED, !GetConfig()->get_bool(PP_PSHUTDOWNAFTREFINISHED));	
	GetConfig()->write(NULL);
}

void CMainWnd::OnPopupRegisterdll() 
{
	CString strPath;
	CCopyHandlerApp* pApp = GetApp();
	if(pApp)
	{
		strPath = pApp->GetProgramPath();
		strPath += _T("\\");
	}

#ifdef _WIN64
	strPath += _T("chext64.dll");
#else
	strPath += _T("chext.dll");
#endif
	HRESULT hResult = RegisterShellExtDll(strPath, true);
	if(FAILED(hResult))
	{
		TCHAR szStr[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hResult, 0, szStr, 256, NULL);
		while (szStr[_tcslen(szStr)-1] == _T('\n') || szStr[_tcslen(szStr)-1] == _T('\r') || szStr[_tcslen(szStr)-1] == _T('.'))
			szStr[_tcslen(szStr)-1]=_T('\0');

		ictranslate::CFormat fmt(GetResManager()->LoadString(IDS_REGISTERERR_STRING));
		fmt.SetParam(_T("%errno"), (ulong_t)hResult);
		fmt.SetParam(_T("%errdesc"), szStr);
		AfxMessageBox(fmt, MB_ICONERROR | MB_OK);
	}
	else if(hResult == S_OK)
		MsgBox(IDS_REGISTEROK_STRING, MB_ICONINFORMATION | MB_OK);
}

void CMainWnd::OnPopupUnregisterdll() 
{
	CString strPath;
	CCopyHandlerApp* pApp = GetApp();
	if(pApp)
	{
		strPath = pApp->GetProgramPath();
		strPath += _T("\\");
	}

#ifdef _WIN64
	strPath += _T("chext64.dll");
#else
	strPath += _T("chext.dll");
#endif

	HRESULT hResult = RegisterShellExtDll(strPath, false);
	if(FAILED(hResult))
	{
		TCHAR szStr[256];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hResult, 0, szStr, 256, NULL);
		while (szStr[_tcslen(szStr)-1] == _T('\n') || szStr[_tcslen(szStr)-1] == _T('\r') || szStr[_tcslen(szStr)-1] == _T('.'))
			szStr[_tcslen(szStr)-1]=_T('\0');

		ictranslate::CFormat fmt(GetResManager()->LoadString(IDS_UNREGISTERERR_STRING));
		fmt.SetParam(_T("%errno"), (ulong_t)hResult);
		fmt.SetParam(_T("%errdesc"), szStr);

		AfxMessageBox(fmt, MB_ICONERROR | MB_OK);
	}
	else if(hResult == S_OK)
		MsgBox(IDS_UNREGISTEROK_STRING, MB_ICONINFORMATION | MB_OK);
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
	m_tasks.SaveProgress();

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
	GetApp()->HtmlHelp(HH_DISPLAY_TOPIC, NULL);
}

void CMainWnd::OnPopupCheckForUpdates()
{
	CUpdaterDlg* pDlg = new CUpdaterDlg;
	pDlg->m_bAutoDelete = true;
	
	pDlg->Create();
}
