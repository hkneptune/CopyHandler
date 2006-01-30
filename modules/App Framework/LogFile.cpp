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

#include "stdafx.h"
#include "LogFile.h"
#include "stdio.h"

#pragma warning( disable : 4127 )

///////////////////////////////////////////////////////////////
// Constructs this log file (sets members to initial state)
///////////////////////////////////////////////////////////////
CLogFile::CLogFile() : CFileEx()
{
	// nullify filename, set std data
	m_szFilename[0]=_T('\0');
	m_bSizeLimit=true;
	m_dwSizeLimit=64UL*1024UL;
	m_bPreciseLimiting=false;
	m_dwTruncateBufferSize=64UL*1024UL;
	m_bEnabled=true;

	InitializeCriticalSection(&m_cs);
}

///////////////////////////////////////////////////////////////
// Initializes this log file. Remembers pathname and sets
// enabled/disabled state.
///////////////////////////////////////////////////////////////
void CLogFile::Init(LPCTSTR pszPath, CResourceManager* pManager, bool bEnable)
{
	// set filename
	_tcscpy(m_szFilename, pszPath);
	m_bEnabled=bEnable;
	m_pResManager=pManager;
}

///////////////////////////////////////////////////////////////
// Allows to set maximum size of this log file. When adding
// next line of text to this file and when this option is
// enabled - the file will be truncated (truncation of data
// that lies at the beginning of file) before text is added.
///////////////////////////////////////////////////////////////
void CLogFile::SetSizeLimit(bool bEnable, DWORD dwSizeLimit)
{
	EnterCriticalSection(&m_cs);
	m_bSizeLimit=bEnable;
	m_dwSizeLimit=dwSizeLimit;
	LeaveCriticalSection(&m_cs);
}

///////////////////////////////////////////////////////////////
// Retrieves size limit and enabled limit status of this file.
// Look @ SetSizeLimit.
///////////////////////////////////////////////////////////////
bool CLogFile::GetSizeLimit(DWORD* pdwSizeLimit)
{
	EnterCriticalSection(&m_cs);
	bool bEnabled=m_bSizeLimit;
	if (pdwSizeLimit)
		*pdwSizeLimit=m_dwSizeLimit;
	LeaveCriticalSection(&m_cs);

	return bEnabled;
}

///////////////////////////////////////////////////////////////
// Allows to enable/disable precise size limiting. Normally 
// when file needs truncation (look @SetSizeLimit) this class
// truncates about 1/3 of the file. When this option is enabled
// truncation will take much less data (but this will cause
// that truncation will be needed more frequently)
///////////////////////////////////////////////////////////////
void CLogFile::SetPreciseLimiting(bool bEnable)
{
	EnterCriticalSection(&m_cs);
	m_bPreciseLimiting=bEnable;
	LeaveCriticalSection(&m_cs);
}

///////////////////////////////////////////////////////////////
// Returns state of precise limiting (look@SetPreciseLimiting)
///////////////////////////////////////////////////////////////
bool CLogFile::GetPreciseLimiting()
{
	return m_bPreciseLimiting;
}

///////////////////////////////////////////////////////////////
// Sets size of the buffer that will be used when truncating
// this file. Truncation is done by copying data from end of
// a file to the beginning and truncating a file. This buffer
// size specifies amount of data that will be read/write at
// once.
///////////////////////////////////////////////////////////////
void CLogFile::SetTruncateBufferSize(DWORD dwSize)
{
	EnterCriticalSection(&m_cs);
	m_dwTruncateBufferSize=dwSize;
	LeaveCriticalSection(&m_cs);
}

///////////////////////////////////////////////////////////////
// Retrieve current size of the truncation buffer. (see
// SetTruncateBufferSize).
// Ret Value [out] - size of the truncation buffer
///////////////////////////////////////////////////////////////
DWORD CLogFile::GetTruncateBufferSize()
{
	return m_dwTruncateBufferSize;
}

