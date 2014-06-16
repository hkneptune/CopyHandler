// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#ifndef __ITIMESTAMPPROVIDER_H__
#define __ITIMESTAMPPROVIDER_H__

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API ITimestampProvider
{
public:
	virtual ~ITimestampProvider() {}
	virtual unsigned long long GetCurrentTimestamp() const = 0;
};

typedef boost::shared_ptr<ITimestampProvider> ITimestampProviderPtr;

END_CHCORE_NAMESPACE

#endif
