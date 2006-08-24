/***************************************************************************
 *   Copyright (C) 2004 by Józef Starosczyk                                *
 *   copyhandler@o2.pl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
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
/** \file str.cpp
 *  \brief Contains the string class implementation.
 */
 
#include "str.h"
#include <string.h>
#include <assert.h>
#include "macros.h"
#include "str_help.h"
#include "dumpctx.h"
#include <stdio.h>

#ifdef _WIN32
	#include <windows.h>
#endif

///< Increment value for internal string buffer
#define CHUNK_INCSIZE		64

// if (buffer_size-string_size > CHUNK_DECSIZE) then the buffer will be shrinked
///< Difference value above which the string will be shrinked
#define CHUNK_DECSIZE		256

BEGIN_ICPF_NAMESPACE

/** Standard constructor
 * \param[in] wFlags - initialization flags (SDF_*)
 */
str_data::str_data(short_t wFlags)
	: m_pszBuffer(NULL),
	 m_tBufSize(0),
	 m_wRefCount(1),
	 m_wFlags(wFlags)
{
}

/** Standard destructor; Frees the internal buffer.
 */
str_data::~str_data()
{
	try
	{
		free_buffer();
	}
	catch(...)
	{
	}
}

/** Resets the object - frees buffer memory and sets internal members into the initial state.
 */
void str_data::free_buffer()
{
#ifdef ALLOW_UNICODE
	if (m_wFlags & SDF_UNICODE)
		delete [] (wchar_t*)m_pszBuffer;
	else
#endif
		delete [] m_pszBuffer;
	m_pszBuffer=NULL;
	m_tBufSize=0;
	m_wFlags=SDF_NONE;
}

/** Creates a duplicate of the string data object. Returned pointer is freshly
 *  allocated and needs to be destroyed.
 * \return Pointer to the newly allocated duplicate data object.
 */
str_data* str_data::dup()
{
	assert(m_pszBuffer);

	str_data* psd=new str_data(m_wFlags);
	psd->m_tBufSize=m_tBufSize;
#ifdef ALLOW_UNICODE
	if (m_wFlags & SDF_UNICODE)
	{
		psd->m_pszBuffer=(char_t*)new wchar_t[m_tBufSize];
		wcscpy((wchar_t*)psd->m_pszBuffer, (const wchar_t*)m_pszBuffer);
	}
	else
	{
#endif
		psd->m_pszBuffer=new char_t[m_tBufSize];
		strcpy(psd->m_pszBuffer, m_pszBuffer);
#ifdef ALLOW_UNICODE
	}
#endif

	return psd;
}

/** Resizes the internal string buffer to the new size (and copies the old buffer
 *  contents into the new one if needed). Does not provide the rounding capabilities.
 *  If the new size is 0 then the buffer is sized to the smallest possible value.
 * \param[in] tNewSize - specifies the new buffer size; if 0 then the CHUNK_INCSIZE is
 *                    taken as the size
 * \param[in] bCopy - if the current buffer contents should be copied to the new buffer
 * \note If the parameter bCopy is false then function puts the null character at the beginning
 *       of the buffer (sets the string length to 0).
 */
void str_data::resize_buffer(size_t tNewSize, bool bCopy)
{
	assert(m_wRefCount == 1);	// is this copy writable ?

	// make sure we resize to some reasonable value
	if (tNewSize == 0)
		tNewSize=CHUNK_INCSIZE;

#ifdef ALLOW_UNICODE
	char_t* psz;
	if (m_wFlags & SDF_UNICODE)
		psz=(char_t*)new wchar_t[tNewSize];
	else
		psz=new char_t[tNewSize];
#else
	char_t* psz=new char_t[tNewSize];
#endif
	
	// now copy if requested
	if (bCopy && m_pszBuffer)
	{
		size_t tCurrent=length();
		tCurrent=(tCurrent < tNewSize) ? tCurrent : tNewSize;

#ifdef ALLOW_UNICODE
		if (m_wFlags & SDF_UNICODE)
		{
			wcsncpy((wchar_t*)psz, (wchar_t*)m_pszBuffer, tCurrent);
			((wchar_t*)psz)[tCurrent]=L'\0';
		}
		else
		{
#endif
			strncpy(psz, m_pszBuffer, tCurrent);
			psz[tCurrent]='\0';
#ifdef ALLOW_UNICODE
		}
#endif

	}
	else
#ifdef ALLOW_UNICODE
	{
		if (m_wFlags & SDF_UNICODE)
			((wchar_t*)psz)[0]=L'\0';
		else
#endif
			psz[0]='\0';
#ifdef ALLOW_UNICODE
	}
#endif
	
	// store data
#ifdef ALLOW_UNICODE
	if (m_wFlags & SDF_UNICODE)
		delete [] (wchar_t*)m_pszBuffer;
	else
#endif
		delete [] m_pszBuffer;
	m_pszBuffer=psz;
	m_tBufSize=tNewSize;
}

/** Resizes the internal string buffer to the new size (and copies the old buffer
 *  contents into the new one if needed). Function checks if the buffer needs to be
 *  resized or not (the buffer is enlarged only if the new size is greater than the
 *  current buffer size; buffer is shrinked if the new size is smaller than
 *  (current size - CHUNK_DECSIZE).
 *  \param[in] tNewSize - specifies the new buffer size; if 0 then the CHUNK_INCSIZE is
 *                    taken as the size
 *  \param[in] bCopy - if the current buffer contents should be copied to the new buffer
 */
void str_data::resize_buffer_check(size_t tNewSize, bool bCopy)
{
	assert(m_wRefCount == 1);	// is this copy writable ?

	// make sure we resize to some reasonable value
	if (tNewSize == 0)
		tNewSize=CHUNK_INCSIZE;

	if (tNewSize >  m_tBufSize || tNewSize+CHUNK_DECSIZE < m_tBufSize)
		resize_buffer(tNewSize, bCopy);
}

/** Function calculates the length of a string in characters (doesn't matter if the string
 *  is unicode or ansi).
 *  \note All length checks should be done through this function, because of possible future
 *  update that will store the string length in the internal member.
 *  \return The string length in characters, not including the terminating '\\0'
 */
size_t str_data::length() const
{
	if (m_pszBuffer)
	{
#ifdef ALLOW_UNICODE
		if (m_wFlags & SDF_UNICODE)
			return wcslen((wchar_t*)m_pszBuffer);
		else
#endif
			return strlen(m_pszBuffer);
	}
	else
		return 0;
}

/** Function calculates the string length using length() function, but outputs
 *  the string length in bytes instead of characters.
 * \return The string length in bytes (including the terminating null character).
 */
size_t str_data::bytelen() const
{
	if (m_wFlags & SDF_UNICODE)
		return (length()+1)*sizeof(wchar_t);
	else
		return (length()+1);
}

