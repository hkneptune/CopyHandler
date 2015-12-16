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
#ifndef __TFILESYSTEMFEEDBACKWRAPPER_H__
#define __TFILESYSTEMFEEDBACKWRAPPER_H__

#include "libchcore.h"
#include "IFilesystem.h"
#include "TSubTaskBase.h"
#include "..\libicpf\log.h"

namespace chcore
{
	class TFilesystemFeedbackWrapper
	{
	public:
		TFilesystemFeedbackWrapper(const IFilesystemPtr& spFilesystem, icpf::log_file& rLog);

		TSubTaskBase::ESubOperationResult CreateDirectoryFB(const IFeedbackHandlerPtr& spFeedbackHandler, const TSmartPath& pathDirectory);
		TSubTaskBase::ESubOperationResult CheckForFreeSpaceFB(const IFeedbackHandlerPtr& spFeedbackHandler, const TSmartPath& pathFirstSrc, const TSmartPath& pathDestination, unsigned long long ullNeededSize);

	private:
		IFilesystemPtr m_spFilesystem;
		icpf::log_file& m_rLog;
	};
}

#endif
