/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
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
#ifndef __SERIALIZATION_HELPERS_H__
#define __SERIALIZATION_HELPERS_H__

#include <boost/serialization/split_free.hpp>
#include <boost/serialization/vector.hpp>

BOOST_SERIALIZATION_SPLIT_FREE(CString);
BOOST_SERIALIZATION_SPLIT_FREE(CTime);

namespace boost {
	namespace serialization {

		template<class Archive>
		void save(Archive& ar, const CString& str, const unsigned int /*version*/)
		{
			std::wstring wstr = str;
			ar << wstr;
		}

		template<class Archive>
		void load(Archive& ar, CString& str, const unsigned int /*version*/)
		{
			std::wstring wstr;
			ar >> wstr;
			str = wstr.c_str();
		}

		template<class Archive>
		void save(Archive& ar, const CTime& tTime, const unsigned int /*version*/)
		{
			long long llTime = tTime.GetTime();
			ar << llTime;
		}

		template<class Archive>
		void load(Archive& ar, CTime& tTime, const unsigned int /*version*/)
		{
			long long llTime = 0;
			ar >> llTime;
			tTime = CTime(llTime);
		}

	} // namespace serialization
} // namespace boost


#endif
