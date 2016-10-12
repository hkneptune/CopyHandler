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
#ifndef __TLOGGERROTATIONINFO_H__
#define __TLOGGERROTATIONINFO_H__

#include <boost/thread/shared_mutex.hpp>

namespace logger
{
	class TLoggerRotationInfo
	{
	public:
		static const unsigned long long DefaultMaxLogSize = 10 * 1024 * 1024;
		static const unsigned long DefaultMaxRotatedFiles = 5;

	public:
		TLoggerRotationInfo();
		TLoggerRotationInfo(unsigned long long ullMaxLogSize, unsigned long ulMaxRotatedCount);

		void SetRotationInfo(unsigned long long ullMaxLogSize, unsigned long ulMaxRotatedCount);
		void GetRotationInfo(unsigned long long& ullMaxLogSize, unsigned long& ulMaxRotatedCount) const;

		unsigned long long GetMaxLogSize() const;
		unsigned long GetMaxRotatedCount() const;

	private:
		unsigned long long m_ullMaxLogSize = 1024 * 1024;
		unsigned long m_ulMaxRotatedCount = 5;
		mutable boost::shared_mutex m_mutex;
	};

	using TLoggerRotationInfoPtr = std::shared_ptr<TLoggerRotationInfo>;
}

#endif
