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

BEGIN_ICPF_NAMESPACE

/** \brief Callback class with one parameter.
 *
 *  Class provides a simple interface for user to call a specific callback
 *  function(s) registered by the user. Good for notifying user that something
 *  had happened.
 */
template<class A1, class P1>
class LIBICPF_API callback1
{
protected:
	/// Callback1-type callback function
	typedef void(*PFNCALLBACKPROC1)(A1, P1);

	/// Helper structure for callback1 class
	struct _CALLBACKDATA1
	{
		PFNCALLBACKPROC1 pfn;	///< Callback function that is to be called
		A1 param;			///< The user parameter of a function to be called
	};

public:
/** \name Construction/destruction */
/**@{*/
	callback1() { };		///< Standard constructor
	~callback1() { };		///< Standard destructor
/**@}*/

/** \name User interface */
/**@{*/
	/** Executes a callback list associated with this object.
	* \param[in] pData - parameter that will be passed to a user callback function
	*/
	void exec(A1 data)
	{
		m_lock.lock();
		try
		{
			for (std::list<_CALLBACKDATA1>::iterator it=m_lCalls.begin();it != m_lCalls.end();it++)
				(*((*it).pfn))((*it).param, data);
			m_lock.unlock();
		}
		catch(...)
		{
			m_lock.unlock();
			throw;
		}
	}
	
	/** Connects a user callback function to this object.
	* \param[in] pfn - user callback function address
	* \param[in] appParam - user parameter to pass to the callback function when executing
	*/
	void connect(PFNCALLBACKPROC1 pfn, A1 appParam)
	{
		_CALLBACKDATA1 cd;
		cd.pfn=pfn;
		cd.param=appParam;

		m_lock.lock();
		m_lCalls.push_back(cd);
		m_lock.unlock();
	}

	/** Disconnects the user callback function if connected earlier.
	* \param[in] pfn - address of a function to remove
	*/
	void disconnect(PFNCALLBACKPROC1 pfn)
	{
		m_lock.lock();
		for (std::list<_CALLBACKDATA1>::iterator it=m_lCalls.begin();it != m_lCalls.end();it++)
		{
			if ( (*it).pfn == pfn )
			{
				m_lCalls.erase(it);
				m_lock.unlock();
				return;
			}
		}
		m_lock.unlock();
	}

	/** Clears the callback list. No function will be left.
	 */
	void clear()
	{
		m_lock.lock();
		m_lCalls.clear();
		m_lock.unlock();
	}
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
template<class A1, class T1, class T2>
class LIBICPF_API callback2
{
protected:
	/// Callback2-type callback function
	typedef void(*PFNCALLBACKPROC2)(A1, T1, T2);

	/// Helper structure for callback2 class
	struct _CALLBACKDATA2
	{
		PFNCALLBACKPROC2 pfn;	///< Callback function that is to be called
		A1 param;				///< The user parameter of a function to be called
	};

public:
/** \name Construction/destruction */
/**@{*/
	callback2() { };		///< Standard constructor
	~callback2() { };		///< Standard destructor
/**@}*/

/** \name User interface */
/**@{*/
	/** Executes a callback list associated with this object.
	* \param[in] data1 - first parameter that will be passed to a user callback function
	* \param[in] data2 - second parameter that will be passed to a user callback function
	*/
	void exec(T1 data1, T2 data2)
	{
		m_lock.lock();
		try
		{
			for (std::list<_CALLBACKDATA2>::iterator it=m_lCalls.begin();it != m_lCalls.end();it++)
				(*((*it).pfn))((*it).param, data1, data2);
			m_lock.unlock();
		}
		catch(...)
		{
			m_lock.unlock();
			throw;
		}
	}
	
	/** Connects a user callback function to this object.
	* \param[in] pfn - user callback function address
	* \param[in] appParam - user parameter to pass to the callback function when executing
	*/
	void connect(PFNCALLBACKPROC2 pfn, A1 appParam)
	{
		_CALLBACKDATA2 cd;
		cd.pfn=pfn;
		cd.param=appParam;

		m_lock.lock();
		m_lCalls.push_back(cd);
		m_lock.unlock();
	}

