/***************************************************************************
*   Copyright (C) 2001-2008 by J�zef Starosczyk                           *
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
#include <boost/serialization/serialization.hpp>
#include "TPath.h"
#include <boost/algorithm/string.hpp>
#include "../libicpf/exception.h"

BEGIN_CHCORE_NAMESPACE

#define DEFAULT_PATH_SEPARATOR _T("\\")

// ============================================================================
/// TPath::TPath
/// @date 2009/11/29
///
/// @brief     Constructs the TPath object.
// ============================================================================
TPath::TPath() :
	m_strPath(),
	m_lRefCount(1)
{
}

// ============================================================================
/// TPath::TPath
/// @date 2009/11/29
///
/// @brief     Constructs the TPath object.
// ============================================================================
TPath::TPath(const TPath& rSrc) :
	m_strPath(rSrc.m_strPath),
	m_lRefCount(1)
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
/// TPath::New
/// @date 2010/10/07
///
/// @brief     Clones this object.
/// @return    Pointer to the newly allocated object.
// ============================================================================
TPath* TPath::Clone()
{
	return new TPath(*this);
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
/// @brief     Constructs path object from another path object.
/// @param[in] spPath - reference to another path object.
// ============================================================================
TSmartPath::TSmartPath(const TSmartPath& spPath) :
	m_pPath(spPath.m_pPath)
{
	if(m_pPath)
		m_pPath->AddRef();
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
	{
		spNewPath.PrepareToWrite();
		spNewPath.m_pPath->m_strPath += rPath.m_pPath->m_strPath;
	}

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
		PrepareToWrite();
		m_pPath->m_strPath += rPath.m_pPath->m_strPath;
	}

	return *this;
}

// ============================================================================
/// chcore::TSmartPath::FromString
/// @date 2010/10/12
///
/// @brief     Initializes this path object with path contained in string.
/// @param[in] pszPath - string containing path.
// ============================================================================
void TSmartPath::FromString(const wchar_t* pszPath)
{
	if(!pszPath)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	PrepareToWrite();
	m_pPath->m_strPath = pszPath;
}

// ============================================================================
/// chcore::TSmartPath::FromString
/// @date 2010/10/12
///
/// @brief     Initializes this path object with path contained in string.
/// @param[in] strPath - string containing path.
// ============================================================================
void TSmartPath::FromString(const std::wstring& strPath)
{
	PrepareToWrite();
	m_pPath->m_strPath = strPath;
}

// ============================================================================
/// chcore::TSmartPath::ToString
/// @date 2010/10/12
///
/// @brief     Retrieves the pointer to a string containing path.
/// @return    Pointer to the string containing path.
// ============================================================================
const wchar_t* TSmartPath::ToString() const
{
	if(m_pPath)
		return m_pPath->m_strPath.c_str();
	return _T("");
}

// ============================================================================
/// chcore::TSmartPath::ToString
/// @date 2010/10/12
///
/// @brief     Retrieves the string containing path.
/// @return    String containing path.
// ============================================================================
std::wstring TSmartPath::ToWString() const
{
	std::wstring wstrPath;
	if(m_pPath)
		wstrPath = m_pPath->m_strPath;
	return wstrPath;
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

// ============================================================================
/// chcore::TSmartPath::MakeRelativePath
/// @date 2010/10/12
///
/// @brief     Converts this path to be relative to the reference, base path.
/// @param[in] rReferenceBasePath - Path which will be base path to this relative path.
/// @param[in] bCaseSensitive - Compare path with case sensitivity on/off.
/// @return    True if conversion to relative path succeeded, false otherwise.
// ============================================================================
void TSmartPath::MakeRelativePath(const TSmartPath& rReferenceBasePath, bool bCaseSensitive)
{
	if(!m_pPath || !rReferenceBasePath.m_pPath)
		return;		// nothing to do; in this case we might as well treat the path as relative one

	bool bStartsWith = false;
	if(bCaseSensitive)
		bStartsWith = boost::starts_with(m_pPath->m_strPath, rReferenceBasePath.m_pPath->m_strPath);
	else
		bStartsWith = boost::istarts_with(m_pPath->m_strPath, rReferenceBasePath.m_pPath->m_strPath);

	if(bStartsWith)
	{
		PrepareToWrite();
		m_pPath->m_strPath.erase(m_pPath->m_strPath.begin(), m_pPath->m_strPath.begin() + rReferenceBasePath.m_pPath->m_strPath.length());
	}
	else
		THROW(_T("Incompatible paths"), 0, 0, 0);
}

// ============================================================================
/// TSmartPath::AppendIfNotExists
/// @date 2009/11/29
///
/// @brief     Appends a specified suffix if not present.
/// @param[in] pszPostfix - string to check against.
// ============================================================================
void TSmartPath::AppendIfNotExists(const wchar_t* pszPostfix, bool bCaseSensitive)
{
	BOOST_ASSERT(pszPostfix);
	if(!pszPostfix)
		return;

	bool bEndsWith = false;
	if(bCaseSensitive)
		bEndsWith = m_pPath && boost::ends_with(m_pPath->m_strPath, pszPostfix);
	else
		bEndsWith = m_pPath && boost::iends_with(m_pPath->m_strPath, pszPostfix);

	if(!bEndsWith)
	{
		PrepareToWrite();
		m_pPath->m_strPath += pszPostfix;
	}
}

// ============================================================================
/// TSmartPath::CutIfExists
/// @date 2010/10/07
///
/// @brief     Cuts a specified suffix if present.
/// @param[in] pszPostfix - string to check against.
// ============================================================================
void TSmartPath::CutIfExists(const wchar_t* pszPostfix, bool bCaseSensitive)
{
	BOOST_ASSERT(pszPostfix);
	if(!pszPostfix)
		return;

	bool bEndsWith = false;
	if(bCaseSensitive)
		bEndsWith = m_pPath && boost::ends_with(m_pPath->m_strPath, pszPostfix);
	else
		bEndsWith = m_pPath && boost::iends_with(m_pPath->m_strPath, pszPostfix);

	if(bEndsWith)
	{
		PrepareToWrite();
		m_pPath->m_strPath.erase(m_pPath->m_strPath.end() - _tcslen(pszPostfix), m_pPath->m_strPath.end());
	}
}

// ============================================================================
/// chcore::TSmartPath::GetLastComponent
/// @date 2010/10/12
///
/// @brief     Retrieves the last component of path (i.e. test.txt for c:\test\test.txt)
/// @return    Sub-path with the last component.
// ============================================================================
TSmartPath TSmartPath::GetLastComponent()
{
	if(!m_pPath)
		return TSmartPath();

	boost::iterator_range<std::wstring::iterator> rangeIter;
	rangeIter = boost::find_last(m_pPath->m_strPath, DEFAULT_PATH_SEPARATOR);

	std::wstring wstrData;
	wstrData.insert(wstrData.end(), rangeIter.end(), m_pPath->m_strPath.end());

	return PathFromString(wstrData);
}

bool TSmartPath::HasLengthExtension() const
{
	return m_pPath && boost::starts_with(m_pPath->m_strPath, _T("\\\\?\\"));
}

bool TSmartPath::HasDrive() const
{
	if(!m_pPath)
		return false;
	if(HasLengthExtension())
		return (m_pPath->m_strPath.length() >= 6 && m_pPath->m_strPath.at(5) == _T(':'));
	else
		return (m_pPath->m_strPath.length() >= 2 && m_pPath->m_strPath.at(1) == _T(':'));
}

TSmartPath TSmartPath::GetDrive() const
{
	if(!m_pPath)
		return TSmartPath();

	if(HasLengthExtension())
	{
		if(m_pPath->m_strPath.length() >= 6 && m_pPath->m_strPath.at(5) == _T(':'))
			return PathFromString(std::wstring(m_pPath->m_strPath.begin() + 4, m_pPath->m_strPath.begin() + 6));	// c: for c:\windows\test.cpp
	}
	else
	{
		if(m_pPath->m_strPath.length() >= 2 && m_pPath->m_strPath.at(1) == _T(':'))
		{
			if(m_pPath->m_strPath.length() == 2)
				return *this;
			else
				return PathFromString(std::wstring(m_pPath->m_strPath.begin(), m_pPath->m_strPath.begin() + 2));	// c: for c:\windows\test.cpp
		}
	}

	return TSmartPath();
}

bool TSmartPath::HasFileDir() const
{
	if(!m_pPath)
		return false;

	size_t stStart = 0;
	if(HasLengthExtension())
		stStart = m_pPath->m_strPath.find_first_of(_T("/\\"), 4);
	else
		stStart = m_pPath->m_strPath.find_first_of(_T("/\\"));

	size_t stEnd = m_pPath->m_strPath.find_last_of(_T("/\\"));
	return (stStart != std::wstring::npos && stEnd >= stStart);
}

TSmartPath TSmartPath::GetFileDir() const
{
	if(!m_pPath)
		return TSmartPath();

	size_t stStart = 0;
	if(HasLengthExtension())
		stStart = m_pPath->m_strPath.find_first_of(_T("/\\"), 4);
	else
		stStart = m_pPath->m_strPath.find_first_of(_T("/\\"));

	size_t stEnd = m_pPath->m_strPath.find_last_of(_T("/\\"));
	if(stStart != std::wstring::npos && stEnd >= stStart)
	{
		std::wstring wstrDir(m_pPath->m_strPath.begin() + stStart, m_pPath->m_strPath.begin() + stEnd + 1);
		return PathFromString(wstrDir);
	}

	return TSmartPath();
}

bool TSmartPath::HasFileTitle() const
{
	if(!m_pPath)
		return false;

	size_t stStart = m_pPath->m_strPath.find_last_of(_T("/\\"));
	size_t stEnd = m_pPath->m_strPath.find_last_of(_T("."));
	if((stStart == std::wstring::npos && stEnd == std::wstring::npos))
		return !IsEmpty();
	if(stStart == std::wstring::npos)	// if does not exist, start from beginning
		stStart = 0;
	if(stEnd == std::wstring::npos || stEnd < stStart)		// if does not exist or we have ".\\", use up to the end
		stEnd = m_pPath->m_strPath.length();

	return stEnd > stStart + 1;
}

TSmartPath TSmartPath::GetFileTitle() const
{
	if(!m_pPath)
		return TSmartPath();

	size_t stStart = m_pPath->m_strPath.find_last_of(_T("/\\"));
	size_t stEnd = m_pPath->m_strPath.find_last_of(_T("."));
	if((stStart == std::wstring::npos && stEnd == std::wstring::npos))
		return *this;
	if(stStart == std::wstring::npos)	// if does not exist, start from beginning
		stStart = 0;
	if(stEnd == std::wstring::npos || stEnd < stStart)		// if does not exist or we have ".\\", use up to the end
		stEnd = m_pPath->m_strPath.length();

	std::wstring wstrDir(m_pPath->m_strPath.begin() + stStart + 1, m_pPath->m_strPath.begin() + stEnd);
	return PathFromString(wstrDir);
}

bool TSmartPath::HasExtension() const
{
	if(!m_pPath)
		return false;

	size_t stIndex = m_pPath->m_strPath.find_last_of(_T("\\/."));

	return stIndex != std::wstring::npos && (m_pPath->m_strPath.at(stIndex) == _T('.'));
}

TSmartPath TSmartPath::GetExtension() const
{
	if(!m_pPath)
		return TSmartPath();

	size_t stIndex = m_pPath->m_strPath.find_last_of(_T("\\/."));

	if(stIndex != std::wstring::npos && m_pPath->m_strPath.at(stIndex) == _T('.'))
		return PathFromString(std::wstring(m_pPath->m_strPath.begin() + stIndex, m_pPath->m_strPath.end()));	// ".txt" for "c:\windows\test.txt"

	return TSmartPath();
}

bool TSmartPath::HasFileName() const
{
	if(!m_pPath)
		return false;

	if(HasLengthExtension())
	{
		size_t stIndex = m_pPath->m_strPath.find_last_of(_T("\\/"));
		return (stIndex != std::wstring::npos && stIndex >= 4 && stIndex != m_pPath->m_strPath.length() - 1);
	}
	else
	{
		size_t stIndex = m_pPath->m_strPath.find_last_of(_T("\\/"));
		return (stIndex != std::wstring::npos && stIndex != m_pPath->m_strPath.length() - 1);
	}
}

TSmartPath TSmartPath::GetFileName() const
{
	if(!m_pPath)
		return TSmartPath();

	if(HasLengthExtension())
	{
		size_t stIndex = m_pPath->m_strPath.find_last_of(_T("\\/"));
		if(stIndex != std::wstring::npos && stIndex >= 4)
			return PathFromString(std::wstring(m_pPath->m_strPath.begin() + stIndex, m_pPath->m_strPath.end()));	// ".txt" for "c:\windows\test.txt"
	}
	else
	{
		size_t stIndex = m_pPath->m_strPath.find_last_of(_T("\\/"));
		if(stIndex != std::wstring::npos)
			return PathFromString(std::wstring(m_pPath->m_strPath.begin() + stIndex, m_pPath->m_strPath.end()));	// ".txt" for "c:\windows\test.txt"
	}

	return TSmartPath();
}

// ============================================================================
/// chcore::TSmartPath::DeleteLastComponent
/// @date 2010/10/12
///
/// @brief     Removes the last component of a path (i.e. test.txt for c:\test\test.txt)
// ============================================================================
void TSmartPath::DeleteLastComponent()
{
	if(m_pPath)
	{
		boost::iterator_range<std::wstring::iterator> rangeIter = boost::find_last(m_pPath->m_strPath, DEFAULT_PATH_SEPARATOR);

		if(rangeIter.end() != m_pPath->m_strPath.end())
		{
			size_t stOffset = std::distance(m_pPath->m_strPath.begin(), rangeIter.end());
			PrepareToWrite();
			m_pPath->m_strPath.erase(m_pPath->m_strPath.begin() + stOffset, m_pPath->m_strPath.end());
		}
	}
}

// ============================================================================
/// TSmartPath::IsEmpty
/// @date 2010/10/07
///
/// @brief     Prepares the path to be written to.
// ============================================================================
bool TSmartPath::IsEmpty() const
{
	return !m_pPath || m_pPath->m_strPath.empty();
}

// ============================================================================
/// TSmartPath::AppendIfNotExists
/// @date 2009/11/29
///
/// @brief     Prepares the path to be written to.
// ============================================================================
void TSmartPath::PrepareToWrite()
{
	if(m_pPath && m_pPath->IsShared())
	{
		TPath* pPath = m_pPath->Clone();
		Clear();
		m_pPath = pPath;
	}

	// create new internal path if does not exist
	if(!m_pPath)
		m_pPath = TPath::New();
}

// ============================================================================
/// chcore::PathFromString
/// @date 2010/10/12
///
/// @brief     Creates a path object from string.
/// @param[in] pszPath - string containing path.
/// @return    New path object.
// ============================================================================
TSmartPath PathFromString(const wchar_t* pszPath)
{
	if(!pszPath)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	TSmartPath spPath;
	spPath.FromString(pszPath);
	return spPath;
}

// ============================================================================
/// chcore::PathFromString
/// @date 2010/10/12
///
/// @brief     Creates a path object from string.
/// @param[in] pszPath - string containing path.
/// @return    New path object.
// ============================================================================
TSmartPath PathFromString(const std::wstring& strPath)
{
	TSmartPath spPath;
	spPath.FromString(strPath);
	return spPath;
}

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
/// TPathContainer::TPathContainer
/// @date 2009/11/30
///
/// @brief     Constructs the path container object from another path container.
/// @param[in] rSrcContainer - path container to copy paths from.
// ============================================================================
TPathContainer::TPathContainer(const TPathContainer& rSrcContainer) :
	m_vPaths(rSrcContainer.m_vPaths)
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
/// TPathContainer::operator=
/// @date 2009/11/30
///
/// @brief     Assigns another path container object to this one.
/// @param[in] rSrcContainer - container with paths to copy from.
/// @return    Reference to this object.
// ============================================================================
TPathContainer& TPathContainer::operator=(const TPathContainer& rSrcContainer)
{
	if(this != &rSrcContainer)
		m_vPaths = rSrcContainer.m_vPaths;

	return *this;
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
	if(stIndex > m_vPaths.size())
		THROW_CORE_EXCEPTION(eBoundsExceeded);

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
	if(stIndex > m_vPaths.size())
		THROW_CORE_EXCEPTION(eBoundsExceeded);

	return m_vPaths.at(stIndex);
}

// ============================================================================
/// chcore::TPathContainer::SetAt
/// @date 2009/11/30
///
/// @brief     Sets a path at a specified index.
/// @param[in] stIndex - index at which to set the path.
/// @param[in] spPath -  path to be set.
// ============================================================================
void TPathContainer::SetAt(size_t stIndex, const TSmartPath& spPath)
{
	if(stIndex > m_vPaths.size())
		THROW_CORE_EXCEPTION(eBoundsExceeded);
	
	m_vPaths[stIndex] = spPath;
}

// ============================================================================
/// chcore::TPathContainer::DeleteAt
/// @date 2009/11/30
///
/// @brief     Removes a path from container at specified index.
/// @param[in] stIndex - index at which to delete.
// ============================================================================
void TPathContainer::DeleteAt(size_t stIndex)
{
	if(stIndex > m_vPaths.size())
		THROW_CORE_EXCEPTION(eBoundsExceeded);

	m_vPaths.erase(m_vPaths.begin() + stIndex);
}

// ============================================================================
/// chcore::TPathContainer::Clear
/// @date 2009/11/30
///
/// @brief     Removes all paths from this container.
// ============================================================================
void TPathContainer::Clear()
{
	m_vPaths.clear();
}

// ============================================================================
/// chcore::TPathContainer::GetCount
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
/// chcore::TPathContainer::GetCount
/// @date 2010/10/12
///
/// @brief     Retrieves info if this container is empty.
/// @return    True if empty, false otherwise.
// ============================================================================
bool TPathContainer::IsEmpty() const
{
	return m_vPaths.empty();
}

END_CHCORE_NAMESPACE
