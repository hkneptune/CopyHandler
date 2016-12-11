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
#include "stdafx.h"
#include "TSQLiteSerializer.h"
#include "TSQLiteSerializerContainer.h"
#include "../libchcore/ErrorCodes.h"
#include "TSQLiteTransaction.h"
#include "../libchcore/TCoreException.h"
#include "../libchcore/TSimpleTimer.h"

using namespace string;
using namespace chcore;

namespace serializer
{
	using namespace sqlite;

	TSQLiteSerializer::TSQLiteSerializer(const TSmartPath& pathDB, const ISerializerSchemaPtr& spSchema, const logger::TLogFileDataPtr& spLogFileData) :
		m_spDatabase(new TSQLiteDatabase(pathDB)),
		m_spSchema(spSchema),
		m_spLog(logger::MakeLogger(spLogFileData, L"Serializer"))
	{
		if(!m_spDatabase)
			throw TCoreException(eErr_InvalidArgument, L"m_spDatabase", LOCATION);
		if(!m_spSchema)
			throw TCoreException(eErr_InvalidArgument, L"m_spSchema", LOCATION);

		// initialize db params
		SetupDBOptions();

		m_spSchema->Setup(m_spDatabase);
	}

	TSQLiteSerializer::~TSQLiteSerializer()
	{
		// clear the containers first, so that we can safely get rid of the strings pool
		m_mapContainers.clear();
		m_poolStrings.Clear(false);
	}

	ISerializerContainerPtr TSQLiteSerializer::GetContainer(const TString& strContainerName)
	{
		ContainerMap::iterator iterMap = m_mapContainers.find(strContainerName);
		if (iterMap == m_mapContainers.end())
			iterMap = m_mapContainers.insert(std::make_pair(
				strContainerName,
				std::make_shared<TSQLiteSerializerContainer>(strContainerName, m_spDatabase, m_poolStrings, m_spLog->GetLogFileData()))).first;

		return iterMap->second;
	}

	TSmartPath TSQLiteSerializer::GetLocation() const
	{
		return m_spDatabase->GetLocation();
	}

	void TSQLiteSerializer::Flush()
	{
		LOG_DEBUG(m_spLog) << L"Starting serializer flushing";

		TSQLiteTransaction tran(m_spDatabase);

		TSimpleTimer timer(true);

		for (ContainerMap::iterator iterContainer = m_mapContainers.begin(); iterContainer != m_mapContainers.end(); ++iterContainer)
		{
			iterContainer->second->Flush();
		}

		unsigned long long ullFlushGatherTime = timer.Checkpoint(); ullFlushGatherTime;

		tran.Commit();

		unsigned long long ullFlushCommitTime = timer.Checkpoint(); ullFlushCommitTime;
		LOG_DEBUG(m_spLog) << L"Container flush time: " << ullFlushGatherTime << L" ms, transaction commit: " << ullFlushCommitTime << L" ms";

		m_mapContainers.clear();
		m_poolStrings.Clear();

		unsigned long long ullFlushClearTime = timer.Checkpoint(); ullFlushClearTime;
		LOG_DEBUG(m_spLog) << L"Container clearing: " << ullFlushClearTime << L" ms";
	}

	void TSQLiteSerializer::SetupDBOptions()
	{
		/*
			TSQLiteStatement tStatement(m_spDatabase);
			tStatement.Prepare(_T("PRAGMA JOURNAL_MODE=WAL"));
			TSQLiteStatement::EStepResult eResult = tStatement.Step();
			if(eResult != TSQLiteStatement::eStep_HasRow)
				throw TCoreException(eErr_CannotSetDatabaseOptions, L"Failed to set database options", LOCATION);

			TString strResult = tStatement.GetText(0);
			if(strResult != _T("wal"))
				throw TCoreException(eErr_CannotSetDatabaseOptions, L"Failed to set database options", LOCATION);*/
	}
}
