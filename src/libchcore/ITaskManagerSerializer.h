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
#ifndef __ITASKMANAGERSERIALIZER_H__
#define __ITASKMANAGERSERIALIZER_H__

#include "libchcore.h"
#include "ITaskSerializer.h"
#include "TTaskInfo.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API ITaskManagerSerializer
{
public:
	virtual ~ITaskManagerSerializer() {}

	virtual void Store(const TTaskInfoContainer& tTasksInfo) = 0;
	virtual void Load(TTaskInfoContainer& tTasksInfo) = 0;

	virtual ITaskSerializerPtr CreateExistingTaskSerializer(const TSmartPath& pathSerialize) = 0;
	virtual ITaskSerializerPtr CreateNewTaskSerializer(const TString& strTaskUuid) = 0;
	virtual void RemoveTaskSerializer(const ITaskSerializerPtr& spTaskSerializer) = 0;
};

typedef boost::shared_ptr<ITaskManagerSerializer> ITaskManagerSerializerPtr;

END_CHCORE_NAMESPACE

#endif