#ifdef ALLOW_UNICODE
/** Changes the internal buffer type to unicode (if ansi). It means only that
 *  unicode flag is being set, and the buffer size (which is measured in chars)
 *  is modified according to the size of type wchar_t.
 */
void str_data::switch_unicode()
{
	assert(m_wRefCount == 1);
	if (!(m_wFlags & SDF_UNICODE))
	{
		m_wFlags |= SDF_UNICODE;
		m_tBufSize /= sizeof(wchar_t);
	}
}

/** Changes the internal buffer type to ansi (if unicode). It means only that
 *  unicode flag is being reset(set to 0), and the buffer size (which is measured in chars)
 *  is modified according to the size of type wchar_t.
 */
void str_data::switch_ansi()
{
	assert(m_wRefCount == 1);
	if (m_wFlags & SDF_UNICODE)
	{
		m_wFlags &= ~SDF_UNICODE;
		m_tBufSize *= sizeof(wchar_t);
	}
}
#endif

/** Assigns a new string buffer to this data object. If there was any other
 *  buffer allocated - it's freed before assign.
 * \param[in] pszSrc - buffer with the source string
 * \param[in] tLen - size of the buffer to be assigned (may be greater than the string length)
 * \note The tLen param should include the termination null character
 */
void str_data::assign(char_t* pszSrc, size_t tLen)
{
	assert(m_wRefCount == 1);
	
	// make sure the buffer is freed
	free_buffer();
	
	m_pszBuffer=pszSrc;
	m_tBufSize=tLen;
	m_wFlags &= ~SDF_UNICODE;
}

#ifdef ALLOW_UNICODE
/** Assigns a new string buffer to this data object. If there was any other
 *  buffer allocated - it's freed before assign.
 * \param[in] pszSrc - buffer with the source string
 * \param[in] tLen - size of the buffer to be assigned (may be greater than the string length)
 * \note The tLen param should include the termination null character
 */
void str_data::assign(wchar_t* pszSrc, size_t tLen)
{
	assert(m_wRefCount == 1);
	
	// make sure the buffer is freed
	free_buffer();
	
	m_pszBuffer=(char_t*)pszSrc;
	m_tBufSize=tLen;
	m_wFlags |= SDF_UNICODE;
}
#endif

/** Standard constructor - allocates the underlying data object
 */
string::string() :
	m_psd(NULL)
{
	m_psd=new str_data(SDF_NONE);
}

/** Constructor allocates the underlying data object and initializes it with
 *  a given ansi string.
 * \param[in] pszStr - source ansi string
 */
string::string(const char_t* pszStr) :
	m_psd(NULL)
{
	m_psd=new str_data(SDF_NONE);
	set_str(pszStr);
}

#ifdef ALLOW_UNICODE
/** Constructor allocates the underlying data object and initializes it with
 *  a given unicode string.
 * \param[in] pszStr - source unicode string
 */
string::string(const wchar_t* pszStr) :
	m_psd(NULL)
{
	m_psd=new str_data(SDF_UNICODE);
	set_str(pszStr);
}
#endif

/** Constructor increases the reference count in the parameter's data object
 *  and copies only the data object address.
 * \param[in] str - source string object
 */
string::string(const string& str) :
	m_psd(str.m_psd)
{
	m_psd->inc_refcount();
}

/** Destructor releases the underlying data object.
 */
string::~string()
{
	try
	{
		release_data();
	}
	catch(...)
	{
	}
}

/** Operator releases the current data object, stores a pointer to
 *  the data object from the given string object and increases a reference
 *  count.
 * \param[in] src - source string object
 * \return A reference to the current string.
 */
const string& string::operator=(const string& src)
{
	if (this != &src)
	{
		release_data();
		m_psd=src.m_psd;
		m_psd->inc_refcount();
	}

	return *this;
}

/** Operator makes an own copy of underlying data object (if needed) and copy
 *  there given ansi string.
 * \param[in] pszSrc - source ansi string
 * \return A reference to the current string object.
 */
const string& string::operator=(const char_t* pszSrc)
{
	set_str(pszSrc);
	
	return *this;
}

#ifdef ALLOW_UNICODE
/** Operator makes an own copy of underlying data object (if needed) and copy
 *  there the given unicode string.
 * \param[in] pszSrc - source unicode string
 * \return A reference to the current string object.
 */
const string& string::operator=(const wchar_t* pszSrc)
{
	set_str(pszSrc);
	
	return *this;
}
#endif

/** Operator concatenates a given string object with the current content of
 *  this string and returns a new string object.
 * \param[in] src - string object that will be appended
 * \return A new string object with concatenated strings.
 */
const string string::operator+(const string& src) const
{
	string str(*this);
	str.merge(src);
	
	return str;
}

/** Operator concatenates a given ansi string with the current content of
 *  this string and returns a new string object.
 * \param[in] pszSrc - ansi string that will be appended
 * \return A new string object with concatenated strings.
 */
const string string::operator+(const char_t* pszSrc) const
{
	string str(*this);
	str.merge(pszSrc);
	
	return str;
}

#ifdef ALLOW_UNICODE
/** Operator concatenates a given unicode string with the current content of
 *  this string and returns a new string object.
 * \param[in] pszSrc - unicode string that will be appended
 * \return A new string object with concatenated strings.
 */
const string string::operator+(const wchar_t* pszSrc) const
{
	string str(*this);
	str.merge(pszSrc);
	
	return str;
}
#endif

/** Operator appends a given string object to own internal buffer.
 * \param[in] src - string object that will be appended
 * \return A reference to this.
 */
const string& string::operator+=(const string& src)
{
	merge(src);
	return *this;
}

/** Operator appends a given ansi string to own internal buffer.
 * \param[in] pszSrc - ansi string that will be appended
 * \return A reference to this.
 */
const string& string::operator+=(const char_t* pszSrc)
{
	merge(pszSrc);
	return *this;
}

#ifdef ALLOW_UNICODE
/** Operator appends a given unicode string to own internal buffer.
 * \param[in] pszSrc - unicode string that will be appended
 * \return A reference to this.
 */
const string& string::operator+=(const wchar_t* pszSrc)
{
	merge(pszSrc);
	return *this;
}
#endif

/** Function counts the length of a string in characters (doesn't matter if the string
 *  is unicode or ansi).
 *  \note All length checks should be done through this function, because of possible future
 *  update that will store the string length in the internal member.
 *  \return The string length in characters, not including the terminating '\\0'
 */
size_t string::length() const
{
	return m_psd->length();
}

/** Function calculates the string length using length() function, but outputs
 *  the string length in bytes instead of characters.
 * \return The string length in bytes (including the terminating null character).
 */
size_t string::bytelen() const
{
	assert(m_psd);
	return m_psd->bytelen();
}

/** Function makes own data object writable and clears it. Does not delete the
 *  internal buffer - only sets the content to '\\0'.
 */
