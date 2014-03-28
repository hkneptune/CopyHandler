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
#include "stdafx.h"
#include "TSQLiteSerializerFactory.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/lexical_cast.hpp>
#include "TSQLiteTaskSchema.h"
#include "TSQLiteSerializer.h"
#include "TSQLiteTaskManagerSchema.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

TSQLiteSerializerFactory::TSQLiteSerializerFactory(const TSmartPath& pathSerializeDir) :
	m_pathSerializeDir(pathSerializeDir)
{
}

TSQLiteSerializerFactory::~TSQLiteSerializerFactory()
{
}

ISerializerPtr TSQLiteSerializerFactory::CreateSerializer(EObjectType eObjType, const TString& strNameHint, bool bForceRecreate)
{
	switch(eObjType)
	{
	case ISerializerFactory::eObj_Task:
		{
			TString strName(strNameHint);
			if(strName.IsEmpty())
			{
				boost::uuids::random_generator gen;
				boost::uuids::uuid u = gen();
				strName = boost::lexical_cast<std::wstring>(u).c_str();
			}

			TSmartPath pathTask = PathFromString(strName);
			if(!pathTask.HasFileRoot())
			{
				if(!strName.EndsWithNoCase(_T(".sqlite")))
					strName += _T(".sqlite");

				pathTask = m_pathSerializeDir;
				pathTask += PathFromString(strName);
			}

			if(bForceRecreate)
			{
				if(!DeleteFile(pathTask.ToString()))
				{
					DWORD dwLastError = GetLastError();
					if(dwLastError != ERROR_FILE_NOT_FOUND)
						THROW_CORE_EXCEPTION_WIN32(eErr_CannotDeleteFile, dwLastError);
				}
			}

			TSQLiteSerializerPtr spSerializer(new TSQLiteSerializer(
				pathTask,
				TSQLiteTaskSchemaPtr(new TSQLiteTaskSchema)));

			return spSerializer;
		}

	case ISerializerFactory::eObj_TaskManager:
		{
			TSmartPath pathTaskManager = m_pathSerializeDir + PathFromString(_T("tasks.sqlite"));

			if(bForceRecreate)
			{
				if(!DeleteFile(pathTaskManager.ToString()))
				{
					DWORD dwLastError = GetLastError();
					if(dwLastError != ERROR_FILE_NOT_FOUND)
						THROW_CORE_EXCEPTION_WIN32(eErr_CannotDeleteFile, dwLastError);
				}
			}

			TSQLiteSerializerPtr spSerializer(new TSQLiteSerializer(
				pathTaskManager,
				TTaskManagerSchemaPtr(new TSQLiteTaskManagerSchema)));

			return spSerializer;
		}

	default:
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);
	}
}

END_CHCORE_NAMESPACE
