// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#ifndef __TPATHCONTAINER_H__
#define __TPATHCONTAINER_H__

#include "TPath.h"
#include "../common/GenericTemplates/RandomAccessIterators.h"
#include "../common/GenericTemplates/RandomAccessContainerWrapper.h"

namespace chcore
{
	template class LIBCHCORE_API RandomAccessIteratorWrapper<TSmartPath>;
	class LIBCHCORE_API TPathArrayIterator : public RandomAccessIteratorWrapper<TSmartPath>
	{
	};

	template class LIBCHCORE_API RandomAccessConstIteratorWrapper<TSmartPath>;
	class LIBCHCORE_API TPathArrayConstIterator : public RandomAccessConstIteratorWrapper<TSmartPath>
	{
	};

	template class LIBCHCORE_API RandomAccessContainerWrapper<TSmartPath>;
	class LIBCHCORE_API TPathContainer : public RandomAccessContainerWrapper<TSmartPath>
	{
	};
}

#endif
