/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
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
#include "stdafx.h"
#include "TPath.h"
#include <boost/algorithm/string.hpp>

// ============================================================================
/// TPath::TPath
/// @date 2009/11/29
///
/// @brief     Constructs the TPath object.
// ============================================================================
TPath::TPath() :
	m_lRefCount(1),
	m_strPath()
{
}

// ============================================================================
/// TPath::~TPath
/// @date 2009/11/29
///
/// @brief     Destructs the TPath object.
// ============================================================================
TPath::~TPath()
{
}

// ============================================================================
/// TPath::Release
/// @date 2009/11/29
///
/// @brief     Releases a reference to this object. Deletes the object if no reference exists.
/// @return    Current reference count.
// ============================================================================
long TPath::Release()
{
	if(--m_lRefCount == 0)
	{
		delete this;
		return 0;
	}
	return m_lRefCount;
}

// ============================================================================
/// TPath::New
/// @date 2009/11/29
///
/// @brief     Allocates a new, empty TPath object.
/// @return    Pointer to the newly allocated object.
// ============================================================================
TPath* TPath::New()
{
	return new TPath();
}

// ============================================================================
/// TPath::Delete
/// @date 2009/11/29
///
/// @brief     Deletes the TPath object 
/// @param[in] pPath - pointer to the object to delete.
// ============================================================================
void TPath::Delete(TPath* pPath)
{
	delete pPath;
}

// ============================================================================
/// TSmartPath::TSmartPath
/// @date 2009/11/29
///
/// @brief     Constructs an empty path.
// ============================================================================
TSmartPath::TSmartPath() :
	m_pPath(NULL)
{
}

// ============================================================================
/// TSmartPath::TSmartPath
/// @date 2009/11/29
///
/// @brief     Constructs path from stl string object.
/// @param[in] strPath - string containing a path.
// ============================================================================
TSmartPath::TSmartPath(const tstring_t& strPath) :
	m_pPath(TPath::New())
{
	if(m_pPath)
		m_pPath->m_strPath = strPath;
}

// ============================================================================
/// TSmartPath::TSmartPath
/// @date 2009/11/29
///
/// @brief     Constructs a path object from string.
/// @param[in] pszPath - string with path.
// ============================================================================
TSmartPath::TSmartPath(const tchar_t* pszPath) :
	m_pPath(TPath::New())
{
	if(m_pPath)
		m_pPath->m_strPath = pszPath;
}

// ============================================================================
/// TSmartPath::TSmartPath
/// @date 2009/11/29
///
/// @brief     Constructs path object from another path object.
/// @param[in] spPath - reference to another path object.
// ============================================================================
TSmartPath::TSmartPath(const TSmartPath& spPath) :
	m_pPath(spPath.m_pPath)
{
}

// ============================================================================
/// TSmartPath::~TSmartPath
/// @date 2009/11/29
///
/// @brief     
/// @return    
// ============================================================================
TSmartPath::~TSmartPath()
{
	Clear();
}

// ============================================================================
/// TSmartPath::Clear
/// @date 2009/11/29
///
/// @brief     Clears this object.
// ============================================================================
void TSmartPath::Clear() throw()
{
	if(m_pPath)
	{
		m_pPath->Release();		// Release will delete object if unused anymore
		m_pPath = NULL;
	}
}

// ============================================================================
/// TSmartPath::operator=
/// @date 2009/11/29
///
/// @brief     Assigns a path from string.
/// @param[in] strPath - string containing a path.
/// @return    Reference to this object.
// ============================================================================
TSmartPath& TSmartPath::operator=(const tstring_t& strPath)
{
	// can we get exclusive access to the member?
	// if not, clear this object
	if(m_pPath && m_pPath->IsShared())
		Clear();

	// create new internal path if does not exist
	if(!m_pPath)
		m_pPath = TPath::New();

	m_pPath->m_strPath = strPath;

	return *this;
}

// ============================================================================
/// TSmartPath::operator=
/// @date 2009/11/29
///
/// @brief     Assigns a path from string.
/// @param[in] strPath - string containing a path.
/// @return    Reference to this object.
// ============================================================================
TSmartPath& TSmartPath::operator=(const tchar_t* pszPath)
{
	// can we get exclusive access to the member?
	// if not, clear this object
	if(m_pPath && m_pPath->IsShared())
		Clear();

	// create new internal path if does not exist
	if(!m_pPath)
		m_pPath = TPath::New();

	m_pPath->m_strPath = pszPath;

	return *this;
}

// ============================================================================
/// TSmartPath::operator=
/// @date 2009/11/29
///
/// @brief     Assigns a path from other path object.
/// @param[in] spPath - path object from which we want to get path.
/// @return    Reference to this object.
// ============================================================================
TSmartPath& TSmartPath::operator=(const TSmartPath& spPath)
{
	if(this != &spPath && m_pPath != spPath.m_pPath)
	{
		Clear();
		m_pPath = spPath.m_pPath;
		m_pPath->AddRef();
	}

	return *this;
}

