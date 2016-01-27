// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TString.cpp
/// @date  2011/06/05
/// @brief Contains implementation of TString class.
// ============================================================================
#include "stdafx.h" 
#include "TString.h"
#include <string.h>
#pragma warning(push)
#pragma warning(disable: 4996)	// boost::split uses unsafe std::copy
	#include <boost/algorithm/string.hpp>
#pragma warning(pop)
#include "TStringArray.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TStringException.h"

/// Rounding up value to the nearest chunk multiplicity
#define ROUNDUP(val,chunk) ((val + chunk - 1) & ~(chunk-1))

///< Increment value for internal TString buffer
#define CHUNK_INCSIZE		64

namespace chcore
{
	/** Standard constructor - allocates the underlying data object
	 */
	TString::TString() :
		m_pszData(NULL),
		m_stBufferSize(0)
	{
	}

	/** Constructor allocates the underlying data object and initializes it with
	 *  a given unicode TString.
	 * \param[in] pszStr - source unicode TString
	 */
	TString::TString(const wchar_t* pszStr) :
		m_pszData(NULL),
		m_stBufferSize(0)
	{
		SetString(pszStr);
	}

	TString::TString(const wchar_t* pszStart, const wchar_t* pszEnd, size_t stMaxStringSize) :
		m_pszData(NULL),
		m_stBufferSize(0)
	{
		// we support either both arguments != NULL or both == NULL
		if (pszEnd != NULL && pszStart == NULL || pszEnd == NULL && pszStart != NULL)
			THROW_STRING_EXCEPTION(eErr_InvalidArgument, _T("End of string specified while start is NULL"));

		// sanity check
		if (pszEnd < pszStart)
			THROW_STRING_EXCEPTION(eErr_InvalidArgument, _T("Paradox: string begins after its end"));

		size_t stCount = pszEnd - pszStart;
		if (stCount > stMaxStringSize)
			THROW_STRING_EXCEPTION(eErr_InvalidArgument, _T("Exceeded maximum expected string size"));

		SetString(pszStart, stCount);
	}

	TString::TString(const wchar_t* pszStart, size_t stCount) :
		m_pszData(NULL),
		m_stBufferSize(0)
	{
		if (!pszStart)
			THROW_STRING_EXCEPTION(eErr_InvalidArgument, _T("String not specified"));

		if (stCount == 0)
			return;

		SetString(pszStart, stCount);
	}

	/** Constructor increases the reference count in the parameter's data object
	 *  and copies only the data object address.
	 * \param[in] rSrc - source TString object
	 */
	TString::TString(const TString& rSrc) :
		m_pszData(NULL),
		m_stBufferSize(0)
	{
		SetString(rSrc.m_pszData);
	}

	/** Destructor releases the underlying data object.
	 */
	TString::~TString()
	{
		delete[] m_pszData;
		m_pszData = NULL;
		m_stBufferSize = 0;
	}

	/** Operator releases the current data object, stores a pointer to
	 *  the data object from the given TString object and increases a reference
	 *  count.
	 * \param[in] src - source TString object
	 * \return A reference to the current TString.
	 */
	TString& TString::operator=(const TString& rSrc)
	{
		if (this != &rSrc)
			SetString(rSrc.m_pszData);

		return *this;
	}

	/** Operator makes an own copy of underlying data object (if needed) and copy
	 *  there the given unicode TString.
	 * \param[in] pszSrc - source unicode TString
	 * \return A reference to the current TString object.
	 */
	const TString& TString::operator=(const wchar_t* pszSrc)
	{
		if (pszSrc != m_pszData)
			SetString(pszSrc);

		return *this;
	}

	/** Operator concatenates a given TString object with the current content of
	 *  this TString and returns a new TString object.
	 * \param[in] src - TString object that will be appended
	 * \return A new TString object with concatenated strings.
	 */
	TString TString::operator+(const TString& src) const
	{
		TString str(*this);
		str.Append(src);

		return str;
	}

	/** Operator concatenates a given unicode TString with the current content of
	 *  this TString and returns a new TString object.
	 * \param[in] pszSrc - unicode TString that will be appended
	 * \return A new TString object with concatenated strings.
	 */
	TString TString::operator+(const wchar_t* pszSrc) const
	{
		TString str(*this);
		str.Append(pszSrc);

		return str;
	}

	/** Operator appends a given TString object to own internal buffer.
	 * \param[in] src - TString object that will be appended
	 * \return A reference to this.
	 */
	const TString& TString::operator+=(const TString& src)
	{
		Append(src);
		return *this;
	}

