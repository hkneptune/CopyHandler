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

namespace chcore
{
	class TMultiFileBackend : public boost::log::sinks::basic_formatted_sink_backend< char >
	{
	public:
		TMultiFileBackend();

		void consume(boost::log::record_view const& rec, string_type const& formatted_message);

	private:
		unsigned long long m_ullRotateSize;
		unsigned int m_uiMaxRotatedFiles;
		unsigned int m_uiHandleCacheTime;

		// map<logname, <handle, last-write-time, list<rotated-files>>>
	};
}

#endif
