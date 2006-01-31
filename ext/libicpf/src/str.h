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
#ifndef __STR_H__
#define __STR_H__

/** \file str.h
 *  \brief Contain string management classes
 */
 
#include "libicpf.h"
#include "gen_types.h"
#include <string.h>
#include "macros.h"

#ifdef ALLOW_UNICODE
	#include <wchar.h>
#endif

#ifndef _WIN32
	#include <iconv.h>
#endif

BEGIN_ICPF_NAMESPACE

/// External class - to avoid inclusion of dumpctx.h
class dumpctx;

///////////////////////////////////////////////////////////////
// str_data flags
/// Standard string data flags
#define SDF_NONE		0x0000

/// The string is unicode
#define SDF_UNICODE		0x0001

//////////////////////////////////////////////////////////////
// make_writable flags
/// Standard - makes a copy of an underlying object if reference count >1
#define MWF_COPY	0x00000000
/// Causes the internal string buffer to be deleted
#define MWF_DELETE	0x00000001
/// Causes new buffer to be allocated with a specified size (empty one)
#define MWF_NEW		0x00000002
/** \brief Partial delete.
 *
 *  If the object has to be allocated then it's done *without* allocating the internal buffer.
 *  Else the buffer is left as is.
 */
#define MWF_PARTIALDEL	0x00000003

/** \brief String storage class (helper)
 *
 *  The class is used by the ch::string class as a helper that contains
 *  all of the string data. Allows to use the reference counting strings.
 */
class LIBICPF_API str_data
{
public:
/** \name Construction/destruction */
/*@{*/
	str_data(short_t wFlags);
	~str_data();
/*@}*/
	
/** \name Operations */
/*@{*/
	/// Increases reference count
	void inc_refcount() { ++m_wRefCount; };
	
	/** \brief Decreases refcount
	 *
	 * \return True when it's safe to delete this object or false if not.
	 */
	bool dec_refcount() { if (--m_wRefCount > 0) return false; else return true; };

	// sizes the buffer with and wo size checking
	void resize_buffer(size_t tNewSize, bool bCopy);			///< Changes the size of the internal buffer with size checking
	void resize_buffer_check(size_t tNewSize, bool bCopy);	///< Changes the size of the internal string buffer without size checking
	
	// length checks
	size_t length() const;	///< Returns the length of this string in chars (without terminating null character)
	size_t bytelen() const;	///< Returns the length of a string in bytes (with terminating null character)
	
#ifdef ALLOW_UNICODE
	void switch_unicode();	///< Changes the internal buffer type to unicode (nullifies the string)
	void switch_ansi();		///< Changes the internal buffer type to ansi (nullifies the string)
#endif

	
	void assign(char_t* pszSrc, size_t tLen);			///< Assigns an ansi buffer as the internal string data
#ifdef ALLOW_UNICODE
	void assign(wchar_t* pszSrc, size_t tLen);		///< Assigns an unicode buffer as the internal string data
#endif
	
	/// Frees the internal buffer
	void free_buffer();
	
	/// Duplicates this object
	str_data* dup();
	
/*@}*/

public:
	char_t* m_pszBuffer;		///< Contents of the string (could be the wchar_t*)
	size_t m_tBufSize;			///< Size of the buffer (in chars, *NOT* bytes)
	short_t m_wRefCount;		///< Reference count
	short_t m_wFlags;			///< Flags
};

///////////////////////////////////////////////////////////////
// string manipulation class
/** \brief String manipulation class
 *
 *  Class allows user to manipulate string objects (either standard ANSI char_t*
 *  based strings or UNICODE ones - wchar_t related) with simple functions and
 *  operators.
 */
class LIBICPF_API string
{
public:
/** \name Construction/destruction */
/*@{*/
	string();						///< Standard constructor
	string(const char_t* pszStr);		///< Constructor that takes const char_t* as an initial string
#ifdef ALLOW_UNICODE
	string(const wchar_t* pszStr);	///< Constructor that takes const wchar_t* as an initial string
#endif
	string(const string& str);		///< Standard copy constructor
	
	~string();						///< Standard destructor
/*@}*/

/** \name Operators */
/**@{*/
	// assignment
	const string& operator=(const string& src);			///< Assign operator for string objects
	const string operator+(const string& src) const;	///< Concatenate operator for string objects
	const string& operator+=(const string& src);		///< Merge operator for string objects
	
