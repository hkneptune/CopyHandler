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
#ifndef __ISERIALIZERROWREADER_H__
#define __ISERIALIZERROWREADER_H__

#include "../libstring/TString.h"
#include "../libchcore/TPath.h"
#include "libserializer.h"

namespace serializer
{
	class LIBSERIALIZER_API ISerializerRowReader
	{
	public:
		virtual ~ISerializerRowReader();

		virtual bool Next() = 0;

		virtual void GetValue(const string::TString& strColName, bool& bValue) = 0;
		virtual void GetValue(const string::TString& strColName, short& iValue) = 0;
		virtual void GetValue(const string::TString& strColName, unsigned short& uiValue) = 0;
		virtual void GetValue(const string::TString& strColName, int& iValue) = 0;
		virtual void GetValue(const string::TString& strColName, unsigned int& uiValue) = 0;
		virtual void GetValue(const string::TString& strColName, long& lValue) = 0;
		virtual void GetValue(const string::TString& strColName, unsigned long& ulValue) = 0;
		virtual void GetValue(const string::TString& strColName, long long& llValue) = 0;
		virtual void GetValue(const string::TString& strColName, unsigned long long& llValue) = 0;
		virtual void GetValue(const string::TString& strColName, double& dValue) = 0;
		virtual void GetValue(const string::TString& strColName, string::TString& strValue) = 0;
		virtual void GetValue(const string::TString& strColName, chcore::TSmartPath& pathValue) = 0;
	};

	typedef std::shared_ptr<ISerializerRowReader> ISerializerRowReaderPtr;
}

#endif
