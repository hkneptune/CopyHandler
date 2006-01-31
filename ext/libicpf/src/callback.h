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
#ifndef __CALLBACK_H__
#define __CALLBACK_H__

/** \file callback.h
 *  \brief Provides callback classes
 */

#include "libicpf.h"
#include "gen_types.h"
#include <list>
#include "mutex.h"

/// Callback1-type callback function
typedef void(*PFNCALLBACKPROC1)(ptr_t, ptr_t);
/// Callback2-type callback function
typedef void(*PFNCALLBACKPROC2)(ptr_t, ptr_t, ptr_t);

BEGIN_ICPF_NAMESPACE

/// Helper structure for callback1 class
struct _CALLBACKDATA1
{
	PFNCALLBACKPROC1 pfn;	///< Callback function that is to be called
	ptr_t pParam;			///< The user parameter of a function to be called
};

/// Helper structure for callback2 class
struct _CALLBACKDATA2
{
	PFNCALLBACKPROC2 pfn;	///< Callback function that is to be called
	ptr_t pParam;			///< The user parameter of a function to be called
};

/** \brief Callback class with one parameter.
 *
 *  Class provides a simple interface for user to call a specific callback
 *  function(s) registered by the user. Good for notifying user that something
 *  had happened.
 */
class LIBICPF_API callback1
{
public:
/** \name Construction/destruction */
/**@{*/
	callback1();		///< Standard constructor
	~callback1();		///< Standard destructor
/**@}*/

/** \name User interface */
/**@{*/
	void exec(ptr_t pData);		///< Executes registered callback functions with the pData as the first param
	
	void connect(PFNCALLBACKPROC1 pfn, ptr_t);		///< Connects the callback function to this callback class
	void disconnect(PFNCALLBACKPROC1 pfn);					///< Disconnects the callback function from this callback class
/**@}*/
	
protected:
	std::list<_CALLBACKDATA1> m_lCalls;		///< List of the callback structures to execute
	mutex m_lock;							///< Mutex for locking connect/disconnect calls
};

/** \brief Callback class with two parameters.
 *
 *  Class provides a simple interface for user to call a specific callback
 *  function(s) registered by the user. Good for notifying user that something
 *  had happened.
 */
class LIBICPF_API callback2
{
public:
/** \name Construction/destruction */
/**@{*/
	callback2();		///< Standard constructor
	~callback2();		///< Standard destructor
/**@}*/

/** \name User interface */
/**@{*/
	void exec(ptr_t pData, ptr_t pData2);		///< Executes registered callback functions with the pData as the first param
	
	void connect(PFNCALLBACKPROC2 pfn, ptr_t pParam);	///< Connects the callback function to this callback class
	void disconnect(PFNCALLBACKPROC2 pfn);				///< Disconnects the callback function from this callback class
/**@}*/
	
protected:
	std::list<_CALLBACKDATA2> m_lCalls;		///< List of the callback structures to execute
	mutex m_lock;							///< Mutex for locking connect/disconnect calls
};

END_ICPF_NAMESPACE

#endif
