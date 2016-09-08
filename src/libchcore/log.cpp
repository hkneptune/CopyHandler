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
/** \file log.cpp
 *  \brief Contains the implamentation of a log class.
 */
#include "stdafx.h"
#include "log.h"
#include <boost/assert.hpp>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

#include <stdlib.h>
#include <fcntl.h>
#include <windows.h>
#include "TCoreException.h"
#include "ErrorCodes.h"
#include <boost/algorithm/string/trim.hpp>

namespace chcore
{
	/// Table of strings representing the log message types
	const wchar_t* __logtype_str[] = { _T("debug"), _T("info"), _T("warning"), _T("error") };

	/** Constructs a log_file object.
	 * \param[in] bGlobal - states if this should be treates as a global instance of the log_file.
	 *						Only one global log_file instance could exist in the application.
	 */
	log_file::log_file() :
		m_pszPath(nullptr),
		m_iMaxSize(262144),
		m_bLogStd(false),
		m_iLogLevel(level_debug),
		m_lock()
	{
		_set_fmode(_O_BINARY);
	}

	/** Standard destructor
	 */
	log_file::~log_file()
	{
		delete[] m_pszPath;
	}

	/** Initializes the constructed log file.
	 * \param[in] pszPath - path to a log file to write to
	 * \param[in] iMaxSize - maximum size of a log file
	 * \param[in] iLogLevel - minimum log level of the messages to log
	 * \param[in] bLogStd - log the messages also to stdout/stderr
	 * \param[in] bClean - cleans the log file upon opening
	 */
	void log_file::init(const wchar_t* pszPath, int iMaxSize, int iLogLevel, bool bLogStd, bool bClean)
	{
		// store the path and other params
		delete[] m_pszPath;
		size_t stInLen = _tcslen(pszPath);
		m_pszPath = new wchar_t[stInLen + 1];
		_tcsncpy_s(m_pszPath, stInLen + 1, pszPath, _TRUNCATE);

		m_iMaxSize = iMaxSize;
		m_bLogStd = bLogStd;
		m_iLogLevel = iLogLevel;

		// try to open a file
		FILE* pFile = _tfopen(pszPath, bClean ? _T("w") : _T("a"));
		if (pFile == nullptr)
			throw TCoreException(eErr_CannotOpenFile, L"Could not open the specified file", LOCATION);

		fclose(pFile);
	}

	// ============================================================================
	/// log_file::is_initialized
	/// @date 2009/05/19
	///
	/// @brief     Checks is the log_file object has been initialized.
	/// @return    True if it has been initialized, false otherwise.
	// ============================================================================
	bool log_file::is_initialized() const throw()
	{
		return m_pszPath != 0;
	}

	// ============================================================================
	/// log_file::set_log_level
	/// @date 2009/05/23
	///
	/// @brief     Changes the log level for this class.
	/// @param[in] iLogLevel      New log level.
	// ============================================================================
	void log_file::set_log_level(int iLogLevel) throw()
	{
		m_iLogLevel = iLogLevel;
	}

	// ============================================================================
	/// log_file::set_max_size
	/// @date 2009/05/23
	///
	/// @brief     Sets the max size of the log file.
	/// @param[in] iMaxSize	Max size of the log file.
	// ============================================================================
	void log_file::set_max_size(int iMaxSize) throw()
	{
		BOOST_ASSERT(iMaxSize > 0);
		if (iMaxSize > 0)
			m_iMaxSize = iMaxSize;
	}

	/** Retrieves the current size of a log file.
	 *  Quite slow function - have to access the file by opening and closing it.
	 * \return Current file size.
	 */
	int log_file::size() const
	{
		assert(m_pszPath);
		if (!m_pszPath)
			return -1;

		int iSize = -1;
		FILE* pFile = _tfopen(m_pszPath, _T("r"));
		if (pFile != nullptr)
		{
			if (fseek(pFile, 0, SEEK_END) == 0)
				iSize = ftell(pFile);

			fclose(pFile);
		}

		return iSize;
	}

