/***************************************************************************
 *   Copyright (C) 2004-2006 by J�zef Starosczyk                           *
 *   ixen@copyhandler.com                                                  *
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

/** \brief Class provides the message logging capability.
 *
 *  Class used to perform message logging to the external file. Provides a possibility
 *  of limiting the max size of a file and to cut the log message types below a specific
 *  level.
 *  Class is thread safe.
 */
class LIBICPF_API log_file
{
public:
	/// Supported log levels
	enum log_levels
	{
		level_debug,		/// Debug level (the most detailed one)
		level_info,			/// Informational level
		level_warning,		/// Warning level
		level_error			/// Error level (the least detailed one)
	};

public:
/** \name Construction/destruction */
/**@{*/
	explicit log_file();		///< Standard constructor
	~log_file();				///< Standard destructor
/**@}*/
	
/** \name Initialization */
/**@{*/
	void init(const tchar_t* pszPath, int_t iMaxSize, int_t iLogLevel, bool bLogStd, bool bClean);	///< Initializes the logging object
/**@}*/

/** \name Logging functions */
/**@{*/
	void logs(int_t iType, bool bStd, const tchar_t* pszStr);				///< Logs a string without formatting
	void log(int_t iType, bool bStd, const tchar_t* pszStr, ...);			///< Logs a string with formatting
	void logv(int_t iType, bool bStd, const tchar_t* pszStr, va_list va);	///< Logs a string using va_list
	
	void logd(const tchar_t* pszStr, ...);		///< Logs a debug message with formatting
	void logds(const tchar_t* pszStr, ...);		///< Logs a debug message with formatting (also prints to stdout)

	void logi(const tchar_t* pszStr, ...);		///< Logs an informational message with formatting
	void logis(const tchar_t* pszStr, ...);		///< Logs an informational message with formatting(also prints to stdout)

	void logw(const tchar_t* pszStr, ...);		///< Logs a warning message with formatting
	void logws(const tchar_t* pszStr, ...);	///< Logs a warning message with formatting(also prints to stdout)

	void loge(const tchar_t* pszStr, ...);		///< Logs an error message with formatting
	void loges(const tchar_t* pszStr, ...);	///< Logs an error message with formatting(also prints to stderr)

	void logerr(const tchar_t* pszStr, int_t iSysErr, ...);	///< Logs an error message with system error number and error description
	void logerrs(const tchar_t* pszStr, int_t iSysErr, ...);	///< Logs an error message with system error number and error description (also prints to stderr)
/**@}*/

protected:
	/// Truncates a log file not to exceed the max file size
	bool truncate(int_t iAdd) const;
	/// Returns the size of a log file
	int_t size() const;

private:
	/// Prepares a new format string for logerr(s) functions
	bool prepare_fmt(const tchar_t* pszStr, int_t iSysErr, tchar_t* pszOut) const;
	
protected:
	tchar_t* m_pszPath;	///< Path to the log file
	int_t m_iMaxSize;	///< Maximum size of the log file
	bool m_bLogStd;		///< Log also to stdout/stderr
	int_t m_iLogLevel;	///< Log level (similar to the _LOG_LEVEL, but change'able after compilation)

protected:
	mutex m_lock;		///< Lock for making the class thread safe
};

END_ICPF_NAMESPACE

#endif