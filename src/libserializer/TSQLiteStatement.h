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
#ifndef __TSQLITESTATEMENT_H__
#define __TSQLITESTATEMENT_H__

#include "TSQLiteDatabase.h"
#include "../libstring/TString.h"
#include "../libchcore/TPath.h"

struct sqlite3_stmt;

namespace serializer
{
	namespace sqlite
	{
		typedef std::shared_ptr<sqlite3_stmt> SQLiteStatementHandle;

		class TSQLiteStatement
		{
		public:
			enum EStepResult
			{
				eStep_Finished,
				eStep_HasRow
			};

		public:
			explicit TSQLiteStatement(const TSQLiteDatabasePtr& spDatabase);
			TSQLiteStatement(const TSQLiteStatement&) = delete;
			~TSQLiteStatement();

			TSQLiteStatement& operator=(const TSQLiteStatement&) = delete;

			void Close();

			void Prepare(PCWSTR pszQuery);

			void BindValue(int iColumn, bool bValue);
			void BindValue(int iColumn, short siValue);
			void BindValue(int iColumn, unsigned short usiValue);
			void BindValue(int iColumn, int iValue);
			void BindValue(int iColumn, unsigned int uiValue);
			void BindValue(int iColumn, long lValue);
			void BindValue(int iColumn, unsigned long ulValue);
			void BindValue(int iColumn, long long llValue);
			void BindValue(int iColumn, unsigned long long ullValue);
			void BindValue(int iColumn, double dValue);
			void BindValue(int iColumn, PCTSTR pszText);
			void BindValue(int iColumn, const string::TString& strText);
			void BindValue(int iColumn, const chcore::TSmartPath& path);

			void ClearBindings();

			EStepResult Step();
			int Changes();
			void Reset();

			bool GetBool(int iCol);
			short GetShort(int iCol);
			unsigned short GetUShort(int iCol);
			int GetInt(int iCol);
			unsigned int GetUInt(int iCol);
			long GetLong(int iCol);
			unsigned long GetULong(int iCol);
			long long GetInt64(int iCol);
			unsigned long long GetUInt64(int iCol);
			double GetDouble(int iCol);
			string::TString GetText(int iCol);
			chcore::TSmartPath GetPath(int iCol);

			void GetValue(int iCol, bool& bValue);
			void GetValue(int iCol, short& iValue);
			void GetValue(int iCol, unsigned short& uiValue);
			void GetValue(int iCol, int& iValue);
			void GetValue(int iCol, unsigned int& uiValue);
			void GetValue(int iCol, long& lValue);
			void GetValue(int iCol, unsigned long& ulValue);
			void GetValue(int iCol, long long& llValue);
			void GetValue(int iCol, unsigned long long& ullValue);
			void GetValue(int iCol, double& dValue);
			void GetValue(int iCol, string::TString& strValue);
			void GetValue(int iCol, chcore::TSmartPath& pathValue);

		private:
			sqlite3_stmt* m_pStatement;
			TSQLiteDatabasePtr m_spDatabase;
			bool m_bHasRow;
		};

		typedef std::shared_ptr<TSQLiteStatement> TSQLiteStatementPtr;
	}
}

#endif
