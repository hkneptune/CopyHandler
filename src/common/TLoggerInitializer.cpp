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
#include "TLoggerInitializer.h"
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/locale.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_multifile_backend.hpp>
#include "TMultiFileBackend.h"

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

using namespace boost::log::trivial;

using Backend = chcore::TMultiFileBackend;
using LogSink = sinks::asynchronous_sink<Backend>;
using LogSinkPtr = boost::shared_ptr<LogSink>;

namespace chcore
{
	struct TLoggerInitializer::InternalData
	{
		LogSinkPtr m_spSink;
	};

	TLoggerInitializer::TLoggerInitializer() :
		m_spData(new InternalData)
	{
	}

	TLoggerInitializer::~TLoggerInitializer()
	{
		Uninit();
	}

	void TLoggerInitializer::Init(const TSmartPath& pathDirWithLogs, unsigned int uiMaxRotatedFiles, unsigned long long ullMaxLogSize)
	{
		if (m_bWasInitialized)
			return;

		boost::shared_ptr< logging::core > spCore = logging::core::get();

		logging::add_common_attributes();

		// sink BACKEND
		boost::shared_ptr<Backend> spBackend = boost::make_shared<Backend>(nullptr, uiMaxRotatedFiles, ullMaxLogSize);
		spBackend->Init(pathDirWithLogs, uiMaxRotatedFiles, ullMaxLogSize);

		// Sink FRONTEND
		LogSinkPtr spSink(new LogSink(spBackend));

		// Set the formatter
		spSink->set_formatter
		(
			expr::stream
			<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
			<< " [" << boost::log::trivial::severity << "] "
			<< expr::attr< std::wstring >("Channel") << ": "
			<< expr::wmessage
		);

		std::locale loc = boost::locale::generator()("en_EN.UTF-8");
		spSink->imbue(loc);

		spCore->add_sink(spSink);

		m_spData->m_spSink = spSink;

		m_bWasInitialized = true;
	}

	void TLoggerInitializer::Uninit()
	{
		if (!m_spData->m_spSink || !m_bWasInitialized)
			return;

		boost::shared_ptr< logging::core > core = logging::core::get();

		// Remove the sink from the core, so that no records are passed to it
		core->remove_sink(m_spData->m_spSink);

		// Break the feeding loop
		m_spData->m_spSink->stop();

		// Flush all log records that may have left buffered
		m_spData->m_spSink->flush();

		m_spData->m_spSink.reset();
	}
}