void string::clear()
{
	// make sure we have the modifiable object without allocated string buffer
	make_writable(MWF_DELETE);
}

/** Function checks if the string is empty.
 *  \return True if this string is empty, false otherwise.
 */
bool string::is_empty() const
{
	assert(m_psd != NULL);
	
	return (m_psd->m_pszBuffer == NULL) || (m_psd->m_pszBuffer[0] == '\0' 
#ifdef ALLOW_UNICODE
			&& (!(m_psd->m_wFlags & SDF_UNICODE) || m_psd->m_pszBuffer[1] == '\0')
#endif
										   );
}

/** Function merges the given ansi string with the current content of an internal buffer.
 * \param[in] pszSrc - ansi string to append
 */
void string::merge(const char_t* pszSrc)
{
	assert(pszSrc);

	make_writable(MWF_COPY);
	
	// append the second
#ifdef ALLOW_UNICODE
	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		// convert ansi->wide and append it to this
		wchar_t *psz=NULL;
		if (convert_to_unicode(pszSrc, &psz) > 0)
		{
			// how much memory do we need ?
			size_t tLen=length()+wcslen(psz)+1;
			tLen=ROUNDUP(tLen, CHUNK_INCSIZE);
			
			m_psd->resize_buffer_check(tLen, true);
			wcscat((wchar_t*)m_psd->m_pszBuffer, psz);
		}
		delete [] psz;
	}
	else
	{
#endif
		size_t tLen=length()+strlen(pszSrc)+1;
		tLen=ROUNDUP(tLen, CHUNK_INCSIZE);
		
		m_psd->resize_buffer_check(tLen, true);
		strcat(m_psd->m_pszBuffer, pszSrc);
#ifdef ALLOW_UNICODE
	}
#endif
}

#ifdef ALLOW_UNICODE
/** Function merges the given unicode string with the current content of an internal buffer.
 * \param[in] pszSrc - unicode string to append
 */
void string::merge(const wchar_t* pszSrc)
{
	assert(pszSrc);

	// make string unicode so we don't lose any unicode character when adding new string
	make_unicode();
	
	// append
	size_t tLen=length()+wcslen(pszSrc)+1;
	tLen=ROUNDUP(tLen, CHUNK_INCSIZE);
	
	m_psd->resize_buffer_check(tLen, true);
	wcscat((wchar_t*)m_psd->m_pszBuffer, pszSrc);
}
#endif

/** Function merges the given string object with the current content of an internal buffer.
 * \param[in] src - string object to append
 */
void string::merge(const string& src)
{
	make_writable(MWF_COPY);
	
	size_t tLen=src.length();
	if (tLen > 0)
	{
		tLen+=length()+1;
		tLen=ROUNDUP(tLen, CHUNK_INCSIZE);
		
		// append the second
#ifdef ALLOW_UNICODE
		if (m_psd->m_wFlags & SDF_UNICODE)
		{
			// resize buffer a bit
			m_psd->resize_buffer_check(tLen, true);
			
			if (src.m_psd->m_wFlags & SDF_UNICODE)
				wcscat((wchar_t*)m_psd->m_pszBuffer, (wchar_t*)src.m_psd->m_pszBuffer);
			else
			{
				// convert ansi->wide and append it to this
				wchar_t *psz=NULL;
				if (convert_to_unicode(src.m_psd->m_pszBuffer, &psz) > 0)
					wcscat((wchar_t*)m_psd->m_pszBuffer, psz);
				delete [] psz;
			}
		}
		else
		{
			if (src.m_psd->m_wFlags & SDF_UNICODE)
			{
				// source is unicode and this is ansi string - make this unicode not to lose any unicode char_t
				make_unicode();
				
				m_psd->resize_buffer_check(tLen, true);

				wcscat((wchar_t*)m_psd->m_pszBuffer, (wchar_t*)src.m_psd->m_pszBuffer);
			}
			else
			{
#endif
				m_psd->resize_buffer_check(tLen, true);
				strcat(m_psd->m_pszBuffer, src.m_psd->m_pszBuffer);
#ifdef ALLOW_UNICODE
			}
		}
#endif
	}
}

/** Returns a new string object with the left part of this string object.
 * \param[in] tLen - count of characters to copy to the new string object
 * \return The string with the left part of the current string.
 */
string string::left(size_t tLen) const
{
	assert(m_psd);
	string str;
	size_t tStrLen=length();
	tStrLen=minval(tStrLen, tLen);
	
#ifdef ALLOW_UNICODE
	if (is_unicode())
	{
		str.make_unicode();
		str.m_psd->resize_buffer_check(ROUNDUP((tStrLen+1), CHUNK_INCSIZE), false);
		wcsncpy((wchar_t*)str.m_psd->m_pszBuffer, (wchar_t*)m_psd->m_pszBuffer, tStrLen);
		((wchar_t*)str.m_psd->m_pszBuffer)[tStrLen]=L'\0';
	}
	else
	{
#endif
		str.m_psd->resize_buffer_check(ROUNDUP((tStrLen+1), CHUNK_INCSIZE), false);
		strncpy(str.m_psd->m_pszBuffer, m_psd->m_pszBuffer, tStrLen);
		str.m_psd->m_pszBuffer[tStrLen]='\0';
#ifdef ALLOW_UNICODE
	}
#endif
	return str;
}

/** Returns a new string object with the right part of this string object.
 * \param[in] tLen - count of characters to copy to the new string object
 * \return The string with the right part of the current string.
 */
string string::right(size_t tLen) const
{
	size_t tFullLen=length();
	size_t tStrLen=minval(tFullLen, tLen);
	string str;
#ifdef ALLOW_UNICODE
	if (is_unicode())
	{
		str.make_unicode();
		str.m_psd->resize_buffer_check(ROUNDUP((tStrLen+1), CHUNK_INCSIZE), false);
		wcsncpy((wchar_t*)str.m_psd->m_pszBuffer, ((wchar_t*)m_psd->m_pszBuffer)+tFullLen-tStrLen, tStrLen+1);
	}
	else
	{
#endif
		str.m_psd->resize_buffer_check(ROUNDUP((tStrLen+1), CHUNK_INCSIZE), false);
		strncpy(str.m_psd->m_pszBuffer, m_psd->m_pszBuffer+tFullLen-tStrLen, tStrLen+1);
#ifdef ALLOW_UNICODE
	}
#endif

	return str;
}

/** Returns a new string object with the middle part of this string object.
 * \param[in] tStart - position of the first character to copy
 * \param[in] tLen - count of chars to copy
 * \return The string with the middle part of the current string.
 */
