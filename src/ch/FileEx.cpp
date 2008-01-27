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

#include "stdafx.h"
#include "FileEx.h"
#include "../libicpf/crc32.h"
#ifndef DISABLE_CRYPT
#include "crypt.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning( disable : 4127 )

// serialization buffer add on every reallocation
#define SERIALBUFFER_DELTA	4096UL

///////////////////////////////////////////////////////////////
// Opens file
// pszFilename [in] - name of the file to open
// dwAccess [in] - type of access to this file (R, W, RW, APPEND)
// dwBufferSize [in] - size of internal buffer when using buffered
//						operations.
///////////////////////////////////////////////////////////////
void CFileEx::Open(PCTSTR pszFilename, DWORD dwAccess, DWORD dwBufferSize)
{
	// check if this object is ready to open a file
	if (m_hFile)
		Close();

	HANDLE hFile=NULL;
	switch (dwAccess & FA_OPERATION)
	{
	case FA_READ:
		hFile=CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		break;
	case FA_WRITE:
		hFile=CreateFile(pszFilename, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		break;
	case FA_RW:
	case FA_APPEND:
		hFile=CreateFile(pszFilename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		break;
	default:
		_ASSERT(false);		// unknown File access value
	}

	// check if operation succeeded
	if (hFile == INVALID_HANDLE_VALUE)
		THROW_FILEEXCEPTIONEX(_T("Cannot open the specified file (CreateFile failed)."), pszFilename, FERR_OPEN, GetLastError());
	else
	{
		// store hFile
		m_hFile=hFile;

		// remember path of this file
		m_pszFilename=new TCHAR[_tcslen(pszFilename)+1];
		_tcscpy(m_pszFilename, pszFilename);

		// remember the mode
		m_dwMode=dwAccess;

		// is this buffered ?
        SetBuffering((dwAccess & 0x8000) != 0, dwBufferSize);
	}

	// if this is the "FA_APPEND" access mode - seek to end
	if ((dwAccess & FA_OPERATION) == FA_APPEND)
		InternalSeek(0, FILE_END);
}

///////////////////////////////////////////////////////////////
// Closes the file (could be used on non-opened files
///////////////////////////////////////////////////////////////
void CFileEx::Close()
{
	// only if the file has been opened
	if (m_hFile)
	{
		// flush all data
		Flush();

		// close da file
		if (!CloseHandle(m_hFile))
		{
			// error closing
			THROW_FILEEXCEPTIONEX(_T("Cannot close the handle associated with a file (CloseHandle failed)."), m_pszFilename, FERR_CLOSE, GetLastError());
		}
		else
			m_hFile=NULL;

		// free strings and other data
		delete [] m_pszFilename;
		m_pszFilename=NULL;
		delete [] m_pbyBuffer;
		m_pbyBuffer=NULL;
	}
}

///////////////////////////////////////////////////////////////
// Flushes internal buffers when using buffered operations
// may be used in non-buffered mode and for not opened files
// sets file pointer to the place resulting from position
// in internal buffer
///////////////////////////////////////////////////////////////
void CFileEx::Flush()
{
	if (m_hFile && m_bBuffered && m_dwDataCount > 0)
	{
		if (m_bLastOperation)
		{
			// last operation - storing data
			WritePacket();
		}
		else
		{
			// last - reading data
			// set file pointer to position current-(m_dwDataCount-m_dwCurrentPos)
			InternalSeek(-(long)(m_dwDataCount-m_dwCurrentPos), FILE_CURRENT);
			m_dwCurrentPos=0;
			m_dwDataCount=0;
		}
	}
}

///////////////////////////////////////////////////////////////
// Reads some data from file (buffered or not)
// pBuffer [in/out] - buffer for data
// dwSize [in] - size of data to read
// Ret Value [out] - count of bytes read (could differ from
//						dwSize)
///////////////////////////////////////////////////////////////
DWORD CFileEx::ReadBuffer(void* pBuffer, DWORD dwSize)
{
	_ASSERT(m_hFile);		// forgot to open the file ?

	// flush if needed
	if (m_bLastOperation)
	{
		Flush();
		m_bLastOperation=false;
	}

	if (!m_bBuffered)
	{
		// unbuffered operation (read what is needed)
		DWORD dwRD=0;
		if (!ReadFile(m_hFile, pBuffer, dwSize, &dwRD, NULL))
			THROW_FILEEXCEPTIONEX(_T("Cannot read data from file (ReadFile failed)."), m_pszFilename, FERR_READ, GetLastError());

		return dwRD;		// if 0 - eof (not treated as exception)
	}
	else
	{
		// reads must be done by packets
		DWORD dwCurrPos=0;			// position in external buffer
        while (dwCurrPos<dwSize)
		{
			// is there any data left ?
			if (m_dwDataCount == 0 || m_dwCurrentPos == m_dwDataCount)
			{
				if (ReadPacket() == 0)
					return dwCurrPos;				// return what was read 'til now
			}

			// copy data into external buffer
			DWORD dwCount=__min(m_dwDataCount-m_dwCurrentPos, dwSize-dwCurrPos);
			memcpy(((BYTE*)pBuffer)+dwCurrPos, m_pbyBuffer+m_dwCurrentPos, dwCount);

			// update positions
            dwCurrPos+=dwCount;
			m_dwCurrentPos+=dwCount;
		}

		return dwCurrPos;
	}
}

///////////////////////////////////////////////////////////////
// Writes data to file (buffered or not)
// pBuffer [in/out] - buffer for data
// dwSize [in] - count of data to read
// Ret Value [out] - count of bytes written
///////////////////////////////////////////////////////////////
DWORD CFileEx::WriteBuffer(void* pBuffer, DWORD dwSize)
{
	_ASSERT(m_hFile);

	if (!m_bLastOperation)
	{
		Flush();
		m_bLastOperation=true;
	}

	if (!m_bBuffered)
	{
		// standard write
		DWORD dwWR=0;
		if (!WriteFile(m_hFile, pBuffer, dwSize, &dwWR, NULL))
			THROW_FILEEXCEPTIONEX(_T("Cannot write data to a file (WriteFile failed)."), m_pszFilename, FERR_WRITE, GetLastError());
		return dwWR;
	}
	else
	{
		DWORD dwPos=0;

		while (dwPos<dwSize)
		{
			// check if buffer need storing
			if (m_dwCurrentPos == m_dwBufferSize)
				WritePacket();

			// now add to internal buffer some data
			DWORD dwCount=__min(m_dwBufferSize-m_dwCurrentPos, dwSize-dwPos);

			memcpy(m_pbyBuffer+m_dwCurrentPos, ((BYTE*)pBuffer)+dwPos, dwCount);

			// update
			m_dwCurrentPos+=dwCount;
			m_dwDataCount+=dwCount;
			dwPos+=dwCount;
		}

		return dwPos;
	}
}

///////////////////////////////////////////////////////////////
// Reads one line of text from text file (buffered). When mode
// is unbuffered - for this operation it's switched for
// buffered one, and then switched again after finished.
// pszStr [in/out] - buffer for data
// dwMaxLen [in] - max size of the buffer
// Ret Value [out] - if the line of text was successfully read
///////////////////////////////////////////////////////////////
bool CFileEx::ReadLine(TCHAR* pszStr, DWORD dwMaxLen)
{
	// for unbuffered operations enable buffering for this op
	if (m_bBuffered)
		return InternalReadString(pszStr, dwMaxLen);
	else
	{
		DWORD dwSize=m_dwBufferSize;
		SetBuffering(true, 4096);

		bool bRet=InternalReadString(pszStr, dwMaxLen);

		SetBuffering(false, dwSize);
		return bRet;
	}
}

///////////////////////////////////////////////////////////////
// Writes string (line of text) into text file
// pszString [in] - line of text to be written
///////////////////////////////////////////////////////////////
void CFileEx::WriteLine(TCHAR* pszString)
{
	_ASSERT(m_hFile);

	if (!m_bLastOperation)
	{
		Flush();
		m_bLastOperation=true;
	}

	// make new string with \r\n at the end - cannot use old buffer - unknown size
	DWORD dwLen=(DWORD)_tcslen(pszString);

	TCHAR *pszData=new TCHAR[dwLen+3];
	_tcscpy(pszData, pszString);
	pszData[dwLen]=_T('\r');
	pszData[dwLen+1]=_T('\n');
	pszData[dwLen+2]=_T('\0');

	try
	{
		if (m_bBuffered)
		{
			DWORD dwStrPos=0;	// current index in pszString
			DWORD dwMin=0;		// helper
			DWORD dwSize=dwLen+2;

			// processing whole string
			while (dwStrPos < dwSize)
			{
				if (m_dwCurrentPos == m_dwBufferSize)
					WritePacket();

				// count of chars to be copied
				dwMin=__min(dwSize-dwStrPos, m_dwBufferSize-m_dwCurrentPos);

				// copy data from pszString into internal buffer (maybe part of it)
				memcpy(m_pbyBuffer+m_dwCurrentPos, pszData+dwStrPos, dwMin);

				// move offsets
				m_dwCurrentPos+=dwMin;
				m_dwDataCount+=dwMin;
				dwStrPos+=dwMin;
			}
		}
		else
		{
			// standard write
			DWORD dwWR=0;
			if (!WriteFile(m_hFile, pszData, dwLen+2, &dwWR, NULL))
				THROW_FILEEXCEPTIONEX(_T("Cannot write data to a file (WriteFile failed)."), m_pszFilename, FERR_WRITE, GetLastError());
		}
	}
	catch(...)
	{
		delete [] pszData;
		throw;
	}

	delete [] pszData;
}

///////////////////////////////////////////////////////////////
// Sets file pointer to some position (flushes buffers if
// needed)
// llOffset [in] - offset to move (look@ SetFilePointer in
//					Platform SDK)
// uiFrom [in] - type of move operation (look @ SetFilePointer
//					in Platform SDK).
///////////////////////////////////////////////////////////////
void CFileEx::Seek(LONGLONG llOffset, UINT uiFrom)
{
	// flush buffer
	Flush();

	// seek
	InternalSeek(llOffset, uiFrom);
}

///////////////////////////////////////////////////////////////
// Returns current file position (physical, corrented with 
// size and position in internal buffer if buffered mode).
// Doesn't flush internal buffer.
// Ret Value [out] - position in a buffer
///////////////////////////////////////////////////////////////
LONGLONG CFileEx::GetPos()
{
	// return corrected by internal members current file position
	return InternalSeek(0, FILE_CURRENT)-m_dwDataCount+m_dwCurrentPos;
}

///////////////////////////////////////////////////////////////
// Sets end of file at current file position (flushes internal
// buffer before).
///////////////////////////////////////////////////////////////
void CFileEx::SetEndOfFile()
{
	_ASSERT(m_hFile);

	// flush da buffers
	Flush();

	// now set the end of file
	if (!::SetEndOfFile(m_hFile))
		THROW_FILEEXCEPTIONEX(_T("Cannot truncate the file (SetEndOfFile failed)."), m_pszFilename, FERR_SETEOF, GetLastError());
}

///////////////////////////////////////////////////////////////
// Returns current size of the file (flushes buffer before
// that)
// Ret Value [out] - current file size
///////////////////////////////////////////////////////////////
LONGLONG CFileEx::GetSize()
{
	Flush();

	ULARGE_INTEGER li;
	li.LowPart=GetFileSize(m_hFile, &li.HighPart);
	if (li.LowPart == INVALID_FILE_SIZE && GetLastError() != NO_ERROR)
		THROW_FILEEXCEPTIONEX(_T("Cannot get the size of the file (GetFileSize failed)."), m_pszFilename, FERR_GETSIZE, GetLastError());

	return li.QuadPart;
}

///////////////////////////////////////////////////////////////
// Sets buffered operation status and/or internal buffer size
// (could be used on the fly - each use cause buffers to flush)
// bEnable [in] - if the buffered operations should be enabled
// dwSize [in] - size of the internal buffer
///////////////////////////////////////////////////////////////
void CFileEx::SetBuffering(bool bEnable, DWORD dwSize)
{
	_ASSERT(dwSize > 0);	// couldn't use 0-sized internal buffer

	// flush
	Flush();

	// delete old buffer
	if (m_bBuffered && (!bEnable || dwSize != m_dwBufferSize))
	{
		delete [] m_pbyBuffer;
		m_pbyBuffer=NULL;
	}

	// alloc new buffer if needed
	if (bEnable && (!m_bBuffered || dwSize != m_dwBufferSize))
		m_pbyBuffer=new BYTE[dwSize];

	m_bBuffered=bEnable;
	m_dwBufferSize=dwSize;
}

///////////////////////////////////////////////////////////////
// Changes current mode to unbuffered. If already unbuffered
// function returns doing nothing
///////////////////////////////////////////////////////////////
void CFileEx::SwitchToUnbuffered()
{
	if (!m_bBuffered)
		return;			// it's already unbuffered - leave it as is
	else
	{
		m_bRememberedState=true;				// mark that we change state to a different one
		SetBuffering(false, m_dwBufferSize);	// do no change internal buffer size
	}
}

///////////////////////////////////////////////////////////////
// Changes current mode to buffered. If already buffered
// function returns doing nothing
///////////////////////////////////////////////////////////////
void CFileEx::SwitchToBuffered()
{
	if (m_bBuffered)
		return;			// already buffered
	else
	{
		m_bRememberedState=true;
		SetBuffering(true, m_dwBufferSize);
	}
}

///////////////////////////////////////////////////////////////
// Function restores (un)buffered state previously remembered
// with SwitchToXXX. If SwitchToXXX doesn't change the state
// - this function also doesn't.
///////////////////////////////////////////////////////////////
void CFileEx::RestoreState()
{
	// restore state only if changed
	if (m_bRememberedState)
	{
		SetBuffering(!m_bBuffered, m_dwBufferSize);
		m_bRememberedState=false;
	}
}

///////////////////////////////////////////////////////////////
// (Internal) Reads next packet of data from file (when using
// buffered operations it cause next data of size of internal
// buffer to be read from file
// Ret Value [out] - Count of data that was read
///////////////////////////////////////////////////////////////
DWORD CFileEx::ReadPacket()
{
	_ASSERT(m_hFile);

	// read data
	DWORD dwRD=0;
	if (!ReadFile(m_hFile, m_pbyBuffer, m_dwBufferSize, &dwRD, NULL))
		THROW_FILEEXCEPTIONEX(_T("Cannot read data from a file (ReadFile failed)."), m_pszFilename, FERR_READ, GetLastError());

	// reset internal members
	m_dwDataCount=dwRD;
	m_dwCurrentPos=0;

	return dwRD;
}

///////////////////////////////////////////////////////////////
// (Internal) Function writes all data from internal buffer to
// a file.
// Ret Value [out] - count of bytes written
///////////////////////////////////////////////////////////////
DWORD CFileEx::WritePacket()
{
	_ASSERT(m_hFile);

	DWORD dwWR=0;
	if (!WriteFile(m_hFile, m_pbyBuffer, m_dwDataCount, &dwWR, NULL))
		THROW_FILEEXCEPTIONEX(_T("Cannot write data to a file (WriteFile failed)."), m_pszFilename, FERR_WRITE, GetLastError());

	// reset internal members
	m_dwDataCount=0;
	m_dwCurrentPos=0;

	return dwWR;
}

#ifndef DISABLE_CRYPT
///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::SetPassword(PCTSTR pszPass)
{
	// delete the old password
	if (m_pszPassword)
		delete [] m_pszPassword;		// delete old password

	// generate the SHA256 from this password
	m_pszPassword=new TCHAR[64+1];
	StringToKey256(pszPass, m_pszPassword);
}

#endif

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::BeginDataBlock(DWORD dwFlags)
{
	// do not call begin data block within another data block
	_ASSERT(!m_bSerializing && m_pbySerialBuffer == NULL);

	// alloc the new buffer and insert there a header (a few bytes)
	m_pbySerialBuffer=new BYTE[SERIALBUFFER_DELTA];
	m_dwSerialBufferSize=SERIALBUFFER_DELTA;

	// determine action from read or write flag
	bool bWrite=false;
	switch(m_dwMode & FA_OPERATION)
	{
	case FA_READ:
		bWrite=false;
		break;
	case FA_WRITE:
		bWrite=true;
		break;
	case FA_RW:
	case FA_APPEND:
		_ASSERT(FALSE);		// cannot use serialization with files opened in rw mode
		break;
	}

	// action
	if (bWrite)
	{
		// reserve some space for a header
		m_dwSerialBufferPos=sizeof(SERIALIZEINFOHEADER);
	}
	else
	{
		// we need to read the block from a file
		if (ReadBuffer(m_pbySerialBuffer, sizeof(SERIALIZEINFOHEADER)) != sizeof(SERIALIZEINFOHEADER))
		{
			ClearSerialization();
			THROW_FILEEXCEPTIONEX(_T("Cannot read the specified amount of data from a file (reading serialization header)."), m_pszFilename, FERR_SERIALIZE, GetLastError());
		}

		// move forward
		m_dwSerialBufferPos=sizeof(SERIALIZEINFOHEADER);

		// determine the size of the remaining data in file
		SERIALIZEINFOHEADER* psih=(SERIALIZEINFOHEADER*)m_pbySerialBuffer;
		DWORD dwSize=psih->dwToRead-sizeof(SERIALIZEINFOHEADER);

		// check the header crc
		DWORD dwhc=icpf::crc32(m_pbySerialBuffer, sizeof(SERIALIZEINFOHEADER)-sizeof(DWORD));
		if (dwhc != psih->dwHeaderCRC32)
		{
			ClearSerialization();
			THROW_FILEEXCEPTIONEX(_T("Block corrupted. Header CRC check failed."), m_pszFilename, FERR_SERIALIZE, 0);
		}

		// resize the buffer
		ResizeSerialBuffer(psih->dwToRead);

		// refresh the psih
		psih=(SERIALIZEINFOHEADER*)m_pbySerialBuffer;

		// read the remaining data
		if (ReadBuffer(m_pbySerialBuffer+m_dwSerialBufferPos, dwSize) != dwSize)
		{
			ClearSerialization();
			THROW_FILEEXCEPTIONEX(_T("Cannot read specified amount of data from file (reading the after-header data)."), m_pszFilename, FERR_SERIALIZE, GetLastError());
		}

#ifndef DISABLE_CRYPT
		// decrypt the data
		if (dwFlags & BF_ENCRYPTED)
		{
			if (AES256Decrypt(m_pbySerialBuffer+m_dwSerialBufferPos, dwSize, m_pszPassword, m_pbySerialBuffer+m_dwSerialBufferPos) < 0)
			{
				ClearSerialization();
				THROW_FILEEXCEPTIONEX(_T("Cannot decrypt the data read from the serialization file."), m_pszFilename, FERR_CRYPT, 0);
			}
		}
#endif
		// NOTE: do not update the position - we need ptr at the beginning of data

		// now we are almost ready to retrieve data - only the crc check for the data
		DWORD dwCRC=icpf::crc32(m_pbySerialBuffer+sizeof(SERIALIZEINFOHEADER), psih->dwDataSize-sizeof(SERIALIZEINFOHEADER));
		if (psih->dwCRC32 != dwCRC)
		{
			ClearSerialization();
			THROW_FILEEXCEPTIONEX(_T("CRC check of the data read from file failed."), m_pszFilename, FERR_SERIALIZE, GetLastError());
		}
	}

	// make a mark
	m_dwDataBlockFlags=dwFlags;
	m_bSerializing=true;
}

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::EndDataBlock()
{
	// make sure everything is ok
	_ASSERT(m_bSerializing && m_pbySerialBuffer != NULL);

	// type of operation
	bool bWrite=false;
	switch(m_dwMode & FA_OPERATION)
	{
	case FA_READ:
		bWrite=false;
		break;
	case FA_WRITE:
		bWrite=true;
		break;
	case FA_RW:
	case FA_APPEND:
		_ASSERT(FALSE);		// cannot use serialization with files opened in rw mode
		break;
	}

	// when writing - make a header, ...; when reading - do nothing important
	if (bWrite)
	{
		// check if there is any data
		if (m_dwSerialBufferPos == sizeof(SERIALIZEINFOHEADER))
			return;												// no data has been serialized

		// fill the header (real data information)
		SERIALIZEINFOHEADER *psih=(SERIALIZEINFOHEADER*)m_pbySerialBuffer;
		psih->dwDataSize=m_dwSerialBufferPos;
		psih->dwCRC32=icpf::crc32(m_pbySerialBuffer+sizeof(SERIALIZEINFOHEADER), m_dwSerialBufferPos-sizeof(SERIALIZEINFOHEADER));

#ifndef DISABLE_CRYPT
		// we could encrypt the data here if needed
		if (m_dwDataBlockFlags & BF_ENCRYPTED)
		{
			int iRes=AES256Crypt(m_pbySerialBuffer+sizeof(SERIALIZEINFOHEADER), m_dwSerialBufferPos-sizeof(SERIALIZEINFOHEADER), m_pszPassword, m_pbySerialBuffer+sizeof(SERIALIZEINFOHEADER));
			if (iRes < 0)
			{
				ClearSerialization();
				THROW_FILEEXCEPTIONEX(_T("Cannot encrypt the data to store."), m_pszFilename, FERR_CRYPT, 0);
			}

			// fill the header
			psih->dwToRead=iRes+sizeof(SERIALIZEINFOHEADER);
		}
		else
		{
#endif
			// the rest of header
			psih->dwToRead=m_dwSerialBufferPos;
#ifndef DISABLE_CRYPT
		}
#endif

		// calc the header crc
		psih->dwHeaderCRC32=icpf::crc32(m_pbySerialBuffer, sizeof(SERIALIZEINFOHEADER)-sizeof(DWORD));

		// write the buffer
		WriteBuffer(m_pbySerialBuffer, psih->dwToRead);
	}

	// remove all the traces of serializing
	ClearSerialization();
}

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::SWrite(void* pData, DWORD dwSize)
{
	InternalAppendSerialBuffer(pData, dwSize);
}

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::SRead(void* pData, DWORD dwSize)
{
	InternalReadSerialBuffer(pData, dwSize);
}

#ifdef _MFC_VER

CFileEx& CFileEx::operator<<(CString strData)
{
	DWORD dwLen=strData.GetLength();
	SWrite(&dwLen, sizeof(DWORD));
	SWrite(strData.GetBuffer(1), dwLen);
	strData.ReleaseBuffer();
	return *this;
}
	
CFileEx& CFileEx::operator>>(CString& strData)
{
	DWORD dwLen;
	SRead(&dwLen, sizeof(DWORD));
	PTSTR pszData=strData.GetBuffer(dwLen+1);
	try
	{
		SRead(pszData, dwLen);
	}
	catch(...)
	{
		pszData[dwLen]=_T('\0');
		strData.ReleaseBuffer();
		throw;
	}

	pszData[dwLen]=_T('\0');
	strData.ReleaseBuffer();
	return *this;
}

#endif

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::ClearSerialization()
{
	// remove all the traces of serializing
    delete [] m_pbySerialBuffer;
	m_pbySerialBuffer=NULL;
	m_dwSerialBufferSize=0;
	m_dwSerialBufferPos=0;
	m_bSerializing=false;
}

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::InternalReadSerialBuffer(void* pData, DWORD dwLen)
{
	// check if we are reading
	_ASSERT((m_dwMode & FA_OPERATION) == FA_READ);

	// check the ranges
	if (m_dwSerialBufferPos+dwLen > m_dwSerialBufferSize)
	{
		// throw an exception - read beyond the data range in a given object
		THROW_FILEEXCEPTIONEX(_T("Trying to read the serialization data beyond the range."), m_pszFilename, FERR_MEMORY, GetLastError());
	}

	// read the data
	memcpy(pData, m_pbySerialBuffer+m_dwSerialBufferPos, dwLen);
	m_dwSerialBufferPos+=dwLen;
}

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::InternalAppendSerialBuffer(void* pData, DWORD dwCount)
{
	// check if we are writing
	_ASSERT((m_dwMode & FA_OPERATION) == FA_WRITE);

	// check serial buffer size (if there is enough room for the data)
	if (m_dwSerialBufferPos+dwCount > m_dwSerialBufferSize)
	{
		// we need a buffer reallocation
		DWORD dwDelta=((dwCount/SERIALBUFFER_DELTA)+1)*SERIALBUFFER_DELTA;

		// alloc the new buffer
		ResizeSerialBuffer(m_dwSerialBufferSize+dwDelta);
	}

	// real storage of the data
	if (dwCount > 0)
	{
		memcpy(m_pbySerialBuffer+m_dwSerialBufferPos, pData, dwCount);
		m_dwSerialBufferPos+=dwCount;
	}
}

///////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////
void CFileEx::ResizeSerialBuffer(DWORD dwNewLen)
{
	// alloc the new buffer
	BYTE* pbyNewBuffer=new BYTE[dwNewLen];

	// copy the old data into the new one
	DWORD dwCount=min(m_dwSerialBufferPos, dwNewLen);
	if (m_dwSerialBufferPos > 0)
		memcpy(pbyNewBuffer, m_pbySerialBuffer, dwCount);

	// delete the old buffer
	delete [] m_pbySerialBuffer;

	// set the new buffer inplace and update the internal size
	m_pbySerialBuffer=pbyNewBuffer;
	m_dwSerialBufferSize=dwNewLen;
}

///////////////////////////////////////////////////////////////
// (Internal) Functions reads line of text from text file
// Only for buffered operation
// pszStr [out] - string that was read from file
// dwMaxLen [in] - size of the string buffer
///////////////////////////////////////////////////////////////
bool CFileEx::InternalReadString(TCHAR* pszStr, DWORD dwMaxLen)
{
	_ASSERT(m_hFile != NULL);	// file wasn't opened - error opening or you've forgotten to do so ?

	// last time was writing - free buffer
	if (m_bLastOperation)
	{
		Flush();
		m_bLastOperation=false;
	}

	// zero all the string
	memset(pszStr, 0, dwMaxLen*sizeof(TCHAR));

	// additional vars
	DWORD dwStrPos=0;			// current pos in external buffer
	bool bSecondPass=false;		// if there is need to check data for 0x0a char

	// copy each char into pszString
	while(true)
	{
		// if buffer is empty - fill it
		if (m_dwDataCount == 0 || m_dwCurrentPos == m_dwDataCount)
		{
			if (ReadPacket() == 0)
				return _tcslen(pszStr) != 0;
		}

		// skipping 0x0a in second pass
		if (bSecondPass)
		{
			if (m_pbyBuffer[m_dwCurrentPos] == 0x0a)
				m_dwCurrentPos++;
			return true;
		}

		// now process chars
		while (m_dwCurrentPos < m_dwDataCount)
		{
			if (m_pbyBuffer[m_dwCurrentPos] == 0x0d)
			{
				bSecondPass=true;
				m_dwCurrentPos++;
				break;
			}
			else if (m_pbyBuffer[m_dwCurrentPos] == 0x0a)
			{
				m_dwCurrentPos++;
				return true;
			}
			else
			{
				if (dwStrPos < dwMaxLen-1)
					pszStr[dwStrPos++]=m_pbyBuffer[m_dwCurrentPos];
				m_dwCurrentPos++;
			}
		}
	}

	return false;
}

///////////////////////////////////////////////////////////////
// (Internal) Sets file pointer in a file
// llOffset [in] - position to move to
// uiFrom [in] - type of moving (see Platform SDK on
//				SetFilePointer)
///////////////////////////////////////////////////////////////
LONGLONG CFileEx::InternalSeek(LONGLONG llOffset, UINT uiFrom)
{
	LARGE_INTEGER li;
	li.QuadPart = llOffset;
	li.LowPart = SetFilePointer (m_hFile, li.LowPart, &li.HighPart, uiFrom);

	if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
	{
		// error seeking - throw an exception
		THROW_FILEEXCEPTIONEX(_T("Seek error."), m_pszFilename, FERR_SEEK, GetLastError());
	}

	return li.QuadPart;
}
