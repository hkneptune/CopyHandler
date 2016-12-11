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
#include <boost/numeric/conversion/cast.hpp>
#include "TLocalFilesystem.h"
#include "TFileException.h"
#include "TFileInfo.h"
#include "TOverlappedMemoryPool.h"
#include "OverlappedCallbacks.h"
#include <fileextd.h>
#include "../libchcore/RoundingFunctions.h"
#include "../libchcore/TFileTime.h"
#include "../libchcore/StreamingHelpers.h"

using namespace chcore;

namespace chengine
{
	// compile-time check - ensure the buffer granularity used for transfers are bigger than expected sector size
	static_assert(TLocalFilesystemFile::MaxSectorSize <= TBufferSizes::BufferGranularity, "Buffer granularity must be equal to or bigger than the max sector size");

	TLocalFilesystemFile::TLocalFilesystemFile(EOpenMode eMode, const TSmartPath& pathFile, bool bNoBuffering, bool bProtectReadOnlyFiles, const logger::TLogFileDataPtr& spLogFileData) :
		m_pathFile(TLocalFilesystem::PrependPathExtensionIfNeeded(pathFile)),
		m_hFile(INVALID_HANDLE_VALUE),
		m_eMode(eMode),
		m_bProtectReadOnlyFiles(bProtectReadOnlyFiles),
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

	void TLocalFilesystemFile::EnsureOpen()
	{
		if(m_hFile != INVALID_HANDLE_VALUE)
			return;

		if(m_eMode == eMode_Read)
			OpenFileForReading();
		else
			OpenFileForWriting();
	}

	void TLocalFilesystemFile::OpenFileForReading()
	{
		LOG_DEBUG(m_spLog) << L"Opening file for reading" << GetFileInfoForLog(m_bNoBuffering);

		m_hFile = ::CreateFile(m_pathFile.ToString(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, GetFlagsAndAttributes(m_bNoBuffering), nullptr);
		if (m_hFile == INVALID_HANDLE_VALUE)
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << "Opening file for reading failed with error: " << dwLastError << GetFileInfoForLog(m_bNoBuffering);

			throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot open for reading", LOCATION);
		}

		LOG_DEBUG(m_spLog) << "Opening file for reading succeeded. New handle: " << m_hFile << GetFileInfoForLog(m_bNoBuffering);
	}

	void TLocalFilesystemFile::OpenFileForWriting()
	{
		Close();

		LOG_DEBUG(m_spLog) << L"Opening file for writing" << GetFileInfoForLog(m_bNoBuffering);

		bool bAttributesChanged = false;

		do
		{
			m_hFile = ::CreateFile(m_pathFile.ToString(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, GetFlagsAndAttributes(m_bNoBuffering), nullptr);
			DWORD dwLastError = GetLastError();
			if(m_hFile == INVALID_HANDLE_VALUE)
			{
				// failed
				if(dwLastError == ERROR_ACCESS_DENIED && !m_bProtectReadOnlyFiles && !bAttributesChanged)
				{
					// handle read-only files
					DWORD dwAttributes = GetFileAttributes(m_pathFile.ToString());
					if(dwAttributes == INVALID_FILE_ATTRIBUTES)
					{
						LOG_ERROR(m_spLog) << "Retrieving file attributes failed while opening file for writing. Error: " << dwLastError << GetFileInfoForLog(m_bNoBuffering);
						throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot retrieve file attributes.", LOCATION);
					}

					if(dwAttributes & FILE_ATTRIBUTE_READONLY)
					{
						if(!SetFileAttributes(m_pathFile.ToString(), dwAttributes & ~FILE_ATTRIBUTE_READONLY))
						{
							LOG_ERROR(m_spLog) << "Error while trying to reset read-only attribute. Error: " << dwLastError << GetFileInfoForLog(m_bNoBuffering);
							throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot reset read-only attribute.", LOCATION);
						}

						bAttributesChanged = true;
						continue;
					}
				}

				// all other errors
				LOG_ERROR(m_spLog) << "Encountered an error while opening file for writing. Error: " << dwLastError << GetFileInfoForLog(m_bNoBuffering);
				throw TFileException(eErr_CannotOpenFile, dwLastError, m_pathFile, L"Cannot open file.", LOCATION);
			}

			// succeeded
			m_bFreshlyCreated = !(dwLastError == ERROR_ALREADY_EXISTS);
			break;
		}
		while(bAttributesChanged);

		LOG_DEBUG(m_spLog) << "Opening file for writing succeeded. New handle: " << m_hFile << GetFileInfoForLog(m_bNoBuffering);
	}

	file_size_t TLocalFilesystemFile::GetSeekPositionForResume(file_size_t fsLastAvailablePosition)
	{
		file_size_t fsMove = (m_bNoBuffering ? RoundDown<file_size_t>(fsLastAvailablePosition, MaxSectorSize) : fsLastAvailablePosition);
		LOG_DEBUG(m_spLog) << "Calculated seek position for last-available-pos: " << fsLastAvailablePosition << L" = " << fsMove << GetFileInfoForLog(m_bNoBuffering);

		return fsMove;
	}

	void TLocalFilesystemFile::SetBasicInfo(DWORD dwAttributes, const TFileTime& ftCreationTime, const TFileTime& ftLastAccessTime, const TFileTime& ftLastWriteTime)
	{
		LOG_TRACE(m_spLog) << "Updating file times" << GetFileInfoForLog(m_bNoBuffering);

		EnsureOpen();

		FILE_BASIC_INFO basicInfo = { 0 };
		basicInfo.FileAttributes = dwAttributes;
		basicInfo.CreationTime.QuadPart = ftCreationTime.ToUInt64();
		basicInfo.LastAccessTime.QuadPart = ftLastAccessTime.ToUInt64();
		basicInfo.LastWriteTime.QuadPart = ftLastWriteTime.ToUInt64();
		basicInfo.ChangeTime.QuadPart = ftLastWriteTime.ToUInt64();

		if(!SetFileInformationByHandle(m_hFile, FileBasicInfo, &basicInfo, sizeof(FILE_BASIC_INFO)))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Failed to set file basic info." << GetFileInfoForLog(m_bNoBuffering);

			throw TFileException(eErr_CannotSetFileInfo, dwLastError, m_pathFile, L"Cannot set basic file info", LOCATION);
		}
	}

