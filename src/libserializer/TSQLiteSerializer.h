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
#ifndef __TSQLITESERIALIZER_H__
#define __TSQLITESERIALIZER_H__

#include <map>
#include "ISerializer.h"
#include "TSQLiteDatabase.h"
#include "../libstring/TString.h"
#include "ISerializerContainer.h"
#include "../libchcore/TPath.h"
#include "TSQLiteSerializerContainer.h"
#include "ISQLiteSerializerSchema.h"
#include "TPlainStringPool.h"
#include "../liblogger/TLogFileData.h"
#include "../liblogger/TLogger.h"

namespace serializer
{
	class LIBSERIALIZER_API TSQLiteSerializer : public ISerializer
	{
	public:
		TSQLiteSerializer(const chcore::TSmartPath& pathDB, const ISerializerSchemaPtr& spSchema, const logger::TLogFileDataPtr& spLogFileData);
		virtual ~TSQLiteSerializer();

		chcore::TSmartPath GetLocation() const override;

		ISerializerContainerPtr GetContainer(const string::TString& strContainerName) override;
		void Flush() override;
		void SetupDBOptions();

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		sqlite::TSQLiteDatabasePtr m_spDatabase;
		ISerializerSchemaPtr m_spSchema;

		typedef std::map<string::TString, TSQLiteSerializerContainerPtr> ContainerMap;
		ContainerMap m_mapContainers;

		TPlainStringPool m_poolStrings;
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)
	};

	typedef std::shared_ptr<TSQLiteSerializer> TSQLiteSerializerPtr;
}

#endif
