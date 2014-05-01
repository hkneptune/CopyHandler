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

BEGIN_CHCORE_NAMESPACE

namespace details
{
	TInternalStringData::TInternalStringData() :
		m_lRefCount(1),
		m_stStringLength(0),
		m_pszData(NULL),
		m_stBufferSize(0)
	{
	}

	TInternalStringData::TInternalStringData(const TInternalStringData& rSrc, size_t stReserveLen) :
		m_pszData(NULL),
		m_stBufferSize(0),
		m_stStringLength(0),
		m_lRefCount(1)
	{
		Reserve(std::max(rSrc.m_stBufferSize, stReserveLen));

		if(rSrc.m_pszData)
			_tcsncpy_s(m_pszData, m_stBufferSize, rSrc.m_pszData, rSrc.m_stStringLength + 1);
		else
			m_pszData[0] = _T('\0');

		m_stStringLength = rSrc.m_stStringLength;
	}

	TInternalStringData::~TInternalStringData()
	{
		delete [] m_pszData;
	}

	void TInternalStringData::Reserve(size_t stLen)
	{
		if(m_stBufferSize < stLen)
		{
			size_t stNewLen = ROUNDUP(stLen, CHUNK_INCSIZE);

			wchar_t* pszNewBuffer = new wchar_t[stNewLen];
			if(m_pszData)
				memcpy(pszNewBuffer, m_pszData, m_stStringLength);
			else
				pszNewBuffer[0] = _T('\0');

			delete [] m_pszData;
			m_pszData = pszNewBuffer;
			m_stBufferSize = stNewLen;
		}
	}

	void TInternalStringData::Clear()
	{
		delete [] m_pszData;
		m_pszData = NULL;
		m_stStringLength = 0;
		m_stBufferSize = 0;
		m_lRefCount = 0;
	}

	void TInternalStringData::SetString(const wchar_t* pszString, size_t stCount)
	{
		Reserve(stCount + 1);	// +1 - additional space for \0

		wcsncpy_s(m_pszData, m_stBufferSize, pszString, stCount);
		m_pszData[stCount] = _T('\0');
		m_stStringLength = _tcslen(m_pszData);		// calculating length as we don't know if the input string did not have \0's inside
	}

	void TInternalStringData::ClearString()
	{
		if(m_pszData)
			m_pszData[0] = _T('\0');
		m_stStringLength = 0;
	}
}

using namespace details;

const size_t TString::npos = std::numeric_limits<size_t>::max();

/** Standard constructor - allocates the underlying data object
 */
TString::TString() :
	m_pData(new details::TInternalStringData)
{
}

/** Constructor allocates the underlying data object and initializes it with
 *  a given unicode TString.
 * \param[in] pszStr - source unicode TString
 */
TString::TString(const wchar_t* pszStr) :
	m_pData(new details::TInternalStringData)
{
	if(pszStr == NULL)
		return;

	size_t stLen = wcslen(pszStr);
	m_pData->SetString(pszStr, stLen);
}

TString::TString(const wchar_t* pszStart, const wchar_t* pszEnd, size_t stMaxStringSize) :
	m_pData(new details::TInternalStringData)
{
	// we support either both arguments != NULL or both == NULL
	if(pszEnd != NULL && pszStart == NULL || pszEnd == NULL && pszStart != NULL)
		THROW_STRING_EXCEPTION(eErr_InvalidArgument, _T("End of string specified while start is NULL"));

	// sanity check
	if(pszEnd < pszStart)
		THROW_STRING_EXCEPTION(eErr_InvalidArgument, _T("Paradox: string begins after its end"));

	size_t stCount = pszEnd - pszStart;
	if(stCount > stMaxStringSize)
		THROW_STRING_EXCEPTION(eErr_InvalidArgument, _T("Exceeded maximum expected string size"));

	m_pData->SetString(pszStart, stCount);
}

TString::TString(const wchar_t* pszStart, size_t stCount) :
	m_pData(new details::TInternalStringData)
{
	if(!pszStart)
		THROW_STRING_EXCEPTION(eErr_InvalidArgument, _T("String not specified"));

	if(stCount == 0)
		return;

	m_pData->SetString(pszStart, stCount);
}

/** Constructor increases the reference count in the parameter's data object
 *  and copies only the data object address.
 * \param[in] rSrc - source TString object
 */