	const string& operator=(const char_t* pszSrc);		///< Assign operator for ansi strings
	const string operator+(const char_t* pszSrc) const;	///< Concatenate operator for ansi strings
	const string& operator+=(const char_t* pszSrc);		///< Merge operator for ansi strings

#ifdef ALLOW_UNICODE
	const string& operator=(const wchar_t* pszSrc);			///< Assign operator from unicode strings
	const string operator+(const wchar_t* pszSrc) const;	///< Concatenate operator for unicode strings
	const string& operator+=(const wchar_t* pszSrc);		///< Merge operator for unicode strings
#endif
	
	// comparison
	/// Makes case sensitive comparison to the ansi string ( see cmp(const char_t* psz) )
	bool operator<(const char_t* psz) const { return cmp(psz) < 0; };
	/// Makes case sensitive comparison to the ansi string ( see cmp(const char_t* psz) )
	bool operator<=(const char_t* psz) const { return cmp(psz) <= 0; };
	/// Makes case sensitive comparison to the ansi string ( see cmp(const char_t* psz) )
	bool operator==(const char_t* psz) const { return cmp(psz) == 0; };
	/// Makes case sensitive comparison to the ansi string ( see cmp(const char_t* psz) )
	bool operator>=(const char_t* psz) const { return cmp(psz) >= 0; };
	/// Makes case sensitive comparison to the ansi string ( see cmp(const char_t* psz) )
	bool operator>(const char_t* psz) const { return cmp(psz) > 0; };
	/// Makes case sensitive comparison to the ansi string ( see cmp(const char_t* psz) )
	bool operator!=(const char_t* psz) const { return cmp(psz) != 0; };
	
	/// Makes case sensitive comparison to the unicode string ( see cmp(const wchar_t* psz) )
	bool operator<(const wchar_t* psz) const { return cmp(psz) < 0; };
	/// Makes case sensitive comparison to the unicode string ( see cmp(const wchar_t* psz) )
	bool operator<=(const wchar_t* psz) const { return cmp(psz) <= 0; };
	/// Makes case sensitive comparison to the unicode string ( see cmp(const wchar_t* psz) )
	bool operator==(const wchar_t* psz) const { return cmp(psz) == 0; };
	/// Makes case sensitive comparison to the unicode string ( see cmp(const wchar_t* psz) )
	bool operator>=(const wchar_t* psz) const { return cmp(psz) >= 0; };
	/// Makes case sensitive comparison to the unicode string ( see cmp(const wchar_t* psz) )
	bool operator>(const wchar_t* psz) const { return cmp(psz) > 0; };
	/// Makes case sensitive comparison to the unicode string ( see cmp(const wchar_t* psz) )
	bool operator!=(const wchar_t* psz) const { return cmp(psz) != 0; };
	
#ifdef ALLOW_UNICODE
	/// Makes case sensitive comparison to the string object ( see cmp(const string& str) )
	bool operator<(const string& str) const { return cmp(str) < 0; };
	/// Makes case sensitive comparison to the string object ( see cmp(const string& str) )
	bool operator<=(const string& str) const { return cmp(str) <= 0; };
	/// Makes case sensitive comparison to the string object ( see cmp(const string& str) )
	bool operator==(const string& str) const { return cmp(str) == 0; };
	/// Makes case sensitive comparison to the string object ( see cmp(const string& str) )
	bool operator>=(const string& str) const { return cmp(str) >= 0; };
	/// Makes case sensitive comparison to the string object ( see cmp(const string& str) )
	bool operator>(const string& str) const { return cmp(str) >= 0; };
	/// Makes case sensitive comparison to the string object ( see cmp(const string& str) )
	bool operator!=(const string& str) const { return cmp(str) != 0; };
#endif
	
	// cast operators
	operator const char_t*() const;			///< Cast operator to char_t*
#ifdef ALLOW_UNICODE
	operator const wchar_t*() const;		///< Cast operator to wchar_t*
#endif
/**@}*/

/** \name Standard operations */
/**@{*/
	// appends the given string to this
	void merge(const char_t* pszSrc);			///< Appends an ansi string to the string object
	void merge(const wchar_t* pszSrc);		///< Appends an unicode string to the string object
	void merge(const string& src);			///< Appends a string object to another string object
	