string string::mid(size_t tStart, size_t tLen) const
{
	string str;
	size_t tStrLen=length();
	
	assert(tStart < tStrLen);		// make sure we start at the right place
	
	// check the count
	if (tLen > tStrLen-tStart)
		tLen=tStrLen-tStart;

#ifdef ALLOW_UNICODE
	if (is_unicode())
	{
		str.make_unicode();
		str.m_psd->resize_buffer_check(ROUNDUP(tLen, CHUNK_INCSIZE), false);
		wcsncpy((wchar_t*)str.m_psd->m_pszBuffer, ((wchar_t*)m_psd->m_pszBuffer)+tStart, tLen);
		((wchar_t*)str.m_psd->m_pszBuffer)[tStrLen]=L'\0';
	}
	else
	{
#endif
		str.m_psd->resize_buffer_check(ROUNDUP(tLen, CHUNK_INCSIZE), false);
		strncpy(str.m_psd->m_pszBuffer, m_psd->m_pszBuffer+tStart, tLen);
		(str.m_psd->m_pszBuffer)[tStrLen]='\0';
#ifdef ALLOW_UNICODE
	}
#endif
	
	return str;
}

/** Makes this string it's left part. Much faster than using standard
 *  left() function.
 * \param[in] tLen - count of characters at the beginning of the string to be left in a string.
 * \param[in] bReallocBuffer - if the internal string buffer is to be reallocated if exceeds
 *                             the allowable range size (CHUNK_INCSIZE, CHUNK_DECSIZE).
 * \see left()
 */
void string::left_self(size_t tLen, bool bReallocBuffer)
{
	assert(tLen <= length());
	
	// insert the '\0' at the requested position
#ifdef ALLOW_UNICODE
	if (is_unicode())
		((wchar_t*)m_psd->m_pszBuffer)[tLen]=L'\0';
	else
#endif
		m_psd->m_pszBuffer[tLen]='\0';
	
	if (bReallocBuffer)
		m_psd->resize_buffer_check(ROUNDUP(tLen, CHUNK_INCSIZE), true);
}

/** Makes this string it's right part. Much faster than using standard
 *  right() function.
 * \param[in] tLen - count of characters at the end of the string to be left in a string.
 * \param[in] bReallocBuffer - if the internal string buffer is to be reallocated if exceeds
 *                             the allowable range size (CHUNK_INCSIZE, CHUNK_DECSIZE).
 * \see right()
 */
void string::right_self(size_t tLen, bool bReallocBuffer)
{
	size_t tStrLen=length();
	assert(tLen <= tStrLen);

#ifdef ALLOW_UNICODE
	if (is_unicode())
		wmemmove((wchar_t*)m_psd->m_pszBuffer, ((wchar_t*)m_psd->m_pszBuffer)+tStrLen-tLen, tLen+1);
	else
#endif
		memmove(m_psd->m_pszBuffer, m_psd->m_pszBuffer+tStrLen-tLen, tLen+1);
	
	if (bReallocBuffer)
		m_psd->resize_buffer_check(ROUNDUP(tLen, CHUNK_INCSIZE), true);
}

/** Makes this string it's middle part. Much faster than using standard
 *  mid() function.
 * \param[in] tStart - starting position of a text to be left in a string
 * \param[in] tLen - count of characters at the middle of the string to be left in a string.
 * \param[in] bReallocBuffer - if the internal string buffer is to be reallocated if exceeds
 *                             the allowable range size (CHUNK_INCSIZE, CHUNK_DECSIZE).
 * \see mid()
 */
void string::mid_self(size_t tStart, size_t tLen, bool bReallocBuffer)
{
	size_t tStrLen=length();
	assert(tStart < tStrLen);

	// check the count
	if (tLen > tStrLen-tStart)
		tLen=tStrLen-tStart;

#ifdef ALLOW_UNICODE
	if (is_unicode())
	{
		wmemmove((wchar_t*)m_psd->m_pszBuffer, ((wchar_t*)m_psd->m_pszBuffer)+tStart, tLen);
		((wchar_t*)m_psd->m_pszBuffer)[tLen]=L'\0';
	}
	else
	{
#endif
		memmove(m_psd->m_pszBuffer, m_psd->m_pszBuffer+tStart, tLen);
		m_psd->m_pszBuffer[tLen]='\0';
#ifdef ALLOW_UNICODE
	}
#endif
	if (bReallocBuffer)
		m_psd->resize_buffer_check(ROUNDUP(tLen, CHUNK_INCSIZE), true);
}

/** Compares a string with the given ansi string. Comparison is case sensitive.
 * \param[in] psz - ansi string to which the string object will be compared
 * \return <0 if this string object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t string::cmp(const char_t* psz) const
{
	assert(psz);

#ifdef ALLOW_UNICODE
	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		char_t* pszConv=NULL;
		int_t iRes=-1;
		if (convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszConv) > 0)
			iRes=strcmp(psz, pszConv);

		delete [] pszConv;

		return iRes;
	}
	else
	{
#endif
		return strcmp(m_psd->m_pszBuffer, psz);
#ifdef ALLOW_UNICODE
	}
#endif
}

#ifdef ALLOW_UNICODE
/** Compares a string with the given unicode string. Comparison is case sensitive.
 * \param[in] psz - unicode string to which the string object will be compared
 * \return <0 if this string object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t string::cmp(const wchar_t* psz) const
{
	assert(psz);

	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		return wcscmp((wchar_t*)m_psd->m_pszBuffer, psz);
	}
	else
	{
		char_t* pszConv=NULL;
		int_t iRes=-1;
		if (convert_to_ansi(psz, &pszConv) > 0)
			iRes=strcmp(m_psd->m_pszBuffer, pszConv);

		delete [] pszConv;

		return iRes;
	}
}
#endif

/** Compares a string with the given string object. Comparison is case sensitive.
 * \param[in] str - string object to which internal string object will be compared
 * \return <0 if this string object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t string::cmp(const string& str) const
{
	assert(m_psd);
#ifdef ALLOW_UNICODE
	if (str.is_unicode())
	{
		if (is_unicode())
		{
			// both strings unicode
			return wcscmp((wchar_t*)str.m_psd->m_pszBuffer, (wchar_t*)m_psd->m_pszBuffer);
		}
		else
		{
			// only the external string is unicode
			char_t* pszConv=NULL;
			int_t iRes=-1;
			if (convert_to_ansi((wchar_t*)str.m_psd->m_pszBuffer, &pszConv) > 0)
				iRes=strcmp(pszConv, m_psd->m_pszBuffer);
				
			delete [] pszConv;
			
			return iRes;
		}
	}
	else
	{
		if (is_unicode())
		{
			// only internal string unicode
			char_t* pszConv=NULL;
			int_t iRes=-1;
			if (convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszConv) > 0)
				iRes=strcmp(pszConv, str.m_psd->m_pszBuffer);
				
			delete [] pszConv;
			
			return iRes;
		}
		else
		{
#endif
			// both string ansi
			return strcmp(str.m_psd->m_pszBuffer, m_psd->m_pszBuffer);
#ifdef ALLOW_UNICODE
		}
	}
#endif
}

/** Compares a string with the given ansi string. Comparison is case insensitive.
 * \param[in] psz - ansi string to which internal string object will be compared
 * \return <0 if this string object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t string::cmpi(const char_t* psz) const
{
	assert(psz);

#ifdef ALLOW_UNICODE
	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		char_t* pszConv=NULL;
		int_t iRes=-1;
		if (convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszConv) > 0)
			iRes=stricmp(psz, pszConv);
	
		delete [] pszConv;
	
		return iRes;
	}
	else
	{
#endif
		return stricmp(m_psd->m_pszBuffer, psz);
#ifdef ALLOW_UNICODE
	}
#endif
}

#ifdef ALLOW_UNICODE
/** Compares a string with the given unicode string. Comparison is case insensitive.
 * \param[in] psz - unicode string to which internal string object will be compared
 * \return <0 if this string object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t string::cmpi(const wchar_t* psz) const
{
	assert(psz);

	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		return wcsicmp((wchar_t*)m_psd->m_pszBuffer, psz);
	}
	else
	{
		char_t* pszConv=NULL;
		int_t iRes=-1;
		if (convert_to_ansi(psz, &pszConv) > 0)
			iRes=stricmp(m_psd->m_pszBuffer, pszConv);

		delete [] pszConv;

		return iRes;
	}
}
#endif

/** Compares a string with the given string object. Comparison is case insensitive.
 * \param[in] str - string object to which internal string object will be compared
 * \return <0 if this string object is "less" than str, 0 if they are equal and >0 otherwise.
 */
