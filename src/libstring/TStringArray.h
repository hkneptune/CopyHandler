// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TStringArray.h
/// @date  2011/06/05
/// @brief Contains string array definition.
// ============================================================================
#ifndef __TSTRINGARRAY_H__
#define __TSTRINGARRAY_H__

#include "TString.h"
#include "libstring.h"
#include <vector>
#include "../common/GenericTemplates/RandomAccessIterators.h"
#include "../common/GenericTemplates/RandomAccessContainerWrapper.h"

namespace string
{
	template class LIBSTRING_API RandomAccessIteratorWrapper<TString>;
	class LIBSTRING_API TStringArrayIterator : public RandomAccessIteratorWrapper<TString>
	{
	};

	template class LIBSTRING_API RandomAccessConstIteratorWrapper<TString>;
	class LIBSTRING_API TStringArrayConstIterator : public RandomAccessConstIteratorWrapper<TString>
	{
	};

	template class LIBSTRING_API RandomAccessContainerWrapper<TString>;
	class LIBSTRING_API TStringArray : public RandomAccessContainerWrapper<TString>
	{
	};
}

#endif
