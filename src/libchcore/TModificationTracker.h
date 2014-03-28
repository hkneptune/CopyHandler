// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#ifndef __TMODIFICATIONTRACKER_H__
#define __TMODIFICATIONTRACKER_H__

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

template<class T>
class TModificationTracker
{
public:
	TModificationTracker() :
		m_tValue(),
		m_bModified(false)
	{
	}

	template<class V>
	TModificationTracker(const V& rValue) :
		m_tValue(rValue),
		m_bModified(true)
	{
	}

	template<class V>
	TModificationTracker& operator=(const V& rValue)
	{
		m_tValue = rValue;
		m_bModified = true;
	}

	operator const T&() const
	{
		return m_tValue;
	}

	operator T&() const
	{
		m_bModified = true;
		return m_tValue;
	}

	bool IsModified() const
	{
		return m_bModified;
	}

private:
	T m_tValue;
	bool m_bModified;
};

template<class T>
class TSharedModificationTracker
{
private:
	TSharedModificationTracker<T>& operator=(const TSharedModificationTracker<T>& rValue);

public:
	TSharedModificationTracker(bool& rbSharedFlag) : m_tValue(), m_rbModified(rbSharedFlag)
	{
	}

	template<class V>
	TSharedModificationTracker(const V& rValue, bool& rbSharedFlag) : m_tValue(rValue), m_rbModified(rbSharedFlag)
	{
	}

	template<class V>
	TSharedModificationTracker& operator=(const V& rValue)
	{
		m_tValue = rValue;
		m_rbModified = true;

		return *this;
	}

	operator const T&() const
	{
		return m_tValue;
	}

	operator T&()
	{
		m_rbModified = true;
		return m_tValue;
	}

	const T* operator->() const
	{
		return &m_tValue;
	}

	T* operator->()
	{
		m_rbModified = true;
		return &m_tValue;
	}

	bool IsModified() const
	{
		return m_rbModified;
	}

private:
	T m_tValue;
	bool& m_rbModified;
};

END_CHCORE_NAMESPACE

#endif
