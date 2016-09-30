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
#include "TLogRotator.h"
#include <boost\date_time\posix_time\ptime.hpp>
#include <boost\date_time\posix_time\posix_time_io.hpp>
#include "..\libchcore\TString.h"
#include "TLogSinkCollection.h"
#include "..\libchcore\TFileException.h"

namespace chcore
{
	TLogRotator::TLogRotator(unsigned int uiMaxRotatedFiles, unsigned long long ullMaxLogSize) :
		m_uiMaxRotatedFiles(uiMaxRotatedFiles),
		m_ullMaxLogSize(ullMaxLogSize)
	{
	}

	void TLogRotator::SetLimits(unsigned int uiMaxRotatedFiles, unsigned long long ullMaxLogSize)
	{
		m_uiMaxRotatedFiles = uiMaxRotatedFiles;
		m_ullMaxLogSize = ullMaxLogSize;
	}

	void TLogRotator::RotateFile(const TSmartPath& pathLog, TLogSink& sinkData, size_t stRequiredSpace)
	{
		unsigned long long ullCurrentLogSize = sinkData.GetCurrentLogSize();
		unsigned long long ullNewSize = ullCurrentLogSize + stRequiredSpace;
		if (ullCurrentLogSize == 0 || ullNewSize < m_ullMaxLogSize)
			return;

		sinkData.CloseLogFile();

		TString pathNew = pathLog.ToWString();
		if (pathNew.EndsWithNoCase(L".log"))
			pathNew.LeftSelf(pathNew.GetLength() - 4);

		boost::posix_time::ptime timeNow = boost::posix_time::second_clock::local_time();
		boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
		facet->format("%Y%m%d%H%M%S");
		std::wstringstream stream;
		stream.imbue(std::locale(std::locale::classic(), facet));
		stream << time;
		pathNew.Append(L".");
		pathNew.Append(stream.str().c_str());
		pathNew.Append(L".log");

		if (!MoveFile(pathLog.ToString(), pathNew.c_str()))
			throw TFileException(eErr_CannotFastMove, GetLastError(), pathLog, L"Cannot rotate file", LOCATION);

		sinkData.AddRotatedFile(PathFromWString(pathNew));
		sinkData.RemoveObsoleteRotatedLogs(m_uiMaxRotatedFiles);
	}

	void TLogRotator::ScanForLogs(const TSmartPath& pathDir, TLogSinkCollection& rCollection)
	{
		TSmartPath pathSearch = pathDir;
		pathSearch += PathFromString(L"*.log");

		std::vector<TSmartPath> vPaths;
		WIN32_FIND_DATA wfd = { 0 };

		HANDLE hFind = FindFirstFile(pathSearch.ToString(), &wfd);
		BOOL bFound = (hFind != INVALID_HANDLE_VALUE);
		while (bFound)
		{
			TSmartPath pathFound = pathDir + PathFromString(wfd.cFileName);
			vPaths.push_back(pathFound);

			bFound = FindNextFile(hFind, &wfd);
		}

		std::sort(vPaths.begin(), vPaths.end(), [](const TSmartPath& path1, const TSmartPath& path2) { return path2 > path1; });

		for (const TSmartPath& rPath : vPaths)
		{
			rCollection.AddPath(rPath);
		}
	}
}
