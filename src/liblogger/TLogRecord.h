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
#ifndef __TLOGRECORD_H__
#define __TLOGRECORD_H__

#include <sstream>
#include "TLogFileData.h"
#include <boost/date_time/time_facet.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

namespace logger
{
	class TLogRecord : public std::wstringstream
	{
	public:
		TLogRecord(const TLogFileDataPtr& spFileData, ESeverityLevel eLevel, const std::wstring& wstrChannel);
		TLogRecord(const TLogRecord&) = delete;
		TLogRecord(TLogRecord&& rSrc) noexcept;

		TLogRecord& operator=(const TLogRecord&) = delete;
		TLogRecord& operator=(TLogRecord&&) = delete;

		~TLogRecord();

		bool IsEnabled() const;
		void Disable();

	private:
		TLogFileDataPtr m_spFileData;
		bool m_bEnabled = true;
	};

	inline TLogRecord::TLogRecord(TLogRecord&& rSrc) noexcept:
		std::wstringstream(std::move(rSrc)),
		m_spFileData(std::move(rSrc.m_spFileData))
	{
	}

	inline TLogRecord::TLogRecord(const TLogFileDataPtr& spFileData, ESeverityLevel eLevel, const std::wstring& wstrChannel) :
		m_spFileData(spFileData)
	{
		boost::posix_time::wtime_facet* facet = new boost::posix_time::wtime_facet();
		facet->format(L"%Y-%m-%d %H:%M:%S.%f");
		imbue(std::locale(std::locale::classic(), facet));

		DWORD dwThreadId = GetCurrentThreadId();

		*this << boost::posix_time::microsec_clock::local_time() << L" [" << SeverityLevelToString(eLevel) << L"] " << wstrChannel << L"[" << dwThreadId << L"]: ";
	}

	inline TLogRecord::~TLogRecord()
	{
		try
		{
			*this << L"\r\n";
			m_spFileData->PushLogEntry(str().c_str());
		}
		catch (const std::exception&)
		{
		}
	}

	inline bool TLogRecord::IsEnabled() const
	{
		return m_bEnabled;
	}

	inline void TLogRecord::Disable()
	{
		m_bEnabled = false;
	}
}

#ifdef _MFC_VER

inline std::wostream& operator<<(std::wostream &os, const CString& str)
{
	return os << (PCTSTR)str;
}

#endif

#endif
