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
#ifndef __TFAKEFILESYSTEMFILE_H__
#define __TFAKEFILESYSTEMFILE_H__

#include "libchcore.h"
#include "IFilesystemFile.h"
#include "TFakeFileDescription.h"

namespace chcore
{
	class TFakeFilesystem;

	class LIBCHCORE_API TFakeFilesystemFile : public IFilesystemFile
	{
	public:
		TFakeFilesystemFile(const TSmartPath& pathFile, TFakeFilesystem* pFilesystem);
		~TFakeFilesystemFile();

		virtual bool OpenExistingForReading(bool bNoBuffering) override;
		virtual bool CreateNewForWriting(bool bNoBuffering) override;
		virtual bool OpenExistingForWriting(bool bNoBuffering) override;
		virtual bool Truncate(long long ullNewSize) override;
		virtual bool ReadFile(TOverlappedDataBuffer& rBuffer) override;

		void GenerateBufferContent(TOverlappedDataBuffer &rBuffer);

		virtual bool WriteFile(TOverlappedDataBuffer& rBuffer) override;
		virtual bool FinalizeFile(TOverlappedDataBuffer& rBuffer) override;
		virtual bool IsOpen() const override;
		virtual unsigned long long GetFileSize() const override;
		virtual void Close() override;

		virtual TSmartPath GetFilePath() const override;

	private:
#pragma warning(push)
#pragma warning(disable: 4251)
		TSmartPath m_pathFile;
#pragma warning(pop)
		bool m_bIsOpen;
		bool m_bNoBuffering;
		bool m_bModeReading;
		TFakeFilesystem* m_pFilesystem;
	};
}

#endif
