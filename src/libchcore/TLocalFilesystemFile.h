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
#include "..\liblogger\TLogger.h"

namespace chcore
{
	class TFileInfo;

	class LIBCHCORE_API TLocalFilesystemFile : public IFilesystemFile
	{
	public:
		virtual ~TLocalFilesystemFile();

		virtual void CreateNewForWriting() override;
		virtual void OpenExistingForWriting() override;

		virtual void Truncate(file_size_t fsNewSize) override;

		virtual void ReadFile(TOverlappedDataBuffer& rBuffer) override;
		virtual void WriteFile(TOverlappedDataBuffer& rBuffer) override;
		virtual void FinalizeFile(TOverlappedDataBuffer& rBuffer) override;

		virtual bool IsOpen() const  override { return m_hFile != INVALID_HANDLE_VALUE; }
		virtual file_size_t GetFileSize() override;
		virtual void GetFileInfo(TFileInfo& tFileInfo) override;

		virtual TSmartPath GetFilePath() const override;

		virtual void Close() override;
		virtual file_size_t GetSeekPositionForResume(file_size_t fsLastAvailablePosition) override;

	private:
		TLocalFilesystemFile(EOpenMode eMode, const TSmartPath& pathFile, bool bNoBuffering, const logger::TLogFileDataPtr& spLogFileData);

		void EnsureOpen();

		void OpenExistingForReading();

		DWORD GetFlagsAndAttributes(bool bNoBuffering) const;
		void OpenExistingForWriting(bool bNoBuffering);

		void InternalClose();

		std::wstring TLocalFilesystemFile::GetFileInfoForLog(bool bNoBuffering) const;

	private:
		TSmartPath m_pathFile;
		HANDLE m_hFile;
		EOpenMode m_eMode;
		bool m_bNoBuffering;
#pragma warning(push)
#pragma warning(disable: 4251)
		logger::TLoggerPtr m_spLog;
#pragma warning(pop)

		friend class TLocalFilesystem;
	};
}

#endif
