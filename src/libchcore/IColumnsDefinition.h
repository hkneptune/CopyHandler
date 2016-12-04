// ============================================================================
//  Copyright (C) 2001-2014 by Jozef Starosczyk
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
#ifndef __ICOLUMNSDEFINITION_H__
#define __ICOLUMNSDEFINITION_H__

#include "TString.h"
#include "TPath.h"

namespace chcore
{
	class LIBCHCORE_API IColumnsDefinition
	{
	public:
		enum ETypes
		{
			eType_bool,
			eType_short,
			eType_ushort,
			eType_int,
			eType_uint,
			eType_long,
			eType_ulong,
			eType_longlong,
			eType_ulonglong,
			eType_double,
			eType_string,
			eType_path,

			eType_Last
		};

	public:
		virtual ~IColumnsDefinition();

		virtual size_t AddColumn(const TString& strColumnName, ETypes eColType) = 0;
		virtual void Clear() = 0;

		virtual size_t GetColumnIndex(const wchar_t* strColumnName) = 0;
		virtual const TString& GetColumnName(size_t stIndex) const = 0;
		virtual size_t GetCount() const = 0;
		virtual bool IsEmpty() const = 0;
	};

	template<class T> struct ColumnType {};
	template<> struct ColumnType<bool> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_bool; };
	template<> struct ColumnType<short> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_short; };
	template<> struct ColumnType<unsigned short> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_ushort; };
	template<> struct ColumnType<int> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_int; };
	template<> struct ColumnType<unsigned int> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_uint; };
	template<> struct ColumnType<long> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_long; };
	template<> struct ColumnType<unsigned long> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_ulong; };
	template<> struct ColumnType<long long> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_longlong; };
	template<> struct ColumnType<unsigned long long> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_ulonglong; };
	template<> struct ColumnType<double> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_double; };
	template<> struct ColumnType<TString> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_string; };
	template<> struct ColumnType<TSmartPath> { static const IColumnsDefinition::ETypes value = IColumnsDefinition::eType_path; };

	template<class T>
	IColumnsDefinition::ETypes GetColumnType(const T&)
	{
		return ColumnType<T>::value;
	}
}

#endif
