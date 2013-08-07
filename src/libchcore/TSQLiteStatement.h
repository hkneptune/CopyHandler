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

#include "libchcore.h"
#include "TSQLiteDatabase.h"
#include "TString.h"

struct sqlite3_stmt;

BEGIN_CHCORE_NAMESPACE

namespace sqlite
{
	typedef boost::shared_ptr<sqlite3_stmt> SQLiteStatementHandle;

	class TSQLiteStatement
	{
	public:
		enum EStepResult
		{
			eStep_Finished,
			eStep_HasRow
		};

	public:
		TSQLiteStatement(const TSQLiteDatabasePtr& spDatabase);
		~TSQLiteStatement();

		void Close();

		void Prepare(PCTSTR pszQuery);

		void BindValue(int iColumn, double dValue);
		void BindValue(int iColumn, int iValue);
		void BindValue(int iColumn, unsigned int uiValue);
		void BindValue(int iColumn, long long llValue);
		void BindValue(int iColumn, unsigned long long ullValue);
		void BindValue(int iColumn, PCTSTR pszText);

		void ClearBindings();

		EStepResult Step();
		void Reset();

		double GetDouble(int iCol);
		int GetInt(int iCol);
		unsigned int GetUInt(int iCol);
		long long GetInt64(int iCol);
		unsigned long long GetUInt64(int iCol);
		TString GetText(int iCol);

	private:
		sqlite3_stmt* m_pStatement;
		TSQLiteDatabasePtr m_spDatabase;
		bool m_bHasRow;
	};

	typedef boost::shared_ptr<TSQLiteStatement> TSQLiteStatementPtr;
}

END_CHCORE_NAMESPACE

#endif
