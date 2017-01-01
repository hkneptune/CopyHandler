// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TBasePathData.cpp
/// @date  2010/10/13
/// @brief Contains implementations of classes related to keeping path data.
// ============================================================================
#include "stdafx.h"
#include "TBasePathData.h"
#include "../libchcore/TCoreException.h"
#include "../libchcore/ErrorCodes.h"
#include "../libchcore/TPathContainer.h"
#include <boost/numeric/conversion/cast.hpp>
#include <boost/thread/locks.hpp>
#include "../libserializer/IColumnsDefinition.h"
#include "../libserializer/ISerializerContainer.h"

using namespace chcore;
using namespace chengine;
using namespace serializer;

namespace chengine
{
	//////////////////////////////////////////////////////////////////////////////
	// TBasePathData

	TBasePathData::TBasePathData() :
		m_oidObjectID(0),
		m_pathSrc(m_setModifications),
		m_bSkipFurtherProcessing(m_setModifications, false),
		m_pathDst(m_setModifications)
	{
		m_setModifications[eMod_Added] = true;
	}

	TBasePathData::TBasePathData(const TBasePathData& rEntry) :
		m_setModifications(rEntry.m_setModifications),
		m_oidObjectID(rEntry.m_oidObjectID),
		m_pathSrc(m_setModifications, rEntry.m_pathSrc),
		m_bSkipFurtherProcessing(m_setModifications, rEntry.m_bSkipFurtherProcessing),
		m_pathDst(m_setModifications, rEntry.m_pathDst)
	{
	}

	TBasePathData::TBasePathData(object_id_t oidObjectID, const TSmartPath& spSrcPath) :
		m_oidObjectID(oidObjectID),
		m_pathSrc(m_setModifications, spSrcPath),
		m_bSkipFurtherProcessing(m_setModifications, false),
		m_pathDst(m_setModifications)
	{
		m_pathSrc.Modify().StripSeparatorAtEnd();
		m_setModifications[eMod_Added] = true;
	}

	void TBasePathData::SetDestinationPath(const TSmartPath& tPath)
	{
		m_pathDst = tPath;
		m_pathDst.Modify().StripSeparatorAtEnd();
	}

	TSmartPath TBasePathData::GetDestinationPath() const
	{
		return m_pathDst;
	}

	bool TBasePathData::GetSkipFurtherProcessing() const
	{
		return m_bSkipFurtherProcessing;
	}

	void TBasePathData::SetSkipFurtherProcessing(bool bSkipFurtherProcessing)
	{
		m_bSkipFurtherProcessing = bSkipFurtherProcessing;
	}

	bool TBasePathData::IsDestinationPathSet() const
	{
		return !m_pathDst.Get().IsEmpty();
	}

	void TBasePathData::Store(const ISerializerContainerPtr& spContainer) const
	{
		if (!spContainer)
			throw TCoreException(eErr_InvalidPointer, L"spContainer", LOCATION);

		bool bAdded = m_setModifications[eMod_Added];
		if (m_setModifications.any())
		{
			ISerializerRowData& rRow = spContainer->GetRow(m_oidObjectID, bAdded);
			if (bAdded || m_setModifications[eMod_SrcPath])
				rRow.SetValue(_T("src_path"), m_pathSrc);
			if (bAdded || m_setModifications[eMod_SkipProcessing])
				rRow.SetValue(_T("skip_processing"), m_bSkipFurtherProcessing);
			if (bAdded || m_setModifications[eMod_DstPath])
				rRow.SetValue(_T("dst_path"), m_pathDst);

			m_setModifications.reset();
		}
	}

	void TBasePathData::InitColumns(IColumnsDefinition& rColumns)
	{
		rColumns.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumns.AddColumn(_T("src_path"), IColumnsDefinition::eType_path);
		rColumns.AddColumn(_T("skip_processing"), IColumnsDefinition::eType_bool);
		rColumns.AddColumn(_T("dst_path"), IColumnsDefinition::eType_path);
	}

	void TBasePathData::Load(const ISerializerRowReaderPtr& spRowReader)
	{
		spRowReader->GetValue(_T("id"), m_oidObjectID);
		spRowReader->GetValue(_T("src_path"), m_pathSrc.Modify());
		spRowReader->GetValue(_T("skip_processing"), m_bSkipFurtherProcessing.Modify());
		spRowReader->GetValue(_T("dst_path"), m_pathDst.Modify());

		m_pathSrc.Modify().StripSeparatorAtEnd();
		m_pathDst.Modify().StripSeparatorAtEnd();

		m_setModifications.reset();
	}

	TSmartPath TBasePathData::GetSrcPath() const
	{
		return m_pathSrc;
	}

	void TBasePathData::SetSrcPath(const TSmartPath& pathSrc)
	{
		m_pathSrc = pathSrc;
		m_pathSrc.Modify().StripSeparatorAtEnd();
	}

	object_id_t TBasePathData::GetObjectID() const
	{
		return m_oidObjectID;
	}

	void TBasePathData::SetObjectID(object_id_t oidObjectID)
	{
		m_oidObjectID = oidObjectID;
	}

}
