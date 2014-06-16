// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TSubTaskDelete.h
/// @date  2010/09/18
/// @brief Contains declarations of classes responsible for delete sub-operation.
// ============================================================================
#ifndef __TSUBTASKDELETE_H__
#define __TSUBTASKDELETE_H__

#include "libchcore.h"
#include "TSubTaskBase.h"

BEGIN_CHCORE_NAMESPACE

class TReadBinarySerializer;
class TWriteBinarySerializer;

namespace details
{
	///////////////////////////////////////////////////////////////////////////
	// TDeleteProgressInfo

	class TDeleteProgressInfo : public TSubTaskProgressInfo
	{
	public:
		TDeleteProgressInfo();
		virtual ~TDeleteProgressInfo();

		virtual void ResetProgress();

		void SetCurrentIndex(size_t stIndex);
		void IncreaseCurrentIndex();
		size_t GetCurrentIndex() const;

		void Store(const ISerializerRowDataPtr& spRowData) const;
		static void InitLoader(IColumnsDefinition& rColumns);
		void Load(const ISerializerRowReaderPtr& spRowReader);
		bool WasSerialized() const;

	private:
		size_t m_stCurrentIndex;
		mutable size_t m_stLastStoredIndex;
		mutable boost::shared_mutex m_lock;
	};
}

///////////////////////////////////////////////////////////////////////////
// TSubTaskDelete

class LIBCHCORE_API TSubTaskDelete : public TSubTaskBase
{
public:
	TSubTaskDelete(TSubTaskContext& rContext);

	virtual void Reset();

	virtual ESubOperationResult Exec();
	virtual ESubOperationType GetSubOperationType() const { return eSubOperation_Deleting; }

	virtual void Store(const ISerializerPtr& spSerializer) const;
	virtual void Load(const ISerializerPtr& spSerializer);

	virtual TSubTaskProgressInfo& GetProgressInfo() { return m_tProgressInfo; }
	virtual void GetStatsSnapshot(TSubTaskStatsSnapshotPtr& spStats) const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	details::TDeleteProgressInfo m_tProgressInfo;
	TSubTaskStatsInfo m_tSubTaskStats;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
