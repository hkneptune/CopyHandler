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
/*************************************************************************
	File: LogFile.h
	Version: 1.0
	Author: Ixen Gerthannes (ixen@interia.pl)
	File description:
		Contain class that handle logging text to file.
	Classes:
		CLogFile (based on CFile - not MFC one).
			- provides functionality of log file
*************************************************************************/
#ifndef __LOGFILE_H__
#define __LOGFILE_H__

#include "FileEx.h"
#include "ResourceManager.h"
#include "charvect.h"

typedef void(*PFNLOGCALLBACK)(PVOID, PCTSTR);

// struct that remembers the callback function and associated parameter
struct LOGCALLBACKENTRY
{
	PFNLOGCALLBACK pfn;
	PVOID pParam;
};

class CLogFile : public CFileEx
{
public:
	CLogFile();
	~CLogFile() { DeleteCriticalSection(&m_cs); };

	void Init(LPCTSTR pszPath, CResourceManager* pManager, bool bEnable=true);		// initializes this Log File

	// cfg functions
	void EnableLogging(bool bEnable=true);				// enables/disables logging
	bool IsLoggingEnabled();							// determines if logging is enabled
	void SetSizeLimit(bool bEnable, DWORD dwSizeLimit=64UL*1024UL);	// enables/disables size limiting for this log file
	bool GetSizeLimit(DWORD* pdwSizeLimit=NULL);
	void SetPreciseLimiting(bool bEnable);
	bool GetPreciseLimiting();
	void SetTruncateBufferSize(DWORD uiSize);
	DWORD GetTruncateBufferSize();

	// callback handling
	void RegisterCallback(PFNLOGCALLBACK pfn, PVOID pParam);
	void UnregisterCallback(PFNLOGCALLBACK pfn);

    // standard log functions
	bool Log(UINT uiStrID, ...);
	bool LogError(UINT uiStrID, DWORD dwError, ...);
	bool Log(LPCTSTR pszText, ...);
	bool LogV(LPCTSTR pszText, va_list arglist);

protected:
	void LimitSize(DWORD dwNewSize);		// active limitation in file

protected:
	TCHAR m_szFilename[_MAX_PATH];

	// cfg section
	bool m_bEnabled;				// is this log file enabled ?
	bool m_bSizeLimit;				// if size limit is needed
	DWORD m_dwSizeLimit;			// size limit
	bool m_bPreciseLimiting;		// increases precision of size limiting (slowly)
	DWORD m_dwTruncateBufferSize;	// buffer size used for truncating file

	// callback handling - calls each function when a new string is about to be saved in log
	vector<LOGCALLBACKENTRY> m_vCallbacks;

	// internal text buffer
	TCHAR m_szText[1024];		// max line
	TCHAR m_szBuffer[1024];		// for using with load from resource

	// res manager
	CResourceManager *m_pResManager;

	// protection
	CRITICAL_SECTION m_cs;
};

#endif