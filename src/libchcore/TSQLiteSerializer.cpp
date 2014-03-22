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

BEGIN_CHCORE_NAMESPACE

using namespace sqlite;

TSQLiteSerializer::TSQLiteSerializer(const TSmartPath& pathDB, const ISerializerSchemaPtr& spSchema) :
	m_spDatabase(new TSQLiteDatabase(pathDB)),
	m_spSchema(spSchema)
{
	if(!m_spDatabase || !m_spSchema)
		THROW_CORE_EXCEPTION(eErr_InvalidArgument);

	m_spSchema->Setup(m_spDatabase);
}

ISerializerContainerPtr TSQLiteSerializer::GetContainer(const TString& strContainerName)
{
	std::map<TString, ISerializerContainerPtr>::iterator iterMap = m_mapContainers.find(strContainerName);
	if(iterMap == m_mapContainers.end())
		iterMap = m_mapContainers.insert(std::make_pair(
		strContainerName,
		TSQLiteSerializerContainerPtr(new TSQLiteSerializerContainer(strContainerName, m_spDatabase)))).first;

	return iterMap->second;
}

chcore::TSmartPath TSQLiteSerializer::GetLocation() const
{
	return m_spDatabase->GetLocation();
}

void TSQLiteSerializer::Flush()
{
	// TODO: generate the necessary queries and execute them
}

END_CHCORE_NAMESPACE