	/** Truncates the current log file content so when adding some new text the
	 *  file size won't exceed the maximum size specified in init().
	 * \param[in] iAdd - size of the new string to be added to the log file
	 * \return True if truncate succeeded or false if not.
	 */
	bool log_file::truncate(int iAdd) const
	{
		assert(m_pszPath);
		if (!m_pszPath)
			return false;

		// if we doesn't need to truncate anything
		if (m_iMaxSize <= 0)
			return true;

		// make some checks
		int iSize = size();
		if (iSize <= 0 || iSize + iAdd < m_iMaxSize)
			return false;

		// establish the new file size (1/3rd of the current size or max_size-add_size)
		int iNewSize = std::min((int)(iSize*0.66), m_iMaxSize - iAdd) & ~1;

#ifdef _WIN32
		// win32 does not have the ftruncate function, so we have to make some API calls
		HANDLE hFile = CreateFile(m_pszPath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			// seek
			if (SetFilePointer(hFile, iSize - iNewSize, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
			{
				// read the string to the eol
				DWORD dwRD;
				wchar_t szBuffer[4096 / sizeof(wchar_t)];
				if (ReadFile(hFile, szBuffer, 4096, &dwRD, nullptr))
				{
					dwRD /= sizeof(wchar_t);
					szBuffer[(dwRD > 0) ? dwRD - 1 : 0] = _T('\0');

					// replace the /r and /n in the log to the \0
					for (DWORD i = 0; i < dwRD; i++)
					{
						if (szBuffer[i] == _T('\r') || szBuffer[i] == _T('\n'))
						{
							szBuffer[i] = _T('\0');
							break;
						}
					}

					iNewSize -= (int)(_tcslen(szBuffer) + 1)*sizeof(wchar_t);			// new size correction

					if (SetFilePointer(hFile, iSize - iNewSize, nullptr, FILE_BEGIN) != INVALID_SET_FILE_POINTER)
					{
						long lSrc = (long)SetFilePointer(hFile, 0, nullptr, FILE_CURRENT);
						long lDst = 0;
						DWORD tRD, tWR;

						do
						{
							// seek to src
							SetFilePointer(hFile, lSrc, nullptr, FILE_BEGIN);

							// read 4k chars from source offset
							if (ReadFile(hFile, szBuffer, 4096, &tRD, nullptr))
							{
								// seek to the dst
								SetFilePointer(hFile, lDst, nullptr, FILE_BEGIN);

								FlushFileBuffers(hFile);
								// write the buffer to the dest offset
								WriteFile(hFile, szBuffer, tRD, &tWR, nullptr);
								lDst += (long)tWR;
							}

							lSrc += (long)tRD;
						} while (tRD != 0);

						// now truncate the file to the needed size
						SetEndOfFile(hFile);
					}
				}

				CloseHandle(hFile);
				return true;
			}

			CloseHandle(hFile);
		}
#else
		FILE* pFile = fopen(m_pszPath, _T("r+"));
		if (pFile)
		{
			// seek
			if (fseek(pFile, iSize - iNewSize, SEEK_SET) == 0)
			{
				// read the string to the eol
				wchar_t szBuffer[4096];
				fgets(szBuffer, 4096, pFile);

				int iSrc = ftell(pFile);
				int iDst = 0;
				size_t tRD, tWR;

				do
				{
					// seek to src
					fseek(pFile, iSrc, SEEK_SET);

					// read 4k chars from source offset
					tRD = fread(szBuffer, 1, 4096, pFile);
					if (tRD > 0)
					{
						// seek to the dst
						fseek(pFile, iDst, SEEK_SET);

						fflush(pFile);
						// write the buffer to the dest offset
						tWR = fwrite(szBuffer, 1, tRD, pFile);
						iDst += tWR;
					}

					iSrc += tRD;
				} while (tRD != 0);

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
	void log_file::log(int iType, bool bStd, const wchar_t* pszStr, ...)
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
	void log_file::logv(int iType, bool bStd, const wchar_t* pszStr, va_list va)
	{
		if (iType < m_iLogLevel)
			return;

		wchar_t szBuf1[2048];
		_vsntprintf(szBuf1, 2048, pszStr, va);		// user passed stuff

		logs(iType, bStd, szBuf1);
	}

	/** Logs an unformatted message to a log file.
	 * \param[in] iType - type of the log message (LT_*)
	 * \param[in] bStd - log also to stdout/stderr if true
	 * \param[in] pszStr - message string
	 */
	void log_file::logs(int iType, bool bStd, const wchar_t* pszStr)
	{
		assert(m_pszPath);
		if (!m_pszPath)
			return;

		if (iType < m_iLogLevel || iType < 0 || iType >= (sizeof(__logtype_str) / sizeof(wchar_t*)))
			return;

		// log time
		time_t t = time(nullptr);
		std::wstring strTime = _tctime(&t);
		boost::trim_right_if(strTime, boost::is_any_of(L"\n"));

		m_lock.lock();

		// check the size constraints
		truncate((int)(_tcslen(pszStr) + 1));
#if defined(UNICODE) && (defined(_WIN32) || defined(_WIN64))
		FILE* pFile = _tfopen(m_pszPath, _T("ab"));
#else
		FILE* pFile = _tfopen(m_pszPath, _T("at"));
#endif
		bool bFailed = false;
		if (pFile)
		{
			if (_ftprintf(pFile, _T("[%s] [%s] %s\r\n"), strTime.c_str(), __logtype_str[iType], pszStr) < 0)
				bFailed = true;
			fclose(pFile);
		}
		else
			bFailed = true;
		if (bFailed || (m_bLogStd && !bStd))
		{
			switch (iType)
			{
			case level_error:
				_ftprintf(stderr, _T("[%s] [%s] %s\r\n"), strTime.c_str(), __logtype_str[iType], pszStr);
				break;
			default:
				_ftprintf(stdout, _T("[%s] [%s] %s\r\n"), strTime.c_str(), __logtype_str[iType], pszStr);
			}
		}
		else if (bStd)
		{
			switch (iType)
			{
			case level_error:
				_ftprintf(stderr, _T("%s: %s\r\n"), __logtype_str[iType], pszStr);
				break;
			case level_info:
				_ftprintf(stdout, _T("%s\r\n"), pszStr);
				break;
			default:
				_ftprintf(stdout, _T("%s: %s\r\n"), __logtype_str[iType], pszStr);
			}
		}

		m_lock.unlock();
	}

#ifndef SKIP_LEVEL_DEBUG
	/** Logs a formatted debug message to a log file.
	 * \param[in] pszStr - format string for the given parameters
	 */
	void log_file::logd(const wchar_t* pszStr)
	{
		if (m_iLogLevel > level_debug)
			return;

		logs(level_debug, false, pszStr);
	}

	/** Logs a formatted debug message to a log file.
	* \param[in] pszStr - format string for the given parameters
	*/
	void log_file::logdv(const wchar_t* pszStr, ...)
	{
		if (m_iLogLevel > level_debug)
			return;

		va_list va;
		va_start(va, pszStr);
		logv(level_debug, false, pszStr, va);
		va_end(va);
	}

	/** Logs a formatted debug message to a log file(also outputs to stdout).
	 * \param[in] pszStr - format string for the given parameters
	 */
	void log_file::logds(const wchar_t* pszStr, ...)
	{
		if (m_iLogLevel > level_debug)
			return;

		va_list va;
		va_start(va, pszStr);
		logv(level_debug, true, pszStr, va);
		va_end(va);
	}

#else
	void log_file::logd(const wchar_t* /*pszStr*/)
	{
	}

	void log_file::logdv(const wchar_t* /*pszStr*/, ...)
	{
	}

	void log_file::logds(const wchar_t* /*pszStr*/, ...)
	{
	}
#endif

#ifndef SKIP_LEVEL_INFO
	/** Logs a formatted informational message to a log file.
	 * \param[in] pszStr - format string for the given parameters
	 */
	void log_file::logi(const wchar_t* pszStr)
	{
		if (m_iLogLevel > level_info)
			return;

		logs(level_info, false, pszStr);
	}

	/** Logs a formatted informational message to a log file.
	* \param[in] pszStr - format string for the given parameters
	*/
	void log_file::logiv(const wchar_t* pszStr, ...)
	{
		if (m_iLogLevel > level_info)
			return;

		va_list va;
		va_start(va, pszStr);
		logv(level_info, false, pszStr, va);
		va_end(va);
	}

	/** Logs a formatted informational message to a log file(also outputs to stdout).
	 * \param[in] pszStr - format string for the given parameters
	 */
	void log_file::logis(const wchar_t* pszStr, ...)
	{
		if (m_iLogLevel > level_info)
			return;

		va_list va;
		va_start(va, pszStr);
		logv(level_info, true, pszStr, va);
		va_end(va);
	}
#else
	void log_file::logi(const wchar_t* /*pszStr*/)
	{
	}

	void log_file::logiv(const wchar_t* /*pszStr*/, ...)
	{
	}

	void log_file::logis(const wchar_t* /*pszStr*/, ...)
	{
	}

#endif

#ifndef SKIP_LEVEL_WARNING
	/** Logs a formatted warning message to a log file.
	 * \param[in] pszStr - format string for the given parameters
	 */
	void log_file::logw(const wchar_t* pszStr)
	{
		if (m_iLogLevel > level_warning)
			return;

		logs(level_warning, false, pszStr);
	}

	/** Logs a formatted warning message to a log file.
	* \param[in] pszStr - format string for the given parameters
	*/
	void log_file::logwv(const wchar_t* pszStr, ...)
	{
		if (m_iLogLevel > level_warning)
			return;

		va_list va;
		va_start(va, pszStr);
		logv(level_warning, false, pszStr, va);
		va_end(va);
	}

	/** Logs a formatted warning message to a log file(also outputs to stdout).
	 * \param[in] pszStr - format string for the given parameters
	 */
	void log_file::logws(const wchar_t* pszStr, ...)
	{
		if (m_iLogLevel > level_warning)
			return;
		va_list va;
		va_start(va, pszStr);
		logv(level_warning, true, pszStr, va);
		va_end(va);
	}

#else
	void log_file::logw(const wchar_t* /*pszStr*/)
	{
	}

	void log_file::logwv(const wchar_t* /*pszStr*/, ...)
	{
	}

	void log_file::logws(const wchar_t* /*pszStr*/, ...)
	{
	}

#endif

	/** Logs a formatted error message to a log file.
	 * \param[in] pszStr - format string for the given parameters
	 */
	void log_file::loge(const wchar_t* pszStr)
	{
		logs(level_error, false, pszStr);
	}

	/** Logs a formatted error message to a log file.
	* \param[in] pszStr - format string for the given parameters
	*/
	void log_file::logev(const wchar_t* pszStr, ...)
	{
		va_list va;
		va_start(va, pszStr);
		logv(level_error, false, pszStr, va);
		va_end(va);
	}

	/** Logs a formatted error message to a log file(also outputs to stderr).
	 * \param[in] pszStr - format string for the given parameters
	 */
	void log_file::loges(const wchar_t* pszStr, ...)
	{
		va_list va;
		va_start(va, pszStr);
		logv(level_error, true, pszStr, va);
		va_end(va);
	}

	/** Logs a formatted error message to a log file(also outputs to stderr).
	 *  As an addition the first string %err is replaced with a given error
	 *  followed by the error description (system-based).
	 * \param[in] pszStr - format string for the given parameters
	 * \param[in] iSysErr - system error to be shown
	 */
	void log_file::logerr(const wchar_t* pszStr, int iSysErr, ...)
	{
		wchar_t szNewFmt[2048];
		if (prepare_fmt(pszStr, iSysErr, szNewFmt))
		{
			va_list va;
			va_start(va, iSysErr);
			logv(level_error, false, szNewFmt, va);
			va_end(va);
		}
		else
		{
			va_list va;
			va_start(va, iSysErr);
			logv(level_error, false, pszStr, va);
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
	void log_file::logerrs(const wchar_t* pszStr, int iSysErr, ...)
	{
		wchar_t szNewFmt[2048];
		if (prepare_fmt(pszStr, iSysErr, szNewFmt))
		{
			va_list va;
			va_start(va, iSysErr);
			logv(level_error, true, szNewFmt, va);
			va_end(va);
		}
		else
		{
			va_list va;
			va_start(va, iSysErr);
			logv(level_error, true, pszStr, va);
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
	bool log_file::prepare_fmt(const wchar_t* pszStr, int iSysErr, wchar_t* pszOut) const
	{
		// find the %err in pszStr
		const wchar_t* pszFnd = _tcsstr(pszStr, _T("%err"));
		if (pszFnd)
		{
			// find an error description for the error
			wchar_t* pszErrDesc = nullptr;
#ifdef _WIN32
			wchar_t szErrDesc[512];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, (DWORD)iSysErr, 0, szErrDesc, 512, nullptr);
			pszErrDesc = szErrDesc;
#else
			pszErrDesc = strerror(iSysErr);
#endif

			// format a string with err no and desc
			wchar_t szError[1024];
			_sntprintf(szError, 1023, _T("0x%lx (%s)"), iSysErr, pszErrDesc);
			szError[1023] = _T('\0');

			// replace %err with the new data
			pszOut[0] = _T('\0');
			_tcsncat(pszOut, pszStr, (size_t)(pszFnd - pszStr));
			_tcscat(pszOut, szError);
			_tcscat(pszOut, pszFnd + 4);

			return true;
		}
		else
			return false;
	}
}
