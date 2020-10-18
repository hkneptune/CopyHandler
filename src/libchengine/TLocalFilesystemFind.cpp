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
#include "TLocalFilesystemFind.h"
#include "TLocalFilesystem.h"
#include "TFileInfo.h"
#include "../libchcore/StreamingHelpers.h"

using namespace chcore;

namespace chengine
{
	TLocalFilesystemFind::TLocalFilesystemFind(const TSmartPath& pathDir, const TSmartPath& pathMask, const logger::TLogFileDataPtr& spLogFileData) :
		m_pathDir(pathDir),
		m_pathMask(pathMask),
		m_hFind(INVALID_HANDLE_VALUE),
		m_spLog(logger::MakeLogger(spLogFileData, L"Filesystem-Find"))
	{
	}

	TLocalFilesystemFind::~TLocalFilesystemFind()
	{
		try
		{
			InternalClose();
		}
		catch (const std::exception& e)
		{
		}
	}

	bool TLocalFilesystemFind::FindNext(TFileInfoPtr& rspFileInfo)
	{
		WIN32_FIND_DATA wfd;

		TSmartPath pathCurrent = TLocalFilesystem::PrependPathExtensionIfNeeded(m_pathDir + m_pathMask);
		// Iterate through dirs & files
		bool bContinue = true;
		if (m_hFind != INVALID_HANDLE_VALUE)
		{
			LOG_TRACE(m_spLog) << "Find next" << GetFindLogData();
			bContinue = (FindNextFile(m_hFind, &wfd) != FALSE);
		}
		else
		{
			LOG_TRACE(m_spLog) << "Find first" << GetFindLogData();

			m_hFind = FindFirstFileEx(pathCurrent.ToString(), FindExInfoStandard, &wfd, FindExSearchNameMatch, nullptr, 0);
			bContinue = (m_hFind != INVALID_HANDLE_VALUE);
		}
		if (bContinue)
		{
			do
			{
				unsigned long long ullObjectSize = (((unsigned long long)wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow;

				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					LOG_TRACE(m_spLog) << "Found directory: " << wfd.cFileName <<
						L", attrs: " << wfd.dwFileAttributes <<
						L", size: " << ullObjectSize <<
						L", created: " << wfd.ftCreationTime <<
						L", last-access: " << wfd.ftLastAccessTime <<
						L", last-write: " << wfd.ftLastWriteTime <<
						GetFindLogData();

					rspFileInfo->Init(m_pathDir + PathFromString(wfd.cFileName), wfd.dwFileAttributes, ullObjectSize, wfd.ftCreationTime,
						wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
					return true;
				}

				if (wfd.cFileName[0] != _T('.') || (wfd.cFileName[1] != _T('\0') && (wfd.cFileName[1] != _T('.') || wfd.cFileName[2] != _T('\0'))))
				{
					LOG_TRACE(m_spLog) << "Found file: " << wfd.cFileName <<
						L", attrs: " << wfd.dwFileAttributes <<
						L", size: " << ullObjectSize <<
						L", created: " << wfd.ftCreationTime <<
						L", last-access: " << wfd.ftLastAccessTime <<
						L", last-write: " << wfd.ftLastWriteTime <<
						GetFindLogData();

					// Add directory itself
					rspFileInfo->Init(m_pathDir + PathFromString(wfd.cFileName),
						wfd.dwFileAttributes, ullObjectSize, wfd.ftCreationTime,
						wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
					return true;
				}
			} while (m_hFind != INVALID_HANDLE_VALUE && ::FindNextFile(m_hFind, &wfd));	// checking m_hFind in case other thread changed it (it shouldn't happen though)

			Close();
		}

		return false;
	}

	void TLocalFilesystemFind::InternalClose()
	{
		if(m_hFind != INVALID_HANDLE_VALUE)
		{
			LOG_TRACE(m_spLog) << "Closing finder" << GetFindLogData();
			FindClose(m_hFind);
		}
		m_hFind = INVALID_HANDLE_VALUE;
	}

	std::wstring TLocalFilesystemFind::GetFindLogData() const
	{
		std::wstringstream wss;
		wss << L" (directory: " << m_pathDir << L", mask: " << m_pathMask << L", handle: " << m_hFind << L")";
		return wss.str();
	}

	void TLocalFilesystemFind::Close()
	{
		InternalClose();
	}
}
