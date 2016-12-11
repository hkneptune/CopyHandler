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
#include "stdafx.h"
#include "TPathContainer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "../libstring/TStringArray.h"

namespace chcore
{
	// ============================================================================
	/// TPathContainer::TPathContainer
	/// @date 2009/11/30
	///
	/// @brief     Constructs an empty path container object.
	// ============================================================================
	TPathContainer::TPathContainer() :
		m_vPaths()
	{
	}

	// ============================================================================
	/// TPathContainer::~TPathContainer
	/// @date 2009/11/30
	///
	/// @brief     Destructs this path container object.
	// ============================================================================
	TPathContainer::~TPathContainer()
	{
	}

	// ============================================================================
	/// TPathContainer::Add
	/// @date 2009/11/30
	///
	/// @brief     Adds a path to the end of list.
	/// @param[in] spPath - path to be added.
	// ============================================================================
	void TPathContainer::Add(const TSmartPath& spPath)
	{
		m_vPaths.push_back(spPath);
	}

	void TPathContainer::Append(const TPathContainer& vPaths)
	{
		m_vPaths.insert(m_vPaths.end(), vPaths.m_vPaths.begin(), vPaths.m_vPaths.end());
	}

	// ============================================================================
	/// TPathContainer::GetAt
	/// @date 2009/11/30
	///
	/// @brief     Retrieves path at specified index.
	/// @param[in] stIndex - index at which to retrieve item.
	/// @return    Reference to the path object.
	// ============================================================================
	const TSmartPath& TPathContainer::GetAt(size_t stIndex) const
	{
		if (stIndex > m_vPaths.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		return m_vPaths.at(stIndex);
	}

	// ============================================================================
	/// TPathContainer::GetAt
	/// @date 2009/11/30
	///
	/// @brief     Retrieves path at specified index.
	/// @param[in] stIndex - index at which to retrieve item.
	/// @return    Reference to the path object.
	// ============================================================================
	TSmartPath& TPathContainer::GetAt(size_t stIndex)
	{
		if (stIndex > m_vPaths.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		return m_vPaths.at(stIndex);
	}

	// ============================================================================
	/// TPathContainer::SetAt
	/// @date 2009/11/30
	///
	/// @brief     Sets a path at a specified index.
	/// @param[in] stIndex - index at which to set the path.
	/// @param[in] spPath -  path to be set.
	// ============================================================================
	void TPathContainer::SetAt(size_t stIndex, const TSmartPath& spPath)
	{
		if (stIndex > m_vPaths.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		m_vPaths[stIndex] = spPath;
	}

	// ============================================================================
	/// TPathContainer::DeleteAt
	/// @date 2009/11/30
	///
	/// @brief     Removes a path from container at specified index.
	/// @param[in] stIndex - index at which to delete.
	// ============================================================================
	void TPathContainer::DeleteAt(size_t stIndex)
	{
		if (stIndex > m_vPaths.size())
			throw TCoreException(eErr_BoundsExceeded, L"stIndex", LOCATION);

		m_vPaths.erase(m_vPaths.begin() + stIndex);
	}

	// ============================================================================
	/// TPathContainer::Clear
	/// @date 2009/11/30
	///
	/// @brief     Removes all paths from this container.
	// ============================================================================
	void TPathContainer::Clear()
	{
		m_vPaths.clear();
	}

	// ============================================================================
	/// TPathContainer::GetCount
	/// @date 2009/11/30
	///
	/// @brief     Retrieves count of elements in the container.
	/// @return    Count of elements.
	// ============================================================================
	size_t TPathContainer::GetCount() const
	{
		return m_vPaths.size();
	}

	// ============================================================================
	/// TPathContainer::GetCount
	/// @date 2010/10/12
	///
	/// @brief     Retrieves info if this container is empty.
	/// @return    True if empty, false otherwise.
	// ============================================================================
	bool TPathContainer::IsEmpty() const
	{
		return m_vPaths.empty();
	}
}
