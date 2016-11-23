// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TDateTime.h
/// @date  2011/10/26
/// @brief Contains declaration of date&time handling class.
// ============================================================================
#ifndef __TDATETIME_H__
#define __TDATETIME_H__

#include "libchcore.h"
#include "TConfig.h"

namespace chcore
{
	class TConfig;

	class LIBCHCORE_API TDateTime
	{
	public:
		TDateTime();
		TDateTime(int iYear, int iMonth, int iDay, int iHour, int iMinute, int iSecond);
		// cppcheck-suppress noExplicitConstructor
		TDateTime(FILETIME ftDateTime);
		// cppcheck-suppress noExplicitConstructor
		TDateTime(SYSTEMTIME sysDateTime);
		// cppcheck-suppress noExplicitConstructor
		TDateTime(time_t tDateTime);

		TDateTime& operator=(FILETIME ftDateTime);
		TDateTime& operator=(SYSTEMTIME sysDateTime);
		TDateTime& operator=(time_t tDateTime);

		bool operator==(const TDateTime& rSrc) const;
		bool operator!=(const TDateTime& rSrc) const;

		// content modification
		void Clear();
		void SetCurrentDateTime();

		// content extraction
		void GetAsSystemTime(SYSTEMTIME& tSystemTime) const;
		time_t GetAsTimeT() const;
		TString Format(bool bUseDate, bool bUseTime) const;

		// comparison
		time_t Compare(const TDateTime& rOtherDateTime, bool bCompareDate, bool bCompareTime) const;

		// serialization
		void StoreInConfig(TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszNodeName);

	private:
		time_t m_tTime;
	};
}

CONFIG_MEMBER_SERIALIZATION(TDateTime)

#endif
