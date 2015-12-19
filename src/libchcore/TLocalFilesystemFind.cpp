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

namespace chcore
{
	TLocalFilesystemFind::TLocalFilesystemFind(const TSmartPath& pathDir, const TSmartPath& pathMask) :
		m_pathDir(pathDir),
		m_pathMask(pathMask),
		m_hFind(INVALID_HANDLE_VALUE)
	{
	}

	TLocalFilesystemFind::~TLocalFilesystemFind()
	{
		Close();
	}

	bool TLocalFilesystemFind::FindNext(TFileInfoPtr& rspFileInfo)
	{
		WIN32_FIND_DATA wfd;
		TSmartPath pathCurrent = m_pathDir + m_pathMask;

		// Iterate through dirs & files
		bool bContinue = true;
		if (m_hFind != INVALID_HANDLE_VALUE)
			bContinue = (FindNextFile(m_hFind, &wfd) != FALSE);
		else
		{
			m_hFind = FindFirstFileEx(TLocalFilesystem::PrependPathExtensionIfNeeded(pathCurrent).ToString(), FindExInfoBasic, &wfd, FindExSearchNameMatch, nullptr, FIND_FIRST_EX_LARGE_FETCH);
			bContinue = (m_hFind != INVALID_HANDLE_VALUE);
		}
		if (bContinue)
		{
			do
			{
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					rspFileInfo->Init(m_pathDir + PathFromString(wfd.cFileName), wfd.dwFileAttributes, (((ULONGLONG)wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
						wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
					return true;
				}
				else if (wfd.cFileName[0] != _T('.') || (wfd.cFileName[1] != _T('\0') && (wfd.cFileName[1] != _T('.') || wfd.cFileName[2] != _T('\0'))))
				{
					// Add directory itself
					rspFileInfo->Init(m_pathDir + PathFromString(wfd.cFileName),
						wfd.dwFileAttributes, (((ULONGLONG)wfd.nFileSizeHigh) << 32) + wfd.nFileSizeLow, wfd.ftCreationTime,
						wfd.ftLastAccessTime, wfd.ftLastWriteTime, 0);
					return true;
				}
			} while (m_hFind != INVALID_HANDLE_VALUE && ::FindNextFile(m_hFind, &wfd));	// checking m_hFind in case other thread changed it (it shouldn't happen though)

			Close();
		}

		return false;
	}

	void TLocalFilesystemFind::Close()
	{
		if (m_hFind != INVALID_HANDLE_VALUE)
			FindClose(m_hFind);
		m_hFind = INVALID_HANDLE_VALUE;
	}
}
