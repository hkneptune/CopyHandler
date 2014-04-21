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
#ifndef __TROWDATA_H__
#define __TROWDATA_H__

#include "libchcore.h"
#include "TString.h"
#include "TPath.h"

#include <boost/variant.hpp>

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TRowData
{
private:
	TRowData(const TRowData&);
	TRowData& operator=(const TRowData&);

public:
	TRowData(const TString& strColName, bool bValue);
	TRowData(const TString& strColName, short iValue);
	TRowData(const TString& strColName, unsigned short uiValue);
	TRowData(const TString& strColName, int iValue);
	TRowData(const TString& strColName, unsigned int uiValue);
	TRowData(const TString& strColName, long lValue);
	TRowData(const TString& strColName, unsigned long ulValue);
	TRowData(const TString& strColName, long long llValue);
	TRowData(const TString& strColName, unsigned long long llValue);
	TRowData(const TString& strColName, double dValue);
	TRowData(const TString& strColName, const TString& strValue);
	TRowData(const TString& strColName, const TSmartPath& pathValue);

	~TRowData();

private:
	typedef boost::variant<
		bool,
		short,
		unsigned short,
		int,
		unsigned int,
		long,
		unsigned long,
		long long,
		unsigned long long,
		double,
		TString,
		TSmartPath
	> InternalVariant;

	TString m_strColName;
#pragma warning(push)
#pragma warning(disable: 4251)
	InternalVariant m_varValue;
#pragma warning(pop)

	friend class TSQLiteSerializerRowData;
};

typedef boost::shared_ptr<TRowData> TRowDataPtr;

END_CHCORE_NAMESPACE

#endif
