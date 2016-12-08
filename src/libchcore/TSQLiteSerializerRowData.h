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

#include "ISerializerRowData.h"
#include "TSQLiteColumnDefinition.h"
#include "ISerializerContainer.h"
#include "TSQLiteDatabase.h"
#include "TSQLiteStatement.h"
#include <boost/dynamic_bitset.hpp>
#include "../liblogger/TLogger.h"

namespace chcore
{
	class TPlainStringPool;

	class LIBCHCORE_API TSQLiteSerializerRowData : public ISerializerRowData
	{
	private:
		static const unsigned long long AddedBit = 1;

	public:
		TSQLiteSerializerRowData(object_id_t oidRowID, TSQLiteColumnsDefinition& rColumnDefinition, bool bAdded, unsigned long long* pPoolMemory, size_t stPoolMemorySizeInBytes, TPlainStringPool& poolStrings, const logger::TLogFileDataPtr& spLogFileData);
		TSQLiteSerializerRowData(const TSQLiteSerializerRowData& rSrc) = delete;
		virtual ~TSQLiteSerializerRowData();

		TSQLiteSerializerRowData& operator=(const TSQLiteSerializerRowData& rSrc) = delete;

		ISerializerRowData& SetValue(size_t stColIndex, bool bValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, short iValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, unsigned short uiValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, int iValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, unsigned int uiValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, long lValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, unsigned long ulValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, long long llValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, unsigned long long llValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, double dValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, const TString& strValue) override;
		ISerializerRowData& SetValue(size_t stColIndex, const TSmartPath& pathValue) override;

		ISerializerRowData& SetValue(const wchar_t* strColumnName, bool bValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, short iValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, unsigned short uiValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, int iValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, unsigned int uiValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, long lValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, unsigned long ulValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, long long llValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, unsigned long long llValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, double dValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, const TString& strValue) override;
		ISerializerRowData& SetValue(const wchar_t* strColumnName, const TSmartPath& pathValue) override;

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
		ISerializerRowData& InternalSetValue(size_t stColIndex, unsigned long ulValue);

	private:
		unsigned long long* m_pPoolMemory;

		TSQLiteColumnsDefinition& m_rColumns;
		TPlainStringPool& m_poolStrings;
#pragma warning(push)
#pragma warning(disable: 4251)
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)

		friend class TSQLiteSerializerContainer;
	};

	typedef std::shared_ptr<TSQLiteSerializerRowData> TSQLiteSerializerRowDataPtr;
}

#endif