TString::TString(const TString& rSrc) :
	m_pData(NULL)
{
	if(InterlockedCompareExchange(&rSrc.m_pData->m_lRefCount, 0, 0) > 0)
	{
		m_pData = rSrc.m_pData;
		InterlockedIncrement(&m_pData->m_lRefCount);
	}
	else
		m_pData = new details::TInternalStringData(*rSrc.m_pData);
}

/** Destructor releases the underlying data object.
 */
TString::~TString()
{
	if(InterlockedDecrement(&m_pData->m_lRefCount) < 1)
		delete m_pData;
}

/** Operator releases the current data object, stores a pointer to
 *  the data object from the given TString object and increases a reference
 *  count.
 * \param[in] src - source TString object
 * \return A reference to the current TString.
 */
const TString& TString::operator=(const TString& rSrc)
{
	if(this != &rSrc && m_pData != rSrc.m_pData)
	{
		Release();

		if(InterlockedCompareExchange(&rSrc.m_pData->m_lRefCount, 0, 0) > 0)
		{
			m_pData = rSrc.m_pData;
			InterlockedIncrement(&m_pData->m_lRefCount);
		}
		else
			m_pData = new details::TInternalStringData(*rSrc.m_pData);
	}

	return *this;
}

/** Operator makes an own copy of underlying data object (if needed) and copy
 *  there the given unicode TString.
 * \param[in] pszSrc - source unicode TString
 * \return A reference to the current TString object.
 */
const TString& TString::operator=(const wchar_t* pszSrc)
{
	Release();
	if(!pszSrc)
	{
		m_pData->Clear();
		return *this;
	}

	size_t stLen = wcslen(pszSrc);
	m_pData->SetString(pszSrc, stLen);

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
	return m_pData->m_stStringLength;
}

/** Function makes own data object writable and clears it. Does not delete the
 *  internal buffer - only sets the content to '\\0'.
 */
void TString::Clear()
{
	// make sure we have the modifiable object without allocated TString buffer
	Release();
}

/** Function checks if the TString is empty.
 *  \return True if this TString is empty, false otherwise.
 */
bool TString::IsEmpty() const
{
	return m_pData->m_stStringLength == 0;
}

/** Function merges the given unicode TString with the current content of an internal buffer.
 * \param[in] pszSrc - unicode TString to append
 */
void TString::Append(const wchar_t* pszSrc)
{
	if(!pszSrc)
		return;

	size_t stAddLen = wcslen(pszSrc);
	EnsureWritable(m_pData->m_stStringLength + stAddLen + 1);

	wcsncpy_s(m_pData->m_pszData + m_pData->m_stStringLength, m_pData->m_stBufferSize - m_pData->m_stStringLength, pszSrc, stAddLen + 1);
	m_pData->m_stStringLength += stAddLen;
}

/** Function merges the given TString object with the current content of an internal buffer.
 * \param[in] src - TString object to append
 */
void TString::Append(const TString& rSrc)
{
	if(rSrc.IsEmpty())
		return;

	size_t stAddLen = rSrc.GetLength();
	EnsureWritable(m_pData->m_stStringLength + stAddLen + 1);

	wcsncpy_s(m_pData->m_pszData + m_pData->m_stStringLength, m_pData->m_stBufferSize - m_pData->m_stStringLength, rSrc.m_pData->m_pszData, stAddLen + 1);
	m_pData->m_stStringLength += stAddLen;
}

/** Returns a new TString object with the Left part of this TString object.
 * \param[in] tLen - count of characters to copy to the new TString object
 * \return The TString with the Left part of the current TString.
 */
TString TString::Left(size_t tLen) const
{
	if(m_pData->m_stStringLength == 0 || tLen == 0)
		return TString();

	if(tLen >= m_pData->m_stStringLength)
		return *this;
	else
		return TString(m_pData->m_pszData, tLen);
}

/** Returns a new TString object with the Right part of this TString object.
 * \param[in] tLen - count of characters to copy to the new TString object
 * \return The TString with the Right part of the current TString.
 */
TString TString::Right(size_t tLen) const
{
	if(m_pData->m_stStringLength == 0 || tLen == 0)
		return TString();

	if(tLen >= m_pData->m_stStringLength)
		return *this;
	else
		return TString(m_pData->m_pszData + m_pData->m_stStringLength - tLen, tLen);
}

/** Returns a new TString object with the middle part of this TString object.
 * \param[in] tStart - position of the first character to copy
 * \param[in] tLen - count of chars to copy
 * \return The TString with the middle part of the current TString.
 */
