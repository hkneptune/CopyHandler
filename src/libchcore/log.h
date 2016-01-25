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
/** \file log.h
 *  \brief Contains the log class.
 */
#ifndef __LOG_H__
#define __LOG_H__

#include <stdarg.h>
#include "libchcore.h"

namespace chcore
{
	/** \brief Class provides the message logging capability.
	 *
	 *  Class used to perform message logging to the external file. Provides a possibility
	 *  of limiting the max size of a file and to cut the log message types below a specific
	 *  level.
	 *  Class is thread safe (is it? most of the methods does not seem to be thread safe).
	 */
	class LIBCHCORE_API log_file
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
		void init(const wchar_t* pszPath, int iMaxSize, int iLogLevel, bool bLogStd, bool bClean);	///< Initializes the logging object
		bool is_initialized() const throw();

		void set_log_level(int iLogLevel) throw();		///< Sets the log level
		void set_max_size(int iMaxSize) throw();			///< Sets the max size

	/**@}*/

	/** \name Logging functions */
	/**@{*/
		void logs(int iType, bool bStd, const wchar_t* pszStr);				///< Logs a string without formatting
		void log(int iType, bool bStd, const wchar_t* pszStr, ...);			///< Logs a string with formatting
		void logv(int iType, bool bStd, const wchar_t* pszStr, va_list va);	///< Logs a string using va_list

		void logd(const wchar_t* pszStr);			///< Logs a debug message with formatting
		void logdv(const wchar_t* pszStr, ...);		///< Logs a debug message with formatting
		void logds(const wchar_t* pszStr, ...);		///< Logs a debug message with formatting (also prints to stdout)

		void logi(const wchar_t* pszStr);			///< Logs an informational message with formatting
		void logiv(const wchar_t* pszStr, ...);		///< Logs an informational message with formatting
		void logis(const wchar_t* pszStr, ...);		///< Logs an informational message with formatting(also prints to stdout)

		void logw(const wchar_t* pszStr);			///< Logs a warning message with formatting
		void logwv(const wchar_t* pszStr, ...);		///< Logs a warning message with formatting
		void logws(const wchar_t* pszStr, ...);		///< Logs a warning message with formatting(also prints to stdout)

		void loge(const wchar_t* pszStr);			///< Logs an error message with formatting
		void logev(const wchar_t* pszStr, ...);		///< Logs an error message with formatting
		void loges(const wchar_t* pszStr, ...);		///< Logs an error message with formatting(also prints to stderr)

		void logerr(const wchar_t* pszStr, int iSysErr, ...);	///< Logs an error message with system error number and error description
		void logerrs(const wchar_t* pszStr, int iSysErr, ...);	///< Logs an error message with system error number and error description (also prints to stderr)
	/**@}*/

	protected:
		/// Truncates a log file not to exceed the max file size
		bool truncate(int iAdd) const;
		/// Returns the size of a log file
		int size() const;

	private:
		/// Prepares a new format string for logerr(s) functions
		bool prepare_fmt(const wchar_t* pszStr, int iSysErr, wchar_t* pszOut) const;

	protected:
		wchar_t* m_pszPath;	///< Path to the log file
		int m_iMaxSize;	///< Maximum size of the log file
		bool m_bLogStd;		///< Log also to stdout/stderr
		int m_iLogLevel;	///< Log level (similar to the _LOG_LEVEL, but changeable after compilation)

	protected:
#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;
#pragma warning(pop)
	};
}

#endif
