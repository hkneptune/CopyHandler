// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#ifndef __TSIZEFORMATTER_H__
#define __TSIZEFORMATTER_H__

#include <memory>
#include "libchengine.h"

namespace string { class TString; }

namespace chengine
{
	class TConfig;

	class LIBCHENGINE_API TSizeFormatter
	{
	public:
		TSizeFormatter();
		TSizeFormatter(const TSizeFormatter&) = delete;
		TSizeFormatter(const wchar_t* strBytes, const wchar_t* strKBytes, const wchar_t* strMBytes, const wchar_t* strGBytes, const wchar_t* strTBytes);

		TSizeFormatter& operator=(const TSizeFormatter&) = delete;

		void SetValues(const wchar_t* strBytes, const wchar_t* strKBytes, const wchar_t* strMBytes, const wchar_t* strGBytes, const wchar_t* strTBytes);

		string::TString GetSizeString(unsigned long long ullData, bool bStrict = false) const;

		void StoreInConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName) const;
		bool ReadFromConfig(chengine::TConfig& rConfig, PCTSTR pszNodeName);

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		std::wstring m_strBytes;
		std::wstring m_strKBytes;
		std::wstring m_strMBytes;
		std::wstring m_strGBytes;
		std::wstring m_strTBytes;
#pragma warning(pop)
	};

	using TSizeFormatterPtr = std::shared_ptr<TSizeFormatter>;
}

#endif
