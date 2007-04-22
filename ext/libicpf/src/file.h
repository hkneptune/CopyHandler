/***************************************************************************
 *   Copyright (C) 2004-2006 by Józef Starosczyk                           *
 *   ixen@copyhandler.com                                                  *
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
/** \file file.h 
 *  \brief Contains system independent file/serializer class
 *  \todo Apply properly handling of the encryption/decryption stuff.
 *  \todo Modify the class so it could handle full 64bit files.
 *  \todo Correct the file creation flags under linux (wrong umask value?).
 */

#ifndef __FILE_H__
#define __FILE_H__

#include "exception.h"
#include "libicpf.h"
//#include "str.h"
#ifdef _WIN32
	#include "windows.h"
#endif

/// A synonym for the file class
#define serializer file;

// file access modes
/// Read access to the file
#define FA_READ			0x0001
/// Write access to the file
#define FA_WRITE		0x0002
/// Create file if does not exist
#define FA_CREATE		0x0004
/// Truncate file if not empty
#define FA_TRUNCATE		0x0008

// additional mode mods
/// Enable buffered access
#define FA_BUFFERED		0x8000

// begin data block flags
/// Standard flag - cannot be combined with others
#define BF_NONE			0x00

// seek constants
#ifdef _WIN32
	/// Seeks from the current file pointer position
	#define FS_CURRENT	FILE_CURRENT
	/// Seeks from the beginning of the file
	#define FS_SET		FILE_BEGIN
	/// Seeks from the end of file
	#define FS_END		FILE_END
#else
	/// Seeks from the current file pointer position
	#define FS_CURRENT	SEEK_CUR
	/// Seeks from the beginning of the file
	#define FS_SET		SEEK_SET
	/// Seeks from the end of file
	#define FS_END		SEEK_END
#endif

BEGIN_ICPF_NAMESPACE

/** \brief Structure describes the data inside a data block
 *
 *  Structure contain crc fields to make sure data block is consistent.
 *  Also the real data size and stored data size are included */
struct SERIALIZEINFOHEADER
{
	// main header
	int_t iDataSize;		///< Size of the meaningful data (including this header)
	int_t iRealSize;		///< Size of the data stored in file (may differ from ulDataSize ie. when encrypting)
	uint_t uiCRC32;			///< Crc32 of the data (only the data part)

	// helper
	uint_t uiHeaderCRC32;	///< Header's crc32 (without this field)
};

/** \brief Platform independent file and serialization class
 *
 *  Class allows to access file objects using system dependent functions. Allow
 *  to use internal buffering for faster reading small amounts of data.
 *  Also allows to serialize data in blocks (with crc checksums).
 */
class LIBICPF_API file
{
public:
	// construction/destruction
/** \name Construction/destruction
@{*/
	file();			///< Constructs a file object
	~file();		///< Destructs a file object
/**@}*/
	
/** \name Standard operations
 *  Standard file operations that works both in buffered and unbuffered mode.
 */
/**@{*/
	// open/close the file
	void open(const tchar_t* pszPath, uint_t uiFlags, uint_t uiBufSize=4096);	///< Opens a file with a given path
	void close();			///< Closes the currently opened file

	// reads or writes the data from/to a file (uses buffering for these operations if enabled)
	ulong_t read(ptr_t pBuffer, ulong_t ulSize);	///< Reads some data from a file
	ulong_t write(ptr_t pBuffer, ulong_t ulSize);	///< Writes some data to a file

	// handling the lines of text in a file (autodetecting the windows/unix style of line ending)
	bool read_line(tchar_t* pszStr, uint_t uiMaxLen);	///< Reads a line of text from a file
	void write_line(tchar_t* pszString);						///< Writes a line of text to a file

	// position related functions
	void seek(longlong_t llOffset, uint_t uiFrom);	///< Moves a file pointer in a file
	longlong_t getpos();									///< Gets the current position of a file pointer

	// size related functions
	void seteof();						///< Sets the end of file in the current file pointer place
	longlong_t get_size();				///< Retrieves the size of a file

	void flush();		///< Flushes the internal buffers
/**@}*/

	
/** \name Buffering state functions
 *  Operations that allow manipulating the file buffering state.
 */
/**@{*/
	// buffered/unbuffered state management
	/// Enables or disables the buffering
	void set_buffering(bool bEnable=true, uint_t dwSize=4096);
	/// Returns the buffering state
	bool is_buffered() const { return m_bBuffered; };			
	/// Returns the current buffer size (for buffered operations)
	uint_t get_buffersize() const { return m_uiBufferSize; };

	void switch_unbuffered();		///< Stores current buffered/unbuffered state and switches to unbuffered
	void switch_buffered();			///< Stores current buffered/unbuffered state and switches to buffered
	void restore_state();			///< Restores (un)buffered last state
/**@}*/
	
/** \name Serialization functions
 *  Operations that allow manipulating the file buffering state.
 */
/**@{*/
	// serialization (block operation)
	void datablock_begin(uint_t dwFlags=BF_NONE);	///< Begins the serialization data block
	void datablock_end();									///< Ends the serialization data block

