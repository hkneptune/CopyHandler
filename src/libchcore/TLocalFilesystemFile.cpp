// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "stdafx.h"
#include "TLocalFilesystemFile.h"
#include "TBufferSizes.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include <atltrace.h>
#include <boost/numeric/conversion/cast.hpp>
#include "RoundingFunctions.h"
#include "TLocalFilesystem.h"
#include "TCoreWin32Exception.h"
#include "TFileException.h"
#include "TFileInfo.h"

namespace chcore
{
	// compile-time check - ensure the buffer granularity used for transfers are bigger than expected sector size
	static_assert(TLocalFilesystemFile::MaxSectorSize <= TBufferSizes::BufferGranularity, "Buffer granularity must be equal to or bigger than the max sector size");

	TLocalFilesystemFile::TLocalFilesystemFile(const TSmartPath& pathFile, bool bNoBuffering) :
		m_pathFile(TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile)),
		m_hFile(INVALID_HANDLE_VALUE),
		m_bNoBuffering(bNoBuffering)
	{
		if (pathFile.IsEmpty())
			throw TCoreException(eErr_InvalidArgument, L"pathFile", LOCATION);
	}

	TLocalFilesystemFile::~TLocalFilesystemFile()
	{
		try
		{
			InternalClose();
		}
		catch (const std::exception&)
		{
		}
	}

	DWORD TLocalFilesystemFile::GetFlagsAndAttributes(bool bNoBuffering) const
	{
		return FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN | (bNoBuffering ? FILE_FLAG_NO_BUFFERING : 0);
	}

	void TLocalFilesystemFile::OpenExistingForReading()
	{
		Close();

		m_hFile = ::CreateFile(m_pathFile.ToString(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, GetFlagsAndAttributes(m_bNoBuffering), NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot open for reading", LOCATION);
		}
	}

	void TLocalFilesystemFile::CreateNewForWriting()
	{
		Close();

		m_hFile = ::CreateFile(m_pathFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, GetFlagsAndAttributes(m_bNoBuffering), NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot create file.", LOCATION);
		}
	}

	void TLocalFilesystemFile::OpenExistingForWriting()
	{
		OpenExistingForWriting(m_bNoBuffering);
	}

	void TLocalFilesystemFile::OpenExistingForWriting(bool bNoBuffering)
	{
		Close();

		m_hFile = CreateFile(m_pathFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, GetFlagsAndAttributes(bNoBuffering), NULL);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot open for reading.", LOCATION);
		}
	}

	file_size_t TLocalFilesystemFile::GetSeekPositionForResume(file_size_t fsLastAvailablePosition)
	{
		file_size_t fsMove = (m_bNoBuffering ? RoundDown<file_size_t>(fsLastAvailablePosition, MaxSectorSize) : fsLastAvailablePosition);
		return fsMove;
	}

	void TLocalFilesystemFile::Truncate(file_size_t fsNewSize)
	{
		if (!IsOpen())
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"File not open yet. Cannot truncate.", LOCATION);

		// when no-buffering is used, there are cases where we'd need to switch to buffered ops
		// to adjust file size
		bool bFileSettingsChanged = false;
		if (m_bNoBuffering)
		{
			file_size_t fsNewAlignedSize = RoundUp<file_size_t>(fsNewSize, MaxSectorSize);
			if (fsNewAlignedSize != fsNewSize)
			{
				Close();
				OpenExistingForWriting(false);

				bFileSettingsChanged = true;
			}
		}

		LARGE_INTEGER li = { 0, 0 };
		LARGE_INTEGER liNew = { 0, 0 };

		li.QuadPart = fsNewSize;

		if (!SetFilePointerEx(m_hFile, li, &liNew, FILE_BEGIN))
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_SeekFailed, dwLastError, m_pathFile, L"Cannot seek to appropriate position", LOCATION);
		}

		if(!::SetEndOfFile(m_hFile))
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotTruncate, dwLastError, m_pathFile, L"Cannot mark the end of file", LOCATION);
		}

		// close the file that was open in inappropriate mode
		if(bFileSettingsChanged)
			Close();
	}

	void TLocalFilesystemFile::ReadFile(TOverlappedDataBuffer& rBuffer)
	{
		if (!IsOpen())
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot read from closed file", LOCATION);

		//ATLTRACE(_T("Reading %lu bytes\n"), rBuffer.GetRequestedDataSize());
		if (!::ReadFileEx(m_hFile, rBuffer.GetBufferPtr(), rBuffer.GetRequestedDataSize(), &rBuffer, OverlappedReadCompleted))
		{
			DWORD dwLastError = GetLastError();
			switch (dwLastError)
			{
			case ERROR_IO_PENDING:
				return;

			case ERROR_HANDLE_EOF:
				{
					rBuffer.SetBytesTransferred(0);
					rBuffer.SetStatusCode(0);
					rBuffer.SetErrorCode(ERROR_SUCCESS);
					rBuffer.SetLastPart(true);

					rBuffer.RequeueAsFull();	// basically the same as OverlappedReadCompleted
					break;
				}

			default:
				throw TFileException(eErr_CannotReadFile, dwLastError, m_pathFile, L"Error reading data from file", LOCATION);
			}
		}
	}

	void TLocalFilesystemFile::WriteFile(TOverlappedDataBuffer& rBuffer)
	{
		if (!IsOpen())
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot write to closed file", LOCATION);

		DWORD dwToWrite = boost::numeric_cast<DWORD>(rBuffer.GetRealDataSize());

		if (m_bNoBuffering && rBuffer.IsLastPart())
			dwToWrite = RoundUp<DWORD>(dwToWrite, MaxSectorSize);

		//ATLTRACE(_T("Writing %lu bytes\n"), dwToWrite);
		if (!::WriteFileEx(m_hFile, rBuffer.GetBufferPtr(), dwToWrite, &rBuffer, OverlappedWriteCompleted))
		{
			DWORD dwLastError = GetLastError();
			if (dwLastError != ERROR_IO_PENDING)
				throw TFileException(eErr_CannotWriteFile, dwLastError, m_pathFile, L"Error while writing to file", LOCATION);
		}
	}

	void TLocalFilesystemFile::FinalizeFile(TOverlappedDataBuffer& rBuffer)
	{
		if (!IsOpen())
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot write to closed file", LOCATION);

		if (m_bNoBuffering && rBuffer.IsLastPart())
		{
			DWORD dwToWrite = boost::numeric_cast<DWORD>(rBuffer.GetRealDataSize());
			DWORD dwReallyWritten = RoundUp<DWORD>(dwToWrite, MaxSectorSize);

			//ATLTRACE(_T("Finalize file - size diff: written: %I64u, required: %I64u\n"), dwReallyWritten, dwToWrite);
			if (dwToWrite != dwReallyWritten)
			{
				file_size_t fsNewFileSize = rBuffer.GetFilePosition() + dwToWrite;	// new size

				//seek
				//ATLTRACE(_T("Truncating file to %I64u bytes\n"), fsNewFileSize);
				Truncate(fsNewFileSize);
			}
		}
	}

	void TLocalFilesystemFile::InternalClose()
	{
		if (m_hFile != INVALID_HANDLE_VALUE)
			::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}

	void TLocalFilesystemFile::Close()
	{
		InternalClose();
	}

	file_size_t TLocalFilesystemFile::GetFileSize() const
	{
		if (!IsOpen())
			return 0;

		BY_HANDLE_FILE_INFORMATION bhfi;

		if (!::GetFileInformationByHandle(m_hFile, &bhfi))
			return 0;

		ULARGE_INTEGER uli;
		uli.HighPart = bhfi.nFileSizeHigh;
		uli.LowPart = bhfi.nFileSizeLow;

		return uli.QuadPart;
	}

	void TLocalFilesystemFile::GetFileInfo(TFileInfo& tFileInfo) const
	{
		if (!IsOpen())
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"File not open. Cannot get file info.", LOCATION);

		BY_HANDLE_FILE_INFORMATION bhfi;

		if (!::GetFileInformationByHandle(m_hFile, &bhfi))
		{
			DWORD dwLastError = GetLastError();
			throw TFileException(eErr_CannotGetFileInfo, dwLastError, m_pathFile, L"Retrieving file info from handle failed.", LOCATION);
		}

		ULARGE_INTEGER uli;
		uli.HighPart = bhfi.nFileSizeHigh;
		uli.LowPart = bhfi.nFileSizeLow;

		tFileInfo.SetFilePath(m_pathFile);
		tFileInfo.SetAttributes(bhfi.dwFileAttributes);
		tFileInfo.SetFileTimes(TFileTime(bhfi.ftCreationTime), TFileTime(bhfi.ftLastAccessTime), TFileTime(bhfi.ftLastWriteTime));
		tFileInfo.SetLength64(uli.QuadPart);
	}

	TSmartPath TLocalFilesystemFile::GetFilePath() const
	{
		return m_pathFile;
	}
}
