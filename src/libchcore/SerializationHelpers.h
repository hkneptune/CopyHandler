/***************************************************************************
*   Copyright (C) 2001-2011 by Józef Starosczyk                           *
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

#include "libchcore.h"
#include <boost\spirit\home\support\container.hpp>

BEGIN_CHCORE_NAMESPACE

namespace Serializers
{
	struct GeneralSerializer
	{
		template<class Serializer, class T> static void StoreValue(Serializer& rSerializer, const T& tValue) { tValue.Serialize(rSerializer); }
		template<class Serializer, class T> static void LoadValue(Serializer& rSerializer, T& tValue) { tValue.Serialize(rSerializer); }
	};

	struct NativeSerializer
	{
		template<class Serializer, class T> static void StoreValue(Serializer& rSerializer, const T& tValue) { rSerializer.Store(tValue); }
		template<class Serializer, class T> static void LoadValue(Serializer& rSerializer, T& tValue) { rSerializer.Load(tValue); }
	};

	struct EnumSerializer
	{
		template<class Serializer, class T> static void StoreValue(Serializer& rSerializer, const T& tValue) { Serialize(rSerializer, (int)tValue); }
		template<class Serializer, class T> static void LoadValue(Serializer& rSerializer, T& tValue) { Serialize(rSerializer, *(int*)tValue); }
	};

	struct StlContainerSerializer
	{
		template<class Serializer, class T> static void StoreValue(Serializer& rSerializer, const T& tValue)
		{
			typedef typename T::value_type ItemType;

			Serialize(rSerializer, tValue.size());
			BOOST_FOREACH(const ItemType& rItem, tValue)
			{
				Serialize(rSerializer, rItem);
			}
		}
		template<class Serializer, class T> static void LoadValue(Serializer& rSerializer, T& tValue)
		{
			typedef typename T::value_type ItemType;
			ItemType item;

			size_t stCount = 0;
			Serialize(rSerializer, stCount);
			while(stCount--)
			{
				Serialize(rSerializer, item);

				std::back_inserter(tValue) = item;
			}
		}
	};

	template<class Serializer, class T>
	struct is_serializer_native_type
	{
		static const bool value = boost::mpl::has_key<Serializer::NativeTypes, T>::value;
	};

	// Main interface for serialization
	template<class T>
	inline void Serialize(TReadBinarySerializer& rSerializer, T& tValue)
	{
		typedef typename 
			boost::mpl::if_<boost::is_enum<T>, EnumSerializer,
			boost::mpl::if_<boost::spirit::traits::is_container<T>, StlContainerSerializer,
			boost::mpl::if_<is_serializer_native_type<TReadBinarySerializer, T>, NativeSerializer,
			GeneralSerializer>::type >::type >::type SerializerType;
		SerializerType::LoadValue(rSerializer, tValue);
	}

	template<class T>
	inline void Serialize(TWriteBinarySerializer& rSerializer, const T& tValue)
	{
		typedef typename 
			boost::mpl::if_<boost::is_enum<T>, EnumSerializer,
			boost::mpl::if_<boost::spirit::traits::is_container<T>, StlContainerSerializer,
			boost::mpl::if_<is_serializer_native_type<TWriteBinarySerializer, T>, NativeSerializer,
			GeneralSerializer>::type >::type >::type SerializerType;
		SerializerType::StoreValue(rSerializer, tValue);
	}

#ifdef _MFC_VER
	template<>
	inline void Serialize<CString>(TReadBinarySerializer& rSerializer, CString& tValue)
	{
		TString strVal;
		Serialize(rSerializer, strVal);
		tValue = strVal;
	}

	template<>
	inline void Serialize<CString>(TWriteBinarySerializer& rSerializer, const CString& tValue)
	{
		Serialize(rSerializer, (PCTSTR)tValue);
	}

#endif
}

END_CHCORE_NAMESPACE

#endif
