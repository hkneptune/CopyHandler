/***************************************************************************
*   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
// File was originally based on FileInfo.cpp by Antonio Tejada Lacaci.
// Almost everything has changed since then.
#include "stdafx.h"
#include <limits>
#include "TFileInfoArray.h"
#include "../libicpf/exception.h"
#include "TFileInfo.h"
#include "ISerializerContainer.h"
#include <boost/numeric/conversion/cast.hpp>

namespace chcore
{
	///////////////////////////////////////////////////////////////////////
	// Array
	TFileInfoArray::TFileInfoArray() :
		m_bComplete(false),
		m_oidLastObjectID(0)
	{
	}

	TFileInfoArray::~TFileInfoArray()
	{
	}

	void TFileInfoArray::AddFileInfo(const TFileInfoPtr& spFileInfo)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		spFileInfo->SetObjectID(++m_oidLastObjectID);
		m_vFiles.push_back(spFileInfo);
	}

	file_count_t TFileInfoArray::GetSize() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return boost::numeric_cast<file_count_t>(m_vFiles.size());
	}

	TFileInfoPtr TFileInfoArray::GetAt(file_count_t fcIndex) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		if (fcIndex >= m_vFiles.size())
			THROW(_T("Out of bounds"), 0, 0, 0);

		return m_vFiles.at(boost::numeric_cast<size_t>(fcIndex));
	}

	TFileInfo TFileInfoArray::GetCopyAt(file_count_t fcIndex) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		if (fcIndex >= m_vFiles.size())
			THROW(_T("Out of bounds"), 0, 0, 0);
		const TFileInfoPtr& spInfo = m_vFiles.at(boost::numeric_cast<size_t>(fcIndex));
		if (!spInfo)
			THROW(_T("Invalid pointer"), 0, 0, 0);

		return *spInfo;
	}

	void TFileInfoArray::Clear()
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_bComplete = false;
		BOOST_FOREACH(const TFileInfoPtr& spFileInfo, m_vFiles)
		{
			m_setRemovedObjects.Add(spFileInfo->GetObjectID());
		}
		m_vFiles.clear();
	}

	unsigned long long TFileInfoArray::CalculateTotalSize() const
	{
		unsigned long long ullSize = 0;

		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		BOOST_FOREACH(const TFileInfoPtr& spFileInfo, m_vFiles)
		{
			ullSize += spFileInfo->GetLength64();
		}

		return ullSize;
	}

	void TFileInfoArray::SetComplete(bool bComplete)
	{
		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_bComplete = bComplete;
	}

	bool TFileInfoArray::IsComplete() const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		return m_bComplete;
	}

	unsigned long long TFileInfoArray::CalculatePartialSize(file_count_t fcCount)
	{
		unsigned long long ullSize = 0;

		boost::shared_lock<boost::shared_mutex> lock(m_lock);
		if (fcCount > m_vFiles.size())
			THROW(_T("Invalid argument"), 0, 0, 0);

		for (std::vector<TFileInfoPtr>::iterator iter = m_vFiles.begin(); iter != m_vFiles.begin() + boost::numeric_cast<size_t>(fcCount); ++iter)
		{
			ullSize += (*iter)->GetLength64();
		}

		return ullSize;
	}

	void TFileInfoArray::Store(const ISerializerContainerPtr& spContainer) const
	{
		boost::shared_lock<boost::shared_mutex> lock(m_lock);

		InitColumns(spContainer);

		// store only if there is a complete collection of items inside
		// (this container is used in the directory scanning process. There is no
		// point storing only partially scanned data in the serializer as we
		// can't use this data after loading serialized data (dir scan will have
		// to scan again)).
		if (m_bComplete)
		{
			spContainer->DeleteRows(m_setRemovedObjects);
			m_setRemovedObjects.Clear();

			BOOST_FOREACH(const TFileInfoPtr& spFileInfo, m_vFiles)
			{
				spFileInfo->Store(spContainer);
			}
		}
	}

	void TFileInfoArray::Load(const ISerializerContainerPtr& spContainer, const TBasePathDataContainerPtr& spBasePaths)
	{
		InitColumns(spContainer);

		std::vector<TFileInfoPtr> vEntries;
		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
		while (spRowReader->Next())
		{
			TFileInfoPtr spFileInfo(new TFileInfo);
			spFileInfo->Load(spRowReader, spBasePaths);

			vEntries.push_back(spFileInfo);

			m_oidLastObjectID = std::max(m_oidLastObjectID, spFileInfo->GetObjectID());
		}

		boost::unique_lock<boost::shared_mutex> lock(m_lock);
		m_vFiles = std::move(vEntries);
		m_bComplete = !m_vFiles.empty();	// we're marking empty/non-empty based on scanned file count; this is due to the
											// fact, that no scanned files are stored in DB unless scanning is complete;
	}

	void TFileInfoArray::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
			TFileInfo::InitColumns(rColumns);
	}
}
