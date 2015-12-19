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
#include "TFakeFilesystemFile.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TFakeFilesystem.h"
#include <boost/numeric/conversion/cast.hpp>
#include "RoundingFunctions.h"
#include "TFileException.h"

namespace
{
	struct APCINFO
	{
		OVERLAPPED* pOverlapped;
		DWORD dwError;
		DWORD dwNumberOfBytesTransfered;
	};

	enum EStatus : DWORD
	{
		STATUS_OK = 0,
		STATUS_END_OF_FILE = 0xc0000011
	};
}

namespace chcore
{
	VOID CALLBACK ReadCompleted(ULONG_PTR dwParam)
	{
		APCINFO* pApcInfo = (APCINFO*)dwParam;
		OverlappedReadCompleted(pApcInfo->dwError, pApcInfo->dwNumberOfBytesTransfered, pApcInfo->pOverlapped);

		delete pApcInfo;
	}

	VOID CALLBACK WriteCompleted(ULONG_PTR dwParam)
	{
		APCINFO* pApcInfo = (APCINFO*)dwParam;
		OverlappedWriteCompleted(pApcInfo->dwError, pApcInfo->dwNumberOfBytesTransfered, pApcInfo->pOverlapped);

		delete pApcInfo;
	}

	TFakeFilesystemFile::TFakeFilesystemFile(const TSmartPath& pathFile, bool bNoBuffering, TFakeFilesystem* pFilesystem) :
		m_pathFile(pathFile),
		m_pFilesystem(pFilesystem),
		m_bIsOpen(false),
		m_bNoBuffering(bNoBuffering),
		m_bModeReading(true)
	{
		if (!pFilesystem || pathFile.IsEmpty())
			THROW_CORE_EXCEPTION(eErr_InvalidArgument);
	}

	TFakeFilesystemFile::~TFakeFilesystemFile()
	{
	}

	void TFakeFilesystemFile::Close()
	{
		m_bIsOpen = false;
	}

	TSmartPath TFakeFilesystemFile::GetFilePath() const
	{
		return m_pathFile;
	}

	unsigned long long TFakeFilesystemFile::GetFileSize() const
	{
		TFakeFileDescriptionPtr spFileDesc = m_pFilesystem->FindFileByLocation(m_pathFile);
		if (!spFileDesc)
			THROW_FILE_EXCEPTION(eErr_CannotGetFileInfo, ERROR_FILE_INVALID, m_pathFile, L"Cannot retrieve file info - file does not exist");

		return spFileDesc->GetFileInfo().GetLength64();
	}

	bool TFakeFilesystemFile::IsOpen() const
	{
		return m_bIsOpen;
	}

	void TFakeFilesystemFile::FinalizeFile(TOverlappedDataBuffer& /*rBuffer*/)
	{
		// does nothing
	}

