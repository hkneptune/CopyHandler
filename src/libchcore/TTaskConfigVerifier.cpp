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
#include "stdafx.h"
#include "TTaskConfigVerifier.h"
#include "TTaskConfiguration.h"
#include <boost\format.hpp>
#include "..\Common\TLogger.h"

namespace chcore
{
	void TTaskConfigVerifier::VerifyAndUpdate(TConfig& rConfig, TLogger* pLog)
	{
		TString strFirstFormat = GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(rConfig);
		if(strFirstFormat.Find(L"%name") == TString::npos || strFirstFormat.Find(L"%ext") == TString::npos)
		{
			TString strDefaultFormat = TaskPropData<eTO_AlternateFilenameFormatString_First>::GetDefaultValue();
			if(pLog)
			{
				LOG_WARNING(*pLog) << boost::str(boost::wformat(L"First alternate filename format string (%1%) does not contain %%name placeholder. Switching to default (%2%).")
						% strFirstFormat.c_str()
						% strDefaultFormat.c_str()).c_str();
			}

			SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(rConfig, strDefaultFormat);
		}

		TString strSubsequentFormat = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(rConfig);
		if(strSubsequentFormat.Find(L"%name") == TString::npos || strSubsequentFormat.Find(L"%count") == TString::npos
			|| strSubsequentFormat.Find(L"%ext") == TString::npos)
		{
			TString strDefaultFormat = TaskPropData<eTO_AlternateFilenameFormatString_AfterFirst>::GetDefaultValue();
			if(pLog)
			{
				LOG_WARNING(*pLog) <<
					boost::str(boost::wformat(L"Subsequent alternate filename format string (%1%) does not contain %%name or %%count placeholder. Switching to default (%2%).")
						% strSubsequentFormat.c_str()
						% strDefaultFormat.c_str()).c_str();
			}

			SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(rConfig, strDefaultFormat);
		}
	}
}