	/** Operator appends a given unicode TString to own internal buffer.
	 * \param[in] pszSrc - unicode TString that will be appended
	 * \return A reference to this.
	 */
	const TString& TString::operator+=(const wchar_t* pszSrc)
	{
		Append(pszSrc);
		return *this;
	}

	/** Function counts the GetLength of a TString in characters (doesn't matter if the TString
	 *  is unicode or ansi).
	 *  \note All GetLength checks should be done through this function, because of possible future
	 *  update that will store the TString GetLength in the internal member.
	 *  \return The TString GetLength in characters, not including the terminating '\\0'
	 */
	size_t TString::GetLength() const
	{
		return m_pszData ? _tcslen(m_pszData) : 0;
	}

	/** Function makes own data object writable and clears it. Does not delete the
	 *  internal buffer - only sets the content to '\\0'.
	 */
	void TString::Clear()
	{
		if (m_pszData)
			m_pszData[0] = L'\0';
	}

	/** Function checks if the TString is empty.
	 *  \return True if this TString is empty, false otherwise.
	 */
	bool TString::IsEmpty() const
	{
		return !m_pszData || m_pszData[0] == L'\0';
	}

	/** Function merges the given unicode TString with the current content of an internal buffer.
	 * \param[in] pszSrc - unicode TString to append
	 */
	void TString::Append(const wchar_t* pszSrc)
	{
		if (!pszSrc)
			return;

		size_t stAddLen = wcslen(pszSrc);
		size_t stThisLen = GetLength();

		Reserve(stThisLen + stAddLen + 1);

		wcsncpy_s(m_pszData + stThisLen, m_stBufferSize - stThisLen, pszSrc, stAddLen + 1);
	}

	/** Function merges the given TString object with the current content of an internal buffer.
	 * \param[in] src - TString object to append
	 */
	void TString::Append(const TString& rSrc)
	{
		if (rSrc.IsEmpty())
			return;

		size_t stAddLen = rSrc.GetLength();
		size_t stThisLen = GetLength();

		Reserve(stThisLen + stAddLen + 1);

		wcsncpy_s(m_pszData + stThisLen, m_stBufferSize - stThisLen, rSrc.m_pszData, stAddLen + 1);
	}

	/** Returns a new TString object with the Left part of this TString object.
	 * \param[in] tLen - count of characters to copy to the new TString object
	 * \return The TString with the Left part of the current TString.
	 */
	TString TString::Left(size_t tLen) const
	{
		size_t stThisLen = GetLength();

		if (stThisLen == 0 || tLen == 0)
			return TString();

		if (tLen >= stThisLen)
			return *this;
		else
			return TString(m_pszData, tLen);
	}

	/** Returns a new TString object with the Right part of this TString object.
	 * \param[in] tLen - count of characters to copy to the new TString object
	 * \return The TString with the Right part of the current TString.
	 */
	TString TString::Right(size_t tLen) const
	{
		size_t stThisLen = GetLength();

		if (stThisLen == 0 || tLen == 0)
			return TString();

		if (tLen >= stThisLen)
			return *this;
		else
			return TString(m_pszData + stThisLen - tLen, tLen);
	}

	/** Returns a new TString object with the middle part of this TString object.
	 * \param[in] tStart - position of the first character to copy
	 * \param[in] tLen - count of chars to copy
	 * \return The TString with the middle part of the current TString.
	 */
	TString TString::Mid(size_t tStart, size_t tLen) const
	{
		size_t stThisLen = GetLength();

		if (stThisLen == 0 || tLen == 0)
			return TString();

		if (tStart >= stThisLen)
			return TString();

		size_t stRealLength = std::min(tLen, stThisLen - tStart);

		TString strNew(m_pszData + tStart, stRealLength);
		return strNew;
	}

	TString TString::MidRange(size_t tStart, size_t stAfterEndPos) const
	{
		if (stAfterEndPos < tStart)
			return TString();
		return Mid(tStart, stAfterEndPos - tStart);
	}

	/** Makes this TString it's Left part. Much faster than using standard
	 *  Left() function.
	 * \param[in] tLen - count of characters at the beginning of the TString to be Left in a TString.
	 * \param[in] bReallocBuffer - if the internal TString buffer is to be reallocated if exceeds
	 *									  the allowable range size (CHUNK_INCSIZE, CHUNK_DECSIZE).
	 * \see Left()
	 */
	void TString::LeftSelf(size_t tLen)
	{
		size_t stThisLen = GetLength();

		// nothing to do if nothing inside
		if (stThisLen == 0)
			return;

		if (tLen < stThisLen)		// otherwise there is nothing to do
			m_pszData[tLen] = _T('\0');
	}

