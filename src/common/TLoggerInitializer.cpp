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
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/locale.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sinks/text_multifile_backend.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;

using namespace boost::log::trivial;

using LogSink = sinks::asynchronous_sink< sinks::text_multifile_backend >;
using LogSinkPtr = boost::shared_ptr<LogSink>;

struct TLoggerInitializer::InternalData
{
	LogSinkPtr m_spSink;
};

TLoggerInitializer::TLoggerInitializer() :
	m_spData(new InternalData)
{
	InitSink();
}

TLoggerInitializer::~TLoggerInitializer()
{
	Uninit();
}

void TLoggerInitializer::InitSink()
{
	if(m_bWasInitialized)
		return;

	boost::shared_ptr< logging::core > core = logging::core::get();

	logging::add_common_attributes();

	// sink BACKEND
	boost::shared_ptr< sinks::text_multifile_backend > backend = boost::make_shared< sinks::text_multifile_backend >();

	// Set up the file naming pattern
	backend->set_file_name_composer
	(
		sinks::file::as_file_name_composer(expr::stream << expr::attr< std::wstring >("LogPath"))
	);

	// Sink FRONTEND
	LogSinkPtr sink(new LogSink(backend));

	// Set the formatter
	sink->set_formatter
	(
		expr::stream
		<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S.%f")
		<< " [" << boost::log::trivial::severity << "] "
		<< expr::attr< std::wstring >("Channel") << ": "
		<< expr::wmessage
	);

	std::locale loc = boost::locale::generator()("en_EN.UTF-8");
	sink->imbue(loc);

	core->add_sink(sink);

	m_spData->m_spSink = sink;

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
