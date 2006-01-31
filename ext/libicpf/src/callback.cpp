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
/** \file callback.cpp
 *  \brief File provides the implementation of callback classes.
 */
#include "callback.h"

BEGIN_ICPF_NAMESPACE

callback1::callback1()
{
	
}

callback1::~callback1()
{
	
}

/** Executes a callback list associated with this object.
 * \param[in] pData - parameter that will be passed to a user callback function
 */
void callback1::exec(ptr_t pData)
{
	m_lock.lock();
	try
	{
		for (std::list<_CALLBACKDATA1>::iterator it=m_lCalls.begin();it != m_lCalls.end();it++)
			(*((*it).pfn))((*it).pParam, pData);
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
 * \param[in] pParam - user parameter to pass to the callback function when executing
 */
void callback1::connect(PFNCALLBACKPROC1 pfn, ptr_t pParam)
{
	_CALLBACKDATA1 cd;
	cd.pfn=pfn;
	cd.pParam=pParam;

	m_lock.lock();
	m_lCalls.push_back(cd);
	m_lock.unlock();
}

/** Disconnects the user callback function if connected earlier.
 * \param[in] pfn - address of a function to remove
 */
void callback1::disconnect(PFNCALLBACKPROC1 pfn)
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

callback2::callback2()
{
	
}

callback2::~callback2()
{
	
}

/** Executes a callback list associated with this object.
 * \param[in] pData - first parameter that will be passed to a user callback function
 * \param[in] pData2 - second parameter that will be passed to a user callback function
 */
void callback2::exec(ptr_t pData, ptr_t pData2)
{
	m_lock.lock();
	try
	{
		for (std::list<_CALLBACKDATA2>::iterator it=m_lCalls.begin();it != m_lCalls.end();it++)
			(*((*it).pfn))((*it).pParam, pData, pData2);
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
 * \param[in] pParam - user parameter to pass to the callback function when executing
 */
void callback2::connect(PFNCALLBACKPROC2 pfn, ptr_t pParam)
{
	_CALLBACKDATA2 cd;
	cd.pfn=pfn;
	cd.pParam=pParam;

	m_lock.lock();
	m_lCalls.push_back(cd);
	m_lock.unlock();
}

/** Disconnects the user callback function if connected earlier.
 * \param[in] pfn - address of a function to remove
 */
void callback2::disconnect(PFNCALLBACKPROC2 pfn)
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

END_ICPF_NAMESPACE
