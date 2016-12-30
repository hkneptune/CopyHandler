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
#include "stdafx.h"
#include "TFileFiltersArray.h"
#include "TFileInfo.h"
#include "TConfig.h"
#include "TConfigArray.h"

using namespace chcore;
using namespace string;
using namespace serializer;

namespace chengine
{
	bool TFileFiltersArray::Match(const TFileInfoPtr& spInfo) const
	{
		if (m_vItems.empty())
			return true;

		// if only one of the filters matches - return true
		for (std::vector<TFileFilter>::const_iterator iterFilter = m_vItems.begin(); iterFilter != m_vItems.end(); ++iterFilter)
		{
			if ((*iterFilter).Match(spInfo))
				return true;
		}

		return false;
	}

	void TFileFiltersArray::StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const
	{
		rConfig.DeleteNode(pszNodeName);
		for(const TFileFilter& rFilter : m_vItems)
		{
			TConfig cfgNode;
			rFilter.StoreInConfig(cfgNode);

			TString strNode = TString(pszNodeName) + _T(".FilterDefinition");
			rConfig.AddSubConfig(strNode.c_str(), cfgNode);
		}
	}

	bool TFileFiltersArray::ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName)
	{
		m_vItems.clear();

		TConfigArray vConfigs;
		if (!rConfig.ExtractMultiSubConfigs(pszNodeName, vConfigs))
			return false;

		for (size_t stIndex = 0; stIndex < vConfigs.GetCount(); ++stIndex)
		{
			const TConfig& rCfg = vConfigs.GetAt(stIndex);
			TFileFilter tFilter;
			tFilter.ReadFromConfig(rCfg);

			m_vItems.push_back(tFilter);
		}
		return true;
	}

	void TFileFiltersArray::Store(const ISerializerContainerPtr& spContainer) const
	{
		InitColumns(spContainer);

		spContainer->DeleteRows(m_setRemovedObjects);
		m_setRemovedObjects.Clear();

		for(const TFileFilter& rFilter : m_vItems)
		{
			rFilter.Store(spContainer);
		}
	}

	void TFileFiltersArray::Load(const ISerializerContainerPtr& spContainer)
	{
		InitColumns(spContainer);

		ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
		while (spRowReader->Next())
		{
			TFileFilter tFileFilter;
			tFileFilter.Load(spRowReader);

			tFileFilter.ResetModifications();

			m_vItems.push_back(tFileFilter);
		}
	}

	void TFileFiltersArray::InitColumns(const ISerializerContainerPtr& spContainer) const
	{
		IColumnsDefinition& rColumns = spContainer->GetColumnsDefinition();
		if (rColumns.IsEmpty())
			TFileFilter::InitColumns(rColumns);
	}
}
