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
#ifndef __TTASKMANAGERSERIALIZER_H__
#define __TTASKMANAGERSERIALIZER_H__

#include "libchcore.h"
#include "ITaskManagerSerializer.h"
#include "TSQLiteDatabase.h"
#include "TPath.h"

BEGIN_CHCORE_NAMESPACE

class TTaskInfoContainer;

class LIBCHCORE_API TTaskManagerSerializer : public ITaskManagerSerializer
{
public:
	TTaskManagerSerializer(const TSmartPath& pathDB, const TSmartPath& pathTasksDir);
	~TTaskManagerSerializer();

	virtual ITaskSerializerPtr CreateTaskSerializer(const TSmartPath& pathSerialize);

	virtual void Setup();		// creates or migrates tables

	void Store(const TTaskInfoContainer& tTasksInfo);
	void Load(TTaskInfoContainer& tTasksInfo);

protected:
	sqlite::TSQLiteDatabasePtr GetDatabase();

private:
	TSmartPath m_pathDB;
	TSmartPath m_pathTasksDir;

#pragma warning(push)
#pragma warning(disable: 4251)
	sqlite::TSQLiteDatabasePtr m_spDatabase;
#pragma warning(pop)

	bool m_bSetupExecuted;
};

typedef boost::shared_ptr<TTaskManagerSerializer> TTaskManagerSerializerPtr;

END_CHCORE_NAMESPACE

#endif