int_t string::cmpi(const string& str) const
{
#ifdef ALLOW_UNICODE
	if (str.is_unicode())
	{
		if (is_unicode())
		{
				// both strings unicode
			return wcsicmp((wchar_t*)str.m_psd->m_pszBuffer, (wchar_t*)m_psd->m_pszBuffer);
		}
		else
		{
				// only the external string is unicode
			char_t* pszConv=NULL;
			int_t iRes=-1;
			if (convert_to_ansi((wchar_t*)str.m_psd->m_pszBuffer, &pszConv) > 0)
				iRes=stricmp(pszConv, m_psd->m_pszBuffer);
					
			delete [] pszConv;
				
			return iRes;
		}
	}
	else
	{
		if (is_unicode())
		{
				// only internal string unicode
			char_t* pszConv=NULL;
			int_t iRes=-1;
			if (convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszConv) > 0)
				iRes=stricmp(pszConv, str.m_psd->m_pszBuffer);
					
			delete [] pszConv;
				
			return iRes;
		}
		else
		{
#endif
			// both string ansi
			return stricmp(str.m_psd->m_pszBuffer, m_psd->m_pszBuffer);
#ifdef ALLOW_UNICODE
		}
	}
#endif
}

/** Function compares a given count of characters of own internal buffer content
 *  with a given ansi string. Comparison is case sensitive.
 * \param[in] psz - ansi string to compare
 * \param[in] tLen - count of characters to compare (-1 to detect shorter string)
 * \param[in] bLenCheck - causes the function to check if the len given as a param
 *                        is valid (if not then -2 is returned)
 * \return <0 if this string object is "less" than str, 0 if they are equal and >0 otherwise.
 * \note Do not use tLen=-1 and bLenCheck=true - it will degrade performance.
 */
int_t string::cmpn(const char_t* psz, size_t tLen, bool bLenCheck) const
{
	assert(psz);

	// if len is -1 then set len to the shorter len
	if (tLen == (size_t)-1)
	{
		tLen=length();
		size_t t=strlen(psz);
		if (tLen > t)
			tLen=t;
	}
	
	// make a length check to avoid running out of range
	if (bLenCheck && (tLen > length() || tLen > strlen(psz)))
		return -2;
	
#ifdef ALLOW_UNICODE
	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		char_t* pszConv=NULL;
		int_t iRes=-2;
		if (convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszConv) > 0)
			iRes=strncmp(psz, pszConv, tLen);
	
		delete [] pszConv;
	
		return iRes;
	}
	else
	{
#endif
		return strncmp(m_psd->m_pszBuffer, psz, tLen);
#ifdef ALLOW_UNICODE
	}
#endif
}

#ifdef ALLOW_UNICODE
/** Function compares a given count of characters of own internal buffer content
 *  with a given unicode string. Comparison is case sensitive.
 * \param[in] psz - unicode string to compare
 * \param[in] tLen - count of characters to compare (-1 to detect shorter string)
 * \param[in] bLenCheck - causes the function to check if the len given as a param
 *                        is valid (if not then -2 is returned)
 * \return <0 if this string object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t string::cmpn(const wchar_t* psz, size_t tLen, bool bLenCheck) const
{
	assert(psz);

	// if len is -1 then set len to the shorter len
	if (tLen == (size_t)-1)
	{
		tLen=length();
		size_t t=wcslen(psz);
		if (tLen > t)
			tLen=t;
	}
	
	// make a length check to avoid running out of range
	if (bLenCheck && (tLen > length() || tLen > wcslen(psz)))
		return -2;

	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		return wcsncmp((wchar_t*)m_psd->m_pszBuffer, psz, tLen);
	}
	else
	{
		char_t* pszConv=NULL;
		int_t iRes=-2;
		if (convert_to_ansi(psz, &pszConv) > 0)
			iRes=strncmp(m_psd->m_pszBuffer, pszConv, tLen);

		delete [] pszConv;

		return iRes;
	}
}
#endif

/** Function compares a given count of characters of own internal buffer content
 *  with a given string object. Comparison is case sensitive.
 * \param[in] str - string object to compare
 * \param[in] tLen - count of characters to compare (-1 to detect shorter string)
 * \param[in] bLenCheck - causes the function to check if the len given as a param
 *                        is valid (if not then -2 is returned)
 * \return <0 if this string object is "less" than str, 0 if they are equal and >0 otherwise.
 */
int_t string::cmpn(const string& str, size_t tLen, bool bLenCheck) const
{
	// if len is -1 then set len to the shorter len
	if (tLen == (size_t)-1)
	{
		tLen=length();
		size_t t=str.length();
		if (tLen > t)
			tLen=t;
	}
	
	// make a length check to avoid running out of range
	if (bLenCheck && (tLen > length() || tLen > str.length()))
		return -2;

#ifdef ALLOW_UNICODE
	if (str.is_unicode())
	{
		if (is_unicode())
		{
				// both strings unicode
			return wcsncmp((wchar_t*)str.m_psd->m_pszBuffer, (wchar_t*)m_psd->m_pszBuffer, tLen);
		}
		else
		{
				// only the external string is unicode
			char_t* pszConv=NULL;
			int_t iRes=-2;
			if (convert_to_ansi((wchar_t*)str.m_psd->m_pszBuffer, &pszConv) > 0)
				iRes=strncmp(pszConv, m_psd->m_pszBuffer, tLen);
					
			delete [] pszConv;
				
			return iRes;
		}
	}
	else
	{
		if (is_unicode())
		{
				// only internal string unicode
			char_t* pszConv=NULL;
			int_t iRes=-2;
			if (convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszConv) > 0)
				iRes=strncmp(pszConv, str.m_psd->m_pszBuffer, tLen);
					
			delete [] pszConv;
				
			return iRes;
		}
		else
		{
#endif
			// both string ansi
			return strncmp(str.m_psd->m_pszBuffer, m_psd->m_pszBuffer, tLen);
#ifdef ALLOW_UNICODE
		}
	}
