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
#include <boost/serialization/serialization.hpp>
#include <boost/bind.hpp>
#include "TPath.h"
#pragma warning(push)
#pragma warning(disable: 4996)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)
#include "../libicpf/exception.h"
#include <cctype>
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

#define DEFAULT_PATH_SEPARATOR _T("\\")

namespace details
{
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


TSmartPath TSmartPath::AppendCopy(const TSmartPath& pathToAppend, bool bEnsurePathSeparatorExists) const
{
	TSmartPath pathNew(*this);
	pathNew.Append(pathToAppend, bEnsurePathSeparatorExists);

	return pathNew;
}

TSmartPath& TSmartPath::Append(const TSmartPath& pathToAppend, bool bEnsurePathSeparatorExists)
{
	// if there is no path inside rPath, then there is no point in doing anything
	if(pathToAppend.m_pPath && pathToAppend.m_pPath->m_strPath.GetLength() > 0)
	{
		// if this path is empty, then optimize by just assigning the input path to this one
		if(!m_pPath || m_pPath->m_strPath.GetLength() == 0)
			*this = pathToAppend;
		else
		{
			// both paths are not empty - do regular concatenation
			PrepareToWrite();

			if(bEnsurePathSeparatorExists)
			{
				// detect separators
				bool bThisEndsWithSeparator = EndsWithSeparator();
				bool bInStartsWithSeparator = pathToAppend.StartsWithSeparator();

				if(!bThisEndsWithSeparator && !bInStartsWithSeparator)
					m_pPath->m_strPath += _T("\\") + pathToAppend.m_pPath->m_strPath;
				else if(bThisEndsWithSeparator ^ bInStartsWithSeparator)
					m_pPath->m_strPath += pathToAppend.m_pPath->m_strPath;
				else
				{
					m_pPath->m_strPath.Delete(m_pPath->m_strPath.GetLength() - 1, 1);
					m_pPath->m_strPath += pathToAppend.m_pPath->m_strPath;
				}
			}
			else
				m_pPath->m_strPath += pathToAppend.m_pPath->m_strPath;
		}
	}

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
	return Compare(rPath) == 0;
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
	return Compare(rPath) < 0;
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
	return Compare(rPath) > 0;
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
	return AppendCopy(rPath, true);
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
	return Append(rPath, true);
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
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

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
void TSmartPath::FromString(const TString& strPath)
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
		return m_pPath->m_strPath;
	return _T("");
}

// ============================================================================
/// chcore::TSmartPath::ToString
/// @date 2010/10/12
///
/// @brief     Retrieves the string containing path.
/// @return    String containing path.
// ============================================================================
TString TSmartPath::ToWString() const
{
	TString wstrPath;
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
int TSmartPath::Compare(const TSmartPath& rPath, bool bCaseSensitive) const
{
	if(m_pPath == rPath.m_pPath)
		return 0;
	else if(m_pPath == NULL || rPath.m_pPath == NULL)
	{
		TString strThis = m_pPath ? m_pPath->m_strPath : _T("");
		TString strOther = rPath.m_pPath ? rPath.m_pPath->m_strPath : _T("");

		if(bCaseSensitive)
			return strThis.Compare(strOther);
		else
			return strThis.CompareNoCase(strOther);
	}
	else
	{
		if(bCaseSensitive)
			return m_pPath->m_strPath.Compare(rPath.m_pPath->m_strPath);
		else
			return m_pPath->m_strPath.CompareNoCase(rPath.m_pPath->m_strPath);
	}
}

// ============================================================================
/// TSmartPath::SplitPath
/// @date 2011/04/05
///
/// @brief     Splits path to components.
/// @param[in] vComponents - receives the split path.
// ============================================================================
void TSmartPath::SplitPath(TPathContainer& vComponents) const
{
	vComponents.Clear();

	if(IsNetworkPath())
	{
		// server name first
		vComponents.Add(GetServerName());

		// now the split directories
		TPathContainer vDirSplit;
		TSmartPath spDir = GetFileDir();
		spDir.SplitPath(vDirSplit);

		vComponents.Append(vDirSplit);

		// and file name last
		vComponents.Add(GetFileName());
	}
	else
	{
		TStringArray vStrings;
		m_pPath->m_strPath.Split(_T("\\/"), vStrings);

		for(size_t stIndex = 0; stIndex < vStrings.GetCount(); ++stIndex)
		{
			const TString& strComponent = vStrings.GetAt(stIndex);
			if(!strComponent.IsEmpty())
				vComponents.Add(PathFromWString(strComponent));
		}
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
		return m_pPath->m_strPath.StartsWith(rPath.m_pPath->m_strPath);
	else
		return m_pPath->m_strPath.StartsWithNoCase(rPath.m_pPath->m_strPath);
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
bool TSmartPath::MakeRelativePath(const TSmartPath& rReferenceBasePath, bool bCaseSensitive)
{
	if(!m_pPath || !rReferenceBasePath.m_pPath)
		return true;		// nothing to do; in this case we might as well treat the path as relative one

	bool bStartsWith = false;
	if(bCaseSensitive)
		bStartsWith = m_pPath->m_strPath.StartsWith(rReferenceBasePath.m_pPath->m_strPath);
	else
		bStartsWith = m_pPath->m_strPath.StartsWithNoCase(rReferenceBasePath.m_pPath->m_strPath);

	if(bStartsWith)
	{
		PrepareToWrite();
		m_pPath->m_strPath.Delete(0, rReferenceBasePath.m_pPath->m_strPath.GetLength());
		return true;
	}
	else
		return false;
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
		bEndsWith = m_pPath && m_pPath->m_strPath.EndsWith(pszPostfix);
	else
		bEndsWith = m_pPath && m_pPath->m_strPath.EndsWithNoCase(pszPostfix);

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
		bEndsWith = m_pPath && m_pPath->m_strPath.EndsWith(pszPostfix);
	else
		bEndsWith = m_pPath && m_pPath->m_strPath.EndsWithNoCase(pszPostfix);

	if(bEndsWith)
	{
		PrepareToWrite();
		m_pPath->m_strPath.Delete(m_pPath->m_strPath.GetLength() - _tcslen(pszPostfix), m_pPath->m_strPath.GetLength() - _tcslen(pszPostfix));
	}
}

// ============================================================================
/// chcore::TSmartPath::IsNetworkPath
/// @date 2010/10/17
///
/// @brief     Checks if the path is network one (\\server_name...)
/// @return    True if it is, false otherwise.
// ============================================================================
bool TSmartPath::IsNetworkPath() const
{
	if(!m_pPath)
		return false;

	return (m_pPath->m_strPath.GetLength() > 2 && IsSeparator(m_pPath->m_strPath.GetAt(0)) && IsSeparator(m_pPath->m_strPath.GetAt(1)));		// "\\server_name"
}

// ============================================================================
/// chcore::TSmartPath::IsDrive
/// @date 2011/04/05
///
/// @brief     Checks if this path contains only drive specification (i.e. c:)
/// @return    True if it is, false otherwise.
// ============================================================================
bool TSmartPath::IsDrive() const
{
	if(!m_pPath)
		return false;

	return (m_pPath->m_strPath.GetLength() == 2 && m_pPath->m_strPath.GetAt(1) == _T(':'));
}

// ============================================================================
/// chcore::TSmartPath::HasDrive
/// @date 2010/10/16
///
/// @brief     Checks if path has a drive component.
/// @return    True if it has, false otherwise.
// ============================================================================
bool TSmartPath::HasDrive() const
{
	if(!m_pPath)
		return false;

	return (m_pPath->m_strPath.GetLength() >= 2 && m_pPath->m_strPath.GetAt(1) == _T(':'));
}

// ============================================================================
/// chcore::TSmartPath::GetDrive
/// @date 2010/10/16
///
/// @brief     Retrieves drive from path.
/// @return    Path with drive, empty if drive does not exist.
// ============================================================================
TSmartPath TSmartPath::GetDrive() const
{
	if(!m_pPath)
		return TSmartPath();

	if(m_pPath->m_strPath.GetLength() >= 2 && m_pPath->m_strPath.GetAt(1) == _T(':'))
	{
		if(m_pPath->m_strPath.GetLength() == 2)
			return *this;
		else
			return PathFromWString(m_pPath->m_strPath.Left(2));	// c: for c:\windows\test.cpp
	}

	return TSmartPath();
}

// ============================================================================
/// chcore::TSmartPath::GetDriveLetter
/// @date 2011/07/17
///
/// @brief     Retrieves drive letter from path.
/// @return    Drive letter or zero in case path does not have drive.
// ============================================================================
wchar_t TSmartPath::GetDriveLetter() const
{
	if(!m_pPath)
		return L'\0';

	if(m_pPath->m_strPath.GetLength() >= 2 && m_pPath->m_strPath.GetAt(1) == _T(':'))
	{
		wchar_t wchDrive = m_pPath->m_strPath.GetAt(0);
		if(wchDrive >= L'a' && wchDrive <= L'z')
			wchDrive = L'A' + wchDrive - L'a';
		return wchDrive;
	}

	return L'\0';
}

// ============================================================================
/// chcore::TSmartPath::IsServerName
/// @date 2011/04/05
///
/// @brief     Checks if this path contains only the server specification (i.e. \\server - witn no ending backslash)
/// @return    True is this path contains only server specification.
// ============================================================================
bool TSmartPath::IsServerName() const
{
	if(!m_pPath)
		return false;

	return (m_pPath->m_strPath.GetLength() > 2 &&			// must have at least 3 characters...
		IsSeparator(m_pPath->m_strPath.GetAt(0)) && IsSeparator(m_pPath->m_strPath.GetAt(1)) &&	// ... the first two of which are separators...
		std::isalnum(m_pPath->m_strPath.GetAt(2)) &&											// ... followed by at least one alphanumeric character...
		m_pPath->m_strPath.FindFirstOf(_T("\\/"), 3) == TString::npos);								// ... with no additional separators (so \\abc is true, \\abc\ is not).
}

// ============================================================================
/// chcore::TSmartPath::HasServerName
/// @date 2010/10/17
///
/// @brief     
/// @return    
// ============================================================================
bool TSmartPath::HasServerName() const
{
	if(!m_pPath)
		return false;

	return (m_pPath->m_strPath.GetLength() > 2 && IsSeparator(m_pPath->m_strPath.GetAt(0)) && IsSeparator(m_pPath->m_strPath.GetAt(1)) && std::isalnum(m_pPath->m_strPath.GetAt(2)));
}

// ============================================================================
/// chcore::TSmartPath::GetServerName
/// @date 2010/10/17
///
/// @brief     Retrieves server name from path (if network path).
/// @return    Path containing server name (with prepended \\)
// ============================================================================
TSmartPath TSmartPath::GetServerName() const
{
	if(!m_pPath)
		return TSmartPath();

	TString wstrPath;
	if(m_pPath->m_strPath.GetLength() > 2 && IsSeparator(m_pPath->m_strPath.GetAt(0)) && IsSeparator(m_pPath->m_strPath.GetAt(1)) && std::isalnum(m_pPath->m_strPath.GetAt(2)))
	{
		size_t stEndPos = m_pPath->m_strPath.FindFirstOf(_T("\\/"), 2);
		if(stEndPos == TString::npos)
			wstrPath = m_pPath->m_strPath;
		else
			wstrPath = m_pPath->m_strPath.Left(stEndPos);
		return PathFromWString(wstrPath);
	}

	return TSmartPath();
}

// ============================================================================
/// chcore::TSmartPath::HasFileRoot
/// @date 2010/10/17
///
/// @brief     Checks if this path has a file root part.
/// @return    True if it has, false otherwise.
// ============================================================================
bool TSmartPath::HasFileRoot() const
{
	if(!m_pPath)
		return false;

	size_t stIndex = m_pPath->m_strPath.FindLastOf(_T("\\/"));
	return (stIndex != TString::npos);
}

// ============================================================================
/// chcore::TSmartPath::GetFileRoot
/// @date 2010/10/17
///
/// @brief     Retrieves the root of the file.
/// @return    File root as path, empty path if does not exist.
// ============================================================================
TSmartPath TSmartPath::GetFileRoot() const
{
	if(!m_pPath)
		return TSmartPath();

	size_t stIndex = m_pPath->m_strPath.FindLastOf(_T("\\/"));
	if(stIndex != TString::npos)
		return PathFromWString(m_pPath->m_strPath.Left(stIndex + 1));

	return TSmartPath();
}

// ============================================================================
/// chcore::TSmartPath::HasFileDir
/// @date 2010/10/16
///
/// @brief     Checks if path contains directory specification.
/// @return	   True if it contains one, false otherwise.
// ============================================================================
bool TSmartPath::HasFileDir() const
{
	if(!m_pPath)
		return false;

	size_t stStart = 0;
	if(IsNetworkPath())
		stStart = m_pPath->m_strPath.FindFirstOf(_T("/\\"), 2);
	else
		stStart = m_pPath->m_strPath.FindFirstOf(_T("/\\"));

	size_t stEnd = m_pPath->m_strPath.FindLastOf(_T("/\\"));
	return (stStart != TString::npos && stEnd >= stStart);
}

// ============================================================================
/// chcore::TSmartPath::GetFileDir
/// @date 2010/10/16
///
/// @brief     Retrieves the directory specification from path.
/// @return    Directory specification, empty path if not found.
// ============================================================================
TSmartPath TSmartPath::GetFileDir() const
{
	if(!m_pPath)
		return TSmartPath();

	size_t stStart = 0;
	if(IsNetworkPath())
		stStart = m_pPath->m_strPath.FindFirstOf(_T("/\\"), 2);
	else if(HasDrive())
		stStart = m_pPath->m_strPath.FindFirstOf(_T("/\\"));
	else
		stStart = 0;

	size_t stEnd = m_pPath->m_strPath.FindLastOf(_T("/\\"));
	if(stStart != TString::npos && stEnd >= stStart)
		return PathFromWString(m_pPath->m_strPath.MidRange(stStart, stEnd + 1));

	return TSmartPath();
}

// ============================================================================
/// chcore::TSmartPath::HasFileTitle
/// @date 2010/10/16
///
/// @brief     Checks if the path has file title part.
/// @return    True if it has one, false otherwise.
// ============================================================================
bool TSmartPath::HasFileTitle() const
{
	if(!m_pPath)
		return false;

	size_t stStart = m_pPath->m_strPath.FindLastOf(_T("/\\"));
	size_t stEnd = m_pPath->m_strPath.FindLastOf(_T("."));
	if((stStart == TString::npos && stEnd == TString::npos))
		return !IsEmpty();
	if(stStart == TString::npos)	// if does not exist, start from beginning
		stStart = 0;
	if(stEnd == TString::npos || stEnd < stStart)		// if does not exist or we have ".\\", use up to the end
		stEnd = m_pPath->m_strPath.GetLength();

	return stEnd > stStart + 1;
}

// ============================================================================
/// chcore::TSmartPath::GetFileTitle
/// @date 2010/10/16
///
/// @brief     Retrieves file title from path.
/// @return    File title. Empty if does not exist.
// ============================================================================
TSmartPath TSmartPath::GetFileTitle() const
{
	if(!m_pPath)
		return TSmartPath();

	size_t stStart = m_pPath->m_strPath.FindLastOf(_T("/\\"));
	size_t stEnd = m_pPath->m_strPath.FindLastOf(_T("."));
	if((stStart == TString::npos && stEnd == TString::npos))
		return *this;
	if(stStart == TString::npos)	// if does not exist, start from beginning
		stStart = 0;
	else
		++stStart;
	if(stEnd == TString::npos || stEnd < stStart)		// if does not exist or we have ".\\", use up to the end
		stEnd = m_pPath->m_strPath.GetLength();

	return PathFromWString(m_pPath->m_strPath.MidRange(stStart, stEnd));
}

// ============================================================================
/// chcore::TSmartPath::HasExtension
/// @date 2010/10/16
///
/// @brief     Checks if this path has a file extension.
/// @return    True if it has, false otherwise.
// ============================================================================
bool TSmartPath::HasExtension() const
{
	if(!m_pPath)
		return false;

	size_t stIndex = m_pPath->m_strPath.FindLastOf(_T("\\/."));

	return stIndex != TString::npos && (m_pPath->m_strPath.GetAt(stIndex) == _T('.'));
}

// ============================================================================
/// chcore::TSmartPath::GetExtension
/// @date 2010/10/16
///
/// @brief     Retrieves file extension from this path.
/// @return    Extension part or empty if does not exist.
// ============================================================================
TSmartPath TSmartPath::GetExtension() const
{
	if(!m_pPath)
		return TSmartPath();

	size_t stIndex = m_pPath->m_strPath.FindLastOf(_T("\\/."));

	if(stIndex != TString::npos && m_pPath->m_strPath.GetAt(stIndex) == _T('.'))
		return PathFromWString(m_pPath->m_strPath.MidRange(stIndex, m_pPath->m_strPath.GetLength()));	// ".txt" for "c:\windows\test.txt"

	return TSmartPath();
}

// ============================================================================
/// chcore::TSmartPath::HasFileName
/// @date 2010/10/16
///
/// @brief     Checks if this path contains filename part.
/// @return    True if filename exists, false otherwise.
// ============================================================================
bool TSmartPath::HasFileName() const
{
	if(!m_pPath)
		return false;

	size_t stIndex = m_pPath->m_strPath.FindLastOf(_T("\\/"));
	if(stIndex == TString::npos)	// no path separator?
		return true;
	else
		return (stIndex != TString::npos && stIndex != m_pPath->m_strPath.GetLength() - 1);
}

// ============================================================================
/// chcore::TSmartPath::GetFileName
/// @date 2010/10/16
///
/// @brief     Retrieves filename part of this path.
/// @return    Filename, or empty if does not exist.
// ============================================================================
TSmartPath TSmartPath::GetFileName() const
{
	if(!m_pPath)
		return TSmartPath();

	size_t stIndex = m_pPath->m_strPath.FindLastOf(_T("\\/"));
	if(stIndex != TString::npos)
		return PathFromWString(m_pPath->m_strPath.MidRange(stIndex + 1, m_pPath->m_strPath.GetLength()));	// "test.txt" for "c:\windows\test.txt"
	else
		return *this;
}

// ============================================================================
/// chcore::TSmartPath::DeleteFileName
/// @date 2010/10/17
///
/// @brief     Deletes the filename part of this path if exists.
// ============================================================================
void TSmartPath::DeleteFileName()
{
	if(!m_pPath)
		return;

	size_t stIndex = m_pPath->m_strPath.FindLastOf(_T("\\/"));
	if(stIndex != TString::npos)
	{
		PrepareToWrite();
		m_pPath->m_strPath.Delete(stIndex + 1, m_pPath->m_strPath.GetLength() - stIndex - 1);	// "test.txt" for "c:\windows\test.txt"
	}
	else
	{
		// no path separator inside - everything in this path is a filename
		Clear();
	}
}

// ============================================================================
/// chcore::TSmartPath::EndsWithSeparator
/// @date 2010/10/16
///
/// @brief     Checks if path end with a path separator (/ or \)
/// @return    True if path ends with separator, false otherwise.
// ============================================================================
bool TSmartPath::EndsWithSeparator() const
{
	if(!m_pPath)
		return false;

	size_t stThisSize = m_pPath->m_strPath.GetLength();
	if(stThisSize > 0)
	{
		wchar_t wchLastChar = m_pPath->m_strPath.GetAt(stThisSize - 1);
		return (wchLastChar == _T('\\') || wchLastChar == _T('/'));
	}

	return false;
}

// ============================================================================
/// chcore::TSmartPath::AppendSeparatorIfDoesNotExist
/// @date 2010/10/16
///
/// @brief     Appends separator to this path if does not exist already.
// ============================================================================
void TSmartPath::AppendSeparatorIfDoesNotExist()
{
	if(!EndsWithSeparator())
	{
		PrepareToWrite();
		m_pPath->m_strPath += _T("\\");
	}
}

// ============================================================================
/// chcore::TSmartPath::StripSeparatorAtEnd
/// @date 2010/10/17
///
/// @brief     Strips separator at the end of path if exists.
// ============================================================================
void TSmartPath::StripSeparatorAtEnd()
{
	if(EndsWithSeparator())
	{
		PrepareToWrite();
		m_pPath->m_strPath.Delete(m_pPath->m_strPath.GetLength() - 1, 1);
	}
}

// ============================================================================
/// chcore::TSmartPath::StartsWithSeparator
/// @date 2010/10/16
///
/// @brief     Checks if path starts with a separator.
/// @return    True if path starts with separator, false otherwise.
// ============================================================================
bool TSmartPath::StartsWithSeparator() const
{
	if(!m_pPath)
		return false;

	wchar_t wchLastChar = 0;
	if(m_pPath->m_strPath.GetLength() > 0)
		wchLastChar = m_pPath->m_strPath.GetAt(0);

	return (wchLastChar == _T('\\') || wchLastChar == _T('/'));
}

// ============================================================================
/// chcore::TSmartPath::PrependSeparatorIfDoesNotExist
/// @date 2010/10/17
///
/// @brief     Prepends a separator to this path if not exist already.
// ============================================================================
void TSmartPath::PrependSeparatorIfDoesNotExist()
{
	if(!StartsWithSeparator())
	{
		PrepareToWrite();
		m_pPath->m_strPath = _T("\\") + m_pPath->m_strPath;
	}
}

// ============================================================================
/// chcore::TSmartPath::StripSeparatorAtFront
/// @date 2010/10/17
///
/// @brief     Strips separator at the front of this path (if exists).
// ============================================================================
void TSmartPath::StripSeparatorAtFront()
{
	if(StartsWithSeparator())
	{
		PrepareToWrite();

		m_pPath->m_strPath.Delete(0, 1);
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
	return !m_pPath || m_pPath->m_strPath.IsEmpty();
}

// ============================================================================
/// chcore::TSmartPath::GetLength
/// @date 2011/04/05
///
/// @brief     Retrieves path length in characters.
/// @return    Path length.
// ============================================================================
size_t TSmartPath::GetLength() const
{
	if(!m_pPath)
		return 0;
	return m_pPath->m_strPath.GetLength();
}

void TSmartPath::Serialize(TReadBinarySerializer& rSerializer)
{
	PrepareToWrite();
	Serializers::Serialize(rSerializer, m_pPath->m_strPath);
}

void TSmartPath::Serialize(TWriteBinarySerializer& rSerializer) const
{
	if(m_pPath)
		Serializers::Serialize(rSerializer, m_pPath->m_strPath);
	else
		Serializers::Serialize(rSerializer, TString());
}

// ============================================================================
/// chcore::TSmartPath::StoreInConfig
/// @date 2011/04/05
///
/// @brief     Stores the path in configuration file.
/// @param[in] rConfig - configuration object to store information in.
/// @param[in] pszPropName - property name under which to store the path.
// ============================================================================
void TSmartPath::StoreInConfig(TConfig& rConfig, PCTSTR pszPropName) const
{
	rConfig.SetValue(pszPropName, m_pPath ? m_pPath->m_strPath : TString());
}

// ============================================================================
/// chcore::TSmartPath::ReadFromConfig
/// @date 2011/04/05
///
/// @brief     Reads a path from configuration file.
/// @param[in] rConfig - configuration object to read path from.
/// @param[in] pszPropName - property name from under which to read the path.
/// @return    True if path properly read, false otherwise.
// ============================================================================
bool TSmartPath::ReadFromConfig(const TConfig& rConfig, PCTSTR pszPropName)
{
	TString wstrPath;
	if(rConfig.GetValue(pszPropName, wstrPath))
	{
		PrepareToWrite();
		m_pPath->m_strPath = wstrPath;
		return true;
	}
	else
		return false;
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
		details::TPath* pPath = m_pPath->Clone();
		Clear();
		m_pPath = pPath;
	}

	// create new internal path if does not exist
	if(!m_pPath)
		m_pPath = details::TPath::New();
}

// ============================================================================
/// chcore::TSmartPath::IsSeparator
/// @date 2010/10/17
///
/// @brief     Checks if the character is a separator.
/// @param[in] wchSeparator - Character to be checked.
/// @return    True if it is a separator, false otherwise.
// ============================================================================
bool TSmartPath::IsSeparator(wchar_t wchSeparator)
{
	return (wchSeparator == _T('\\') || wchSeparator == _T('/'));
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
/// chcore::PathFromWString
/// @date 2010/10/12
///
/// @brief     Creates a path object from string.
/// @param[in] pszPath - string containing path.
/// @return    New path object.
// ============================================================================
TSmartPath PathFromWString(const TString& strPath)
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
	if(stIndex > m_vPaths.size())
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

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
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

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
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

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
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

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

void TPathContainer::StoreInConfig(TConfig& rConfig, PCTSTR pszPropName) const
{
	TStringArray vPaths;

	// store as vector of strings (ineffective; should be done better)
	BOOST_FOREACH(const TSmartPath& spPath, m_vPaths)
	{
		vPaths.Add(spPath.ToWString());
	}

	rConfig.SetValue(pszPropName, vPaths);
}

bool TPathContainer::ReadFromConfig(const TConfig& rConfig, PCTSTR pszPropName)
{
	m_vPaths.clear();

	TStringArray vPaths;
	if(rConfig.GetValue(pszPropName, vPaths))
	{
		for(size_t stIndex = 0; stIndex < vPaths.GetCount(); ++stIndex)
		{
			m_vPaths.push_back(PathFromWString(vPaths.GetAt(stIndex)));
		}
		return true;
	}
	else
		return false;
}

END_CHCORE_NAMESPACE
