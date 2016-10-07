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
#ifndef __TMULTIFILEBACKEND_H__
#define __TMULTIFILEBACKEND_H__

#include <boost/log/sinks/basic_sink_backend.hpp>
#include "../libchcore/TAutoHandles.h"
#include "../libchcore/ITimestampProvider.h"
#include "../libchcore/TPath.h"
#include "TLogSink.h"
#include "TLogSinkCollection.h"
#include "TLogRotator.h"

namespace chcore
{
	class TMultiFileBackend : public boost::log::sinks::basic_formatted_sink_backend<char>
	{
	public:
		const unsigned int MaxHandleCacheTime = 60000;

	public:
		TMultiFileBackend(ITimestampProviderPtr spTimestampProvider, unsigned int uiMaxRotatedFiles, unsigned long long ullMaxLogSize);

		void Init(const TSmartPath& pathDirectory, unsigned int uiMaxRotatedFiles, unsigned long long ullMaxLogSize);

		void consume(const boost::log::record_view& rec, const string_type& formatted_message);

	private:
		void SetDirectory(const TSmartPath& pathDirectory);

		static TSmartPath GetLogName(const boost::log::record_view &rec);
		HANDLE GetLogFile(const TSmartPath& pathLog, TLogSink& sinkData, size_t stRequiredSpace);

	private:
		unsigned int m_uiHandleCacheTime = MaxHandleCacheTime;

		TLogSinkCollection m_mapLogs;
		TLogRotator m_logRotator;
		ITimestampProviderPtr m_spTimestampProvider;
		bool m_bInitialized = false;
	};
}

#endif