	/** Makes this TString it's Right part. Much faster than using standard
	 *  Right() function.
	 * \param[in] tLen - count of characters at the end of the TString to be Left in a TString.
	 * \param[in] bReallocBuffer - if the internal TString buffer is to be reallocated if exceeds
	 *									  the allowable range size (CHUNK_INCSIZE, CHUNK_DECSIZE).
	 * \see Right()
	 */
	void TString::RightSelf(size_t tLen)
	{
		size_t stThisLen = GetLength();

		// nothing to do if nothing inside
		if (stThisLen == 0)
			return;

		if (tLen < stThisLen)		// otherwise there is nothing to do
			wmemmove(m_pszData, m_pszData + stThisLen - tLen, tLen + 1);
	}

	/** Makes this TString it's middle part. Much faster than using standard
	 *  Mid() function.
	 * \param[in] tStart - starting position of a text to be Left in a TString
	 * \param[in] tLen - count of characters at the middle of the TString to be Left in a TString.
	 * \param[in] bReallocBuffer - if the internal TString buffer is to be reallocated if exceeds
	 *									  the allowable range size (CHUNK_INCSIZE, CHUNK_DECSIZE).
	 * \see Mid()
	 */
	void TString::MidSelf(size_t tStart, size_t tLen)
	{
		size_t stThisLen = GetLength();

		if (stThisLen == 0)
			return;

		if (tStart >= stThisLen)
			Clear();
		else
		{
			size_t stRealNewLength = std::min(tLen, stThisLen - tStart);

			wmemmove(m_pszData, m_pszData + tStart, stRealNewLength);
			m_pszData[stRealNewLength] = _T('\0');
		}
	}

	void TString::TrimRightSelf(const wchar_t* pszElements)
	{
		if (!pszElements || pszElements[0] == L'\0')
			return;

		size_t stThisLen = GetLength();
		if (stThisLen == 0)
			return;

		size_t stLen = stThisLen;

		const wchar_t* pszElementsEnd = pszElements + wcslen(pszElements);
		while (stLen-- > 0)
		{
			if (std::find(pszElements, pszElementsEnd, m_pszData[stLen]) != pszElementsEnd)
				m_pszData[stLen] = _T('\0');
			else
				break;
		}
	}

	bool TString::Delete(size_t stIndex, size_t stCount)
	{
		size_t stThisLen = GetLength();

		if (stIndex >= stThisLen || stCount == 0)
			return false;

		bool bResult = true;
		if (stIndex + stCount > stThisLen)	// in case there is not enough data to delete, then we want to delete what we can, but return false
			bResult = false;

		size_t stCountToDelete = std::min(stThisLen - stIndex, stCount);

		// should also copy the terminating null character
		errno_t err = wmemmove_s(m_pszData + stIndex, stThisLen - stIndex + 1, m_pszData + stIndex + stCountToDelete, stThisLen - stIndex - stCountToDelete + 1);
		if (err != 0)
			throw TCoreException(eErr_InternalProblem, L"Failed to move memory", LOCATION);

		return bResult;
	}

	void TString::Split(const wchar_t* pszSeparators, TStringArray& rStrings) const
	{
		rStrings.Clear();

		size_t stThisLen = GetLength();
		if (stThisLen == 0 || !pszSeparators)
			return;

		// ugly version - many reallocations due to the usage of stl wstrings
		std::vector<std::wstring> vStrings;
		boost::split(vStrings, m_pszData, boost::is_any_of(pszSeparators));

		BOOST_FOREACH(const std::wstring& strPart, vStrings)
		{
			rStrings.Add(strPart.c_str());
		}
	}

	/** Compares a TString with the given unicode TString. Comparison is case sensitive.
	 * \param[in] psz - unicode TString to which the TString object will be compared
	 * \return <0 if this TString object is "less" than psz, 0 if they are equal and >0 otherwise.
	 */
	int TString::Compare(const wchar_t* psz) const
	{
		return wcscmp(m_pszData ? m_pszData : L"", psz ? psz : L"");
	}

	/** Compares a TString with the given TString object. Comparison is case sensitive.
	 * \param[in] str - TString object to which internal TString object will be compared
	 * \return <0 if this TString object is "less" than psz, 0 if they are equal and >0 otherwise.
	 */
	int TString::Compare(const TString& str) const
	{
		return Compare(str.m_pszData);
	}