	void TFakeFilesystemFile::WriteFile(TOverlappedDataBuffer& rBuffer)
	{
		if (!IsOpen())
			THROW_FILE_EXCEPTION(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot write to closed file");

		// file should have been created already by create for write functions
		TFakeFileDescriptionPtr spFileDesc = m_pFilesystem->FindFileByLocation(m_pathFile);
		if (!spFileDesc)
			THROW_FILE_EXCEPTION(eErr_CannotWriteFile, ERROR_FILE_INVALID, m_pathFile, L"Cannot write to non-existent file");

		APCINFO* pInfo = new APCINFO;
		unsigned long long ullNewSize = 0;
		unsigned long long ullGrow = 0;
		if (rBuffer.GetFilePosition() >= spFileDesc->GetFileInfo().GetLength64())
		{
			ullNewSize = rBuffer.GetFilePosition() + rBuffer.GetRealDataSize();
			ullGrow = ullNewSize - spFileDesc->GetFileInfo().GetLength64();
		}
		else
		{
			ullNewSize = std::max(rBuffer.GetFilePosition() + rBuffer.GetRealDataSize(), spFileDesc->GetFileInfo().GetLength64());
			ullGrow = ullNewSize - spFileDesc->GetFileInfo().GetLength64();
		}

		spFileDesc->GetFileInfo().SetLength64(ullNewSize);

		rBuffer.SetStatusCode(STATUS_OK);
		rBuffer.SetBytesTransferred(rBuffer.GetRealDataSize());
		pInfo->dwError = ERROR_SUCCESS;
		pInfo->dwNumberOfBytesTransfered = rBuffer.GetRealDataSize();
		pInfo->pOverlapped = &rBuffer;

		if (QueueUserAPC(WriteCompleted, GetCurrentThread(), (ULONG_PTR)pInfo) == 0)
			THROW_CORE_EXCEPTION(eErr_InternalProblem);
	}

	void TFakeFilesystemFile::ReadFile(TOverlappedDataBuffer& rBuffer)
	{
		if (!IsOpen())
			THROW_FILE_EXCEPTION(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot read from closed file");

		// check if we're reading the undamaged data
		TFakeFileDescriptionPtr spFileDesc = m_pFilesystem->FindFileByLocation(m_pathFile);
		if (!spFileDesc)
			THROW_FILE_EXCEPTION(eErr_CannotReadFile, ERROR_FILE_INVALID, m_pathFile, L"Cannot read from non-existent file");

		const TSparseRangeMap& rmapDamage = spFileDesc->GetDamageMap();
		if (rmapDamage.OverlapsRange(rBuffer.GetFilePosition(), rBuffer.GetRequestedDataSize()))
		{
			APCINFO* pInfo = new APCINFO;
			pInfo->dwError = ERROR_READ_FAULT;
			pInfo->dwNumberOfBytesTransfered = 0;
			pInfo->pOverlapped = &rBuffer;

			if (QueueUserAPC(ReadCompleted, GetCurrentThread(), (ULONG_PTR)pInfo) == 0)
				THROW_CORE_EXCEPTION(eErr_InternalProblem);
		}
		else
		{
			APCINFO* pInfo = new APCINFO;

			if (rBuffer.GetFilePosition() >= spFileDesc->GetFileInfo().GetLength64())
			{
				rBuffer.SetStatusCode(STATUS_END_OF_FILE);
				rBuffer.SetBytesTransferred(0);
				pInfo->dwError = ERROR_HANDLE_EOF;
				pInfo->dwNumberOfBytesTransfered = 0;
				pInfo->pOverlapped = &rBuffer;
			}
			else if (rBuffer.GetFilePosition() + rBuffer.GetRequestedDataSize() > spFileDesc->GetFileInfo().GetLength64())
			{
				file_size_t fsRemaining = spFileDesc->GetFileInfo().GetLength64() - rBuffer.GetFilePosition();

				rBuffer.SetStatusCode(STATUS_OK);
				rBuffer.SetBytesTransferred(boost::numeric_cast<ULONG_PTR>(fsRemaining));
				pInfo->dwError = ERROR_SUCCESS;
				pInfo->dwNumberOfBytesTransfered = boost::numeric_cast<DWORD>(fsRemaining);
				pInfo->pOverlapped = &rBuffer;
			}
			else
			{
				rBuffer.SetStatusCode(STATUS_OK);
				rBuffer.SetBytesTransferred(rBuffer.GetRequestedDataSize());
				pInfo->dwError = ERROR_SUCCESS;
				pInfo->dwNumberOfBytesTransfered = rBuffer.GetRequestedDataSize();
				pInfo->pOverlapped = &rBuffer;
			}

			GenerateBufferContent(rBuffer);

			if (QueueUserAPC(ReadCompleted, GetCurrentThread(), (ULONG_PTR)pInfo) == 0)
				THROW_CORE_EXCEPTION(eErr_InternalProblem);
		}
	}

	file_size_t TFakeFilesystemFile::GetSeekPositionForResume(file_size_t fsLastAvailablePosition)
	{
		file_size_t fsMove = (m_bNoBuffering ? RoundDown<file_size_t>(fsLastAvailablePosition, MaxSectorSize) : fsLastAvailablePosition);
		return fsMove;
	}

	void TFakeFilesystemFile::Truncate(file_size_t fsNewSize)
	{
		if (!IsOpen())
			THROW_FILE_EXCEPTION(eErr_FileNotOpen, ERROR_INVALID_HANDLE, m_pathFile, L"Cannot truncate closed file");

		// check if we're reading the undamaged data
		TFakeFileDescriptionPtr spFileDesc = m_pFilesystem->FindFileByLocation(m_pathFile);
		if (!spFileDesc)
			THROW_FILE_EXCEPTION(eErr_CannotTruncate, ERROR_FILE_INVALID, m_pathFile, L"Cannot truncate non-existent file");

		spFileDesc->GetFileInfo().SetLength64(fsNewSize);
	}

	void TFakeFilesystemFile::OpenExistingForWriting()
	{
		TFakeFileDescriptionPtr spFileDesc = m_pFilesystem->FindFileByLocation(m_pathFile);
		if (!spFileDesc)
			THROW_FILE_EXCEPTION(eErr_CannotOpenFile, ERROR_FILE_INVALID, m_pathFile, L"Cannot open existing for writing");

		Close();

		m_bIsOpen = true;
		m_bModeReading = false;
	}

	void TFakeFilesystemFile::CreateNewForWriting()
	{
		TFakeFileDescriptionPtr spFileDesc = m_pFilesystem->FindFileByLocation(m_pathFile);
		if(!spFileDesc)
		{
			TFakeFileDescriptionPtr parentDesc = m_pFilesystem->FindFileByLocation(m_pathFile.GetParent());
			if (!parentDesc)
				THROW_FILE_EXCEPTION(eErr_CannotOpenFile, ERROR_FUNCTION_FAILED, m_pathFile, L"Cannot open existing for writing");

			FILETIME ftCurrent = m_pFilesystem->GetCurrentFileTime();
			TFakeFileDescriptionPtr spNewFile(std::make_shared<TFakeFileDescription>(
				TFileInfo(nullptr, m_pathFile, FILE_ATTRIBUTE_NORMAL, 0, ftCurrent, ftCurrent, ftCurrent, 0),
				TSparseRangeMap()
				));

			m_pFilesystem->m_listFilesystemContent.push_back(spNewFile);
		}

		Close();

		m_bIsOpen = true;
		m_bModeReading = false;
	}

	void TFakeFilesystemFile::OpenExistingForReading()
	{
		TFakeFileDescriptionPtr spFileDesc = m_pFilesystem->FindFileByLocation(m_pathFile);
		if (!spFileDesc)
			THROW_FILE_EXCEPTION(eErr_CannotOpenFile, ERROR_FILE_INVALID, m_pathFile, L"Cannot find file");

		Close();

		m_bIsOpen = true;
		m_bModeReading = true;
	}

	void TFakeFilesystemFile::GenerateBufferContent(TOverlappedDataBuffer &rBuffer)
	{
		if(rBuffer.GetBytesTransferred() > 0)
		{
			ZeroMemory(rBuffer.GetBufferPtr(), rBuffer.GetBufferSize());

			size_t stCount = rBuffer.GetBytesTransferred() / sizeof(file_size_t);
			if (stCount > 0)
			{
				file_size_t* pBuffer = (file_size_t*)rBuffer.GetBufferPtr();
				for (size_t stIndex = 0; stIndex != stCount; ++stIndex)
				{
					pBuffer[stIndex] = rBuffer.GetFilePosition() + stIndex * sizeof(file_size_t);
				}
			}
		}
	}

	void TFakeFilesystemFile::GetFileInfo(TFileInfo& tFileInfo) const
	{
		TFakeFileDescriptionPtr spFileDesc = m_pFilesystem->FindFileByLocation(m_pathFile);
		if (!spFileDesc)
			THROW_FILE_EXCEPTION(eErr_CannotGetFileInfo, ERROR_FILE_INVALID, m_pathFile, L"Cannot retrieve file info - file does not exist");

		tFileInfo = spFileDesc->GetFileInfo();
	}
}
