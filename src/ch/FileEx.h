/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2003 Ixen Gerthannes (ixen@interia.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
/*************************************************************************
	File: File.h
	Version: 1.0
	Author: Ixen Gerthannes (ixen@interia.pl)
	File description:
		Contain class that handle buffered and unbuffered access to
		file. Primarily designed to work without mfc. Possible use
		with ms foundation classes.
	Classes:
		CFileException (based on CException)
			- exception class designed for usage with class CFile
			(not MFC based).
			- when used with MFC class name is CFileExException
		CFile
			- handles file-based operations (buffered and
				unbuffered):
			- reading/writing of data buffers
			- reading/writing of text lines (for text files)
			- reading/writing values of internal types (int,
				...)
		- when used with MFC class name is CFileEx
*************************************************************************/
#ifndef __FILE_H__
#define __FILE_H__

#pragma warning (disable : 4786)

#include "ExceptionEx.h"

#ifndef __FUNCTION__
	#define __FUNCTION__ "<unknown function>"
#endif

#define THROW_FILEEXCEPTIONEX(str_reason, filename, app_code, last_error) throw new CFileExceptionEx(__FILE__, __LINE__, __FUNCTION__, str_reason, filename, app_code, last_error)

// File exception errors
#define FERR_UNKNOWN	0
#define FERR_OPEN		1
#define FERR_CLOSE		2
#define FERR_READ		3
#define FERR_WRITE		4
#define FERR_SEEK		5
#define FERR_EOF		6	/* eof encountered - currently unused*/
#define FERR_SETEOF		7	/* error while setting an eof in file */
#define FERR_GETSIZE	8	/* error getting file size */
#define FERR_SERIALIZE	9	/* serialization error */
#define FERR_MEMORY		10	/* trying to read the data beyond the index */
#define FERR_CRYPT		11	/* encryption/decryption error */

class CFileExceptionEx : public CExceptionEx
{
public:
	CFileExceptionEx(PCTSTR pszSrcFile, DWORD dwLine, PCTSTR pszFunc, PCTSTR pszReason, PCTSTR pszFilename, DWORD dwReason=FERR_UNKNOWN, DWORD dwLastError=0) : CExceptionEx(pszSrcFile, dwLine, pszFunc, pszReason, dwReason, dwLastError) { SetFilename(pszFilename); };
	CFileExceptionEx(PCTSTR pszSrcFile, DWORD dwLine, PCTSTR pszFunc, TCHAR* pszReason, PCTSTR pszFilename, DWORD dwReason=FERR_UNKNOWN, DWORD dwLastError=0) : CExceptionEx(pszSrcFile, dwLine, pszFunc, pszReason, dwReason, dwLastError) { SetFilename(pszFilename); };
	virtual ~CFileExceptionEx() { delete [] m_pszFilename; };

	virtual int RegisterInfo(__EXCPROPINFO* pInfo)
	{
		// if the pInfo is null - return count of a needed props
		if (pInfo == NULL)
			return 1+CExceptionEx::RegisterInfo(NULL);

		// call base class RegisterInfo
		size_t tIndex=CExceptionEx::RegisterInfo(pInfo);

		// function has to register the info to be displayed (called from within GetInfo)
		RegisterProp(pInfo+tIndex+0, _T("Filename"), PropType::dtPtrToString, &m_pszFilename);

		return 1;
	};

protected:
	void SetFilename(PCTSTR pszFilename) { if (pszFilename) { m_pszFilename=new TCHAR[_tcslen(pszFilename)+1]; _tcscpy(m_pszFilename, pszFilename); } else m_pszFilename=NULL; };

public:
	TCHAR* m_pszFilename;
};

// header information for serialziation block
struct SERIALIZEINFOHEADER
{
	// main header
	DWORD dwDataSize;		// size of the meaningful data (with the header size)
	DWORD dwToRead;			// count of data to read at once (may be greater than dwDataSize)
	DWORD dwCRC32;			// crc32 of the data (only the data part)

	// helper
	DWORD dwHeaderCRC32;	// the above's header crc
};

// file access modes
#define FA_OPERATION	0x7fff
#define FA_READ			0
#define FA_WRITE		1
#define FA_RW			2
#define FA_APPEND		3	/* second rw access but with seek to end */

// additional mode mods
#define FA_BUFFERED		0x8000	/* if this is buffered */

// begin data block flags
#define BF_NONE			0x00
#define BF_ENCRYPTED	0x01

// currently CSerializer will be the same as CFileEx (later it may change)
#define CSerializer CFileEx

// real file class
class CFileEx
{
public:
	// construction/destruction
	CFileEx() { m_hFile=NULL; m_pszFilename=NULL; m_dwMode=0;
				m_dwBufferSize=0; m_pbyBuffer=NULL; m_dwCurrentPos=0;
				m_dwDataCount=0; m_bBuffered=false; m_bLastOperation=false; m_bRememberedState=false; 
				m_pbySerialBuffer=NULL; m_dwSerialBufferSize=NULL; m_dwSerialBufferPos=0; m_bSerializing=0;
#ifndef DISABLE_CRYPT
				m_pszPassword=NULL;
#endif
	};
	~CFileEx() { Close(); 
#ifndef DISABLE_CRYPT
				delete [] m_pszPassword;
#endif
				};
    
	// opening/closing file
	void Open(PCTSTR pszFilename, DWORD dwAccess, DWORD dwBufferSize=4096);
	void Close();

	// flushing (flushes all the internal's buffer data to the external file)
	void Flush();

	// reads or writes the data from/to a file (uses buffering for this if enabled)
	DWORD ReadBuffer(void* pBuffer, DWORD dwSize);
	DWORD WriteBuffer(void* pBuffer, DWORD dwSize);

	// handling the lines of text in a file (autodetecting the windows/unix style of line ending)
	bool ReadLine(TCHAR* pszStr, DWORD dwMaxLen);
	void WriteLine(TCHAR* pszString);

	// position related functions
	void Seek(LONGLONG llOffset, UINT uiFrom);	// seeks in a file - flushes buffers before
	LONGLONG GetPos();							// returns the current position of a file pointer (corrected by a pos in the internal buffer)

	// size related functions
	void SetEndOfFile();						// sets the end of file in the current file pointer place
	LONGLONG GetSize();			// retrieves the size of this file

	// buffered/unbuffered state managing
	void SetBuffering(bool bEnable=true, DWORD dwSize=4096);	// on-the-fly changing operation types
	bool IsBuffered() { return m_bBuffered; };
	DWORD GetBufferSize() { return m_dwBufferSize; };

	void SwitchToUnbuffered();		// remembers current state and changes to unbuffered
	void SwitchToBuffered();		// remembers current state and changes to buffered
	void RestoreState();			// restores (un)buffered last state

	// serialization (block operation)
	void BeginDataBlock(DWORD dwFlags=BF_NONE);
	void EndDataBlock();

#ifndef DISABLE_CRYPT
	void SetPassword(PCTSTR pszPass);
#endif
	// serialization stuff
	void SWrite(void* pData, DWORD dwSize);
	void SRead(void* pData, DWORD dwSize);

	bool IsStoring() { return (m_dwMode & FA_OPERATION) == FA_WRITE; };
	bool IsLoading() { return (m_dwMode & FA_OPERATION) == FA_READ; };

	// storing data
	template<class T> CFileEx& operator<<(T tData) { SWrite(&tData, sizeof(T)); return *this; };

#ifdef _MFC_VER
	CFileEx& operator<<(CString strData);
#endif

	// reading data
	template<class T> CFileEx& operator>>(T& tData) { SRead(&tData, sizeof(T)); return *this; };

#ifdef _MFC_VER
	CFileEx& operator>>(CString& strData);
#endif

protected:
	// serialization related internal functions
	void InternalAppendSerialBuffer(void* pData, DWORD dwCount);
	void ResizeSerialBuffer(DWORD dwNewLen);
	void InternalReadSerialBuffer(void* pData, DWORD dwLen);
	void ClearSerialization();

	// file-buffering related operations
	DWORD ReadPacket();				// reads next packet of data - for buffered operations
	DWORD WritePacket();			// writes next packet of data - buffered only

	bool InternalReadString(TCHAR* pszStr, DWORD dwMaxLen);
	LONGLONG InternalSeek(LONGLONG llOffset, UINT uiFrom);	// real seek-w/o any flushing

protected:
	HANDLE m_hFile;			// handle to file
	TCHAR* m_pszFilename;	// path to this file
	DWORD m_dwMode;			// mode in which this file has been opened

	bool m_bLastOperation;	// false=>READ, true=>WRITE

	// read/write buffering
	bool m_bBuffered;		// is this file additionally buffered ?
	DWORD m_dwBufferSize;	// specifies internal buffer size
	BYTE* m_pbyBuffer;		// internal buffer address
	DWORD m_dwCurrentPos;	// current position in above buffer
	DWORD m_dwDataCount;	// count of data in buffer (counting from beginning)

	// serialization stuff
	bool m_bSerializing;		// are we currently serializing (between BeginDataBlock and EndDataBlock) ?
	BYTE* m_pbySerialBuffer;	// buffer used for serialization
	DWORD m_dwSerialBufferSize;	// current size of the serialization buffer
	DWORD m_dwSerialBufferPos;	// current position in the serialization buffer
	DWORD m_dwDataBlockFlags;	// flags used in begin data block
	
#ifndef DISABLE_CRYPT
	// encryption related
	TCHAR *m_pszPassword;
#endif
	// state
    bool m_bRememberedState;	// was state remembered (it means also that last state was different) ?
};

#endif