TString TString::Mid(size_t tStart, size_t tLen) const
{
	if(m_pData->m_stStringLength == 0 || tLen == 0)
		return TString();

	if(tStart >= m_pData->m_stStringLength)
		return TString();

	size_t stRealLength = std::min(tLen, m_pData->m_stStringLength - tStart);

	TString strNew(m_pData->m_pszData + tStart, stRealLength);
	return strNew;
}

TString TString::MidRange(size_t tStart, size_t stAfterEndPos) const
{
	if(stAfterEndPos < tStart)
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
	// nothing to do if nothing inside
	if(m_pData->m_stStringLength == 0)
		return;

	if(tLen < m_pData->m_stStringLength)		// otherwise there is nothing to do
	{
		EnsureWritable(0);
		m_pData->m_pszData[tLen] = _T('\0');
		m_pData->m_stStringLength = tLen;
	}
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
	// nothing to do if nothing inside
	if(m_pData->m_stStringLength == 0)
		return;

	if(tLen < m_pData->m_stStringLength)		// otherwise there is nothing to do
	{
		EnsureWritable(0);

		wmemmove(m_pData->m_pszData, m_pData->m_pszData + m_pData->m_stStringLength - tLen, tLen + 1);
		m_pData->m_stStringLength = tLen;
	}
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
	if(m_pData->m_stStringLength == 0)
		return;

	if(tStart >= m_pData->m_stStringLength)
	{
		EnsureWritable(0);
		m_pData->ClearString();
	}
	else
	{
		size_t stRealLength = std::min(tLen, m_pData->m_stStringLength - tStart);

		EnsureWritable(stRealLength + 1);
		wmemmove(m_pData->m_pszData, m_pData->m_pszData + tStart, stRealLength);
		m_pData->m_pszData[stRealLength] = _T('\0');

		m_pData->m_stStringLength = stRealLength;
	}
}


void TString::TrimRightSelf(const wchar_t* pszElements)
{
	if(!pszElements || pszElements[0] == L'\0')
		return;

	if(m_pData->m_stStringLength == 0)
		return;

	EnsureWritable(0);

	size_t stLen = m_pData->m_stStringLength;

	const wchar_t* pszElementsEnd = pszElements + wcslen(pszElements);
	while(stLen-- > 0)
	{
		if(std::find(pszElements, pszElementsEnd, m_pData->m_pszData[stLen]) != pszElementsEnd)
		{
			m_pData->m_pszData[stLen] = _T('\0');
			m_pData->m_stStringLength = stLen;
		}
		else
			break;
	}
}

bool TString::Delete(size_t stIndex, size_t stCount)
{
	if(stIndex >= m_pData->m_stStringLength || stCount == 0)
		return false;

	bool bResult = true;
	if(stIndex + stCount > m_pData->m_stStringLength)	// in case there is not enough data to delete, then we want to delete what we can, but return false
		bResult = false;

	EnsureWritable(0);

	size_t stCountToDelete = std::min(m_pData->m_stStringLength - stIndex, stCount);

	// should also copy the terminating null character
	errno_t err = wmemmove_s(m_pData->m_pszData + stIndex, m_pData->m_stStringLength - stIndex + 1, m_pData->m_pszData + stIndex + stCountToDelete, m_pData->m_stStringLength - stIndex - stCountToDelete + 1);
	if(err != 0)
		THROW_CORE_EXCEPTION(eErr_InternalProblem);

	m_pData->m_stStringLength -= stCountToDelete;

	return bResult;
}

void TString::Split(const wchar_t* pszSeparators, TStringArray& rStrings) const
{
	rStrings.Clear();
	if(m_pData->m_stStringLength == 0 || !pszSeparators)
		return;

	// ugly version - many reallocations due to the usage of stl wstrings
	std::vector<std::wstring> vStrings;
	boost::split(vStrings, m_pData->m_pszData, boost::is_any_of(pszSeparators));

	BOOST_FOREACH(const std::wstring& strPart, vStrings)
	{
		rStrings.Add(strPart.c_str());
	}
}