///////////////////////////////////////////////////////////////
// Enables (or disables) log functionality of this class.
// Disabling logging allows to "transparently" use of this
// class when we do not want to log any data.
// bEnable [in] - enables logging (true) or disables (false)
///////////////////////////////////////////////////////////////
void CLogFile::EnableLogging(bool bEnable)
{
	EnterCriticalSection(&m_cs);
	m_bEnabled=bEnable;
	LeaveCriticalSection(&m_cs);
}

///////////////////////////////////////////////////////////////
// Returns current state of the logging (see EnableLogging)
// Ret Value [out] - if logging is enabled (true)
///////////////////////////////////////////////////////////////
bool CLogFile::IsLoggingEnabled()
{
	return m_bEnabled;
}

///////////////////////////////////////////////////////////////
// Logs string with a given va_list.
// pszText [in] - text to log (it'll be expanded with data from
//				va_list.
// arglist [in] - contains data that'll be used to expand
//				pszText
// Ret Value [out] - if the text has been successfully written
///////////////////////////////////////////////////////////////
bool CLogFile::LogV(LPCTSTR pszText, va_list arglist)
{
	// logging disabled - so we succeeded in processing
	if (!m_bEnabled)
		return true;

	EnterCriticalSection(&m_cs);
    
	try
	{
		// open the file
		Open(m_szFilename, FA_APPEND);

		// process string
		int iSize=_vstprintf(m_szText, pszText, arglist);
		va_end(arglist);

		// trace it
		TRACE("[Log file] %s\n", m_szText);

		// call the callbacks
		for (vector<LOGCALLBACKENTRY>::iterator it=m_vCallbacks.begin();it != m_vCallbacks.end();it++)
		{
			(*it->pfn)(it->pParam, m_szText);
		}
		
		// check if log file isn't too large
		if (m_bSizeLimit)
		{
			LONGLONG llCurrentSize=GetSize();

			if (llCurrentSize+iSize+2 > m_dwSizeLimit)	// limit check
			{
				// need to make this file shorter
				// find end of line at about 1/3 file size
				LONGLONG llNewSize=llCurrentSize-llCurrentSize/3;
				if (m_bPreciseLimiting || m_dwSizeLimit-llNewSize < iSize+2)
				{
					// more place needed
					llNewSize=(LONGLONG)m_dwSizeLimit-(iSize+2);
					if (llNewSize < 0)
						llNewSize=0;
				}

				// truncate this file
				LimitSize((DWORD)llNewSize);
			}
		}

		// now write data to file - append "\r\n"
		WriteLine(m_szText);

		// Close
		Close();
	}
	catch (CFileException* e)
	{
		LeaveCriticalSection(&m_cs);
		delete e;
		return false;
	}

	LeaveCriticalSection(&m_cs);
	return true;
}

///////////////////////////////////////////////////////////////
// Logs string. Ellipsis param.
// pszText [in] - text to log (it'll be expanded with data from
//				va_list.
// ... [in] - data that'll be used to expand pszText
// Ret Value [out] - if the text has been successfully written
///////////////////////////////////////////////////////////////
bool CLogFile::Log(LPCTSTR pszText, ...)
{
	if (!m_bEnabled)
		return true;

	va_list marker;
	va_start(marker, pszText);
	return LogV(pszText, marker);
}

bool CLogFile::Log(UINT uiStrID, ...)
{
	if (!m_bEnabled)
		return true;

	// load string
	_ASSERT(m_pResManager);

	va_list marker;
	va_start(marker, uiStrID);
	return LogV(m_pResManager->LoadString(uiStrID), marker);
}

