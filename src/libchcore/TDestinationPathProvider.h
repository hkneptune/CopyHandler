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

#include "TPath.h"
#include "TFileInfo.h"
#include "IFilesystem.h"

namespace chcore
{
	class TDestinationPathProvider
	{
	public:
		TDestinationPathProvider(const IFilesystemPtr& spFilesystem, const TSmartPath& pathDestinationBase, bool bIgnoreFolders, bool bForceDirectories,
			const TString& strFirstAltName, const TString& strNextAltName);

		TSmartPath CalculateDestinationPath(const TFileInfoPtr& spFileInfo);

	private:
		TSmartPath FindFreeSubstituteName(TSmartPath pathSrcPath) const;

	private:
		IFilesystemPtr m_spFilesystem;
		TSmartPath m_pathDestinationBase;
		bool m_bIgnoreFolders;
		bool m_bForceDirectories;
		TString m_strFirstAltName;
		TString m_strNextAltName;
	};
}

#endif