/** Compares a TString with the given unicode TString. Comparison is case sensitive.
 * \param[in] psz - unicode TString to which the TString object will be compared
 * \return <0 if this TString object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t TString::Compare(const wchar_t* psz) const
{
	return wcscmp(m_pData->m_pszData ? m_pData->m_pszData : L"", psz ? psz : L"");
}

/** Compares a TString with the given TString object. Comparison is case sensitive.
 * \param[in] str - TString object to which internal TString object will be compared
 * \return <0 if this TString object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t TString::Compare(const TString& str) const
{
	return Compare(str.m_pData->m_pszData);
}

/** Compares a TString with the given unicode TString. Comparison is case insensitive.
 * \param[in] psz - unicode TString to which internal TString object will be compared
 * \return <0 if this TString object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t TString::CompareNoCase(const wchar_t* psz) const
{
	return _wcsicmp(m_pData->m_pszData ? m_pData->m_pszData : L"", psz ? psz : L"");
}

/** Compares a TString with the given TString object. Comparison is case insensitive.
 * \param[in] str - TString object to which internal TString object will be compared
 * \return <0 if this TString object is "less" than str, 0 if they are equal and >0 otherwise.
 */
int_t TString::CompareNoCase(const TString& str) const
{
	return CompareNoCase(str.m_pData->m_pszData);
}

bool TString::StartsWith(const wchar_t* pszText) const
{
	if(!m_pData || !pszText)
		return false;

	return boost::starts_with(m_pData->m_pszData, pszText);
}

bool TString::StartsWithNoCase(const wchar_t* pszText) const
{
	if(!m_pData || !pszText)
		return false;

	return boost::istarts_with(m_pData->m_pszData, pszText);
}

bool TString::EndsWith(const wchar_t* pszText) const
{
	if(!m_pData || !pszText)
		return false;

	return boost::ends_with(m_pData->m_pszData, pszText);
}

bool TString::EndsWithNoCase(const wchar_t* pszText) const
{
	if(!m_pData || !pszText)
		return false;

	return boost::iends_with(m_pData->m_pszData, pszText);
}

size_t TString::FindFirstOf(const wchar_t* pszChars, size_t stStartFromPos) const
{
	if(!m_pData || !pszChars)
		return npos;

	size_t stCurrentLength = GetLength();
	for(size_t stIndex = stStartFromPos; stIndex < stCurrentLength; ++stIndex)
	{
		if(wcschr(pszChars, m_pData->m_pszData[stIndex]))
			return stIndex;
	}

	return npos;
}

size_t TString::FindLastOf(const wchar_t* pszChars) const
{
	if(!m_pData || !pszChars)
		return npos;

	for(size_t stIndex = GetLength(); stIndex != 0; --stIndex)
	{
		if(wcschr(pszChars, m_pData->m_pszData[stIndex - 1]))
			return stIndex - 1;
	}

	return npos;
}

size_t TString::Find(const wchar_t* pszFindText, size_t stStartPos)
{
	if(!pszFindText || m_pData->m_stStringLength == 0)
		return npos;

	size_t stFindTextLen = _tcslen(pszFindText);
	size_t stThisLen = GetLength();
	if(stFindTextLen > stThisLen)
		return TString::npos;

	if(stStartPos > stThisLen - stFindTextLen)
		return TString::npos;

	boost::iterator_range<wchar_t*> rangeText = boost::make_iterator_range(m_pData->m_pszData + stStartPos, m_pData->m_pszData + stThisLen);
	boost::iterator_range<wchar_t*> rangeFind = boost::find_first(rangeText, pszFindText);

	if(rangeFind.begin() != rangeText.end())
		return rangeFind.begin() - rangeText.begin() + stStartPos;
	else
		return TString::npos;
}

