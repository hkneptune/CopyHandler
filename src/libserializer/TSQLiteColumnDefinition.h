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
#ifndef __TSQLITECOLUMNDEFINITION_H__
#define __TSQLITECOLUMNDEFINITION_H__

#include "../libstring/TString.h"
#include <vector>
#include "IColumnsDefinition.h"

namespace serializer
{
	class LIBSERIALIZER_API TSQLiteColumnsDefinition : public IColumnsDefinition
	{
	public:
		TSQLiteColumnsDefinition();
		virtual ~TSQLiteColumnsDefinition();

		size_t AddColumn(const string::TString& strColumnName, ETypes eColType) override;
		void Clear() override;

		size_t GetColumnIndex(const wchar_t* strColumnName) override;
		virtual ETypes GetColumnType(size_t stIndex) const;
		const string::TString& GetColumnName(size_t stIndex) const override;
		size_t GetCount() const override;
		bool IsEmpty() const override;

		virtual string::TString GetCommaSeparatedColumns() const;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		typedef std::vector<std::pair<string::TString, ETypes>> VecColumns;
		VecColumns m_vColumns;
#pragma warning(pop)
	};
}

#endif
