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
#include "../libchcore/ErrorCodes.h"
#include "../libchcore/TCoreWin32Exception.h"

using namespace string;
using namespace chcore;

namespace serializer
{
	TSQLiteSerializerFactory::TSQLiteSerializerFactory(const TSmartPath& pathSerializeDir, logger::TLogFileDataPtr& spLogFileData) :
		m_pathSerializeDir(pathSerializeDir),
		m_spLog(logger::MakeLogger(spLogFileData, L"Serializer"))
	{
	}

	TSQLiteSerializerFactory::~TSQLiteSerializerFactory()
	{
	}

	ISerializerPtr TSQLiteSerializerFactory::CreateTaskManagerSerializer(bool bForceRecreate)
	{
		TSmartPath pathTaskManager = m_pathSerializeDir + PathFromString(_T("tasks.sqlite"));

		if (bForceRecreate)
		{
			if (!DeleteFile(pathTaskManager.ToString()))
			{
				DWORD dwLastError = GetLastError();
				if (dwLastError != ERROR_FILE_NOT_FOUND)
					throw TCoreWin32Exception(eErr_CannotDeleteFile, dwLastError, L"Cannot delete tasks.sqlite file", LOCATION);
			}
		}

		TSQLiteSerializerPtr spSerializer(std::make_shared<TSQLiteSerializer>(
			pathTaskManager,
			std::make_shared<TSQLiteTaskManagerSchema>(),
			m_spLog->GetLogFileData()));

		return spSerializer;
	}

	ISerializerPtr TSQLiteSerializerFactory::CreateTaskSerializer(const logger::TLogFileDataPtr& spLogFileData, const TString& strNameHint, bool bForceRecreate)
	{
		TString strName(strNameHint);
		if (strName.IsEmpty())
		{
			boost::uuids::random_generator gen;
			boost::uuids::uuid u = gen();
			strName = boost::lexical_cast<std::wstring>(u).c_str();
		}

		TSmartPath pathTask = PathFromWString(strName);
		if (!pathTask.HasFileRoot())
		{
			if (!strName.EndsWithNoCase(_T(".sqlite")))
				strName += _T(".sqlite");

			pathTask = m_pathSerializeDir;
			pathTask += PathFromWString(strName);
		}

		if (bForceRecreate)
		{
			if (!DeleteFile(pathTask.ToString()))
			{
				DWORD dwLastError = GetLastError();
				if (dwLastError != ERROR_FILE_NOT_FOUND)
					throw TCoreWin32Exception(eErr_CannotDeleteFile, dwLastError, L"Cannot delete task database file", LOCATION);
			}
		}

		TSQLiteSerializerPtr spSerializer(std::make_shared<TSQLiteSerializer>(pathTask, std::make_shared<TSQLiteTaskSchema>(), spLogFileData));

		return spSerializer;
	}
}
