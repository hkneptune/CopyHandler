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
#ifndef __TLOCALFILESYSTEMFILE_H__
#define __TLOCALFILESYSTEMFILE_H__

#include "libchcore.h"
#include "TPath.h"
#include "TOverlappedDataBuffer.h"
#include "IFilesystemFile.h"

namespace chcore
{
	class LIBCHCORE_API TLocalFilesystemFile : public IFilesystemFile
	{
	public:
		virtual ~TLocalFilesystemFile();

		virtual bool OpenExistingForReading(bool bNoBuffering) override;
		virtual bool CreateNewForWriting(bool bNoBuffering) override;
		virtual bool OpenExistingForWriting(bool bNoBuffering) override;

		virtual bool Truncate(long long llNewSize) override;

		virtual bool ReadFile(TOverlappedDataBuffer& rBuffer) override;
		virtual bool WriteFile(TOverlappedDataBuffer& rBuffer) override;
		virtual bool FinalizeFile(TOverlappedDataBuffer& rBuffer) override;

		virtual bool IsOpen() const  override { return m_hFile != INVALID_HANDLE_VALUE; }
		virtual unsigned long long GetFileSize() const override;
		virtual TSmartPath GetFilePath() const override;

		virtual void Close() override;

	private:
		TLocalFilesystemFile(const TSmartPath& pathFile);
		DWORD GetFlagsAndAttributes(bool bNoBuffering) const;

	private:
		TSmartPath m_pathFile;
		HANDLE m_hFile;
		bool m_bNoBuffering;

		friend class TLocalFilesystem;
	};
}

#endif