	string left(size_t tLen) const;		///< Returns string with the left part of a source string
	string right(size_t tLen) const;		///< Returns string with the right part of a source string
	string mid(size_t tStart, size_t tLen=(size_t)-1) const;	///< Returns string with the middle part of a source string
	
	void left_self(size_t tLen, bool bReallocBuffer=true);	///< Makes this string it's left part
	void right_self(size_t tLen, bool bReallocBuffer=true);	///< Makes this string it's right part
	void mid_self(size_t tStart, size_t tLen=(size_t)-1, bool bReallocBuffer=true);	///< Makes this string it's middle part
	
	// comparation
	int_t cmp(const char_t* psz) const;		///< Comparison of this string object with a given ansi string
	int_t cmp(const wchar_t* psz) const;	///< Comparison of this string object with a given unicode string
	int_t cmp(const string& str) const;	///< Comparison of this string object with another string object
	
	int_t cmpi(const char_t* psz) const;	///< Comparison (case insensitive) of this string object with a given ansi string
	int_t cmpi(const wchar_t* psz) const;	///< Comparison (case insensitive) of this string object with a given unicode string
	int_t cmpi(const string& str) const;	///< Comparison (case insensitive) of this string object with another string object

	int_t cmpn(const char_t* psz, size_t tLen, bool bLenCheck=true) const;		///< Compares some chars of the string object with a given ansi string
	int_t cmpn(const wchar_t* psz, size_t tLen, bool bLenCheck=true) const;	///< Compares some chars of the string object with a given unicode string
	int_t cmpn(const string& str, size_t tLen, bool bLenCheck=true) const;		///< Compares some chars of the string object with another string object
	
	int_t cmpin(const char_t* psz, size_t tLen, bool bLenCheck=true) const;		///< Compares (case insensitive) some chars of the string object with a given ansi string
	int_t cmpin(const wchar_t* psz, size_t tLen, bool bLenCheck=true) const;	///< Compares (case insensitive) some chars of the string object with a given unicode string
	int_t cmpin(const string& str, size_t tLen, bool bLenCheck=true) const;	///< Compares (case insensitive) some chars of the string object with another string object

	int_t at(size_t tPos) const;						///< Gets a character at a specified position
	char_t* get_buffera(size_t tMinSize);		///< Gives user access to the ansi internal buffer
	wchar_t* get_bufferu(size_t tMinSize);		///< Gives user access to the unicode internal buffer
	void release_buffer();						///< Releases the buffer get from get_bufferx functions
	
	size_t length() const;	///< Returns the length of this string in chars
	size_t bytelen() const;	///< Returns the length of a string in bytes (with terminating null character)

	void clear();			///< Clear the contents of the string object

	bool is_empty() const;	///< Returns true if the string is empty
/**@}*/
	
/** \name Conversion routines */
/**@{*/
#ifdef ALLOW_UNICODE
	/// Checks if string is in unicode storage mode
	bool is_unicode() const { return m_psd->m_wFlags & SDF_UNICODE; };
	
	void make_unicode();		///< Makes the string internal buffer unicode
	void make_ansi();			///< Makes the string internal buffer ansi
#endif
/**@}*/

/** \name Debug stuff */
/**@{*/
	void dump(dumpctx* pctx);	///< Dumps the contents of this class to the dump context
	void print();				///< Prints the contents of this class
/**@}*/

protected:
	void set_str(const char_t* pszStr);		///< Makes a copy of a given ansi string and store it in internal string buffer
#ifdef ALLOW_UNICODE
	void set_str(const wchar_t* pszStr);	///< Makes a copy of a given unicode string and store it in internal string buffer
#endif
	
	// converts the internal buffer to the other format
#ifdef ALLOW_UNICODE
	static size_t convert_to_ansi(const wchar_t* pszIn, char_t** pszOut);		///< Converts an unicode string into the ansi one
	static size_t convert_to_unicode(const char_t* pszIn, wchar_t** pszOut);	///< Converts an ansi string into the unicode one
#endif

	void make_writable(int_t iType, ptr_t pParam=NULL);		///< Makes an underlying data object writable
	
	// checks the refcount and frees the data (refcount <=0) or leaves untouched (refcount >0)
	void release_data();		///< Releases an underlying data object from this string (if refcount <= 0 then data object is deleted)
	
public:
	str_data *m_psd;		///< Pointer to an underlying data object (str_data)
};

END_ICPF_NAMESPACE

#endif
