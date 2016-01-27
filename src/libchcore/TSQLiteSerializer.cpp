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
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TSQLiteTransaction.h"
#include "TSQLiteStatement.h"
#include "TSimpleTimer.h"
#include "SerializerTrace.h"

namespace chcore
{
	using namespace sqlite;

	TSQLiteSerializer::TSQLiteSerializer(const TSmartPath& pathDB, const ISerializerSchemaPtr& spSchema) :
		m_spDatabase(new TSQLiteDatabase(pathDB)),
		m_spSchema(spSchema)
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
				TSQLiteSerializerContainerPtr(new TSQLiteSerializerContainer(strContainerName, m_spDatabase, m_poolStrings)))).first;

		return iterMap->second;
	}

	TSmartPath TSQLiteSerializer::GetLocation() const
	{
		return m_spDatabase->GetLocation();
	}

	void TSQLiteSerializer::Flush()
	{
		DBTRACE0(_T("   ## Serializer::Flush() - started\n"));

		TSQLiteTransaction tran(m_spDatabase);

		TSimpleTimer timer(true);

		for (ContainerMap::iterator iterContainer = m_mapContainers.begin(); iterContainer != m_mapContainers.end(); ++iterContainer)
		{
			iterContainer->second->Flush();
		}

		unsigned long long ullFlushGatherTime = timer.Checkpoint(); ullFlushGatherTime;

		tran.Commit();

		unsigned long long ullFlushCommitTime = timer.Checkpoint(); ullFlushCommitTime;
		DBTRACE2(_T("   ## Serializer::Flush() - container flushes: %I64u ms, transaction commit: %I64u ms\n"), ullFlushGatherTime, ullFlushCommitTime);

		m_mapContainers.clear();
		m_poolStrings.Clear();

		unsigned long long ullFlushClearTime = timer.Checkpoint(); ullFlushClearTime;
		DBTRACE1(_T("   ## Serializer::Flush() - container clearing: %I64u ms\n"), ullFlushClearTime);
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
