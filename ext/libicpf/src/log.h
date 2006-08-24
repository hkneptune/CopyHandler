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
/** \file log.h
 *  \brief Contains the log class.
 */
#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include "mutex.h"
#include "libicpf.h"
#include "gen_types.h"

BEGIN_ICPF_NAMESPACE

/// Log message type - debug
#define LT_DEBUG	0x00
/// Log message type - informational
#define LT_INFO		0x01
/// Log message type - warning
#define LT_WARNING	0x02
/// Log message type - error
#define LT_ERROR	0x03

class log_file;

extern log_file* __g_log;

#ifndef _LOG_LEVEL
/** \brief Specifies the log level used by program.
 *
 *  All the log messages used in code will be checked against this log level
 *  and if below then they will be removed from the code (probably).
 */
#define _LOG_LEVEL LT_DEBUG
#endif

/// Logs a message using the global instance of the log class.
#define LOG\
	__g_log->log

/// Logs a debug message using the global instance of the log class.
#define LOGD\
	__g_log->logd
/// Logs a debug message (with outputting it to stdout) using the global instance of the log class.
#define LOGDS\
	__g_log->logds

/// Logs an informational message using the global instance of the log class.
#define LOGI\
	__g_log->logi
/// Logs an informational message (with outputting it to stdout) using the global instance of the log class.
#define LOGIS\
	__g_log->logis

/// Logs a warning message using the global instance of the log class.
#define LOGW\
	__g_log->logw
/// Logs a warning (with outputting it to stdout) message using the global instance of the log class.
#define LOGWS\
	__g_log->logws

/// Logs an error message using the global instance of the log class.
#define LOGE\
	__g_log->loge
/// Logs an error message (with outputting it to stdout) using the global instance of the log class.
#define LOGES\
	__g_log->loges

/** \brief Class provides the message logging capability.
 *
 *  Class used to perform message logging to the external file. Provides a possibility
 *  of limiting the max size of a file and to cut the log message types below a specific
 *  level. When constructed, class makes a global variable accessed by the friend function
 *  get_log() . As a helpers there has been some macros introduced (LOGXXX) that makes usage
 *  of the global variable.
 *  Usage: either construct a log_file object and init() it, or call a friend create_log()
 *  and delete_log() when finished using the class.
 *  Class is thread safe.
 */
class LIBICPF_API log_file
{
public:
/** \name Construction/destruction */
/**@{*/
	explicit log_file(bool bGlobal=false);		///< Standard constructor
	~log_file();				///< Standard destructor
/**@}*/
	
/** \name Initialization */
/**@{*/
	bool init(const char_t* pszPath, int_t iMaxSize, int_t iLogLevel, bool bLogStd, bool bClean);	///< Initializes the logging object
/**@}*/

/** \name Logging functions */
/**@{*/
	void logs(int_t iType, bool bStd, const char_t* pszStr);		///< Logs a string without formatting
	void log(int_t iType, bool bStd, const char_t* pszStr, ...);	///< Logs a string with formatting
	void logv(int_t iType, bool bStd, const char_t* pszStr, va_list va);	///< Logs a string using va_list
	
#if _LOG_LEVEL <= LT_DEBUG
	void logd(const char_t* pszStr, ...);		///< Logs a debug message with formatting
	void logds(const char_t* pszStr, ...);	///< Logs a debug message with formatting (also prints to stdout)
#else
	void logd(const char_t* pszStr, ...);
	void logds(const char_t* pszStr, ...);
#endif

#if _LOG_LEVEL <= LT_INFO
	void logi(const char_t* pszStr, ...);		///< Logs an informational message with formatting
	void logis(const char_t* pszStr, ...);	///< Logs an informational message with formatting(also prints to stdout)
#else
	void logi(const char_t* pszStr, ...);
	void logis(const char_t* pszStr, ...);
#endif

#if _LOG_LEVEL <= LT_WARNING
	void logw(const char_t* pszStr, ...);		///< Logs a warning message with formatting
	void logws(const char_t* pszStr, ...);	///< Logs a warning message with formatting(also prints to stdout)
#else
	void logw(const char_t* pszStr, ...);
	void logws(const char_t* pszStr, ...);
#endif

	void loge(const char_t* pszStr, ...);		///< Logs an error message with formatting
	void loges(const char_t* pszStr, ...);	///< Logs an error message with formatting(also prints to stderr)

	void logerr(const char_t* pszStr, int iSysErr, ...);	///< Logs an error message with system error number and error description
	void logerrs(const char_t* pszStr, int iSysErr, ...);	///< Logs an error message with system error number and error description (also prints to stderr)
/**@}*/

	/// Gets the global instance of the log file
	friend log_file* get_log() { return __g_log; };
	/// Creates a global instance of a log file
	friend bool create_log(const char_t* pszPath, int_t iMaxSize, int_t iLogLevel, bool bLogStd, bool bClean);
	/// Deletes a global instance of a log dile
	friend void delete_log() { delete __g_log; };
	
protected:
	/// Truncates a log file not to exceed the max file size
	bool truncate(int_t iAdd) const;
	/// Returns the size of a log file
	int_t size() const;

private:
	/// Prepares a new format string for logerr(s) functions
	bool prepare_fmt(const char_t* pszStr, int iSysErr, char_t* pszOut) const;
	
protected:
	char_t* m_pszPath;	///< Path to the log file
	int_t m_iMaxSize;	///< Maximum size of the log file
	bool m_bLogStd;		///< Log also to stdout/stderr
	int_t m_iLogLevel;	///< Log level (similar to the _LOG_LEVEL, but change'able after compilation)
	bool m_bGlobal;		///< Is this the global instance of app log ? (so the LOG* macros would use it)

protected:
	mutex m_lock;		///< Lock for making the class thread safe
};

END_ICPF_NAMESPACE

#endif
