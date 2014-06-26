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
#ifndef __TSQLITESERIALIZERCONTAINER_H__
#define __TSQLITESERIALIZERCONTAINER_H__

#include "libchcore.h"
#include "ISerializerRowReader.h"
#include "ISerializerContainer.h"
#include <map>
#include <boost/optional.hpp>
#include <boost/pool/poolfwd.hpp>
#include "TSQLiteColumnDefinition.h"
#include "TSQLiteDatabase.h"
#include "TSQLiteSerializerRowData.h"
#include <boost/container/flat_map.hpp>

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TSQLiteSerializerContainer : public ISerializerContainer
{
private:
	TSQLiteSerializerContainer(const TSQLiteSerializerContainer&);
	TSQLiteSerializerContainer& operator=(const TSQLiteSerializerContainer&);

public:
	TSQLiteSerializerContainer(const TString& strName, const sqlite::TSQLiteDatabasePtr& spDB, TPlainStringPool& poolStrings);
	virtual ~TSQLiteSerializerContainer();

	virtual IColumnsDefinition& GetColumnsDefinition();

	virtual ISerializerRowData& GetRow(size_t stRowID, bool bMarkAsAdded);
	virtual void DeleteRow(size_t stRowID);
	virtual void DeleteRows(const TRemovedObjects& setObjects);

	virtual ISerializerRowReaderPtr GetRowReader();

	void Flush();

private:
	void FlushDeletions();
	boost::pool<>& GetPool();
	size_t CalculateRowMemorySize() const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	TSQLiteColumnsDefinition m_tColumns;

	boost::pool<>* m_pPoolRows;

	typedef boost::container::flat_map<size_t, TSQLiteSerializerRowData> RowMap;	// maps row id to row data
	RowMap m_mapRows;

	std::set<size_t> m_setDeleteItems;

	TString m_strName;
	sqlite::TSQLiteDatabasePtr m_spDB;

	TPlainStringPool& m_poolStrings;
#pragma warning(pop)
};

typedef boost::shared_ptr<TSQLiteSerializerContainer> TSQLiteSerializerContainerPtr;

END_CHCORE_NAMESPACE

#endif
