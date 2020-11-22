// ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
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
#pragma once

#include <boost/bimap.hpp>
#include "../libstring/TString.h"
#include <vector>

namespace serializer
{
	template<class Enum>
	class DataMapper
	{
	public:
		using Bimap = boost::bimap<Enum, string::TString>;

	public:
		DataMapper(const std::initializer_list<typename Bimap::value_type>&& list) :
			m_mapData(list.begin(), list.end())
		{
		}

		string::TString Map(Enum eData) const
		{
			auto iterFnd = m_mapData.left.find(eData);
			if(iterFnd != m_mapData.left.end())
				return iterFnd->second;

			throw std::invalid_argument("No mapping available ");
		}

		Enum Unmap(const string::TString& mappedData) const
		{
			auto iterFnd = m_mapData.right.find(mappedData);
			if(iterFnd != m_mapData.right.end())
				return iterFnd->second;

			// check the numeric value
			try
			{
				// for numeric values check if they exist in internal bimap before returning
				int iValue = std::stoi(mappedData.c_str());
				auto iterNumFnd = m_mapData.left.find((Enum)iValue);
				if(iterNumFnd != m_mapData.left.end())
					return iterNumFnd->first;
			}
			catch(const std::exception&)
			{
			}

			throw std::invalid_argument("No reverse mapping available ");
		}

	private:
		Bimap m_mapData;
	};
}

template<class Enum>
string::TString MapEnum(Enum eValue)
{
}

template<class Enum>
Enum UnmapEnum(const string::TString& strValue)
{
}
