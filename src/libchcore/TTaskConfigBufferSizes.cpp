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
#include "stdafx.h"
#include "TTaskConfigBufferSizes.h"

BEGIN_CHCORE_NAMESPACE

TBufferSizes GetTaskPropBufferSizes(const TConfig& rConfig)
{
	return TBufferSizes(GetTaskPropValue<eTO_UseOnlyDefaultBuffer>(rConfig),
		GetTaskPropValue<eTO_BufferQueueDepth>(rConfig),
		GetTaskPropValue<eTO_DefaultBufferSize>(rConfig),
		GetTaskPropValue<eTO_OneDiskBufferSize>(rConfig),
		GetTaskPropValue<eTO_TwoDisksBufferSize>(rConfig),
		GetTaskPropValue<eTO_CDBufferSize>(rConfig),
		GetTaskPropValue<eTO_LANBufferSize>(rConfig));
}

void SetTaskPropBufferSizes(TConfig& rConfig, const TBufferSizes& rBufferSizes)
{
	SetTaskPropValue<eTO_UseOnlyDefaultBuffer>(rConfig, rBufferSizes.IsOnlyDefault());
	SetTaskPropValue<eTO_BufferQueueDepth>(rConfig, rBufferSizes.GetBufferCount());
	SetTaskPropValue<eTO_DefaultBufferSize>(rConfig, rBufferSizes.GetDefaultSize());
	SetTaskPropValue<eTO_OneDiskBufferSize>(rConfig, rBufferSizes.GetOneDiskSize());
	SetTaskPropValue<eTO_TwoDisksBufferSize>(rConfig, rBufferSizes.GetTwoDisksSize());
	SetTaskPropValue<eTO_CDBufferSize>(rConfig, rBufferSizes.GetCDSize());
	SetTaskPropValue<eTO_LANBufferSize>(rConfig, rBufferSizes.GetLANSize());
}

END_CHCORE_NAMESPACE
