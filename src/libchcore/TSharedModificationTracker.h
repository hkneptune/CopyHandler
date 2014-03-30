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
#ifndef __TSHAREDMODIFICATIONTRACKER_H__
#define __TSHAREDMODIFICATIONTRACKER_H__

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

template<class T>
class TSharedModificationTracker
{
public:
	TSharedModificationTracker(bool& rbSharedFlag) :
		m_tValue(),
		m_rbModified(rbSharedFlag)
	{
	}

	TSharedModificationTracker(const TSharedModificationTracker<T>& rSrc) :
		m_tValue(rSrc.m_tValue),
		m_rbModified(rSrc.m_rbModified)
	{
	}

	template<class V>
	TSharedModificationTracker(const V& rValue, bool& rbSharedFlag) :
		m_tValue(rValue),
		m_rbModified(rbSharedFlag)
	{
	}

	TSharedModificationTracker& operator=(const TSharedModificationTracker<T>& rValue)
	{
		if(this != &rValue)
		{
			m_rbModified = rValue.m_rbModified;
			m_tValue = rValue.m_tValue;
		}

		return *this;
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

	T& Value()
	{
		m_rbModified = true;
		return m_tValue;
	}

	const T* operator->() const
	{
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
