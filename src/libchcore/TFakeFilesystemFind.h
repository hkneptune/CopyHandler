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
#ifndef __TFAKEFILESYSTEMFIND_H__
#define __TFAKEFILESYSTEMFIND_H__

#include "libchcore.h"
#include "IFilesystemFind.h"
#include "TPath.h"

namespace chcore
{
	class TFakeFilesystem;

	class LIBCHCORE_API TFakeFilesystemFind : public IFilesystemFind
	{
	public:
		TFakeFilesystemFind(const TSmartPath& pathDir, const TSmartPath& pathMask, TFakeFilesystem* pFakeFilesystem);
		~TFakeFilesystemFind();

		virtual bool FindNext(TFileInfoPtr& rspFileInfo) override;
		virtual void Close() override;

	private:
		void Prescan();

	private:
		TSmartPath m_pathDir;
		TSmartPath m_pathMask;
		TFakeFilesystem* m_pFilesystem;

#pragma warning(push)
#pragma warning(disable: 4251)
		bool m_bScanned = false;
		std::vector<TFileInfo> m_vItems;
		std::vector<TFileInfo>::iterator m_iterCurrent;
#pragma warning(pop)
	};
}

#endif