	/** Compares a TString with the given unicode TString. Comparison is case insensitive.
	 * \param[in] psz - unicode TString to which internal TString object will be compared
	 * \return <0 if this TString object is "less" than psz, 0 if they are equal and >0 otherwise.
	 */
	int TString::CompareNoCase(const wchar_t* psz) const
	{
		return _wcsicmp(m_pszData ? m_pszData : L"", psz ? psz : L"");
	}

	/** Compares a TString with the given TString object. Comparison is case insensitive.
	 * \param[in] str - TString object to which internal TString object will be compared
	 * \return <0 if this TString object is "less" than str, 0 if they are equal and >0 otherwise.
	 */
	int TString::CompareNoCase(const TString& str) const
	{
		return CompareNoCase(str.m_pszData);
	}

	bool TString::StartsWith(const wchar_t* pszText) const
	{
		if (!m_pszData || !pszText)
			return false;

		return boost::starts_with(m_pszData, pszText);
	}

	bool TString::StartsWithNoCase(const wchar_t* pszText) const
	{
		if (!m_pszData || !pszText)
			return false;

		return boost::istarts_with(m_pszData, pszText);
	}

	bool TString::EndsWith(const wchar_t* pszText) const
	{
		if (!m_pszData || !pszText)
			return false;

		return boost::ends_with(m_pszData, pszText);
	}

	bool TString::EndsWithNoCase(const wchar_t* pszText) const
	{
		if (!m_pszData || !pszText)
			return false;

		return boost::iends_with(m_pszData, pszText);
	}

	size_t TString::FindFirstOf(const wchar_t* pszChars, size_t stStartFromPos) const
	{
		if (!m_pszData || !pszChars)
			return npos;

		size_t stCurrentLength = GetLength();
		for (size_t stIndex = stStartFromPos; stIndex < stCurrentLength; ++stIndex)
		{
			if (wcschr(pszChars, m_pszData[stIndex]))
				return stIndex;
		}

		return npos;
	}

	size_t TString::FindLastOf(const wchar_t* pszChars) const
	{
		if (!m_pszData || !pszChars)
			return npos;

		for (size_t stIndex = GetLength(); stIndex != 0; --stIndex)
		{
			if (wcschr(pszChars, m_pszData[stIndex - 1]))
				return stIndex - 1;
		}

		return npos;
	}

	size_t TString::Find(const wchar_t* pszFindText, size_t stStartPos)
	{
		size_t stThisLen = GetLength();
		if (!pszFindText || stThisLen == 0)
			return npos;

		size_t stFindTextLen = _tcslen(pszFindText);
		if (stFindTextLen > stThisLen)
			return TString::npos;

		if (stStartPos > stThisLen - stFindTextLen)
			return TString::npos;

		boost::iterator_range<wchar_t*> rangeText = boost::make_iterator_range(m_pszData + stStartPos, m_pszData + stThisLen);
		boost::iterator_range<wchar_t*> rangeFind = boost::find_first(rangeText, pszFindText);

		if (rangeFind.begin() != rangeText.end())
			return rangeFind.begin() - rangeText.begin() + stStartPos;
		else
			return TString::npos;
	}

	void TString::Replace(const wchar_t* pszWhat, const wchar_t* pszWithWhat)
	{
		size_t stThisLen = GetLength();
		if (stThisLen == 0)
			return;

		if (!pszWhat || !pszWithWhat)
			return;

		// find all occurrences of pszWhat in this string, so we can calculate new required size of the string
		size_t stWhatLen = _tcslen(pszWhat);
		size_t stWithWhatLen = _tcslen(pszWithWhat);

		size_t stNewLen = stThisLen;

		// resize internal string if needed
		if (stWithWhatLen > stWhatLen)
		{
			size_t stStartPos = 0;
			size_t stFindPos = 0;
			size_t stSizeDiff = 0;
			while ((stFindPos = Find(pszWhat, stStartPos)) != npos)
			{
				stSizeDiff += stWithWhatLen - stWhatLen;
				stStartPos = stFindPos + stWhatLen;	 // offset by what_len because we don't replace anything at this point
			}

			if (stSizeDiff > 0)
				stNewLen = stThisLen + stSizeDiff + 1;
		}

		Reserve(stNewLen);

		// replace
		size_t stStartPos = 0;
		size_t stFindPos = 0;
		while ((stFindPos = Find(pszWhat, stStartPos)) != npos)
		{
			// Sample string "ABCdddb" (len:6), searching for "dd" (len 2) to replace with "x" (len 1)
			// found string pos is: [stFindPos, stFindPos + stWhatLen)  -- sample ref: [3, 3 + 2)
			// we need to
			// - move string from position [stFindPos + stWhatLen, stCurrentLength) to position [stFindPos + stWithWhatLen, stCurrentLength + stWithWhatLen - stWhatLen] -- sample ref: [3+2, 6) to [3+1, 5)
			size_t stCountToCopy = stThisLen - stFindPos - stWhatLen + 1;

			memmove_s((void*)(m_pszData + stFindPos + stWithWhatLen), stCountToCopy * sizeof(wchar_t), (void*)(m_pszData + stFindPos + stWhatLen), stCountToCopy * sizeof(wchar_t));

			// - copy pszWithWhat to position (stFindPos + stWhatLen)
			memcpy_s((void*)(m_pszData + stFindPos), stWithWhatLen * sizeof(wchar_t), pszWithWhat, stWithWhatLen * sizeof(wchar_t));

			stStartPos = stFindPos + stWithWhatLen;	// offset by stWithWhatLen because we replaced text
		}
	}

