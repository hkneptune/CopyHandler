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
#ifndef __TDESTINATIONPATHPROVIDER_H__
#define __TDESTINATIONPATHPROVIDER_H__

#include "../libchcore/TPath.h"
#include "TFileInfo.h"
#include "IFilesystem.h"

namespace chengine
{
	class TDestinationPathProvider
	{
	public:
		TDestinationPathProvider(const IFilesystemPtr& spFilesystem, const chcore::TSmartPath& pathDestinationBase, bool bIgnoreFolders, bool bForceDirectories,
			const string::TString& strFirstAltName, const string::TString& strNextAltName);

		chcore::TSmartPath CalculateDestinationPath(const TFileInfoPtr& spFileInfo) const;
		chcore::TSmartPath CalculateSuggestedDestinationPath(chcore::TSmartPath pathDstPath) const;

	private:
		chcore::TSmartPath CalculateForceDirectories(const TFileInfoPtr& spFileInfo) const;
		chcore::TSmartPath CalculateIgnoreDirectories(const TFileInfoPtr& spFileInfo) const;
		chcore::TSmartPath FindFreeSubstituteName(chcore::TSmartPath pathSrcPath) const;

	private:
		IFilesystemPtr m_spFilesystem;
		chcore::TSmartPath m_pathDestinationBase;
		bool m_bIgnoreFolders;
		bool m_bForceDirectories;
		string::TString m_strFirstAltName;
		string::TString m_strNextAltName;
	};
}

#endif
