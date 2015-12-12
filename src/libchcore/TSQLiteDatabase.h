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
#ifndef __TSQLITEDATABASE_H__
#define __TSQLITEDATABASE_H__

#include "libchcore.h"
#include "TPath.h"

struct sqlite3;

namespace chcore
{
	namespace sqlite
	{
		class TSQLiteDatabase
		{
		public:
			explicit TSQLiteDatabase(const TSmartPath& strFilename);
			~TSQLiteDatabase();

			HANDLE GetHandle();

			TSmartPath GetLocation() const;

			bool GetInTransaction() const;

		protected:
			void SetInTransaction(bool bInTransaction);

		private:
			TSmartPath m_pathDatabase;
			sqlite3* m_pDBHandle;
			bool m_bInTransaction;		// global transaction state

			friend class TSQLiteTransaction;
		};

		typedef boost::shared_ptr<TSQLiteDatabase> TSQLiteDatabasePtr;
	}
}

#endif
