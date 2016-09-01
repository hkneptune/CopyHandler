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
#ifndef __TSQLITETRANSACTION_H__
#define __TSQLITETRANSACTION_H__

#include "TSQLiteDatabase.h"

namespace chcore
{
	namespace sqlite
	{
		class TSQLiteTransaction
		{
		public:
			explicit TSQLiteTransaction(const TSQLiteDatabasePtr& spDatabase);
			~TSQLiteTransaction();

			void Begin();
			void Rollback();
			void Commit();

		private:
			TSQLiteDatabasePtr m_spDatabase;
			bool m_bTransactionStarted;		// states if transaction was started by this object
		};
	}
}

#endif