#endif
}

/** Function compares a given count of characters of own internal buffer content
 *  with a given ansi string. Comparison is case insensitive.
 * \param[in] psz - ansi string to compare
 * \param[in] tLen - count of characters to compare (-1 to detect shorter string)
 * \param[in] bLenCheck - causes the function to check if the len given as a param
 *                        is valid (if not then -2 is returned)
 * \return <0 if this string object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t string::cmpin(const char_t* psz, size_t tLen, bool bLenCheck) const
{
	assert(psz);

	// if len is -1 then set len to the shorter len
	if (tLen == (size_t)-1)
	{
		tLen=length();
		size_t t=strlen(psz);
		if (tLen > t)
			tLen=t;
	}
	
	// make a length check to avoid running out of range
	if (bLenCheck && (tLen > length() || tLen > strlen(psz)))
		return -2;

#ifdef ALLOW_UNICODE
	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		char_t* pszConv=NULL;
		int_t iRes=-1;
		if (convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszConv) > 0)
			iRes=strnicmp(psz, pszConv, tLen);
	
		delete [] pszConv;
	
		return iRes;
	}
	else
	{
#endif
		return strnicmp(m_psd->m_pszBuffer, psz, tLen);
#ifdef ALLOW_UNICODE
	}
#endif
}

#ifdef ALLOW_UNICODE
/** Function compares a given count of characters of own internal buffer content
 *  with a given unicode string. Comparison is case insensitive.
 * \param[in] psz - unicode string to compare
 * \param[in] tLen - count of characters to compare (-1 to detect shorter string)
 * \param[in] bLenCheck - causes the function to check if the len given as a param
 *                        is valid (if not then -2 is returned)
 * \return <0 if this string object is "less" than psz, 0 if they are equal and >0 otherwise.
 */
int_t string::cmpin(const wchar_t* psz, size_t tLen, bool bLenCheck) const
{
	assert(psz);

	// if len is -1 then set len to the shorter len
	if (tLen == (size_t)-1)
	{
		tLen=length();
		size_t t=wcslen(psz);
		if (tLen > t)
			tLen=t;
	}
	
	// make a length check to avoid running out of range
	if (bLenCheck && (tLen > length() || tLen > wcslen(psz)))
		return -2;

	if (m_psd->m_wFlags & SDF_UNICODE)
	{
		return wcsnicmp((wchar_t*)m_psd->m_pszBuffer, psz, tLen);
	}
	else
	{
		char_t* pszConv=NULL;
		int_t iRes=-1;
		if (convert_to_ansi(psz, &pszConv) > 0)
			iRes=strnicmp(m_psd->m_pszBuffer, pszConv, tLen);

		delete [] pszConv;

		return iRes;
	}
}
#endif

/** Function compares a given count of characters of own internal buffer content
 *  with a given string object. Comparison is case insensitive.
 * \param[in] str - string object to compare
 * \param[in] tLen - count of characters to compare (-1 to detect shorter string)
 * \param[in] bLenCheck - causes the function to check if the len given as a param
 *                        is valid (if not then -2 is returned)
 * \return <0 if this string object is "less" than str, 0 if they are equal and >0 otherwise.
 */
int_t string::cmpin(const string& str, size_t tLen, bool bLenCheck) const
{
	// if len is -1 then set len to the shorter len
	if (tLen == (size_t)-1)
	{
		tLen=length();
		size_t t=str.length();
		if (tLen > t)
			tLen=t;
	}
	
	// make a length check to avoid running out of range
	if (bLenCheck && (tLen > length() || tLen > str.length()))
		return -2;

#ifdef ALLOW_UNICODE
	if (str.is_unicode())
	{
		if (is_unicode())
		{
			// both strings unicode
			return wcsnicmp((wchar_t*)str.m_psd->m_pszBuffer, (wchar_t*)m_psd->m_pszBuffer, tLen);
		}
		else
		{
			// only the external string is unicode
			char_t* pszConv=NULL;
			int_t iRes=-1;
			if (convert_to_ansi((wchar_t*)str.m_psd->m_pszBuffer, &pszConv) > 0)
				iRes=strnicmp(pszConv, m_psd->m_pszBuffer, tLen);

			delete [] pszConv;

			return iRes;
		}
	}
	else
	{
		if (is_unicode())
		{
			// only internal string unicode
			char_t* pszConv=NULL;
			int_t iRes=-1;
			if (convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszConv) > 0)
				iRes=strnicmp(pszConv, str.m_psd->m_pszBuffer, tLen);
						
			delete [] pszConv;
					
			return iRes;
		}
		else
		{
#endif
			// both string ansi
			return strnicmp(str.m_psd->m_pszBuffer, m_psd->m_pszBuffer, tLen);
#ifdef ALLOW_UNICODE
		}
	}
#endif
}

/** Returns a character at a given position. Function is very slow (needs to recalc the size of the string
 *  and make a few comparisons), but quite safe - if the index is out of range then -1 is returned.
 *  Make sure to interpret the returned character according to unicode flag. If the string is unicode, then the
 *  character returned is also unicode (and vice versa).
 * \param[in] tPos - index of the character to return.
 * \return Character code of character on a specified position, or -1 if out of range.
 */
int_t string::at(size_t tPos) const
{
	size_t tSize=length();		// very slow
#ifdef ALLOW_UNICODE
	if (is_unicode())
	{
		if (tPos < tSize)
			return (int_t)(((wchar_t*)m_psd->m_pszBuffer)[tPos]);
		else
			return -1;
	}
	else
	{
#endif
		if (tPos < tSize)
			return (int_t)(((char_t*)m_psd->m_pszBuffer)[tPos]);
		else
			return -1;
#ifdef ALLOW_UNICODE
	}
#endif
}

/** Returns a pointer to the ansi internal buffer. If the buffer is in unicode format
 *  then NULL value is returned. Internal buffer is resized to the specified value
 *  if currently smaller than requested (if -1 is specified as tMinSize then the buffer
 *  is not resized, and the return value could be NULL).
 * \param[in] tMinSize - requested minimal size of the internal buffer (-1 if the size is not to be changed).
 * \return Pointer to the internal ansi buffer.
 */
