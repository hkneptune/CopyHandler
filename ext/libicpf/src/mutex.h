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
 */
#ifndef __MUTEX_H__
#define __MUTEX_H__

#ifdef _WIN32
	#include <windows.h>
#else
	#include <pthread.h>
#endif

#include "libicpf.h"
#include "gen_types.h"

BEGIN_ICPF_NAMESPACE

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
	/** \brief Standard constructor
	 */
	mutex()
	{
#ifdef _WIN32
		::InitializeCriticalSection(&m_cs);
#else
		pthread_mutexattr_t mta;
		pthread_mutexattr_init(&mta);
//#warning Recursive mutexes are disabled; Make sure you use them the right way.
		pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_RECURSIVE_NP);
		pthread_mutex_init(&m_mutex, &mta);

		pthread_mutexattr_destroy(&mta);
#endif
	};

	/** \brief Standard destructor
	 */
	~mutex()
	{
#ifdef _WIN32
		::DeleteCriticalSection(&m_cs);
#else
		pthread_mutex_destroy(&m_mutex);
#endif
	};
/**@}*/
	
	// standard locking
/** \name Locking/unlocking */
/**@{*/

	/** \brief Locks access to some part of code for the current thread
	 *
	 * Locks access to some code using the platform specific functions.
	 * \return True if succeeded or false if not.
	 * \note The call under windows always return true.
	 */
	bool lock()
	{
#ifdef _WIN32
		::EnterCriticalSection(&m_cs);
		return true;
#else
		return pthread_mutex_lock(&m_mutex) == 0;
#endif
	};

	/** \brief Unlock access to some locked part of code
	 *
	 * Unlocks access to some code using the platform specific functions.
	 * \return True if succeeded or false if not.
	 * \note The call under windows always return true.
	 */
	bool unlock()
	{
#ifdef _WIN32
		::LeaveCriticalSection(&m_cs);
		return true;
#else
		return pthread_mutex_unlock(&m_mutex) == 0;		// return 0 on success
#endif
	};
/**@}*/

protected:
#ifdef _WIN32
	/// Underlying windows locking structure
	CRITICAL_SECTION m_cs;
#else
	/// Underlying linux locking structure/handle
	pthread_mutex_t m_mutex;
#endif
};

END_ICPF_NAMESPACE

#endif
