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
#ifndef __ISERIALIZERROWWRITER_H__
#define __ISERIALIZERROWWRITER_H__

#include "libchcore.h"
#include "TString.h"
#include "TPath.h"

BEGIN_CHCORE_NAMESPACE

class ISerializerContainer;
typedef boost::shared_ptr<ISerializerContainer> ISerializerContainerPtr;

class LIBCHCORE_API ISerializerRowData
{
public:
	virtual ~ISerializerRowData();

	virtual ISerializerRowData& SetValue(size_t stColIndex, bool bValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, short iValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, unsigned short uiValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, int iValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, unsigned int uiValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, long lValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, unsigned long ulValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, long long llValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, unsigned long long llValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, double dValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, const TString& strValue) = 0;
	virtual ISerializerRowData& SetValue(size_t stColIndex, const TSmartPath& pathValue) = 0;

	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, bool bValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, short iValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, unsigned short uiValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, int iValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, unsigned int uiValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, long lValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, unsigned long ulValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, long long llValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, unsigned long long llValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, double dValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, const TString& strValue) = 0;
	virtual ISerializerRowData& SetValue(const wchar_t* strColumnName, const TSmartPath& pathValue) = 0;
};

END_CHCORE_NAMESPACE

#endif