char_t* string::get_buffera(size_t tMinSize)
{
#ifdef ALLOW_UNICODE
	assert(!(m_psd->m_wFlags & SDF_UNICODE));
	
	if (m_psd->m_wFlags & SDF_UNICODE)
		return NULL;
#endif

	// resize buffer if needed
	if (tMinSize != (size_t)-1 && tMinSize > m_psd->m_tBufSize)
		m_psd->resize_buffer_check(ROUNDUP(tMinSize, CHUNK_INCSIZE), true);
	
	return m_psd->m_pszBuffer;
}

#ifdef ALLOW_UNICODE
/** Returns a pointer to the unicode internal buffer. If the buffer is in ansi format
 *  then NULL value is returned. Internal buffer is resized to the specified value
 *  if currently smaller than requested (if -1 is specified as tMinSize then the buffer
 *  is not resized, and the return value could be NULL).
 * \param[in] tMinSize - requested minimal size of the internal buffer (-1 if the size of the string should not be changed)
 * \return Pointer to the internal unicode buffer.
 */
wchar_t* string::get_bufferu(size_t tMinSize)
{
	assert(m_psd->m_wFlags & SDF_UNICODE);
	
	if (!(m_psd->m_wFlags & SDF_UNICODE))
		return NULL;
	
	if (tMinSize != (size_t)-1 && tMinSize > m_psd->m_tBufSize)
		m_psd->resize_buffer_check(ROUNDUP(tMinSize, CHUNK_INCSIZE), true);

	return (wchar_t*)m_psd->m_pszBuffer;
}
#endif

/** Releases buffer got by user by calling get_bufferx functions. The current
 *  job of this function is to make sure the string will terminate with null
 *  character at the end of the buffer.
 */
void string::release_buffer() const
{
	// just to make sure user does not crash everything
#ifdef ALLOW_UNICODE
	if (is_unicode())
		((wchar_t*)m_psd->m_pszBuffer)[m_psd->m_tBufSize-1]=L'\0';
	else
#endif
		m_psd->m_pszBuffer[m_psd->m_tBufSize-1]='\0';
}

#ifdef ALLOW_UNICODE
/** Converts the internal buffer contents to the unicode format (if not already
 *  unicode).
 */
void string::make_unicode()
{
	assert(m_psd);
	if (!is_unicode())
	{
		// make a string conversion
		if (m_psd->m_pszBuffer)
		{
			wchar_t* pszOut;
			size_t tSize=convert_to_unicode(m_psd->m_pszBuffer, &pszOut);
			
			// delete the current buffer
			make_writable(MWF_DELETE);
			
			// assign a new buffer
			m_psd->assign(pszOut, tSize);
		}
		else
			m_psd->switch_unicode();
	}
}

/** Converts the internal buffer contents to the ansi format (if not already
 *  ansi).
 */
void string::make_ansi()
{
	if (is_unicode())
	{
		if (m_psd->m_pszBuffer)
		{
			// make a string conversion
			char_t* pszOut;
			size_t tSize=convert_to_ansi((wchar_t*)m_psd->m_pszBuffer, &pszOut);
			
			// delete the current buffer
			make_writable(MWF_DELETE);
			
			// assign a new buffer
			m_psd->assign(pszOut, tSize);
		}
		else
			m_psd->switch_ansi();
	}
}
#endif

/** Dumps internal contents of this class.
 * \param[in] pctx - dump context to receive object info
 */
void string::dump(dumpctx* pctx)
{
	pctx->open("string");
	pctx->dump("m_psd", (ptr_t)m_psd);
#ifdef ALLOW_UNICODE
	if (is_unicode())
		pctx->dump("m_psd->m_pszBuffer [unicode]", (wchar_t*)m_psd->m_pszBuffer);
	else
#endif
		pctx->dump("m_psd->m_pszBuffer [ansi]", m_psd->m_pszBuffer);
	
	pctx->dump("m_psd->m_tBufSize", (longlong_t)m_psd->m_tBufSize);
	pctx->dump("m_psd->m_wRefCount", m_psd->m_wRefCount);
	pctx->dump("m_psd->m_wFlags", m_psd->m_wFlags);
	pctx->close();
}

/** Displays the string contents on screen using standard printf function. The format of displayed
 *  string is either ansi("string_content") or unicode("string_content").
 */
void string::print() const
{
#ifdef ALLOW_UNICODE
	if (is_unicode())
		printf("unicode(\"" WSTRFMT "\")", (wchar_t*)m_psd->m_pszBuffer);
	else
#endif
		printf("ansi(\"" STRFMT "\")", (char_t*)m_psd->m_pszBuffer);
}

/** Cast operator - tries to return a pointer to char_t* using the current internal
 *  buffer. If the internal buffer is in unicode format, then the debug version asserts
 *  and release return NULL.
 * \return Pointer to an ansi string (could be null).
 */
string::operator const char_t*() const
{
	if (m_psd->m_pszBuffer)
	{
#ifdef ALLOW_UNICODE
		if (m_psd->m_wFlags & SDF_UNICODE)
		{
			// what to do here - convert using external buffer or assert ?
			assert(false);		// cannot cast unicode string to the ansi one using operator
			return NULL;
		}
		else
#endif
			return m_psd->m_pszBuffer;
	}
	else
		return "";
}

#ifdef ALLOW_UNICODE
/** Cast operator - tries to return a pointer to wchar_t* using the current internal
 *  buffer. If the internal buffer is in ansi format, then the debug version asserts
 *  and release return NULL.
 * \return Pointer to an unicode string (could be null).
 */
string::operator const wchar_t*() const
{
	assert(m_psd);
	if (m_psd->m_pszBuffer)
	{
		if (m_psd->m_wFlags & SDF_UNICODE)
			return (wchar_t*)m_psd->m_pszBuffer;
		else
		{
			assert(false);	// cannot cast ansi string to unicode using operator
			return NULL;
		}
	}
	else
		return L"";
}
#endif

#ifdef ALLOW_UNICODE
/** Function converts unicode string into the ansi one. The output buffer is
 *  allocated by this function and needs to be deleted by user.
 * \param[in] pszIn - unicode string to be converted
 * \param[out] pszOut - ptr to the char_t* that will receive new buffer's address
 * \return Size of the output string (including the terminating '\\0' character).
 * \todo Add detecting default system charset (linux) instead of hard-coding iso8859-2.
 */