	// serialization stuff
	void swrite(ptr_t pData, uint_t dwSize);		///< Appends some data to the serialialization buffer
	void sread(ptr_t pData, uint_t dwSize);		///< Reads some data from serialization buffer

	// state checking
	/// Checks if the class is performing write-type serialization
	bool is_storing() const { return (m_uiFlags & FA_WRITE) != 0; };
	/// Checks if the class is performing read-type serialization
	bool is_loading() const { return (m_uiFlags & FA_READ) != 0; };

	// storing&reading data
	file& operator<<(bool val);			///< Stores a given 'val' parameter in the file
	file& operator>>(bool& val);		///< Reads a value of a given type from the file
	file& operator<<(tchar_t val);			///< Stores a given 'val' parameter in the file
	file& operator>>(tchar_t& val);		///< Reads a value of a given type from the file
	file& operator<<(uchar_t val);			///< Stores a given 'val' parameter in the file
	file& operator>>(uchar_t& val);		///< Reads a value of a given type from the file
	file& operator<<(short_t val);			///< Stores a given 'val' parameter in the file
	file& operator>>(short_t& val);		///< Reads a value of a given type from the file
	file& operator<<(ushort_t val);			///< Stores a given 'val' parameter in the file
	file& operator>>(ushort_t& val);		///< Reads a value of a given type from the file
	file& operator<<(int_t val);			///< Stores a given 'val' parameter in the file
	file& operator>>(int_t& val);		///< Reads a value of a given type from the file
	file& operator<<(uint_t val);			///< Stores a given 'val' parameter in the file
	file& operator>>(uint_t& val);		///< Reads a value of a given type from the file
	file& operator<<(ll_t val);			///< Stores a given 'val' parameter in the file
	file& operator>>(ll_t& val);		///< Reads a value of a given type from the file
	file& operator<<(ull_t val);			///< Stores a given 'val' parameter in the file
	file& operator>>(ull_t& val);		///< Reads a value of a given type from the file

/*	/// Stores some integral type as a part of serialization data block
	template<class T> file& operator<<(T tData) { swrite(&tData, sizeof(T)); return *this; };
	/// Reads some integral type from a serialization data block
	template<class T> file& operator>>(T& tData) { sread(&tData, sizeof(T)); return *this; };*/

	// specialized serialization stuff
//	file& operator<<(icpf::string& str);		///< Stores a CString object in this file (only usable when used in an MFC program)
//	file& operator>>(icpf::string& str);		///< Reads a CString object from this file (only usable when used in an mfc program)
/**@}*/

protected:
	// serialization related internal functions
	void _sbuf_append(ptr_t pData, uint_t dwCount);	///< Adds some data to the end of serialization buffer
	void _sbuf_resize(uint_t dwNewLen);				///< Resizes the serialization buffer to make some more additional space
	void _sbuf_read(ptr_t pData, uint_t dwLen);		///< Gets some data from the serialization buffer
	void _clear_serialization();							///< Cancels the serialization

	// file-buffering related operations
	uint_t _read_packet();			///< Reads next packet of data into the internal buffer
	uint_t _write_packet();			///< Writes next packet of data into a file

	bool _read_string(tchar_t* pszStr, uint_t dwMaxLen);	///< Reads a string from an internal buffer
	longlong_t _seek(longlong_t llOffset, uint_t uiFrom);	///< A standard seek command done wo any flushing

protected:
#ifdef _WIN32
	HANDLE m_hFile;				///< Handle to a real file
#else
	intptr_t m_hFile;				///< Handle to a real file
#endif
	tchar_t* m_pszPath;			///< Path to the opened file as passed to file::open()
	uint_t m_uiFlags;	///< File flags as passed to file::open()

	bool m_bLastOperation;		///< States the last operation performed - false=>READ, true=>WRITE

	// read/write buffering
	bool m_bBuffered;				///< States if the file is currently in buffered state
	uint_t m_uiBufferSize;	///< Internal buffer size for buffering
	byte_t* m_pbyBuffer;		///< Ptr to the internal buffer
	uint_t m_uiCurrentPos;	///< Current position in the internal buffer
	uint_t m_uiDataCount;	///< Count of data in the internal buffer (counting from beginning)

	// state
	bool m_bRememberedState;		///< Specifies if the buffering state was saved
	
	// serialization stuff
	bool m_bSerializing;				///< States if the serialization is in progress
	byte_t* m_pbySerialBuffer;	///< Serialization buffer
	uint_t m_uiSerialBufferSize;	///< Current size of the serialization buffer
	uint_t m_uiSerialBufferPos;	///< Current position in the serialization buffer
	uint_t m_uiDataBlockFlags;	///< Flags of the current serialization block
};

END_ICPF_NAMESPACE

#endif
