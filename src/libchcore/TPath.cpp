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
#pragma warning(push)
#pragma warning(disable: 4996)
#include <boost/algorithm/string.hpp>
#pragma warning(pop)
#include <cctype>
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TPathContainer.h"
#include "TStringArray.h"

namespace chcore
{
#define DEFAULT_PATH_SEPARATOR _T("\\")

	// ============================================================================
	/// TSmartPath::TSmartPath
	/// @date 2009/11/29
	///
	/// @brief     Constructs an empty path.
	// ============================================================================
	TSmartPath::TSmartPath() :
		m_strPath()
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
	}

	// ============================================================================
	/// TSmartPath::Clear
	/// @date 2009/11/29
	///
	/// @brief     Clears this object.
	// ============================================================================
	void TSmartPath::Clear() throw()
	{
		m_strPath.Clear();
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
		if (!pathToAppend.m_strPath.IsEmpty())
		{
			// if this path is empty, then optimize by just assigning the input path to this one
			if (m_strPath.IsEmpty())
				*this = pathToAppend;
			else
			{
				if (bEnsurePathSeparatorExists)
				{
					// detect separators
					bool bThisEndsWithSeparator = EndsWithSeparator();
					bool bInStartsWithSeparator = pathToAppend.StartsWithSeparator();

					if (!bThisEndsWithSeparator && !bInStartsWithSeparator)
						m_strPath += _T("\\") + pathToAppend.m_strPath;
					else if (bThisEndsWithSeparator ^ bInStartsWithSeparator)
						m_strPath += pathToAppend.m_strPath;
					else
					{
						m_strPath.Delete(m_strPath.GetLength() - 1, 1);
						m_strPath += pathToAppend.m_strPath;
					}
				}
				else
					m_strPath += pathToAppend.m_strPath;
			}
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


	bool TSmartPath::operator!=(const TSmartPath& rPath) const
	{
		return Compare(rPath) != 0;
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
	/// TSmartPath::FromString
	/// @date 2010/10/12
	///
	/// @brief     Initializes this path object with path contained in string.
	/// @param[in] pszPath - string containing path.
	// ============================================================================
	void TSmartPath::FromString(const wchar_t* pszPath)
	{
		if (!pszPath)
			throw TCoreException(eErr_InvalidArgument, L"pszPath", LOCATION);

		m_strPath = pszPath;
	}

	// ============================================================================
	/// TSmartPath::FromString
	/// @date 2010/10/12
	///
	/// @brief     Initializes this path object with path contained in string.
	/// @param[in] strPath - string containing path.
	// ============================================================================
	void TSmartPath::FromString(const TString& strPath)
	{
		m_strPath = strPath;
	}

	// ============================================================================
	/// TSmartPath::ToString
	/// @date 2010/10/12
	///
	/// @brief     Retrieves the pointer to a string containing path.
	/// @return    Pointer to the string containing path.
	// ============================================================================
	const wchar_t* TSmartPath::ToString() const
	{
		return m_strPath.c_str();
	}

	// ============================================================================
	/// TSmartPath::ToString
	/// @date 2010/10/12
	///
	/// @brief     Retrieves the string containing path.
	/// @return    String containing path.
	// ============================================================================
	TString TSmartPath::ToWString() const
	{
		return m_strPath;
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
		if (bCaseSensitive)
			return m_strPath.Compare(rPath.m_strPath);
		else
			return m_strPath.CompareNoCase(rPath.m_strPath);
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

		if (IsNetworkPath())
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
			m_strPath.Split(_T("\\/"), vStrings);

			for (size_t stIndex = 0; stIndex < vStrings.GetCount(); ++stIndex)
			{
				const TString& strComponent = vStrings.GetAt(stIndex);
				if (!strComponent.IsEmpty())
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
		if (bCaseSensitive)
			return m_strPath.StartsWith(rPath.m_strPath.c_str());
		else
			return m_strPath.StartsWithNoCase(rPath.m_strPath.c_str());
	}

	// ============================================================================
	/// TSmartPath::MakeRelativePath
	/// @date 2010/10/12
	///
	/// @brief     Converts this path to be relative to the reference, base path.
	/// @param[in] rReferenceBasePath - Path which will be base path to this relative path.
	/// @param[in] bCaseSensitive - Compare path with case sensitivity on/off.
	/// @return    True if conversion to relative path succeeded, false otherwise.
	// ============================================================================
	bool TSmartPath::MakeRelativePath(const TSmartPath& rReferenceBasePath, bool bCaseSensitive)
	{
		bool bStartsWith = false;
		if (bCaseSensitive)
			bStartsWith = m_strPath.StartsWith(rReferenceBasePath.m_strPath.c_str());
		else
			bStartsWith = m_strPath.StartsWithNoCase(rReferenceBasePath.m_strPath.c_str());

		if (bStartsWith)
		{
			m_strPath.Delete(0, rReferenceBasePath.m_strPath.GetLength());
			return true;
		}
		else
			return false;
	}

	bool TSmartPath::MakeAbsolutePath(const TSmartPath& rReferenceBasePath)
	{
		if (!IsRelativePath())
			return false;

		bool bHasSeparator = rReferenceBasePath.EndsWithSeparator();
		if (!bHasSeparator)
			PrependSeparatorIfDoesNotExist();
		else
			StripSeparatorAtFront();

		m_strPath = rReferenceBasePath.ToString() + m_strPath;

		return true;
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
		if (!pszPostfix)
			return;

		bool bEndsWith = false;
		if (bCaseSensitive)
			bEndsWith = m_strPath.EndsWith(pszPostfix);
		else
			bEndsWith = m_strPath.EndsWithNoCase(pszPostfix);

		if (!bEndsWith)
			m_strPath += pszPostfix;
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
		if (!pszPostfix)
			return;

		bool bEndsWith = false;
		if (bCaseSensitive)
			bEndsWith = m_strPath.EndsWith(pszPostfix);
		else
			bEndsWith = m_strPath.EndsWithNoCase(pszPostfix);

		if (bEndsWith)
			m_strPath.Delete(m_strPath.GetLength() - _tcslen(pszPostfix), m_strPath.GetLength() - _tcslen(pszPostfix));
	}

	// ============================================================================
	/// TSmartPath::IsNetworkPath
	/// @date 2010/10/17
	///
	/// @brief     Checks if the path is network one (\\server_name...)
	/// @return    True if it is, false otherwise.
	// ============================================================================
	bool TSmartPath::IsNetworkPath() const
	{
		return (m_strPath.GetLength() > 2 && IsSeparator(m_strPath.GetAt(0)) && IsSeparator(m_strPath.GetAt(1)));		// "\\server_name"
	}

	// ============================================================================
	/// TSmartPath::IsDrive
	/// @date 2011/04/05
	///
	/// @brief     Checks if this path contains only drive specification (i.e. c:)
	/// @return    True if it is, false otherwise.
	// ============================================================================
	bool TSmartPath::IsDrive() const
	{
		return (m_strPath.GetLength() == 2 && m_strPath.GetAt(1) == _T(':'));
	}

	bool TSmartPath::IsDriveWithRootDir() const
	{
		return (m_strPath.GetLength() == 3 && m_strPath.GetAt(1) == _T(':') && m_strPath.GetAt(2) == _T('\\'));
	}

	// ============================================================================
	/// TSmartPath::HasDrive
	/// @date 2010/10/16
	///
	/// @brief     Checks if path has a drive component.
	/// @return    True if it has, false otherwise.
	// ============================================================================
	bool TSmartPath::HasDrive() const
	{
		return (m_strPath.GetLength() >= 2 && m_strPath.GetAt(1) == _T(':'));
	}

	// ============================================================================
	/// TSmartPath::GetDrive
	/// @date 2010/10/16
	///
	/// @brief     Retrieves drive from path.
	/// @return    Path with drive, empty if drive does not exist.
	// ============================================================================
	TSmartPath TSmartPath::GetDrive() const
	{
		if (m_strPath.GetLength() >= 2 && m_strPath.GetAt(1) == _T(':'))
		{
			if (m_strPath.GetLength() == 2)
				return *this;
			else
				return PathFromWString(m_strPath.Left(2));	// c: for c:\windows\test.cpp
		}

		return TSmartPath();
	}

	// ============================================================================
	/// TSmartPath::GetDriveLetter
	/// @date 2011/07/17
	///
	/// @brief     Retrieves drive letter from path.
	/// @return    Drive letter or zero in case path does not have drive.
	// ============================================================================
	wchar_t TSmartPath::GetDriveLetter() const
	{
		if (m_strPath.GetLength() >= 2 && m_strPath.GetAt(1) == _T(':'))
		{
			wchar_t wchDrive = m_strPath.GetAt(0);
			if (wchDrive >= L'a' && wchDrive <= L'z')
				wchDrive = L'A' + wchDrive - L'a';
			return wchDrive;
		}

		return L'\0';
	}

	TSmartPath TSmartPath::GetDriveLetterAsPath() const
	{
		if(m_strPath.GetLength() >= 2 && m_strPath.GetAt(1) == _T(':'))
			return PathFromWString(m_strPath.Left(1));	// c for c:\windows\test.cpp

		return TSmartPath();
	}

	// ============================================================================
	/// TSmartPath::IsServerName
	/// @date 2011/04/05
	///
	/// @brief     Checks if this path contains only the server specification (i.e. \\server - witn no ending backslash)
	/// @return    True is this path contains only server specification.
	// ============================================================================
	bool TSmartPath::IsServerName() const
	{
		return (m_strPath.GetLength() > 2 &&			// must have at least 3 characters...
			IsSeparator(m_strPath.GetAt(0)) && IsSeparator(m_strPath.GetAt(1)) &&	// ... the first two of which are separators...
			std::isalnum(m_strPath.GetAt(2)) &&											// ... followed by at least one alphanumeric character...
			m_strPath.FindFirstOf(_T("\\/"), 3) == TString::npos);								// ... with no additional separators (so \\abc is true, \\abc\ is not).
	}

	// ============================================================================
	/// TSmartPath::HasServerName
	/// @date 2010/10/17
	///
	/// @brief     
	/// @return    
	// ============================================================================
	bool TSmartPath::HasServerName() const
	{
		return (m_strPath.GetLength() > 2 && IsSeparator(m_strPath.GetAt(0)) && IsSeparator(m_strPath.GetAt(1)) && std::isalnum(m_strPath.GetAt(2)));
	}

	// ============================================================================
	/// TSmartPath::GetServerName
	/// @date 2010/10/17
	///
	/// @brief     Retrieves server name from path (if network path).
	/// @return    Path containing server name (with prepended \\)
	// ============================================================================
	TSmartPath TSmartPath::GetServerName() const
	{
		TString wstrPath;
		if (m_strPath.GetLength() > 2 && IsSeparator(m_strPath.GetAt(0)) && IsSeparator(m_strPath.GetAt(1)) && std::isalnum(m_strPath.GetAt(2)))
		{
			size_t stEndPos = m_strPath.FindFirstOf(_T("\\/"), 2);
			if (stEndPos == TString::npos)
				wstrPath = m_strPath;
			else
				wstrPath = m_strPath.Left(stEndPos);
			return PathFromWString(wstrPath);
		}

		return TSmartPath();
	}

	// ============================================================================
	/// TSmartPath::HasFileRoot
	/// @date 2010/10/17
	///
	/// @brief     Checks if this path has a file root part.
	/// @return    True if it has, false otherwise.
	// ============================================================================
	bool TSmartPath::HasFileRoot() const
	{
		size_t stIndex = m_strPath.FindLastOf(_T("\\/"));
		return (stIndex != TString::npos);
	}

	// ============================================================================
	/// TSmartPath::GetFileRoot
	/// @date 2010/10/17
	///
	/// @brief     Retrieves the root of the file.
	/// @return    File root as path, empty path if does not exist.
	// ============================================================================
	TSmartPath TSmartPath::GetFileRoot() const
	{
		size_t stIndex = m_strPath.FindLastOf(_T("\\/"));
		if (stIndex != TString::npos)
			return PathFromWString(m_strPath.Left(stIndex + 1));

		return TSmartPath();
	}

	// ============================================================================
	/// TSmartPath::HasFileDir
	/// @date 2010/10/16
	///
	/// @brief     Checks if path contains directory specification.
	/// @return	   True if it contains one, false otherwise.
	// ============================================================================
	bool TSmartPath::HasFileDir() const
	{
		size_t stStart = 0;
		if (IsNetworkPath())
			stStart = m_strPath.FindFirstOf(_T("/\\"), 2);
		else
			stStart = m_strPath.FindFirstOf(_T("/\\"));

		size_t stEnd = m_strPath.FindLastOf(_T("/\\"));
		return (stStart != TString::npos && stEnd >= stStart);
	}

	// ============================================================================
	/// TSmartPath::GetFileDir
	/// @date 2010/10/16
	///
	/// @brief     Retrieves the directory specification from path.
	/// @return    Directory specification, empty path if not found.
	// ============================================================================
	TSmartPath TSmartPath::GetFileDir() const
	{
		size_t stStart = 0;
		if (IsNetworkPath())
			stStart = m_strPath.FindFirstOf(_T("/\\"), 2);
		else if (HasDrive())
			stStart = m_strPath.FindFirstOf(_T("/\\"));
		else
			stStart = 0;

		size_t stEnd = m_strPath.FindLastOf(_T("/\\"));
		if (stStart != TString::npos && stEnd >= stStart)
			return PathFromWString(m_strPath.MidRange(stStart, stEnd + 1));

		return TSmartPath();
	}

	// ============================================================================
	/// TSmartPath::HasFileTitle
	/// @date 2010/10/16
	///
	/// @brief     Checks if the path has file title part.
	/// @return    True if it has one, false otherwise.
	// ============================================================================
	bool TSmartPath::HasFileTitle() const
	{
		size_t stStart = m_strPath.FindLastOf(_T("/\\"));
		size_t stEnd = m_strPath.FindLastOf(_T("."));
		if ((stStart == TString::npos && stEnd == TString::npos))
			return !IsEmpty();
		if (stStart == TString::npos)	// if does not exist, start from beginning
			stStart = 0;
		if (stEnd == TString::npos || stEnd < stStart)		// if does not exist or we have ".\\", use up to the end
			stEnd = m_strPath.GetLength();

		return stEnd > stStart + 1;
	}

	// ============================================================================
	/// TSmartPath::GetFileTitle
	/// @date 2010/10/16
	///
	/// @brief     Retrieves file title from path.
	/// @return    File title. Empty if does not exist.
	// ============================================================================
	TSmartPath TSmartPath::GetFileTitle() const
	{
		size_t stStart = m_strPath.FindLastOf(_T("/\\"));
		size_t stEnd = m_strPath.FindLastOf(_T("."));
		if ((stStart == TString::npos && stEnd == TString::npos))
			return *this;
		if (stStart == TString::npos)	// if does not exist, start from beginning
			stStart = 0;
		else
			++stStart;
		if (stEnd == TString::npos || stEnd < stStart)		// if does not exist or we have ".\\", use up to the end
			stEnd = m_strPath.GetLength();

		return PathFromWString(m_strPath.MidRange(stStart, stEnd));
	}

	// ============================================================================
	/// TSmartPath::HasExtension
	/// @date 2010/10/16
	///
	/// @brief     Checks if this path has a file extension.
	/// @return    True if it has, false otherwise.
	// ============================================================================
	bool TSmartPath::HasExtension() const
	{
		size_t stIndex = m_strPath.FindLastOf(_T("\\/."));

		return stIndex != TString::npos && (m_strPath.GetAt(stIndex) == _T('.'));
	}

	// ============================================================================
	/// TSmartPath::GetExtension
	/// @date 2010/10/16
	///
	/// @brief     Retrieves file extension from this path.
	/// @return    Extension part or empty if does not exist.
	// ============================================================================
	TSmartPath TSmartPath::GetExtension() const
	{
		size_t stIndex = m_strPath.FindLastOf(_T("\\/."));

		if (stIndex != TString::npos && m_strPath.GetAt(stIndex) == _T('.'))
			return PathFromWString(m_strPath.MidRange(stIndex, m_strPath.GetLength()));	// ".txt" for "c:\windows\test.txt"

		return TSmartPath();
	}

	// ============================================================================
	/// TSmartPath::HasFileName
	/// @date 2010/10/16
	///
	/// @brief     Checks if this path contains filename part.
	/// @return    True if filename exists, false otherwise.
	// ============================================================================
	bool TSmartPath::HasFileName() const
	{
		size_t stIndex = m_strPath.FindLastOf(_T("\\/"));
		if (stIndex == TString::npos)	// no path separator?
			return true;
		else
			return (stIndex != TString::npos && stIndex != m_strPath.GetLength() - 1);
	}

	// ============================================================================
	/// TSmartPath::GetFileName
	/// @date 2010/10/16
	///
	/// @brief     Retrieves filename part of this path.
	/// @return    Filename, or empty if does not exist.
	// ============================================================================
	TSmartPath TSmartPath::GetFileName() const
	{
		size_t stIndex = m_strPath.FindLastOf(_T("\\/"));
		if (stIndex != TString::npos)
			return PathFromWString(m_strPath.MidRange(stIndex + 1, m_strPath.GetLength()));	// "test.txt" for "c:\windows\test.txt"
		else
			return *this;
	}

	// ============================================================================
	/// TSmartPath::DeleteFileName
	/// @date 2010/10/17
	///
	/// @brief     Deletes the filename part of this path if exists.
	// ============================================================================
	void TSmartPath::DeleteFileName()
	{
		size_t stIndex = m_strPath.FindLastOf(_T("\\/"));
		if (stIndex != TString::npos)
			m_strPath.Delete(stIndex + 1, m_strPath.GetLength() - stIndex - 1);	// "test.txt" for "c:\windows\test.txt"
		else
		{
			// no path separator inside - everything in this path is a filename
			Clear();
		}
	}

	TSmartPath TSmartPath::GetParent() const
	{
		if (IsServerName() || IsDrive())
			return TSmartPath();

		TSmartPath pathResult(*this);

		if (pathResult.EndsWithSeparator())
		{
			pathResult.StripSeparatorAtEnd();
			if (pathResult.IsDrive() || pathResult.IsServerName())
				return pathResult;
		}

		pathResult.DeleteFileName();

		return pathResult;
	}

	// ============================================================================
	/// TSmartPath::EndsWithSeparator
	/// @date 2010/10/16
	///
	/// @brief     Checks if path end with a path separator (/ or \)
	/// @return    True if path ends with separator, false otherwise.
	// ============================================================================
	bool TSmartPath::EndsWithSeparator() const
	{
		size_t stThisSize = m_strPath.GetLength();
		if (stThisSize > 0)
		{
			wchar_t wchLastChar = m_strPath.GetAt(stThisSize - 1);
			return (wchLastChar == _T('\\') || wchLastChar == _T('/'));
		}

		return false;
	}

	// ============================================================================
	/// TSmartPath::AppendSeparatorIfDoesNotExist
	/// @date 2010/10/16
	///
	/// @brief     Appends separator to this path if does not exist already.
	// ============================================================================
	void TSmartPath::AppendSeparatorIfDoesNotExist()
	{
		if (!EndsWithSeparator())
			m_strPath += _T("\\");
	}

	// ============================================================================
	/// TSmartPath::StripSeparatorAtEnd
	/// @date 2010/10/17
	///
	/// @brief     Strips separator at the end of path if exists.
	// ============================================================================
	void TSmartPath::StripSeparatorAtEnd()
	{
		if (EndsWithSeparator())
			m_strPath.Delete(m_strPath.GetLength() - 1, 1);
	}

	// ============================================================================
	/// TSmartPath::StartsWithSeparator
	/// @date 2010/10/16
	///
	/// @brief     Checks if path starts with a separator.
	/// @return    True if path starts with separator, false otherwise.
	// ============================================================================
	bool TSmartPath::StartsWithSeparator() const
	{
		wchar_t wchLastChar = 0;
		if (m_strPath.GetLength() > 0)
			wchLastChar = m_strPath.GetAt(0);

		return (wchLastChar == _T('\\') || wchLastChar == _T('/'));
	}

	// ============================================================================
	/// TSmartPath::PrependSeparatorIfDoesNotExist
	/// @date 2010/10/17
	///
	/// @brief     Prepends a separator to this path if not exist already.
	// ============================================================================
	void TSmartPath::PrependSeparatorIfDoesNotExist()
	{
		if (!StartsWithSeparator())
			m_strPath = _T("\\") + m_strPath;
	}

	// ============================================================================
	/// TSmartPath::StripSeparatorAtFront
	/// @date 2010/10/17
	///
	/// @brief     Strips separator at the front of this path (if exists).
	// ============================================================================
	void TSmartPath::StripSeparatorAtFront()
	{
		if (StartsWithSeparator())
			m_strPath.Delete(0, 1);
	}

	void TSmartPath::StripPath(const wchar_t* pszToStrip)
	{
		m_strPath.Replace(pszToStrip, L"");
	}

	// ============================================================================
	/// TSmartPath::IsEmpty
	/// @date 2010/10/07
	///
	/// @brief     Prepares the path to be written to.
	// ============================================================================
	bool TSmartPath::IsEmpty() const
	{
		return m_strPath.IsEmpty();
	}

	// ============================================================================
	/// TSmartPath::GetLength
	/// @date 2011/04/05
	///
	/// @brief     Retrieves path length in characters.
	/// @return    Path length.
	// ============================================================================
	size_t TSmartPath::GetLength() const
	{
		return m_strPath.GetLength();
	}

	// ============================================================================
	/// TSmartPath::IsSeparator
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

	bool TSmartPath::IsRelativePath() const
	{
		return !HasDrive() && !HasServerName();
	}

	bool TSmartPath::StartsWith(const TSmartPath& rPath, bool bCaseSensitive) const
	{
		if(bCaseSensitive)
			return m_strPath.StartsWith(rPath.m_strPath.c_str());

		return m_strPath.StartsWithNoCase(rPath.m_strPath.c_str());
	}

	// ============================================================================
	/// PathFromString
	/// @date 2010/10/12
	///
	/// @brief     Creates a path object from string.
	/// @param[in] pszPath - string containing path.
	/// @return    New path object.
	// ============================================================================
	TSmartPath PathFromString(const wchar_t* pszPath)
	{
		if (!pszPath)
			throw TCoreException(eErr_InvalidArgument, L"pszPath", LOCATION);

		TSmartPath spPath;
		spPath.FromString(pszPath);
		return spPath;
	}

	TSmartPath PathFromWString(const TString& strPath)
	{
		TSmartPath spPath;
		spPath.FromString(strPath);
		return spPath;
	}
}