	void TLocalFilesystemFile::Truncate(file_size_t fsNewSize)
	{
		LOG_TRACE(m_spLog) << "Truncating file to: " << fsNewSize << GetFileInfoForLog(m_bNoBuffering);

		EnsureOpen();

		FILE_END_OF_FILE_INFO eofInfo = { 0 };
		eofInfo.EndOfFile.QuadPart = fsNewSize;
		if(!SetFileInformationByHandle(m_hFile, FileEndOfFileInfo, &eofInfo, sizeof(FILE_END_OF_FILE_INFO)))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Truncating file to " << fsNewSize << L" failed." << GetFileInfoForLog(m_bNoBuffering);

			throw TFileException(eErr_CannotTruncate, dwLastError, m_pathFile, L"Cannot truncate file", LOCATION);
		}
	}

	void TLocalFilesystemFile::ReadFile(TOverlappedDataBuffer& rBuffer)
	{
		LOG_TRACE(m_spLog) << L"Requesting read of " << rBuffer.GetRequestedDataSize() <<
			L" bytes at position " << rBuffer.GetFilePosition() <<
			L"; buffer-order: " << rBuffer.GetFilePosition() <<
			GetFileInfoForLog(m_bNoBuffering);

		EnsureOpen();

		if (!::ReadFileEx(m_hFile, rBuffer.GetBufferPtr(), rBuffer.GetRequestedDataSize(), &rBuffer, OverlappedReadCompleted))
		{
			DWORD dwLastError = GetLastError();
			switch (dwLastError)
			{
			case ERROR_HANDLE_EOF:
				{
					LOG_TRACE(m_spLog) << L"Read request marked as EOF" << L"; buffer-order: " << rBuffer.GetFilePosition() << GetFileInfoForLog(m_bNoBuffering);

					rBuffer.SetBytesTransferred(0);
					rBuffer.SetStatusCode(0);
					rBuffer.SetErrorCode(ERROR_SUCCESS);
					rBuffer.SetLastPart(true);

					OverlappedReadCompleted(rBuffer.GetErrorCode(), 0, &rBuffer);

					break;
				}

			default:
				{
					LOG_ERROR(m_spLog) << L"Read request failed with error " << dwLastError << L"; buffer-order: " << rBuffer.GetFilePosition() << GetFileInfoForLog(m_bNoBuffering);

					throw TFileException(eErr_CannotReadFile, dwLastError, m_pathFile, L"Error reading data from file", LOCATION);
				}
			}
		}
		else
			LOG_TRACE(m_spLog) << L"Read request succeeded" << L"; buffer-order: " << rBuffer.GetFilePosition() << GetFileInfoForLog(m_bNoBuffering);
	}

	void TLocalFilesystemFile::WriteFile(TOverlappedDataBuffer& rBuffer)
	{
		LOG_TRACE(m_spLog) << L"Requesting writing of " << rBuffer.GetRealDataSize() <<
			L" bytes at position " << rBuffer.GetFilePosition() <<
			L"; buffer-order: " << rBuffer.GetFilePosition() <<
			GetFileInfoForLog(m_bNoBuffering);

		EnsureOpen();

		DWORD dwToWrite = boost::numeric_cast<DWORD>(rBuffer.GetRealDataSize());

		if (m_bNoBuffering && rBuffer.IsLastPart())
		{
			dwToWrite = RoundUp<DWORD>(dwToWrite, MaxSectorSize);
			LOG_TRACE(m_spLog) << L"Writing last part of file in no-buffering mode. Rounding up last write to " << dwToWrite << L"; buffer-order: " << rBuffer.GetFilePosition() << GetFileInfoForLog(m_bNoBuffering);
		}

		if (!::WriteFileEx(m_hFile, rBuffer.GetBufferPtr(), dwToWrite, &rBuffer, OverlappedWriteCompleted))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Write request failed with error " << dwLastError << L"; buffer-order: " << rBuffer.GetFilePosition() << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_CannotWriteFile, dwLastError, m_pathFile, L"Error while writing to file", LOCATION);
		}

		LOG_TRACE(m_spLog) << L"Write request succeeded" << L"; buffer-order: " << rBuffer.GetFilePosition() << GetFileInfoForLog(m_bNoBuffering);
	}

	void TLocalFilesystemFile::FinalizeFile(TOverlappedDataBuffer& rBuffer)
	{
		LOG_TRACE(m_spLog) << L"Finalizing file" <<
			L"; buffer-order: " << rBuffer.GetFilePosition() <<
			GetFileInfoForLog(m_bNoBuffering);

		EnsureOpen();

		if (m_bNoBuffering && rBuffer.IsLastPart())
		{
			DWORD dwToWrite = boost::numeric_cast<DWORD>(rBuffer.GetRealDataSize());
			DWORD dwReallyWritten = RoundUp<DWORD>(dwToWrite, MaxSectorSize);

			if (dwToWrite != dwReallyWritten)
			{
				file_size_t fsNewFileSize = rBuffer.GetFilePosition() + dwToWrite;	// new size
				Truncate(fsNewFileSize);
			}
		}
	}

	void TLocalFilesystemFile::CancelIo()
	{
		if(IsOpen())
		{
			if(!::CancelIo(m_hFile))
			{
				DWORD dwLastError = GetLastError();
				LOG_ERROR(m_spLog) << L"CancelIo request failed with error " << dwLastError << GetFileInfoForLog(m_bNoBuffering);
				throw TFileException(eErr_CancelIoFailed, dwLastError, m_pathFile, L"Error while writing to file", LOCATION);
			}
		}
	}

	bool TLocalFilesystemFile::IsOpen() const
	{
		return m_hFile != INVALID_HANDLE_VALUE;
	}

	bool TLocalFilesystemFile::IsFreshlyCreated()
	{
		EnsureOpen();
		return m_bFreshlyCreated;
	}

	void TLocalFilesystemFile::InternalClose()
	{
		if (m_hFile != INVALID_HANDLE_VALUE)
		{
			LOG_DEBUG(m_spLog) << L"Closing file" << GetFileInfoForLog(m_bNoBuffering);
			if(!::CloseHandle(m_hFile))
			{
				DWORD dwLastError = GetLastError();
				LOG_ERROR(m_spLog) << L"CloseHandle failed with error " << dwLastError << L". Ignoring." << GetFileInfoForLog(m_bNoBuffering);
			}
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

	file_size_t TLocalFilesystemFile::GetFileSize()
	{
		LOG_DEBUG(m_spLog) << L"Retrieving file size" << GetFileInfoForLog(m_bNoBuffering);

		EnsureOpen();

		BY_HANDLE_FILE_INFORMATION bhfi;

		if (!::GetFileInformationByHandle(m_hFile, &bhfi))
		{
			DWORD dwLastError = GetLastError();
			LOG_ERROR(m_spLog) << L"Retrieving file size failed with error " << dwLastError << GetFileInfoForLog(m_bNoBuffering);
			throw TFileException(eErr_CannotGetFileInfo, dwLastError, m_pathFile, L"Error while trying to retrieve file size.", LOCATION);
		}

		ULARGE_INTEGER uli;
		uli.HighPart = bhfi.nFileSizeHigh;
		uli.LowPart = bhfi.nFileSizeLow;

		LOG_DEBUG(m_spLog) << L"File size retrieved -> " << uli.QuadPart << GetFileInfoForLog(m_bNoBuffering);

		return uli.QuadPart;
	}

	void TLocalFilesystemFile::GetFileInfo(TFileInfo& tFileInfo)
	{
		LOG_DEBUG(m_spLog) << L"Retrieving file information" << GetFileInfoForLog(m_bNoBuffering);

		EnsureOpen();

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
