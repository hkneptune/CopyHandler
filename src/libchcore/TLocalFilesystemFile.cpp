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
#include <boost/numeric/conversion/cast.hpp>
#include "RoundingFunctions.h"
#include "TLocalFilesystem.h"
#include "TFileException.h"
#include "TFileInfo.h"
#include "StreamingHelpers.h"
#include "TOverlappedDataBufferQueue.h"

namespace chcore
{
	// compile-time check - ensure the buffer granularity used for transfers are bigger than expected sector size
	static_assert(TLocalFilesystemFile::MaxSectorSize <= TBufferSizes::BufferGranularity, "Buffer granularity must be equal to or bigger than the max sector size");

	TLocalFilesystemFile::TLocalFilesystemFile(const TSmartPath& pathFile, bool bNoBuffering, const logger::TLogFileDataPtr& spLogFileData) :
		m_pathFile(TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile)),
		m_hFile(INVALID_HANDLE_VALUE),
		m_bNoBuffering(bNoBuffering),
		m_spLog(logger::MakeLogger(spLogFileData, L"Filesystem-File"))
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

		LOG_DEBUG(m_spLog) << L"Opening existing file for reading" << GetFileInfoForLog(m_bNoBuffering);

		m_hFile = ::CreateFile(m_pathFile.ToString(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, GetFlagsAndAttributes(m_bNoBuffering), nullptr);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << "Opening file for reading failed with error: " << dwLastError << GetFileInfoForLog(m_bNoBuffering);

			throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot open for reading", LOCATION);
		}

		LOG_DEBUG(m_spLog) << "Opening file for reading succeeded. New handle: " << m_hFile << GetFileInfoForLog(m_bNoBuffering);
	}

	void TLocalFilesystemFile::CreateNewForWriting()
	{
		Close();

		LOG_DEBUG(m_spLog) << "Creating new file for writing" << GetFileInfoForLog(m_bNoBuffering);

		m_hFile = ::CreateFile(m_pathFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, GetFlagsAndAttributes(m_bNoBuffering), nullptr);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << "CreateNewForWriting failed with error: " << dwLastError << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot create file.", LOCATION);
		}
		LOG_DEBUG(m_spLog) << "CreateNewForWriting succeeded. New handle: " << m_hFile << GetFileInfoForLog(m_bNoBuffering);
	}

	void TLocalFilesystemFile::OpenExistingForWriting()
	{
		OpenExistingForWriting(m_bNoBuffering);
	}

	void TLocalFilesystemFile::OpenExistingForWriting(bool bNoBuffering)
	{
		Close();

		LOG_DEBUG(m_spLog) << "OpenExistingForWriting" << GetFileInfoForLog(bNoBuffering);

		m_hFile = CreateFile(m_pathFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_EXISTING, GetFlagsAndAttributes(bNoBuffering), nullptr);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << "OpenExistingForWriting failed with error: " << dwLastError << GetFileInfoForLog(bNoBuffering);

			throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot open for reading.", LOCATION);
		}
		LOG_DEBUG(m_spLog) << "OpenExistingForWriting succeeded. New handle: " << m_hFile << GetFileInfoForLog(bNoBuffering);
	}

	file_size_t TLocalFilesystemFile::GetSeekPositionForResume(file_size_t fsLastAvailablePosition)
	{
		file_size_t fsMove = (m_bNoBuffering ? RoundDown<file_size_t>(fsLastAvailablePosition, MaxSectorSize) : fsLastAvailablePosition);
		LOG_DEBUG(m_spLog) << "Calculated seek position for last-available-pos: " << fsLastAvailablePosition << L" = " << fsMove << GetFileInfoForLog(m_bNoBuffering);

		return fsMove;
	}

	void TLocalFilesystemFile::Truncate(file_size_t fsNewSize)
	{
		LOG_TRACE(m_spLog) << "Truncating file to: " << fsNewSize << GetFileInfoForLog(m_bNoBuffering);

		if (!IsOpen())
		{
			LOG_ERROR(m_spLog) << L"Cannot truncate - file not open" << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"File not open yet. Cannot truncate.", LOCATION);
		}

		// when no-buffering is used, there are cases where we'd need to switch to buffered ops
		// to adjust file size
		bool bFileSettingsChanged = false;
		if (m_bNoBuffering)
		{
			file_size_t fsNewAlignedSize = RoundUp<file_size_t>(fsNewSize, MaxSectorSize);
			if (fsNewAlignedSize != fsNewSize)
			{
				LOG_TRACE(m_spLog) << "Truncating to non-aligned size. Requested: " << fsNewSize << L", aligned: " << fsNewAlignedSize << L". Will reopen file in buffering mode." << GetFileInfoForLog(m_bNoBuffering);

				Close();
				OpenExistingForWriting(false);

				bFileSettingsChanged = true;
			}
		}

		LARGE_INTEGER li = { 0, 0 };
		LARGE_INTEGER liNew = { 0, 0 };

		li.QuadPart = fsNewSize;

		LOG_TRACE(m_spLog) << L"Setting file pointer to: " << li.QuadPart << GetFileInfoForLog(m_bNoBuffering);
		if (!SetFilePointerEx(m_hFile, li, &liNew, FILE_BEGIN))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Setting file pointer to: " << li.QuadPart << L" failed." << GetFileInfoForLog(m_bNoBuffering);

			throw TFileException(eErr_SeekFailed, dwLastError, m_pathFile, L"Cannot seek to appropriate position", LOCATION);
		}

		LOG_TRACE(m_spLog) << L"Setting EOF" << GetFileInfoForLog(m_bNoBuffering);
		if(!::SetEndOfFile(m_hFile))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Setting EOF failed" << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_CannotTruncate, dwLastError, m_pathFile, L"Cannot mark the end of file", LOCATION);
		}

		// close the file that was open in inappropriate mode
		if(bFileSettingsChanged)
		{
			LOG_DEBUG(m_spLog) << L"Closing file due to mode change in truncate function" << GetFileInfoForLog(m_bNoBuffering);

			Close();
		}
	}

	void TLocalFilesystemFile::ReadFile(TOverlappedDataBuffer& rBuffer)
	{
		LOG_TRACE(m_spLog) << L"Requesting read of " << rBuffer.GetRequestedDataSize() <<
			L" bytes at position " << rBuffer.GetFilePosition() <<
			L"; buffer-order: " << rBuffer.GetBufferOrder() <<
			GetFileInfoForLog(m_bNoBuffering);

		if (!IsOpen())
		{
			LOG_ERROR(m_spLog) << L"Read request failed - file not open" << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot read from closed file", LOCATION);
		}

		if (!::ReadFileEx(m_hFile, rBuffer.GetBufferPtr(), rBuffer.GetRequestedDataSize(), &rBuffer, OverlappedReadCompleted))
		{
			DWORD dwLastError = GetLastError();
			switch (dwLastError)
			{
			case ERROR_IO_PENDING:
				LOG_TRACE(m_spLog) << L"Read requested and is pending" << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
				return;

			case ERROR_HANDLE_EOF:
				{
					LOG_TRACE(m_spLog) << L"Read request marked as EOF" << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);

					rBuffer.SetBytesTransferred(0);
					rBuffer.SetStatusCode(0);
					rBuffer.SetErrorCode(ERROR_SUCCESS);
					rBuffer.SetLastPart(true);

					TOverlappedDataBufferQueue* pQueue = (TOverlappedDataBufferQueue*)rBuffer.GetParam();

					pQueue->AddFullBuffer(&rBuffer);	// basically the same as OverlappedReadCompleted
					break;
				}

			default:
				{
					LOG_ERROR(m_spLog) << L"Read request failed with error " << dwLastError << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);

					throw TFileException(eErr_CannotReadFile, dwLastError, m_pathFile, L"Error reading data from file", LOCATION);
				}
			}
		}
		else
			LOG_TRACE(m_spLog) << L"Read request succeeded" << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
	}

	void TLocalFilesystemFile::WriteFile(TOverlappedDataBuffer& rBuffer)
	{
		LOG_TRACE(m_spLog) << L"Requesting writing of " << rBuffer.GetRealDataSize() <<
			L" bytes at position " << rBuffer.GetFilePosition() <<
			L"; buffer-order: " << rBuffer.GetBufferOrder() <<
			GetFileInfoForLog(m_bNoBuffering);

		if (!IsOpen())
		{
			LOG_ERROR(m_spLog) << L"Write request failed - file not open" << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot write to closed file", LOCATION);
		}

		DWORD dwToWrite = boost::numeric_cast<DWORD>(rBuffer.GetRealDataSize());

		if (m_bNoBuffering && rBuffer.IsLastPart())
		{
			dwToWrite = RoundUp<DWORD>(dwToWrite, MaxSectorSize);
			LOG_TRACE(m_spLog) << L"Writing last part of file in no-buffering mode. Rounding up last write to " << dwToWrite << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
		}

		if (!::WriteFileEx(m_hFile, rBuffer.GetBufferPtr(), dwToWrite, &rBuffer, OverlappedWriteCompleted))
		{
			DWORD dwLastError = GetLastError();
			if (dwLastError != ERROR_IO_PENDING)
			{
				LOG_ERROR(m_spLog) << L"Write request failed with error " << dwLastError << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
				throw TFileException(eErr_CannotWriteFile, dwLastError, m_pathFile, L"Error while writing to file", LOCATION);
			}

			LOG_TRACE(m_spLog) << L"Write requested and is pending" << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
		}
		else
			LOG_TRACE(m_spLog) << L"Write request succeeded" << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
	}

	void TLocalFilesystemFile::FinalizeFile(TOverlappedDataBuffer& rBuffer)
	{
		LOG_TRACE(m_spLog) << L"Finalizing file" <<
			L"; buffer-order: " << rBuffer.GetBufferOrder() <<
			GetFileInfoForLog(m_bNoBuffering);

		if (!IsOpen())
		{
			LOG_ERROR(m_spLog) << L"Cannot finalize file - file not open" << L"; buffer-order: " << rBuffer.GetBufferOrder() << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot write to closed file", LOCATION);
		}

		if (m_bNoBuffering && rBuffer.IsLastPart())
		{
			DWORD dwToWrite = boost::numeric_cast<DWORD>(rBuffer.GetRealDataSize());
			DWORD dwReallyWritten = RoundUp<DWORD>(dwToWrite, MaxSectorSize);

			if (dwToWrite != dwReallyWritten)
			{
				file_size_t fsNewFileSize = rBuffer.GetFilePosition() + dwToWrite;	// new size

				LOG_TRACE(m_spLog) << L"File need truncating - really written " << dwReallyWritten <<
					L", should write " << dwToWrite <<
					L". Truncating file to " << fsNewFileSize <<
					L"; buffer-order: " << rBuffer.GetBufferOrder() <<
					GetFileInfoForLog(m_bNoBuffering);

				Truncate(fsNewFileSize);
			}
		}
	}

	void TLocalFilesystemFile::InternalClose()
	{
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			LOG_DEBUG(m_spLog) << L"Closing file" << GetFileInfoForLog(m_bNoBuffering);
			::CloseHandle(m_hFile);
		}
		m_hFile = INVALID_HANDLE_VALUE;
	}

	std::wstring TLocalFilesystemFile::GetFileInfoForLog(bool bNoBuffering) const
	{
		std::wstringstream wss;
		wss << L" (handle: " << m_hFile << ", path : " << m_pathFile << L", no-buffering : " << bNoBuffering << L")";
		return wss.str();
	}

	void TLocalFilesystemFile::Close()
	{
		InternalClose();
	}

	file_size_t TLocalFilesystemFile::GetFileSize() const
	{
		LOG_DEBUG(m_spLog) << L"Retrieving file size" << GetFileInfoForLog(m_bNoBuffering);

		if (!IsOpen())
		{
			LOG_ERROR(m_spLog) << L"Retrieving file size failed - file not open" << GetFileInfoForLog(m_bNoBuffering);
			return 0;
		}

		BY_HANDLE_FILE_INFORMATION bhfi;

		if (!::GetFileInformationByHandle(m_hFile, &bhfi))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Retrieving file size failed with error " << dwLastError << GetFileInfoForLog(m_bNoBuffering);

			return 0;
		}

		ULARGE_INTEGER uli;
		uli.HighPart = bhfi.nFileSizeHigh;
		uli.LowPart = bhfi.nFileSizeLow;

		LOG_DEBUG(m_spLog) << L"File size retrieved -> " << uli.QuadPart << GetFileInfoForLog(m_bNoBuffering);

		return uli.QuadPart;
	}

	void TLocalFilesystemFile::GetFileInfo(TFileInfo& tFileInfo) const
	{
		LOG_DEBUG(m_spLog) << L"Retrieving file information" << GetFileInfoForLog(m_bNoBuffering);

		if (!IsOpen())
		{
			LOG_ERROR(m_spLog) << L"Retrieving file information failed - file not open" << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"File not open. Cannot get file info.", LOCATION);
		}

		BY_HANDLE_FILE_INFORMATION bhfi;

		if (!::GetFileInformationByHandle(m_hFile, &bhfi))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Retrieving file information failed with error " << dwLastError << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_CannotGetFileInfo, dwLastError, m_pathFile, L"Retrieving file info from handle failed.", LOCATION);
		}

		ULARGE_INTEGER uli;
		uli.HighPart = bhfi.nFileSizeHigh;
		uli.LowPart = bhfi.nFileSizeLow;

		tFileInfo.SetFilePath(m_pathFile);
		tFileInfo.SetAttributes(bhfi.dwFileAttributes);
		tFileInfo.SetFileTimes(TFileTime(bhfi.ftCreationTime), TFileTime(bhfi.ftLastAccessTime), TFileTime(bhfi.ftLastWriteTime));
		tFileInfo.SetLength64(uli.QuadPart);

		LOG_DEBUG(m_spLog) << L"Retrieving file information succeeded. Attributes: " <<
			bhfi.dwFileAttributes <<
			L", creation-time: " << bhfi.ftCreationTime <<
			L", last-access-time: " << bhfi.ftLastAccessTime<<
			L", last-write-time: " << bhfi.ftLastWriteTime <<
			L", size: " << uli.QuadPart <<
			GetFileInfoForLog(m_bNoBuffering);
	}

	TSmartPath TLocalFilesystemFile::GetFilePath() const
	{
		return m_pathFile;
	}
}
