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
		m_chModified(eMod_None)
	{
	}

	TModificationTracker(const TModificationTracker<T>& rSrc) :
		m_chModified(rSrc.m_chModified),
		m_tValue(rSrc.m_tValue)
	{
	}

	template<class V>
	TModificationTracker(const V& rValue, bool bAdded) :
		m_tValue(rValue),
		m_chModified((char)eMod_Modified | (bAdded ? (char)eMod_Added : (char)eMod_None))
	{
	}

	TModificationTracker& operator=(const TModificationTracker<T>& rSrc)
	{
		m_chModified = rSrc.m_chModified;
		m_tValue = rSrc.m_tValue;

		return *this;
	}

	template<class V>
	TModificationTracker& operator=(const V& rValue)
	{
		if(m_tValue != rValue)
		{
			m_tValue = rValue;
			m_chModified |= eMod_Modified;
		}

		return *this;
	}

	operator const T&() const
	{
		return m_tValue;
	}

	T& Modify()
	{
		m_chModified |= eMod_Modified;
		return m_tValue;
	}

	void ClearModifications() const
	{
		m_chModified = eMod_None;
	}

	bool IsModified() const
	{
		return m_chModified != 0;
	}

	bool IsAdded() const
	{
		return (m_chModified & eMod_Added) != 0;
	}

private:
	enum EModifiedFlags
	{
		eMod_None = 0,
		eMod_Added = 1,
		eMod_Modified = 2
	};

	T m_tValue;
	mutable char m_chModified;
};

END_CHCORE_NAMESPACE

#endif
