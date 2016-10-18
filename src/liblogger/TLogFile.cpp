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
#include "TLogFile.h"
#include <functional>
#include <boost\date_time\posix_time\posix_time_io.hpp>
#include <boost\algorithm\string.hpp>
#include <codecvt>

namespace logger
{
	namespace internal
	{
		TLogFile::TLogFile(PCTSTR pszPath, const TLoggerRotationInfoPtr& spRotationInfo) :
			m_strLogPath(pszPath ? pszPath : L""),
			m_spFileHandle(),
			m_spRotationInfo(spRotationInfo)
		{
			if (!pszPath)
				throw std::invalid_argument("pszPath");
			if (!spRotationInfo)
				throw std::invalid_argument("spRotationInfo");

			ScanForRotatedLogs();
		}

		void TLogFile::Write(std::list<std::wstring>& rListEntries)
		{
			if (rListEntries.empty())
				return;

			try
			{
				std::wstring_convert<std::codecvt_utf8<wchar_t>> utf8Converter;

				for (const std::wstring& rstrEntry : rListEntries)
				{
					std::string strUtf8Line = utf8Converter.to_bytes(rstrEntry);

					size_t stEntryLen = strUtf8Line.length();
					if (NeedRotation(stEntryLen))
						RotateFile();

					DWORD dwWritten = 0;
					if (!WriteFile(GetFileHandle(), strUtf8Line.c_str(), boost::numeric_cast<DWORD>(stEntryLen), &dwWritten, nullptr))
						throw std::runtime_error("Cannot write to log, system error");
				}
			}
			catch (const std::exception&)
			{
				rListEntries.clear();
				return;
			}

			m_timeLastWriteTime = time(nullptr);
			rListEntries.clear();
		}

		void TLogFile::CloseIfUnused()
		{
			if (time(nullptr) - m_timeLastWriteTime > MaxHandleCacheTime)
				CloseLogFile();
		}

		void TLogFile::CloseLogFile()
		{
			m_spFileHandle.reset();
		}

		const std::vector<std::wstring>& TLogFile::GetRotatedLogs() const
		{
			return m_vRotatedFiles;
		}

		std::wstring TLogFile::GetLogPath() const
		{
			return m_strLogPath;
		}

		HANDLE TLogFile::GetFileHandle()
		{
			if (m_spFileHandle != nullptr)
				return m_spFileHandle.get();

			HANDLE hFile = CreateFile(m_strLogPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
			if (hFile == INVALID_HANDLE_VALUE)
				throw std::runtime_error("Cannot open log file");

			m_spFileHandle.reset(hFile, CloseHandle);

			LARGE_INTEGER liSeek = { 0 };

			BOOL bRes = SetFilePointerEx(hFile, liSeek, nullptr, SEEK_END);
			if (!bRes)
				throw std::runtime_error("Cannot seek to the end of log file");

			return m_spFileHandle.get();
		}

		unsigned long long TLogFile::GetCurrentLogSize()
		{
			LARGE_INTEGER liSize = { 0 };
			if (!GetFileSizeEx(GetFileHandle(), &liSize))
				throw std::runtime_error("Cannot determine current log size");

			return liSize.QuadPart;
		}

		void TLogFile::RotateFile()
		{
			m_spFileHandle.reset();

			std::wstring pathNew = m_strLogPath;
			if (boost::iends_with(pathNew, L".log"))
				pathNew.erase(pathNew.end() - 4, pathNew.end());

			boost::posix_time::ptime timeNow = boost::posix_time::microsec_clock::local_time();
			boost::posix_time::wtime_facet* facet = new boost::posix_time::wtime_facet();
			facet->format(L"%Y%m%d%H%M%S%f");
			std::wstringstream stream;
			stream.imbue(std::locale(std::locale::classic(), facet));
			stream << timeNow;
			pathNew += L".";
			pathNew += stream.str().c_str();
			pathNew += L".log";

			if (!MoveFile(m_strLogPath.c_str(), pathNew.c_str()) && GetLastError() != ERROR_FILE_NOT_FOUND)
				throw std::runtime_error("Cannot rotate file");

			m_vRotatedFiles.push_back(std::move(pathNew));
			RemoveObsoleteRotatedLogs();
		}

		void TLogFile::RemoveObsoleteRotatedLogs()
		{
			while (m_vRotatedFiles.size() > m_spRotationInfo->GetMaxRotatedCount())
			{
				auto iterRotatedFile = m_vRotatedFiles.begin();
				if (!DeleteFile(iterRotatedFile->c_str()))
					break;

				m_vRotatedFiles.erase(iterRotatedFile);
			}
		}

		void TLogFile::ScanForRotatedLogs()
		{
			std::wstring strSearchMask = m_strLogPath;
			std::wstring strDir;

			size_t stDirPos = strSearchMask.find_last_of(L"\\/");
			if (stDirPos != std::wstring::npos)
				strDir = strSearchMask.substr(0, stDirPos + 1);

			if (boost::iends_with(strSearchMask, L".log"))
				strSearchMask.erase(strSearchMask.end() - 4, strSearchMask.end());
			strSearchMask += L".*.log";

			std::vector<std::wstring> vPaths;
			WIN32_FIND_DATA wfd = { 0 };

			HANDLE hFind = FindFirstFile(strSearchMask.c_str(), &wfd);
			BOOL bFound = (hFind != INVALID_HANDLE_VALUE);
			while (bFound)
			{
				std::wstring strLogFullPath = strDir + wfd.cFileName;
				vPaths.push_back(strLogFullPath);

				bFound = FindNextFile(hFind, &wfd);
			}

			std::sort(vPaths.begin(), vPaths.end(), [](const std::wstring& path1, const std::wstring& path2) { return boost::ilexicographical_compare(path1, path2); });
			std::swap(m_vRotatedFiles, vPaths);
		}

		bool TLogFile::NeedRotation(size_t stDataSize)
		{
			unsigned long long ullCurrentSize = GetCurrentLogSize();
			unsigned long long ullMaxLogSize = m_spRotationInfo->GetMaxLogSize();

			return ullCurrentSize + stDataSize > ullMaxLogSize;
		}
	}
}
