// ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
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
#pragma once

#include "../libchengine/ETaskCurrentState.h"

class GuiOptions
{
public:
	static bool IsPauseAvailable(chengine::ETaskCurrentState eState);
	static bool IsResumeAvailable(chengine::ETaskCurrentState eState);
	static bool IsRestartAvailable(chengine::ETaskCurrentState eState);
	static bool IsCancelAvailable(chengine::ETaskCurrentState eState);
	static bool IsDeleteAvailable(chengine::ETaskCurrentState eState);

	static bool IsShowLogAvailable(chengine::ETaskCurrentState eState);
	static bool IsResetUserFeedbackAvailable(chengine::ETaskCurrentState eState);
};