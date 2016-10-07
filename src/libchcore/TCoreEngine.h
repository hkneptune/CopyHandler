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
#ifndef __TCOREENGINE_H__
#define __TCOREENGINE_H__

#include "libchcore.h"
#include "../liblogger/TLoggerInitializer.h"

namespace chcore
{
	class LIBCHCORE_API TCoreEngine
	{
	public:
		TCoreEngine();
		~TCoreEngine();

		void Init(const TSmartPath& pathLogs, unsigned int uiMaxRotatedFiles, unsigned long long ullMaxLogSize);
		void Uninitialize();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		TLoggerInitializer m_loggerInitializer;
#pragma warning(pop)
	};
}

#endif