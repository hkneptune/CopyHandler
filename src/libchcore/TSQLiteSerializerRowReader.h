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
#ifndef __TSQLITESERIALIZERROWREADER_H__
#define __TSQLITESERIALIZERROWREADER_H__

#include "ISerializerRowReader.h"
#include "TSQLiteStatement.h"
#include "TSQLiteColumnDefinition.h"

namespace chcore
{
	class LIBCHCORE_API TSQLiteSerializerRowReader : public ISerializerRowReader
	{
	private:
		TSQLiteSerializerRowReader(const TSQLiteSerializerRowReader&);
		TSQLiteSerializerRowReader& operator=(const TSQLiteSerializerRowReader&);

	public:
		TSQLiteSerializerRowReader(const sqlite::TSQLiteDatabasePtr& spDatabase, TSQLiteColumnsDefinition& rColumns, const TString& strContainerName);
		virtual ~TSQLiteSerializerRowReader();

		virtual bool Next();

		virtual void GetValue(const TString& strColName, bool& bValue);
		virtual void GetValue(const TString& strColName, short& iValue);
		virtual void GetValue(const TString& strColName, unsigned short& uiValue);
		virtual void GetValue(const TString& strColName, int& iValue);
		virtual void GetValue(const TString& strColName, unsigned int& uiValue);
		virtual void GetValue(const TString& strColName, long& lValue);
		virtual void GetValue(const TString& strColName, unsigned long& ulValue);
		virtual void GetValue(const TString& strColName, long long& llValue);
		virtual void GetValue(const TString& strColName, unsigned long long& ullValue);
		virtual void GetValue(const TString& strColName, double& dValue);
		virtual void GetValue(const TString& strColName, TString& strValue);
		virtual void GetValue(const TString& strColName, TSmartPath& pathValue);

	private:
		int GetColumnIndex(const TString& strColName) const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		bool m_bInitialized;
		sqlite::TSQLiteStatementPtr m_spStatement;
		TSQLiteColumnsDefinition& m_rColumns;
		TString m_strContainerName;
#pragma warning(pop)
	};

	typedef std::shared_ptr<TSQLiteSerializerRowReader> TSQLiteSerializerRowReaderPtr;
}

#endif
