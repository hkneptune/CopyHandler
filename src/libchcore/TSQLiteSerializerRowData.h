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
#ifndef __TSQLITESERIALIZERROWWRITER_H__
#define __TSQLITESERIALIZERROWWRITER_H__

#include "libchcore.h"
#include "ISerializerRowData.h"
#include "TSQLiteColumnDefinition.h"
#include "ISerializerContainer.h"
#include "TRowData.h"
#include "TSQLiteDatabase.h"
#include "TSQLiteStatement.h"
#include <boost/dynamic_bitset.hpp>

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TRowID
{
public:
	TRowID(const TSQLiteColumnDefinitionPtr& spColumnDefinition);
	~TRowID();

	void Clear();

	void SetAddedBit(bool bAdded);
	void SetColumnBit(size_t stIndex, bool bColumnExists);

	bool HasAny() const;

	bool operator==(const TRowID rSrc) const;
	bool operator<(const TRowID rSrc) const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	boost::dynamic_bitset<> m_bitset;
#pragma warning(pop)
};

class LIBCHCORE_API TSQLiteSerializerRowData : public ISerializerRowData
{
public:
	TSQLiteSerializerRowData(size_t stRowID, const TSQLiteColumnDefinitionPtr& spColumnDefinition, bool bAdded);
	virtual ~TSQLiteSerializerRowData();

	virtual ISerializerRowData& operator%(const TRowData& rData);
	virtual ISerializerRowData& SetValue(const TRowData& rData);

	TString GetQuery(const TString& strContainerName) const;
	TRowID GetChangeIdentification() const;

	void BindParamsAndExec(sqlite::TSQLiteStatement& tStatement);

private:
	size_t m_stRowID;
	bool m_bAdded;
#pragma warning(push)
#pragma warning(disable: 4251)
	TSQLiteColumnDefinitionPtr m_spColumns;

	typedef std::map<size_t, TRowData::InternalVariant> MapVariants;	// column id -> variant data
	MapVariants m_mapValues;
#pragma warning(pop)
};

typedef boost::shared_ptr<TSQLiteSerializerRowData> TSQLiteSerializerRowDataPtr;

END_CHCORE_NAMESPACE

#endif
