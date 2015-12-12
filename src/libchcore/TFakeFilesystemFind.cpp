// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#include "TFakeFilesystemFind.h"
#include "TFakeFilesystem.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include "TStringPattern.h"

namespace chcore
{
	TFakeFilesystemFind::TFakeFilesystemFind(const TSmartPath& pathDir, const TSmartPath& pathMask, TFakeFilesystem* pFakeFilesystem) :
		m_pathDir(pathDir),
		m_pathMask(pathMask),
		m_pFilesystem(pFakeFilesystem)
	{
		if (!pFakeFilesystem)
			THROW_CORE_EXCEPTION(eErr_InvalidArgument);
	}

	TFakeFilesystemFind::~TFakeFilesystemFind()
	{
	}

	bool TFakeFilesystemFind::FindNext(TFileInfoPtr& rspFileInfo)
	{
		if (!m_bScanned)
		{
			Prescan();
			m_bScanned = true;
		}

		if (m_iterCurrent == m_vItems.end())
			return false;

		*rspFileInfo = *m_iterCurrent++;
		return true;
	}

	void TFakeFilesystemFind::Close()
	{
		m_vItems.clear();
		m_iterCurrent = m_vItems.end();
		m_bScanned = false;
	}

	void TFakeFilesystemFind::Prescan()
	{
		m_vItems.clear();
		m_iterCurrent = m_vItems.end();

		for (TFakeFileDescriptionPtr spFileInfoDesc : m_pFilesystem->m_listFilesystemContent)
		{
			if (spFileInfoDesc->GetFileInfo().GetFullFilePath().StartsWith(m_pathDir))
			{
				TStringPattern pattern(m_pathMask.ToString());
				if (pattern.Matches(spFileInfoDesc->GetFileInfo().GetFullFilePath().ToWString()))
				{
					m_vItems.push_back(spFileInfoDesc->GetFileInfo());
				}
			}
		}

		m_iterCurrent = m_vItems.begin();
	}
}
