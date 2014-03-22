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
#ifndef __ICOLUMNSDEFINITION_H__
#define __ICOLUMNSDEFINITION_H__

#include "libchcore.h"
#include "TString.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API IColumnsDefinition
{
public:
	virtual ~IColumnsDefinition();

	virtual size_t AddColumn(const TString& strColumnName) = 0;
	virtual void Clear() = 0;

	virtual size_t GetColumnIndex(const TString& strColumnName, bool bAdd = true) = 0;
	virtual TString GetColumnName(size_t stIndex) const = 0;
	virtual size_t GetCount() const = 0;
	virtual bool IsEmpty() const = 0;

	virtual IColumnsDefinition& operator%(const TString& strColName) = 0;
};

typedef boost::shared_ptr<IColumnsDefinition> IColumnsDefinitionPtr;

END_CHCORE_NAMESPACE

#endif
