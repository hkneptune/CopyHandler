/***************************************************************************
 *   Copyright (C) 2004 by Józef Starosczyk                                *
 *   copyhandler@o2.pl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
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
/** \file log.cpp
 *  \brief Contains the implamentation of a log class.
 */
#include "log.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <assert.h>
#include "macros.h"

#ifdef WIN32
	#include <stdlib.h>
	#include <fcntl.h>
	#include <windows.h>
#else	
	#include <unistd.h>
#endif

#ifdef WIN32
	#undef vsnprintf
	#define vsnprintf _vsnprintf
	#undef snprintf
	#define snprintf _snprintf
#endif

BEGIN_ICPF_NAMESPACE

/// Table of strings representing the log message types
const char_t* __logtype_str[] = { "debug", "info", "warning", "error" };

/// Global variable initialized when constructing log_file object
log_file* __g_log=NULL;

/** Constructs a log_file object.
 * \param[in] bGlobal - states if this should be treates as a global instance of the log_file.
 *						Only one global log_file instance could exist in the application.
 */
log_file::log_file(bool bGlobal) : 
	m_bGlobal(bGlobal),
	m_pszPath(NULL),
	m_bLogStd(false),
	m_iLogLevel(LT_DEBUG)
{
	if (m_bGlobal)
	{
		assert(__g_log == NULL);		// there is another instance of a global log running
		__g_log=this;
	}
#ifdef WIN32
	_fmode=_O_BINARY;
#endif
}

/** Standard destructor
 */
log_file::~log_file()
{
	if (m_bGlobal)
		__g_log=NULL;
	delete [] m_pszPath;
}

/** Creates a global instance of a log file.
 * \param[in] pszPath - path to a log file to write to
 * \param[in] iMaxSize - maximum size of a log file
 * \param[in] iLogLevel - minimum log level of the messages to log
 * \param[in] bLogStd - log the messages also to stdout/stderr
 * \param[in] bClean - cleans the log file upon opening
 * \return True if the log file has been successfully initialized or false if not.
 */
bool create_log(const char_t* pszPath, int_t iMaxSize, int_t iLogLevel, bool bLogStd, bool bClean)
{
	log_file* pLog=new log_file(true);
	if (!pLog->init(pszPath, iMaxSize, iLogLevel, bLogStd, bClean))
	{
		delete pLog;
		return false;
	}
	
	return true;
}

/** Initializes the constructed log file.
 * \param[in] pszPath - path to a log file to write to
 * \param[in] iMaxSize - maximum size of a log file
 * \param[in] iLogLevel - minimum log level of the messages to log
 * \param[in] bLogStd - log the messages also to stdout/stderr
 * \param[in] bClean - cleans the log file upon opening
 * \return True if the log file has been successfully initialized or false if not.
 */
bool log_file::init(const char_t* pszPath, int_t iMaxSize, int_t iLogLevel, bool bLogStd, bool bClean)
{
	delete [] m_pszPath;
	m_pszPath=new char_t[strlen(pszPath)+1];
	strcpy(m_pszPath, pszPath);
	
	m_iMaxSize=iMaxSize;
	m_bLogStd=bLogStd;
	m_iLogLevel=iLogLevel;
	
	FILE* pFile=fopen(pszPath, bClean ? "w" : "a");
	if (pFile == NULL)
		return false;
	
	fclose(pFile);
	return true;
}

/** Retrieves the current size of a log file.
 *  Quite slow function - have to access the file by opening and closing it.
 * \return Current file size.
 */
int_t log_file::size()
{
	assert(m_pszPath);
	
	int_t iSize=-1;
	FILE* pFile=fopen(m_pszPath, "r");
	if (pFile != NULL)
	{
		if (fseek(pFile, 0, SEEK_END) == 0)
			iSize=ftell(pFile);
	}
	
	fclose(pFile);
	
	return iSize;
}

// @lAdd - count of bytes that would be appended to the file
/** Truncates the current log file content so when adding some new text the
 *  file size won't exceed the maximum size specified in init().
 * \param[in] iAdd - size of the new string to be added to the log file
 * \return True if truncate succeeded or false if not.
 */