void TString::Replace(const wchar_t* pszWhat, const wchar_t* pszWithWhat)
{
	if(m_pData->m_stStringLength == 0)
		return;

	if(!pszWhat || !pszWithWhat)
		return;

	// make sure nobody modifies the internal text while we process it
	EnsureWritable(0);

	// find all occurrences of pszWhat in this string, so we can calculate new required size of the string
	size_t stCurrentLength = GetLength();
	size_t stWhatLen = _tcslen(pszWhat);
	size_t stWithWhatLen = _tcslen(pszWithWhat);

	size_t stNewLen = stCurrentLength;

	// resize internal string if needed
	if(stWithWhatLen > stWhatLen)
	{
		size_t stStartPos = 0;
		size_t stFindPos = 0;
		size_t stSizeDiff = 0;
		while((stFindPos = Find(pszWhat, stStartPos)) != npos)
		{
			stSizeDiff += stWithWhatLen - stWhatLen;
			stStartPos = stFindPos + stWhatLen;	 // offset by what_len because we don't replace anything at this point
		}

		if(stSizeDiff > 0)
			stNewLen = stCurrentLength + stSizeDiff + 1;
	}

	EnsureWritable(stNewLen);

	// replace
	size_t stStartPos = 0;
	size_t stFindPos = 0;
	while((stFindPos = Find(pszWhat, stStartPos)) != npos)
	{
		// Sample string "ABCdddb" (len:6), searching for "dd" (len 2) to replace with "x" (len 1)
		// found string pos is: [stFindPos, stFindPos + stWhatLen)  -- sample ref: [3, 3 + 2)
		// we need to
		// - move string from position [stFindPos + stWhatLen, stCurrentLength) to position [stFindPos + stWithWhatLen, stCurrentLength + stWithWhatLen - stWhatLen] -- sample ref: [3+2, 6) to [3+1, 5)
		size_t stCountToCopy = stCurrentLength - stFindPos - stWhatLen + 1;

		memmove_s((void*)(m_pData->m_pszData + stFindPos + stWithWhatLen), stCountToCopy * sizeof(wchar_t), (void*)(m_pData->m_pszData + stFindPos + stWhatLen), stCountToCopy * sizeof(wchar_t));

		// - copy pszWithWhat to position (stFindPos + stWhatLen)
		memcpy_s((void*)(m_pData->m_pszData + stFindPos), stWithWhatLen * sizeof(wchar_t), pszWithWhat, stWithWhatLen * sizeof(wchar_t));

		stStartPos = stFindPos + stWithWhatLen;	// offset by stWithWhatLen because we replaced text
		stCurrentLength = stCurrentLength + stWithWhatLen - stWhatLen;

		m_pData->m_stStringLength = stCurrentLength;
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
	if(tPos < m_pData->m_stStringLength)
	{
		wch = m_pData->m_pszData[tPos];
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
	if(tPos < m_pData->m_stStringLength)
		return m_pData->m_pszData[tPos];
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
	EnsureUnshareable(tMinSize);
	return m_pData->m_pszData;
}

/** Releases buffer got by user by calling get_bufferx functions. The current
 *  job of this function is to make sure the TString will terminate with null
 *  character at the end of the buffer.
 */
void TString::ReleaseBuffer()
{
	EnsureWritable(0);

	m_pData->m_pszData[m_pData->m_stBufferSize - 1] = L'\0';
	m_pData->m_stStringLength = wcslen(m_pData->m_pszData);
}

void TString::ReleaseBufferSetLength(size_t tSize)
{
	EnsureWritable(tSize + 1);

	m_pData->m_pszData[tSize] = L'\0';
	m_pData->m_stStringLength = tSize;
}

/** Cast operator - tries to return a pointer to wchar_t* using the current internal
 *  buffer. If the internal buffer is in ansi format, then the debug version asserts
 *  and release return NULL.
 * \return Pointer to an unicode TString (could be null).
 */
TString::operator const wchar_t*() const
{
	return m_pData->m_pszData ? m_pData->m_pszData : L"";
}

void TString::SetString(const wchar_t* pszStart, size_t stCount)
{
	if(!pszStart || stCount == 0)
	{
		EnsureWritable(stCount);
		m_pData->ClearString();
	}
	else
	{
		EnsureWritable(stCount + 1);
		m_pData->SetString(pszStart, stCount);
	}
}

void TString::EnsureWritable(size_t stRequestedSize)
{
	if(InterlockedCompareExchange(&m_pData->m_lRefCount, 1, 1) > 1)
	{
		TInternalStringData* pNewData = new TInternalStringData(*m_pData, stRequestedSize);
		if(InterlockedDecrement(&m_pData->m_lRefCount) < 1)
		{
			delete pNewData;
			m_pData->m_lRefCount = 1;
		}
		else
			m_pData = pNewData;
	}
	else
	{
		m_pData->Reserve(stRequestedSize);
		m_pData->m_lRefCount = 1;
	}
}

void TString::EnsureUnshareable(size_t stRequestedSize)
{
	EnsureWritable(stRequestedSize);
	m_pData->m_lRefCount = -1;
}

void TString::Release()
{
	if(InterlockedDecrement(&m_pData->m_lRefCount) < 1)
	{
		m_pData->ClearString();		// buffer is preserved here
		m_pData->m_lRefCount = 1;
	}
	else
		m_pData = new details::TInternalStringData;
}

size_t TString::GetCurrentBufferSize() const
{
	return m_pData->m_stBufferSize;
}

END_CHCORE_NAMESPACE

chcore::TString operator+(const wchar_t* pszString, const chcore::TString& str)
{
	chcore::TString strNew(pszString);
	strNew += str;
	return strNew;
}