	/** Returns a character at a given position. Function is very slow (needs to recalc the size of the TString
	 *  and make a few comparisons), but quite safe - if the index is out of range then -1 is returned.
	 *  Make sure to interpret the returned character according to unicode flag. If the TString is unicode, then the
	 *  character returned is also unicode (and vice versa).
	 * \param[in] tPos - index of the character to return.
	 * \return Character code of character on a specified position, or -1 if out of range.
	 */
	bool TString::GetAt(size_t tPos, wchar_t& wch) const
	{
		if (tPos < GetLength())
		{
			wch = m_pszData[tPos];
			return true;
		}
		else
		{
			wch = L'\0';
			return false;
		}
	}

	wchar_t TString::GetAt(size_t tPos) const
	{
		if (tPos < GetLength())
			return m_pszData[tPos];
		else
			return L'\0';
	}

	/** Returns a pointer to the unicode internal buffer. If the buffer is in ansi format
	 *  then NULL value is returned. Internal buffer is resized to the specified value
	 *  if currently smaller than requested (if -1 is specified as tMinSize then the buffer
	 *  is not resized, and the return value could be NULL).
	 * \param[in] tMinSize - requested minimal size of the internal buffer (-1 if the size of the TString should not be changed)
	 * \return Pointer to the internal unicode buffer.
	 */
	wchar_t* TString::GetBuffer(size_t tMinSize)
	{
		Reserve(tMinSize + 1);

		return m_pszData;
	}

	/** Releases buffer got by user by calling get_bufferx functions. The current
	 *  job of this function is to make sure the TString will terminate with null
	 *  character at the end of the buffer.
	 */
	void TString::ReleaseBuffer()
	{
		m_pszData[m_stBufferSize - 1] = L'\0';
	}

	void TString::ReleaseBufferSetLength(size_t tSize)
	{
		Reserve(tSize + 1);

		m_pszData[tSize] = L'\0';
	}

	void TString::SetString(const wchar_t* pszStart, size_t stCount)
	{
		if (!pszStart || stCount == 0)
			Clear();
		else
		{
			Reserve(stCount + 1);

			wcsncpy_s(m_pszData, m_stBufferSize, pszStart, stCount);
			m_pszData[stCount] = _T('\0');
		}
	}

	void TString::SetString(const wchar_t* pszString)
	{
		if (!pszString)
			Clear();
		else
		{
			size_t stLen = _tcslen(pszString);
			SetString(pszString, stLen);
		}
	}

	void TString::Reserve(size_t stLen)
	{
		if (m_stBufferSize < stLen)
		{
			size_t stNewLen = stLen;//ROUNDUP(stLen, CHUNK_INCSIZE);

			wchar_t* pszNewBuffer = new wchar_t[stNewLen];
			if (m_pszData && m_pszData[0] != L'\0')
				_tcsncpy_s(pszNewBuffer, stNewLen, m_pszData, GetLength() + 1);
			else
				pszNewBuffer[0] = _T('\0');

			delete[] m_pszData;
			m_pszData = pszNewBuffer;
			m_stBufferSize = stNewLen;
		}
	}

	const wchar_t* TString::c_str() const
	{
		return m_pszData ? m_pszData : L"";
	}
}

chcore::TString operator+(const wchar_t* pszString, const chcore::TString& str)
{
	chcore::TString strNew(pszString);
	strNew += str;
	return strNew;
}