bool CLogFile::LogError(UINT uiStrID, DWORD dwError, ...)
{
	if (!m_bEnabled)
		return true;

	TCHAR szBuffer[_MAX_PATH];
	const TCHAR* pszText=m_pResManager->LoadString(uiStrID);

	// find the first %lu in the string
	TCHAR* pszPos=_tcsstr(pszText, _T("%lu"));
	if (pszPos == NULL)
		return false;
	_tcsncpy(m_szBuffer, pszText, pszPos-pszText);
	m_szBuffer[pszPos-pszText]=_T('\0');

	// append error number
	_itot(dwError, szBuffer, 10);
	_tcscat(m_szBuffer, szBuffer);

	// format error
	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwError, 0, szBuffer, _MAX_PATH, NULL);

	while (szBuffer[_tcslen(szBuffer)-1] == _T('\n') || szBuffer[_tcslen(szBuffer)-1] == _T('\r'))
		szBuffer[_tcslen(szBuffer)-1] = _T('\0');

	// now find the first %s in a string
	pszPos+=3;
	TCHAR *pszPos2=_tcsstr(pszPos, _T("%s"));
	int iSize=(int)_tcslen(m_szBuffer);
	_tcsncpy(m_szBuffer+iSize, pszPos, pszPos2-pszPos);
	m_szBuffer[iSize+pszPos2-pszPos]=_T('\0');

	// paste the string (error desc)
	_tcscat(m_szBuffer, szBuffer);

	// copy rest of the string
	_tcscat(m_szBuffer, pszPos2+2);

	va_list marker;
	va_start(marker, dwError);
	return LogV(m_szBuffer, marker);
}

void CLogFile::RegisterCallback(PFNLOGCALLBACK pfn, PVOID pParam)
{
	LOGCALLBACKENTRY lce;
	lce.pfn=pfn;
	lce.pParam=pParam;
	m_vCallbacks.push_back(lce);
}

void CLogFile::UnregisterCallback(PFNLOGCALLBACK pfn)
{
	// find the callback with the given address and remove it
	for (vector<LOGCALLBACKENTRY>::iterator it=m_vCallbacks.begin();it != m_vCallbacks.end();it++)
	{
		if (it->pfn == pfn)
		{
			m_vCallbacks.erase(it);
			return;
		}
	}
	TRACE("Warning: Unregister callback function in CLogFile::UnregisterCallback failed. No function found.\n");
}

///////////////////////////////////////////////////////////////
// (Internal) Limits size of the file. Causes to find the
// nearest eol sign in file, and move all further data to
// beginning of the file.
// dwNewSize [in] - specifies new maximum file size needed
///////////////////////////////////////////////////////////////
void CLogFile::LimitSize(DWORD dwNewSize)
{
	BYTE* pszBuffer=NULL;
	try
	{
		// seek to end of file-dwNewSize
		Seek(-(LONG)dwNewSize, FILE_END);

		// establish end of line after that position
		pszBuffer=new BYTE[m_dwTruncateBufferSize];		// buffer for other data
	
		ReadLine((TCHAR*)pszBuffer, 1);		// read line to the end - doesn't matter what it is
		Flush();							// flush buffer (if used) - sets file pointer to proper pos.

		// now move data from that position to the beginning
		DWORD dwDst=0, dwSrc=(DWORD)GetPos();

		// buffer for data
		DWORD dwRD, dwWR;

		// switch file to unbuffered mode (faster)
		SwitchToUnbuffered();

		// move
		while(true)
		{
			// seek to src
			Seek(dwSrc, FILE_BEGIN);

			// read data
			if ((dwRD=ReadBuffer(pszBuffer, m_dwTruncateBufferSize)) == 0)
			{
				Seek(dwDst, FILE_BEGIN);
				SetEndOfFile();		// leaves file ptr at the end of file
				break;
			}
			else
			{
				Seek(dwDst, FILE_BEGIN);
				dwWR=WriteBuffer(pszBuffer, dwRD);

				// update pos
				dwDst+=dwWR;
				dwSrc+=dwRD;
			}
		}
	}
	catch(...)
	{
		// something happened (shouldn't)
		// restore file state
		RestoreState();
		delete [] pszBuffer;

		throw;
	}

	// free buffer
	RestoreState();
	delete [] pszBuffer;
}
