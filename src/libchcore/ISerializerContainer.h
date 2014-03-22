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
#ifndef __ISERIALIZERCONTAINER_H__
#define __ISERIALIZERCONTAINER_H__

#include "libchcore.h"
#include "IColumnsDefinition.h"
#include "ISerializerRowReader.h"

BEGIN_CHCORE_NAMESPACE

class ISerializerRowWriter;
typedef boost::shared_ptr<ISerializerRowWriter> ISerializerRowWriterPtr;

class LIBCHCORE_API ISerializerContainer
{
public:
	virtual ~ISerializerContainer();

	// columns
	virtual IColumnsDefinitionPtr GetColumnsDefinition() const = 0;

	// prepare data to be stored
	virtual ISerializerRowWriterPtr AddRow(size_t stRowID) = 0;
	virtual ISerializerRowWriterPtr GetRow(size_t stRowID) = 0;
	virtual void DeleteRow(size_t stRowID) = 0;

	// getting data from the serialized archive
	virtual ISerializerRowReaderPtr GetRowReader() = 0;
};

typedef boost::shared_ptr<ISerializerContainer> ISerializerContainerPtr;

END_CHCORE_NAMESPACE

#endif