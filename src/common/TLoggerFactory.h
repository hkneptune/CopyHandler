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
#ifndef __TLOGGERFACTORY_H__
#define __TLOGGERFACTORY_H__

#include "TMultiLoggerConfig.h"
#include "TLogger.h"
#include "TLoggerLocationConfig.h"
#include <memory>

namespace chcore
{
	class TLoggerFactory
	{
	public:
		TLoggerFactory(const TSmartPath& pathLog, const TMultiLoggerConfigPtr& spMultiLoggerConfig);

		std::unique_ptr<TLogger> CreateLogger(PCTSTR pszChannel);

	private:
		TMultiLoggerConfigPtr m_spMultiLoggerConfig;
		TLoggerLocationConfigPtr m_spLogLocation;
	};

	using TLoggerFactoryPtr = std::shared_ptr<TLoggerFactory>;
}

#endif
