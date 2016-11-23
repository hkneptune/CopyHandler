// ============================================================================
//  Copyright (C) 2001-2012 by Jozef Starosczyk
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
#ifndef __ETASKCURRENTSTATE_H__
#define __ETASKCURRENTSTATE_H__

namespace chcore
{
	// enum representing current processing state of the task
	enum ETaskCurrentState
	{
		eTaskState_None,
		eTaskState_Waiting,
		eTaskState_Processing,
		eTaskState_Paused,
		eTaskState_Cancelled,
		eTaskState_Error,
		eTaskState_Finished,
		eTaskState_LoadError,

		// insert new values before this one
		eTaskState_Max
	};
}

#endif
