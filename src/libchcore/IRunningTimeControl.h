// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#ifndef __IRUNNINGTIMESTATS_H__
#define __IRUNNINGTIMESTATS_H__

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

class IRunningTimeControl
{
public:
	virtual ~IRunningTimeControl();

	// time tracking
	virtual void EnableTimeTracking() = 0;
	virtual void DisableTimeTracking() = 0;

	// running/not running state
	virtual void MarkAsRunning() = 0;
	virtual void MarkAsNotRunning() = 0;
};

END_CHCORE_NAMESPACE

#endif