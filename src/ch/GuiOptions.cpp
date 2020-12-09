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
#include "stdafx.h"
#include "GuiOptions.h"

using namespace chengine;

bool GuiOptions::IsPauseAvailable(chengine::ETaskCurrentState eState)	//ok
{
	switch(eState)
	{
	case eTaskState_None:
	case eTaskState_Max:
	case eTaskState_LoadError:
	case eTaskState_Finished:
	case eTaskState_Cancelled:
	case eTaskState_Paused:
		return false;

	default:
		return true;
	}
}

bool GuiOptions::IsResumeAvailable(chengine::ETaskCurrentState eState)	//ok
{
	switch(eState)
	{
	case eTaskState_None:
	case eTaskState_Max:
	case eTaskState_LoadError:
	case eTaskState_Finished:
	case eTaskState_Cancelled:
	case eTaskState_Processing:
	case eTaskState_Error:
		return false;

	case eTaskState_Paused:
	case eTaskState_Waiting:
	default:
		return true;
	}
}

bool GuiOptions::IsRestartAvailable(chengine::ETaskCurrentState eState)	//ok
{
	switch(eState)
	{
	case eTaskState_None:
	case eTaskState_Max:
	case eTaskState_LoadError:
		return false;

	default:
		return true;
	}
}

bool GuiOptions::IsCancelAvailable(chengine::ETaskCurrentState eState)	//ok
{
	switch(eState)
	{
	case eTaskState_None:
	case eTaskState_Max:
	case eTaskState_LoadError:
	case eTaskState_Finished:
	case eTaskState_Cancelled:
		return false;

	default:
		return true;
	}
}

bool GuiOptions::IsDeleteAvailable(chengine::ETaskCurrentState eState)	//ok
{
	switch(eState)
	{
	case eTaskState_None:
	case eTaskState_Max:
	case eTaskState_LoadError:
		return false;

	default:
		return true;
	}
}

bool GuiOptions::IsShowLogAvailable(chengine::ETaskCurrentState eState)	//ok
{
	switch(eState)
	{
	case eTaskState_None:
	case eTaskState_Max:
		return false;

	default:
		return true;
	}
}

bool GuiOptions::IsResetUserFeedbackAvailable(chengine::ETaskCurrentState eState)
{
	switch(eState)
	{
	case eTaskState_None:
	case eTaskState_Max:
	case eTaskState_LoadError:
		return false;

	default:
		return true;
	}
}
