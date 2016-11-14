// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#ifndef __TOVERLAPPEDREADERFB_H__
#define __TOVERLAPPEDREADERFB_H__

#include "TOverlappedReader.h"
#include "TFilesystemFileFeedbackWrapper.h"

namespace chcore
{
	class TFilesystemFileFeedbackWrapper;

	class TOverlappedReaderFB
	{
	public:
		TOverlappedReaderFB(const TFilesystemFileFeedbackWrapperPtr& spSrcFile, const TSubTaskStatsInfoPtr& spStats,
			const TFileInfoPtr& spSrcFileInfo,
			const logger::TLogFileDataPtr& spLogFileData, const TBufferListPtr& spEmptyBuffers,
			unsigned long long ullFilePos, DWORD dwChunkSize);
		~TOverlappedReaderFB();

		TOverlappedReaderPtr GetReader() const { return m_spReader; }
		void SetReleaseMode();

		TSubTaskBase::ESubOperationResult OnReadPossible();
		TSubTaskBase::ESubOperationResult OnReadFailed();

	private:
		TOverlappedReaderPtr m_spReader;
		TFilesystemFileFeedbackWrapperPtr m_spSrcFile;
		TSubTaskStatsInfoPtr m_spStats;
		TFileInfoPtr m_spSrcFileInfo;
	};

	using TOverlappedReaderFBPtr = std::shared_ptr<TOverlappedReaderFB>;
}

#endif
