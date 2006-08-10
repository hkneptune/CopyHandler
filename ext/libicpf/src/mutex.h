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
/** \file mutex.h
 *  \brief Contains mutex class for thread safe access.
 *  \see The d_mutex class.
 */
#ifndef __MUTEX_H__
#define __MUTEX_H__

#include "libicpf.h"
#include "gen_types.h"

BEGIN_ICPF_NAMESPACE

#ifdef _DEBUG_MUTEX
	#define MLOCK(mutex) (mutex).lock(__FILE__, __LINE__, __FUNCTION__)
	#define MUNLOCK(mutex) (mutex).lock(__FILE__, __LINE__, __FUNCTION__)
#else
	#define MLOCK(mutex) (mutex).lock()
	#define MUNLOCK(mutex) (mutex).lock()
#endif

/** \brief Class provides the locking and unlocking capabilities for use with threads.
 *
 *  Class is a simple wrapper over the system related thread locking functions. In linux
 *  those functions are pthread_mutex_* and in windoze the functions related to CRITICAL_SECTION
 *  structure.
 */
class LIBICPF_API mutex
{
public:
/** \name Construction/destruction */
/**@{*/
	mutex();				///< Standard constructor
	mutex(void* /*pUnused*/);	///< Helper constructor, used as a compatibility layer with d_mutex
	mutex(const char_t* /*pszStr*/, void* /*pUnused*/);	///< Helper constructor, used as a compatibility layer with d_mutex

	virtual ~mutex();				///< Standard destructor
/**@}*/
	
/** \name Locking/unlocking */
/**@{*/
	void lock();			///< Locks this mutex
	void unlock();			///< Unlocks this mutex
	void lock(const char_t* /*pszFile*/, ulong_t /*ulLine*/, const char_t* /*pszFunction*/);	///< Locks this mutex (compatibility layer with d_mutex)
	void unlock(const char_t* /*pszFile*/, ulong_t /*ulLine*/, const char_t* /*pszFunction*/);	///< Unlocks this mutex (compatibility layer with d_mutex)
/**@}*/

protected:
	void construct();		///< Helper function - initializes the internal members, used by constructors

private:
	void* m_pLock;			///< Pointer to a system-specific structure used to lock
//#ifdef _WIN32
//	/// Underlying windows locking structure
//	CRITICAL_SECTION m_cs;
//#else
//	/// Underlying linux locking structure/handle
//	pthread_mutex_t m_mutex;
//#endif
};

END_ICPF_NAMESPACE

#endif