size_t string::convert_to_ansi(const wchar_t* pszIn, char_t** pszOut)
{
	assert(pszIn && pszOut);
	
#ifdef _WIN32
	size_t tLen;
	if ( (tLen=(size_t)WideCharToMultiByte(CP_ACP, 0, pszIn, -1, NULL, 0, NULL, NULL)) != 0 )
	{
		*pszOut=new char_t[tLen];
		return (size_t)WideCharToMultiByte(CP_ACP, 0, pszIn, -1, *pszOut, (int_t)tLen, NULL, NULL);
	}
	else
	{
		*pszOut=NULL;
		return 0;
	}
#else
	// FIXME: the destination charset should be the default system one and not hard-coded iso8859-2
	// NOTE: do not use this func to convert strings from/to a file (especially on gnu linux
	// where unicode (wchar_t) characters has 4 bytes instead of 2
	iconv_t it=iconv_open("ISO8859-2", "WCHAR_T");

	// alloc some memory
	size_t tOutLen=wcslen(pszIn)+1, tInLen=tOutLen*sizeof(wchar_t);
	size_t tTotal=tOutLen;
	char_t* psz=new char_t[tInLen], *pszStart=psz;
	
	if (iconv(it, (char_t**)&pszIn, &tInLen, (char_t**)&psz, &tOutLen) != (size_t)-1)
	{
		// close conversion object
		iconv_close(it);
		
		// store ptr
		*pszOut=pszStart;
		return tTotal-tOutLen;
	}
	else
	{
		// close conversion object
		iconv_close(it);
		
		// return nothing
		delete [] psz;
		*pszOut=NULL;
		return 0;
	}
#endif
}

/** Function converts ansi string into the unicode one. The output buffer is
 *  allocated by this function and needs to be deleted by user.
 * \param[in] pszIn - ansi string to be converted
 * \param[out] pszOut - ptr to the wchar_t* that will receive new buffer's address
 * \return Size of the output string (including the terminating '\\0' character).
 * \todo Add detecting default system charset (linux) instead of hard-coding iso8859-2.
 */
size_t string::convert_to_unicode(const char_t* pszIn, wchar_t** pszOut)
{
	assert(pszIn && pszOut);

#ifdef _WIN32
	size_t tLen;
	if ( (tLen=(size_t)MultiByteToWideChar(CP_ACP, 0, pszIn, -1, NULL, 0)) != 0 )
	{
		*pszOut=new wchar_t[tLen];
		return (size_t)MultiByteToWideChar(CP_ACP, 0, pszIn, -1, *pszOut, (int_t)tLen);
	}
	else
	{
		*pszOut=NULL;
		return 0;
	}
#else
	// FIXME: correct the character set from hard-coded iso8859-2 to the native system's one
	// NOTE: do not use to convert charsets when reading/writing to file
	iconv_t it=iconv_open("WCHAR_T", "ISO8859-2");
	if (it == (iconv_t)-1)
	{
		*pszOut=NULL;
		return 0;
	}
	
	// alloc some memory
	// NOTE: outlen is only inlen*sizeof(wchar_t) because of 4-byte wchar_t chars in gnu/linux
	size_t tInLen=strlen(pszIn)+1, tOutLen=tInLen*sizeof(wchar_t);
	size_t tTotal=tInLen;
	wchar_t* psz=new wchar_t[tInLen], *pszStart=psz;
	
	if (iconv(it, (char_t**)&pszIn, &tInLen, (char_t**)&psz, &tOutLen) != (size_t)-1)
	{
		// close conversion object
		iconv_close(it);
		
		// store the pointer
		*pszOut=pszStart;
		
		return tTotal-tOutLen;
	}
	else
	{
		// close conversion handle
		iconv_close(it);
		
		// return nothing
		delete [] pszStart;
		*pszOut=NULL;
		return 0;
	}
#endif
}
#endif

/** Function makes the internal data object writable, copies the given ansi string into
 *  the internal string buffer and removes the unicode flag if needed.
 * \param[in] pszStr - source ansi string
 */
void string::set_str(const char_t* pszStr)
{
	assert(pszStr);

	// make writable without allocating any buffer
	make_writable(MWF_PARTIALDEL);
	
	// make sure the buffer is in ansi mode
	m_psd->switch_ansi();

	size_t tLen=strlen(pszStr)+1;		// new str length
	m_psd->resize_buffer_check(ROUNDUP(tLen, CHUNK_INCSIZE), false);
	strcpy(m_psd->m_pszBuffer, pszStr);
}

#ifdef ALLOW_UNICODE
/** Function makes the internal data object writable, copies the given unicode string into
 *  the internal string buffer and sets the unicode flag if needed.
 * \param[in] pszStr - source unicode string
 */
void string::set_str(const wchar_t* pszStr)
{
	assert(pszStr);

	make_writable(MWF_PARTIALDEL);
	m_psd->switch_unicode();

	size_t tLen=wcslen(pszStr)+1;		// new str length
	m_psd->resize_buffer_check(ROUNDUP(tLen, CHUNK_INCSIZE), false);
	wcscpy((wchar_t*)m_psd->m_pszBuffer, pszStr);
}
#endif

/** Function prepares the internal data object to make any modifications to it.
 *  If the reference count of the underlying data object is greater than 1 (it means
 *  more than 1 string is using that object) the function makes an own copy of it, else
 *  nothing is done.
 * \param[in] iType - type of making writable (MWF_*)
 * \param[in] pParam - type dependent parameter
 */
void string::make_writable(int_t iType, ptr_t pParam)
{
	assert(m_psd->m_wRefCount > 0);
	
	// check if we need to make our own copy of data
	if (m_psd->m_wRefCount > 1)
	{
		switch(iType)
		{
			case MWF_COPY:
			{
				// make a copy of the internal data object and set it as internal after releasing the old one
				str_data* psd=m_psd->dup();
				release_data();
				m_psd=psd;
				break;
			}
			case MWF_PARTIALDEL:
			case MWF_DELETE:
			{
				// alloc only the internal data object and do not allocate the string buffer
				str_data* psd=new str_data(m_psd->m_wFlags);
				release_data();
				m_psd=psd;
				break;
			}
			case MWF_NEW:
			{
				// alloc the new internal string data object and make there an empty string
				str_data* psd=new str_data(m_psd->m_wFlags);
				psd->resize_buffer_check(ROUNDUP((size_t)pParam, CHUNK_INCSIZE), false);
				release_data();
				m_psd=psd;
				break;
			}
			default:
				assert(false);		// some strange flag
				break;
		}
	}
	else
	{
		switch(iType)
		{
			case MWF_COPY:
			{
				// do nothing - the object is in requested state
				break;
			}
			case MWF_DELETE:
			{
				// the thing to do is to delete the internal string without deleting the internal data object
				m_psd->free_buffer();
				break;
			}
			case MWF_NEW:
			{
				// only resize the internal buffer
				m_psd->resize_buffer_check(ROUNDUP((size_t)pParam, CHUNK_INCSIZE), false);
				break;
			}
			case MWF_PARTIALDEL:
			{
				// left the string buffer as is
				break;
			}
			default:
				assert(false);		// some strange flag
		}
	}
}

/** Function releases the underlying data object. It means that reference count for
 *  that object is decreased and if the ref count is 0 - the object is freed.
 */
void string::release_data()
{
	assert(m_psd);
	if (m_psd->dec_refcount())
		delete m_psd;
	m_psd=NULL;
}

END_ICPF_NAMESPACE
