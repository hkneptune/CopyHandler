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
#ifndef __TLOCALFILESYSTEMFIND_H__
#define __TLOCALFILESYSTEMFIND_H__

#include "TFileInfoFwd.h"
#include "TPath.h"
#include "IFilesystemFind.h"
#include "../liblogger/TLogger.h"

namespace chcore
{
	class LIBCHCORE_API TLocalFilesystemFind : public IFilesystemFind
	{
	public:
		virtual ~TLocalFilesystemFind();

		bool FindNext(TFileInfoPtr& rspFileInfo) override;
		void Close() override;

	private:
		TLocalFilesystemFind(const TSmartPath& pathDir, const TSmartPath& pathMask, const logger::TLogFileDataPtr& spLogFileData);

		void InternalClose();
		std::wstring GetFindLogData() const;

	private:
		TSmartPath m_pathDir;
		TSmartPath m_pathMask;
		HANDLE m_hFind;

#pragma warning(push)
#pragma warning(disable: 4251)
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)

		friend class TLocalFilesystem;
	};
}

#endif