// ============================================================================
/// TSmartPath::operator==
/// @date 2009/11/29
///
/// @brief     Compares paths (case sensitive).
/// @param[in] rPath - path to compare this object to.
/// @return    True if equal, false otherwise.
// ============================================================================
bool TSmartPath::operator==(const TSmartPath& rPath) const
{
	if(m_pPath == rPath.m_pPath)
		return true;
	else if(m_pPath == NULL || rPath.m_pPath == NULL)
		return false;
	else
		return m_pPath->m_strPath == rPath.m_pPath->m_strPath;
}

// ============================================================================
/// TSmartPath::operator<
/// @date 2009/11/29
///
/// @brief     Compares paths (case sensitive).
/// @param[in] rPath - input path to compare.
/// @return    True if this object is less than rPath, false otherwise.
// ============================================================================
bool TSmartPath::operator<(const TSmartPath& rPath) const
{
	if(m_pPath == rPath.m_pPath)
		return false;
	else if(m_pPath == NULL || rPath.m_pPath == NULL)
		return m_pPath < rPath.m_pPath;
	else
		return m_pPath->m_strPath < rPath.m_pPath->m_strPath;
}

// ============================================================================
/// TSmartPath::operator>
/// @date 2009/11/29
///
/// @brief     Compares paths (case sensitive).
/// @param[in] rPath - input path to compare.
/// @return    True if this object is less than rPath, false otherwise.
// ============================================================================
bool TSmartPath::operator>(const TSmartPath& rPath) const
{
	if(m_pPath == rPath.m_pPath)
		return false;
	else if(m_pPath == NULL || rPath.m_pPath == NULL)
		return m_pPath > rPath.m_pPath;
	else
		return m_pPath->m_strPath > rPath.m_pPath->m_strPath;
}

// ============================================================================
/// TSmartPath::operator+
/// @date 2009/11/29
///
/// @brief     Concatenates two paths, returns the result.
/// @param[in] rPath - path to concatenate.
/// @return    New path object with the results of concatenation.
// ============================================================================
TSmartPath TSmartPath::operator+(const TSmartPath& rPath) const
{
	TSmartPath spNewPath(*this);
	if(rPath.m_pPath)
		spNewPath += rPath.m_pPath->m_strPath;

	return spNewPath;
}

// ============================================================================
/// TSmartPath::operator+=
/// @date 2009/11/29
///
/// @brief     Concatenates provided path to our own.
/// @param[in] rPath - path to concatenate.
/// @return    Reference to this object.
// ============================================================================
TSmartPath& TSmartPath::operator+=(const TSmartPath& rPath)
{
	// if there is no path inside rPath, then there is no point in doing anything
	if(rPath.m_pPath)
	{
		// can we use this object exclusively?
		if(m_pPath && m_pPath->IsShared())
			Clear();

		if(!m_pPath)
			m_pPath = TPath::New();

		m_pPath->m_strPath += rPath.m_pPath->m_strPath;
	}

	return *this;
}

// ============================================================================
/// TSmartPath::operator tstring_t
/// @date 2009/11/29
///
/// @brief     
/// @return    
// ============================================================================
TSmartPath::operator tstring_t() const
{
	tstring_t strPath;
	if(m_pPath)
		strPath = m_pPath->m_strPath;

	return strPath;
}

// ============================================================================
/// TSmartPath::Compare
/// @date 2009/11/29
///
/// @brief     Compares paths.
/// @param[in] rPath - path to compare to.
/// @return    Result of the comparison.
// ============================================================================
bool TSmartPath::Compare(const TSmartPath& rPath, bool bCaseSensitive) const
{
	if(m_pPath == rPath.m_pPath)
		return true;
	else if(m_pPath == NULL || rPath.m_pPath == NULL)
		return m_pPath == rPath.m_pPath;
	else
	{
		if(bCaseSensitive)
			return boost::equals(m_pPath->m_strPath, rPath.m_pPath->m_strPath);
		else
			return boost::iequals(m_pPath->m_strPath, rPath.m_pPath->m_strPath);
	}
}

// ============================================================================
/// TSmartPath::IsChildOf
/// @date 2009/11/29
///
/// @brief     Checks if this path starts with the path specified as parameter.
/// @param[in] rPath - path to check this one against.
/// @return    True if this path starts with the provided one, false otherwise.
// ============================================================================
bool TSmartPath::IsChildOf(const TSmartPath& rPath, bool bCaseSensitive) const
{
	if(!m_pPath || !rPath.m_pPath)
		return false;

	if(bCaseSensitive)
		return boost::starts_with(m_pPath->m_strPath, rPath.m_pPath->m_strPath);
	else
		return boost::istarts_with(m_pPath->m_strPath, rPath.m_pPath->m_strPath);
}
