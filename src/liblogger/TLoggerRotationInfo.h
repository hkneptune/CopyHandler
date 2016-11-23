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
#include "liblogger.h"

namespace logger
{
	class LIBLOGGER_API TLoggerRotationInfo
	{
	public:
		static const unsigned int DefaultMaxLogSize = 10 * 1024 * 1024;
		static const unsigned int DefaultMaxRotatedFiles = 5;

	public:
		TLoggerRotationInfo();
		TLoggerRotationInfo(unsigned int uiMaxLogSize, unsigned int uiMaxRotatedCount);

		void SetMaxLogSize(unsigned int uiMaxLogSize);
		void SetRotatedCount(unsigned int uiMaxRotatedCount);

		unsigned int GetMaxLogSize() const;
		unsigned int GetMaxRotatedCount() const;

	private:
		volatile mutable unsigned int m_uiMaxLogSize = DefaultMaxLogSize;
		volatile mutable unsigned int m_uiMaxRotatedCount = DefaultMaxRotatedFiles;
	};

	using TLoggerRotationInfoPtr = std::shared_ptr<TLoggerRotationInfo>;
}

#endif
