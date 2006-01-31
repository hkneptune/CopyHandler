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
/** \file exception.h
 *  \brief Contain an exception handling class.
 */
 
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include "log.h"
#include "libicpf.h"
#include "gen_types.h"

/** \brief Macro used for throwing an exception the easy way.
 *
 *  Macro throws an exception specified by the parameters. Other params needed by
 *  the exception class are taken indirectly from the position of a macro in the
 *  source file.
 * \param[in] desc - error description; if any formatting should be used use exception::format() function
 * \param[in] app_code - application error code (definitions are in engine_defs.h)
 * \param[in] sys_code - system error code (platform specific)
 * \param[in] reserved_code - currently unused; must be 0
 */
#undef THROW
//#define THROW(desc,app_code,sys_code,reserved_code) throw new icpf::exception(desc, __FILE__, __FUNCTION__, __LINE__,app_code,sys_code,reserved_code)
#define THROW(desc,app_code,sys_code,reserved_code) icpf::exception::raise(desc, __FILE__, __FUNCTION__, __LINE__, app_code, sys_code, reserved_code)
/// Logs an exception in a log file
#define LOG_EXCEPTION(except, ptr_log) (except)->log("Caught an exception in ", __FUNCTION__, ptr_log)
/// Logs an unknown exception in a log file
#define LOG_UEXCEPTION(ptr_log) (ptr_log)->loge("Caught an unknown exception in " STRFMT, __FUNCTION__)

BEGIN_ICPF_NAMESPACE

/** \brief Exception class used by most of the engine.
 *
 *  Exception class thrown by most of the engine functions. Provides user
 *  with an additional formatting and outputting capabilities.
 */
class LIBICPF_API exception
{
public:
/** \name Construction/destruction */
/**@{*/
	/// Standard constructor that takes the const description
	exception(const char_t* pszDesc, const char_t* pszFilename, const char_t* pszFunction, uint_t uiLine, uint_t uiAppCode, uint_t uiSystemCode, uint_t uiReserved);
	/// Standard constructor that takes non-const ptr to a buffer as the description
	exception(char_t* pszDesc, const char_t* pszFilename, const char_t* pszFunction, uint_t uiLine, uint_t uiAppCode, uint_t uiSystemCode, uint_t uiReserved);
	/// Standard destructor
	~exception();

	/// Raises an exception
	static void raise(const char* pszDesc, const char_t* pszFilename, const char_t* pszFunction, uint_t uiLine, uint_t uiAppCode, uint_t uiSystemCode, uint_t uiReserved);
	void del();				///< Deletes this class (if allocated with new operator)
/**@}*/
	
/** \name Outputting */
/**@{*/
	const char_t* get_info(char_t* pszInfo, intptr_t tMaxLen);	///< Retrieves the exception information to a specified string buffer
	void log(const char_t* pszDesc, log_file* plog);		///< Logs the exception information to the log file
	void log(const char_t* pszDesc, const char_t* pszDesc2, log_file* plog);	///< Logs the exception to the log file with an additional description
/**@}*/

/** \name Formatting */
/**@{*/
	static char_t* format(const char_t* pszFormat, ...);	///< Description formatting function
/**@}*/

protected:
	void set_string(char_t** pszOut, const char_t* pszIn);	///< Makes a copy of an input string

public:
	char_t* m_pszDesc;			///< Exception description
	char_t* m_pszFilename;		///< Source file in which the exception has been thrown
	char_t* m_pszFunction;		///< Function name in the source file in which the exception has been thrown
	uint_t m_uiLine;			///< Line in the source file in which the exception has been thrown
	uint_t m_uiAppCode;			///< Application error code
	uint_t m_uiSystemCode;		///< System error code (platform dependent)
	uint_t m_uiReserved;		///< Reserved code - currently unused and should be 0
};

END_ICPF_NAMESPACE

#endif