bool log_file::truncate(int_t iAdd)
{
	assert(m_pszPath);
	
	// if we doesn't need to truncate anything
	if (m_iMaxSize <= 0)
		return true;
	
	// make some checks
	int_t iSize=size();
	if (iSize <= 0 || iSize+iAdd < m_iMaxSize)
		return false;
	
	// establish the new file size (1/3rd of the current size or max_size-add_size)
	int_t iNewSize=minval((int_t)(iSize*0.66), m_iMaxSize-iAdd);
	
#ifdef _WIN32
	// win32 does not have the ftruncate function, so we have to make some API calls
	HANDLE hFile=CreateFile(m_pszPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		// seek
		if (SetFilePointer(hFile, iSize-iNewSize, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
		{
			// read the string to the eol
			DWORD dwRD;
			char_t szBuffer[4096];
			if (ReadFile(hFile, szBuffer, 4096, &dwRD, NULL))
			{
				szBuffer[(dwRD > 0) ? dwRD-1 : 0]='\0';

				// replace the /r and /n in the log to the \0
				for (DWORD i=0;i<dwRD;i++)
				{
					if (szBuffer[i] == '\r' || szBuffer[i] == '\n')
					{
						szBuffer[i]='\0';
						break;
					}
				}

				iNewSize-=(int_t)strlen(szBuffer)+1;			// new size correction

				if (SetFilePointer(hFile, iSize-iNewSize, NULL, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
				{
					int_t iSrc=SetFilePointer(hFile, 0, NULL, FILE_CURRENT);
					int_t iDst=0;
					DWORD tRD, tWR;

					do
					{
						// seek to src
						SetFilePointer(hFile, iSrc, NULL, FILE_BEGIN);

						// read 4k chars from source offset
						if (ReadFile(hFile, szBuffer, 4096, &tRD, NULL))
						{
							// seek to the dst
							SetFilePointer(hFile, iDst, NULL, FILE_BEGIN);

							FlushFileBuffers(hFile);
							// write the buffer to the dest offset
							WriteFile(hFile, szBuffer, tRD, &tWR, NULL);
							iDst+=tWR;
						}

						iSrc+=tRD;
					}
					while(tRD != 0);

					// now truncate the file to the needed size
					SetEndOfFile(hFile);
				}
			}

			CloseHandle(hFile);
			return true;
		}
	}
#else
	FILE* pFile=fopen(m_pszPath, "r+");
	if (pFile)
	{
		// seek
		if (fseek(pFile, iSize-iNewSize, SEEK_SET) == 0)
		{
			// read the string to the eol
			char_t szBuffer[4096];
			fgets(szBuffer, 4096, pFile);
			iNewSize-=strlen(szBuffer);			// new size correction

			int_t iSrc=ftell(pFile);
			int_t iDst=0;
			size_t tRD, tWR;
					
			do
			{
				// seek to src
				fseek(pFile, iSrc, SEEK_SET);

				// read 4k chars from source offset
				tRD=fread(szBuffer, 1, 4096, pFile);
				if (tRD > 0)
				{
					// seek to the dst
					fseek(pFile, iDst, SEEK_SET);

					fflush(pFile);
					// write the buffer to the dest offset
					tWR=fwrite(szBuffer, 1, tRD, pFile);
					iDst+=tWR;
				}

				iSrc+=tRD;
			}
			while(tRD != 0);

			// now truncate the file to the needed size
			ftruncate(fileno(pFile), iDst);

			fclose(pFile);
			return true;
		}

		fclose(pFile);
	}
#endif

	return false;
}

/** Logs a formatted message to a log file.
 * \param[in] iType - type of the log message (LT_*)
 * \param[in] bStd - log also to stdout/stderr if true
 * \param[in] pszStr - format string for the following parameters
 */
void log_file::log(int_t iType, bool bStd, const char_t* pszStr, ...)
{
	if (iType < m_iLogLevel)
		return;
	
	va_list va;
	va_start(va, pszStr);
	logv(iType, bStd, pszStr, va);
	va_end(va);
}

/** Logs a formatted message to a log file.
 * \param[in] iType - type of the log message (LT_*)
 * \param[in] bStd - log also to stdout/stderr if true
 * \param[in] pszStr - format string for the following parameters
 * \param[in] va - variable argument list
 */
void log_file::logv(int_t iType, bool bStd, const char_t* pszStr, va_list va)
{
	if (iType < m_iLogLevel)
		return;
	
	char_t szBuf1[2048];
	vsnprintf(szBuf1, 2048, pszStr, va);		// user passed stuff
	
	logs(iType, bStd, szBuf1);
}
	
/** Logs an unformatted message to a log file.
 * \param[in] iType - type of the log message (LT_*)
 * \param[in] bStd - log also to stdout/stderr if true
 * \param[in] pszStr - message string
 */
void log_file::logs(int_t iType, bool bStd, const char_t* pszStr)
{
	assert(m_pszPath);
	
	if (iType < m_iLogLevel)
		return;
	
	// log time
	time_t t=time(NULL);
	char_t szData[128];
	strcpy(szData, ctime(&t));
	size_t tLen=strlen(szData)-1;
	while(szData[tLen] == '\n')
		szData[tLen--]='\0';

	m_lock.lock();
	
	// check the size constraints
	truncate((int_t)(strlen(pszStr)+1));
	FILE* pFile=fopen(m_pszPath, "a");
	bool bFailed=false;
	if (pFile)
	{
		if (fprintf(pFile, "[" STRFMT "] [" STRFMT "] " STRFMT "\n", szData, __logtype_str[iType], pszStr) < 0)
			bFailed=true;
		fclose(pFile);
	}
	else
		bFailed=true;
	if (bFailed || (m_bLogStd && !bStd))
	{
		switch(iType)
		{
		case LT_ERROR:
			{
				fprintf(stderr, "[" STRFMT "] [" STRFMT "] " STRFMT "\n", szData, __logtype_str[iType], pszStr);
				break;
			}
		default:
			{
				fprintf(stdout, "[" STRFMT "] [" STRFMT "] " STRFMT "\n", szData, __logtype_str[iType], pszStr);
				break;
			}
		}
	}
	else if (bStd)
	{
		switch(iType)
		{
		case LT_ERROR:
			{
				fprintf(stderr, STRFMT ": " STRFMT "\n", __logtype_str[iType], pszStr);
				break;
			}
		case LT_INFO:
			{
				fprintf(stdout, STRFMT "\n", pszStr);
				break;
			}
		default:
			{
				fprintf(stdout, STRFMT ": " STRFMT "\n", __logtype_str[iType], pszStr);
				break;
			}
		}
	}

	m_lock.unlock();
}

#if _LOG_LEVEL <= LT_DEBUG
/** Logs a formatted debug message to a log file.
 * \param[in] pszStr - format string for the given parameters
 */
void log_file::logd(const char_t* pszStr, ...)
{
	if (m_iLogLevel > LT_DEBUG)
		return;
	
	va_list va;
	va_start(va, pszStr);
	logv(LT_DEBUG, false, pszStr, va);
	va_end(va);
}

/** Logs a formatted debug message to a log file(also outputs to stdout).
 * \param[in] pszStr - format string for the given parameters
 */
void log_file::logds(const char_t* pszStr, ...)
{
	if (m_iLogLevel > LT_DEBUG)
		return;
	
	va_list va;
	va_start(va, pszStr);
	logv(LT_DEBUG, true, pszStr, va);
	va_end(va);
}

#else
void log_file::logd(const char_t* pszStr, ...)
{
}

void log_file::logds(const char_t* pszStr, ...)
{
}
#endif

#if _LOG_LEVEL <= LT_INFO
/** Logs a formatted informational message to a log file.
 * \param[in] pszStr - format string for the given parameters
 */
void log_file::logi(const char_t* pszStr, ...)
{
	if (m_iLogLevel > LT_INFO)
		return;
	
	va_list va;
	va_start(va, pszStr);
	logv(LT_INFO, false, pszStr, va);
	va_end(va);
}

/** Logs a formatted informational message to a log file(also outputs to stdout).
 * \param[in] pszStr - format string for the given parameters
 */
void log_file::logis(const char_t* pszStr, ...)
{
	if (m_iLogLevel > LT_INFO)
		return;
	
	va_list va;
	va_start(va, pszStr);
	logv(LT_INFO, true, pszStr, va);
	va_end(va);
}
#else
void log_file::logi(const char_t* pszStr, ...)
{
}

void log_file::logis(const char_t* pszStr, ...)
{
}

#endif

#if _LOG_LEVEL <= LT_WARNING
/** Logs a formatted warning message to a log file.
 * \param[in] pszStr - format string for the given parameters
 */
void log_file::logw(const char_t* pszStr, ...)
{
	if (m_iLogLevel > LT_WARNING)
		return;
	
	va_list va;
	va_start(va, pszStr);
	logv(LT_WARNING, false, pszStr, va);
	va_end(va);
}

/** Logs a formatted warning message to a log file(also outputs to stdout).
 * \param[in] pszStr - format string for the given parameters
 */
void log_file::logws(const char_t* pszStr, ...)
{
	if (m_iLogLevel > LT_WARNING)
		return;
	va_list va;
	va_start(va, pszStr);
	logv(LT_WARNING, true, pszStr, va);
	va_end(va);
}

#else
void log_file::logw(const char_t* pszStr, ...)
{
}

void log_file::logws(const char_t* pszStr, ...)
{
}

#endif

/** Logs a formatted error message to a log file.
 * \param[in] pszStr - format string for the given parameters
 */
void log_file::loge(const char_t* pszStr, ...)
{
	va_list va;
	va_start(va, pszStr);
	logv(LT_ERROR, false, pszStr, va);
	va_end(va);
}

/** Logs a formatted error message to a log file(also outputs to stderr).
 * \param[in] pszStr - format string for the given parameters
 */
void log_file::loges(const char_t* pszStr, ...)
{
	va_list va;
	va_start(va, pszStr);
	logv(LT_ERROR, true, pszStr, va);
	va_end(va);
}

/** Logs a formatted error message to a log file(also outputs to stderr).
 *  As an addition the first string %err is replaced with a given error 
 *  followed by the error description (system-based).
 * \param[in] pszStr - format string for the given parameters
 * \param[in] iSysErr - system error to be shown
 */
void log_file::logerr(const char_t* pszStr, int iSysErr, ...)
{
	char_t szNewFmt[2048];
	if (prepare_fmt(pszStr, iSysErr, szNewFmt))
	{
		va_list va;
		va_start(va, iSysErr);
		logv(LT_ERROR, false, szNewFmt, va);
		va_end(va);
	}
	else
	{
		va_list va;
		va_start(va, iSysErr);
		logv(LT_ERROR, false, pszStr, va);
		va_end(va);
	}
}

/** Logs a formatted error message to a log file(also outputs to stderr).
 *  As an addition the first string %err is replaced with a given error 
 *  followed by the error description (system-based).
 *  This function differ from logerr() with logging the output string
 *  also to the stderr.
 * \param[in] pszStr - format string for the given parameters
 * \param[in] iSysErr - system error to be shown
 */
void log_file::logerrs(const char_t* pszStr, int iSysErr, ...)
{
	char_t szNewFmt[2048];
	if (prepare_fmt(pszStr, iSysErr, szNewFmt))
	{
		va_list va;
		va_start(va, iSysErr);
		logv(LT_ERROR, true, szNewFmt, va);
		va_end(va);
	}
	else
	{
		va_list va;
		va_start(va, iSysErr);
		logv(LT_ERROR, true, pszStr, va);
		va_end(va);
	}
}

/** Function prepares a format string with error number and an error message
 *  for use with logerr() and logerrs() functions.
 * \param[in] pszStr - input format string (%err will be replaced with a 0x%lx (error message)
 * \param[in] iSysError - system error to parse
 * \param[out] pszOut - pointer to a buffer that will receive the data (must be 2048 bytes in size)
 * \return If the %err string was found and replaced within a given format string.
 */
bool log_file::prepare_fmt(const char_t* pszStr, int iSysErr, char_t* pszOut)
{
	// find the %err in pszStr
	char_t* pszFnd=strstr(pszStr, "%err");
	if (pszFnd)
	{
		// find an error description for the error
		char_t* pszErrDesc=NULL;
#ifdef _WIN32
		char_t szErrDesc[512];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, (DWORD)iSysErr, 0, szErrDesc, 512, NULL);
		pszErrDesc=szErrDesc;
#else
		pszErrDesc=strerror(iSysErr);
#endif

		// format a string with err no and desc
		char_t szError[1024];
		snprintf(szError, 1024, "0x%lx (%s)", iSysErr, pszErrDesc);

		// replace %err with the new data
		pszOut[0]='\0';
		strncat(pszOut, pszStr, pszFnd-pszStr);
		strcat(pszOut, szError);
		strcat(pszOut, pszFnd+4);

		return true;
	}
	else
		return false;
}

END_ICPF_NAMESPACE
