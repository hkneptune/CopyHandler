// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TSubTaskBase.cpp
/// @date  2010/09/19
/// @brief Contains implementation of some common subtask elements.
// ============================================================================
#include "stdafx.h"
#include "TSubTaskBase.h"
#include "TBasePathData.h"
#include "TSubTaskContext.h"
#include "TTaskConfiguration.h"
#include <boost/lexical_cast.hpp>
#include "TFileInfo.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

namespace chcore
{
	///////////////////////////////////////////////////////////////////////////
	// TSubTaskBase

	TSubTaskBase::TSubTaskBase(TSubTaskContext& rContext) :
		m_rContext(rContext)
	{
	}

	TSubTaskBase::~TSubTaskBase()
	{
	}

	TSmartPath TSubTaskBase::CalculateDestinationPath(const TFileInfoPtr& spFileInfo, TSmartPath pathDst, int iFlags)
	{
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		if (!spFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spFileInfo", LOCATION);

		// iFlags: bit 0-ignore folders; bit 1-force creating directories
		if (iFlags & 0x02)
		{
			// force create directories
			TSmartPath pathCombined = pathDst + spFileInfo->GetFullFilePath().GetFileDir();

			// force create directory
			spFilesystem->CreateDirectory(pathCombined, true);

			return pathCombined + spFileInfo->GetFullFilePath().GetFileName();
		}
		else
		{
			TBasePathDataPtr spPathData = spFileInfo->GetBasePathData();

			if (!(iFlags & 0x01) && spPathData)
			{
				// generate new dest name
				if (!spPathData->IsDestinationPathSet())
				{
					// generate something - if dest folder == src folder - search for copy
					if (pathDst == spFileInfo->GetFullFilePath().GetFileRoot())
					{
						TSmartPath pathSubst = FindFreeSubstituteName(spFileInfo->GetFullFilePath(), pathDst);
						spPathData->SetDestinationPath(pathSubst);
					}
					else
					{
						TSmartPath pathFilename = spFileInfo->GetFullFilePath().GetFileName();
						pathFilename.StripPath(L":");
						spPathData->SetDestinationPath(pathFilename);
					}
				}

				return pathDst + spPathData->GetDestinationPath() + spFileInfo->GetFilePath();
			}
			else
				return pathDst + spFileInfo->GetFilePath();
		}
	}

	// finds another name for a copy of src file(folder) in dest location
	TSmartPath TSubTaskBase::FindFreeSubstituteName(TSmartPath pathSrcPath, TSmartPath pathDstPath) const
	{
		const TConfig& rConfig = GetContext().GetConfig();
		IFilesystemPtr spFilesystem = GetContext().GetLocalFilesystem();

		// get the name from src path
		pathSrcPath.StripSeparatorAtEnd();

		TSmartPath pathFilename = pathSrcPath.GetFileName();
		pathFilename.StripPath(L":");

		// set the dest path
		TString strCheckPath = GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(rConfig);
		strCheckPath.Replace(_T("%name"), pathFilename.ToString());
		TSmartPath pathCheckPath(PathFromWString(strCheckPath));

		// when adding to strDstPath check if the path already exists - if so - try again
		int iCounter = 1;
		TString strFmt = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(rConfig);
		while (spFilesystem->PathExist(pathDstPath + pathCheckPath))
		{
			strCheckPath = strFmt;
			strCheckPath.Replace(_T("%name"), pathFilename.ToString());
			strCheckPath.Replace(_T("%count"), boost::lexical_cast<std::wstring>(++iCounter).c_str());
			pathCheckPath.FromString(strCheckPath);
		}

		return pathCheckPath;
	}
}
