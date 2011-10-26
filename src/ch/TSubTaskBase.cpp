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
#include "../libchcore/TBasePathData.h"
#include "TLocalFilesystem.h"
#include "TSubTaskContext.h"
#include "../libchcore/TTaskDefinition.h"
#include "TTaskConfiguration.h"

///////////////////////////////////////////////////////////////////////////
// TSubTaskBase

TSubTaskBase::TSubTaskBase(TSubTaskContext& rContext) :
m_rContext(rContext)
{
}

TSubTaskBase::~TSubTaskBase()
{
}

chcore::TSmartPath TSubTaskBase::CalculateDestinationPath(const chcore::TFileInfoPtr& spFileInfo, chcore::TSmartPath pathDst, int iFlags) const
{
	const chcore::TBasePathDataContainer& rSourcePathsInfo = GetContext().GetBasePathDataContainer();

	if(!spFileInfo)
		THROW(_T("Invalid pointer"), 0, 0, 0);

	// iFlags: bit 0-ignore folders; bit 1-force creating directories
	if(iFlags & 0x02)
	{
		// force create directories
		chcore::TSmartPath pathCombined = pathDst + spFileInfo->GetFullFilePath().GetFileDir();

		// force create directory
		TLocalFilesystem::CreateDirectory(pathCombined, true);

		return pathCombined + spFileInfo->GetFullFilePath().GetFileName();
	}
	else
	{
		size_t stSrcIndex = spFileInfo->GetSrcIndex();

		if (!(iFlags & 0x01) && stSrcIndex != std::numeric_limits<size_t>::max())
		{
			// generate new dest name
			if(!rSourcePathsInfo.GetAt(stSrcIndex)->IsDestinationPathSet())
			{
				chcore::TSmartPath pathSubst = FindFreeSubstituteName(spFileInfo->GetFullFilePath(), pathDst);
				rSourcePathsInfo.GetAt(stSrcIndex)->SetDestinationPath(pathSubst);
			}

			return pathDst + rSourcePathsInfo.GetAt(stSrcIndex)->GetDestinationPath() + spFileInfo->GetFilePath();
		}
		else
			return pathDst + spFileInfo->GetFullFilePath().GetFileName();
	}
}


// finds another name for a copy of src file(folder) in dest location
chcore::TSmartPath TSubTaskBase::FindFreeSubstituteName(chcore::TSmartPath pathSrcPath, chcore::TSmartPath pathDstPath) const
{
	const chcore::TTaskDefinition& rTaskDefinition = GetContext().GetTaskDefinition();

	// get the name from srcpath
	pathSrcPath.StripSeparatorAtEnd();

	chcore::TSmartPath pathFilename = pathSrcPath.GetFileName();

	// set the dest path
	CString strCheckPath;
	ictranslate::CFormat fmt(GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(rTaskDefinition.GetConfiguration()));
	fmt.SetParam(_t("%name"), pathFilename.ToString());
	chcore::TSmartPath pathCheckPath(chcore::PathFromString((PCTSTR)fmt));

	// when adding to strDstPath check if the path already exists - if so - try again
	int iCounter = 1;
	CString strFmt = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(rTaskDefinition.GetConfiguration());
	while(TLocalFilesystem::PathExist(pathDstPath + pathCheckPath))
	{
		fmt.SetFormat(strFmt);
		fmt.SetParam(_t("%name"), pathFilename.ToString());
		fmt.SetParam(_t("%count"), ++iCounter);
		pathCheckPath.FromString((PCTSTR)fmt);
	}

	return pathCheckPath;
}