	/** Disconnects the user callback function if connected earlier.
	* \param[in] pfn - address of a function to remove
	*/
	void disconnect(PFNCALLBACKPROC2 pfn)
	{
		m_lock.lock();
		for (std::list<_CALLBACKDATA2>::iterator it=m_lCalls.begin();it != m_lCalls.end();it++)
		{
			if ( (*it).pfn == pfn )
			{
				m_lCalls.erase(it);
				m_lock.unlock();
				return;
			}
		}
		m_lock.unlock();
	}

	/** Clears the callback list. No function will be left.
	 */
	void clear()
	{
		m_lock.lock();
		m_lCalls.clear();
		m_lock.unlock();
	}
/**@}*/
	
protected:
	std::list<_CALLBACKDATA2> m_lCalls;		///< List of the callback structures to execute
	mutex m_lock;							///< Mutex for locking connect/disconnect calls
};

/** \brief Callback class with three parameters.
 *
 *  Class provides a simple interface for user to call a specific callback
 *  function(s) registered by the user. Good for notifying user that something
 *  had happened.
 */
template<class A1, class T1, class T2, class T3>
class LIBICPF_API callback3
{
protected:
	/// Callback3-type callback function
	typedef void(*PFNCALLBACKPROC3)(A1, T1, T2, T3);

	/// Helper structure for callback2 class
	struct _CALLBACKDATA3
	{
		PFNCALLBACKPROC3 pfn;	///< Callback function that is to be called
		A1 param;				///< The user parameter of a function to be called
	};

public:
/** \name Construction/destruction */
/**@{*/
	callback3() { };		///< Standard constructor
	~callback3() { };		///< Standard destructor
/**@}*/

/** \name User interface */
/**@{*/
	/** Executes a callback list associated with this object.
	* \param[in] data1 - first parameter that will be passed to a user callback function
	* \param[in] data2 - second parameter that will be passed to a user callback function
	* \param[in] data3 - third parameter that will be passed to a user callback function
	*/
	void exec(T1 data1, T2 data2, T3 data3)
	{
		m_lock.lock();
		try
		{
			for (std::list<_CALLBACKDATA3>::iterator it=m_lCalls.begin();it != m_lCalls.end();it++)
				(*((*it).pfn))((*it).param, data1, data2, data3);
			m_lock.unlock();
		}
		catch(...)
		{
			m_lock.unlock();
			throw;
		}
	}
	
	/** Connects a user callback function to this object.
	* \param[in] pfn - user callback function address
	* \param[in] param - user parameter to pass to the callback function when executing
	*/
	void connect(PFNCALLBACKPROC3 pfn, A1 param)
	{
		_CALLBACKDATA2 cd;
		cd.pfn=pfn;
		cd.param=param;

		m_lock.lock();
		m_lCalls.push_back(cd);
		m_lock.unlock();
	}

	/** Disconnects the user callback function if connected earlier.
	* \param[in] pfn - address of a function to remove
	*/
	void disconnect(PFNCALLBACKPROC3 pfn)
	{
		m_lock.lock();
		for (std::list<_CALLBACKDATA3>::iterator it=m_lCalls.begin();it != m_lCalls.end();it++)
		{
			if ( (*it).pfn == pfn )
			{
				m_lCalls.erase(it);
				m_lock.unlock();
				return;
			}
		}
		m_lock.unlock();
	}

	/** Clears the callback list. No function will be left.
	 */
	void clear()
	{
		m_lock.lock();
		m_lCalls.clear();
		m_lock.unlock();
	}
/**@}*/
	
protected:
	std::list<_CALLBACKDATA3> m_lCalls;		///< List of the callback structures to execute
	mutex m_lock;							///< Mutex for locking connect/disconnect calls
};

END_ICPF_NAMESPACE

#endif
