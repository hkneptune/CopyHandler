// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#include "TLogSink.h"
#include "..\libchcore\TFileException.h"

namespace chcore
{
	TLogSink::TLogSink(const chcore::TSmartPath& pathLog)
		: m_pathLog(pathLog)
	{
	}

	HANDLE TLogSink::GetFileHandle()
	{
		if (m_handleFile != INVALID_HANDLE_VALUE)
			return m_handleFile;

		m_handleFile = CreateFile(m_pathLog.ToString(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (m_handleFile == INVALID_HANDLE_VALUE)
			throw TFileException(eErr_CannotOpenFile, GetLastError(), m_pathLog, L"Cannot open log file", LOCATION);

		return m_handleFile;
	}

	unsigned long long TLogSink::GetCurrentLogSize()
	{
		LARGE_INTEGER liSize = { 0 };
		if (!GetFileSizeEx(GetFileHandle(), &liSize))
			throw new TFileException(eErr_CannotGetFileInfo, GetLastError(), m_pathLog, L"Cannot determine current log size", LOCATION);

		return liSize.QuadPart;
	}

	void TLogSink::AddRotatedFile(const TSmartPath& rPath)
	{
		m_setRotatedFiles.insert(rPath);
	}

	void TLogSink::RemoveObsoleteRotatedLogs(unsigned int uiMaxRotatedFiles)
	{
		while (m_setRotatedFiles.size() > uiMaxRotatedFiles)
		{
			auto iterRotatedFile = m_setRotatedFiles.begin();
			if (!DeleteFile(iterRotatedFile->ToString()))
				break;

			m_setRotatedFiles.erase(iterRotatedFile);
		}
	}

	void chcore::TLogSink::CloseLogFile()
	{
		m_handleFile.Close();
	}

	void chcore::TLogSink::CloseIfTimedOut(unsigned long long ullCurrentTimestamp, unsigned long long ullMaxHandleCacheTime)
	{
		if (ullCurrentTimestamp - m_ullLastUsageTimestamp > ullMaxHandleCacheTime)
			CloseLogFile();
	}
}
