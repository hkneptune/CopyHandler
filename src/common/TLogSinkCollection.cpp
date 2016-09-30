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
#include "TLogSinkCollection.h"
#include "..\libchcore\TStringArray.h"

namespace chcore
{
	TLogSinkCollection::TLogSinkCollection()
	{

	}

	void TLogSinkCollection::AddPath(const TSmartPath& rPath)
	{
		TStringArray arrComponents;
		TSmartPath pathToFileWithoutExtension = rPath.GetFileTitle();
		pathToFileWithoutExtension.ToWString().Split(L".", arrComponents);

		switch (arrComponents.GetCount())
		{
		case 0:
			break;
		case 1:
		{
			GetSinkData(rPath);
			break;
		}
		default:
		{
			TSmartPath pathBase = rPath.GetFileDir() + pathToFileWithoutExtension.GetFileTitle();
			TLogSink& rSinkData = GetSinkData(pathBase);

			std::wstring strRotateInfo = arrComponents.GetAt(arrComponents.GetCount() - 1).c_str();
			if (std::all_of(strRotateInfo.begin(), strRotateInfo.end(), isdigit))
			{
				rSinkData.AddRotatedFile(rPath);
			}
		}
		}
	}

	TLogSink& TLogSinkCollection::GetSinkData(const TSmartPath& path)
	{
		auto iterFind = m_mapLogs.find(path);
		if (iterFind == m_mapLogs.end())
			iterFind = m_mapLogs.insert(std::make_pair(path, TLogSink(path))).first;

		return iterFind->second;
	}

	void TLogSinkCollection::Clear()
	{
		m_mapLogs.clear();
	}

	void TLogSinkCollection::CloseExpiredFiles(unsigned long long ullCurrentTimestamp, unsigned long long ullMaxHandleCacheTime)
	{
		for (auto& pairSink : m_mapLogs)
		{
			pairSink.second.CloseIfTimedOut(ullCurrentTimestamp, ullMaxHandleCacheTime);
		}
	}
}
