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
#include "TSQLiteDatabase.h"
#include "TSQLiteStatement.h"
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include <boost/variant/variant.hpp>

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TSQLiteSerializerRowData : public ISerializerRowData
{
private:
	static const unsigned long long AddedBit = 1;

private:
	TSQLiteSerializerRowData(size_t stRowID, TSQLiteColumnsDefinition& rColumnDefinition, bool bAdded, unsigned long long* pPoolMemory, size_t stPoolMemorySizeInBytes);

public:
	TSQLiteSerializerRowData(const TSQLiteSerializerRowData& rSrc);
	virtual ~TSQLiteSerializerRowData();

	TSQLiteSerializerRowData& operator=(const TSQLiteSerializerRowData& rSrc);

	virtual ISerializerRowData& SetValue(size_t stColIndex, bool bValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, short iValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, unsigned short uiValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, int iValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, unsigned int uiValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, long lValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, unsigned long ulValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, long long llValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, unsigned long long llValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, double dValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, const TString& strValue);
	virtual ISerializerRowData& SetValue(size_t stColIndex, const TSmartPath& pathValue);

	virtual ISerializerRowData& SetValue(const TString& strColumnName, bool bValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, short iValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, unsigned short uiValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, int iValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, unsigned int uiValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, long lValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, unsigned long ulValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, long long llValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, unsigned long long llValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, double dValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, const TString& strValue);
	virtual ISerializerRowData& SetValue(const TString& strColumnName, const TSmartPath& pathValue);

	TString GetQuery(const TString& strContainerName) const;
	unsigned long long GetChangeIdentification() const;

	void BindParamsAndExec(sqlite::TSQLiteStatement& tStatement);

private:
	const unsigned long long& GetDataForColumn(size_t stColIndex) const;
	unsigned long long GetDataHeader() const;
	unsigned long long& ModifyColumnData(size_t stColIndex);
	void FreeColumnData(size_t stColumnID);
	void FreeAllColumnData();

	void MarkAsAdded();
	bool IsAdded() const;

	bool HasAnyData() const;
	void MarkColumnUsage(size_t stIndex, bool bUsed);
	bool HasData(size_t stColumnIndex) const;

	void BindParams(sqlite::TSQLiteStatement &tStatement, int& iSQLiteColumnNumber, size_t bSkipColumn = (size_t)-1);

private:
	unsigned long long* m_pPoolMemory;

	TSQLiteColumnsDefinition& m_rColumns;

	friend class TSQLiteSerializerContainer;
};

typedef boost::shared_ptr<TSQLiteSerializerRowData> TSQLiteSerializerRowDataPtr;

END_CHCORE_NAMESPACE

#endif
