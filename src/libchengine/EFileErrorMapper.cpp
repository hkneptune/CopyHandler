{ // ============================================================================
//  Copyright (C) 2001-2020 by Jozef Starosczyk
//  ixen {at} copyhandler [dot] com
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
#include "stdafx.h"
#include "EFileErrorMapper.h"

namespace chengine
{
	const serializer::DataMapper<EFileError>& GetFileErrorMapper()
	{
		static serializer::DataMapper<EFileError> compareTypeMapper = {
			{ eDeleteError, L"delete" },
			{ eSeekError, L"seek" },
			{ eResizeError, L"resize" },
			{ eReadError, L"read" },
			{ eWriteError, L"write" },
			{ eFinalizeError, L"finalize" },
			{ eFastMoveError, L"fast-move" },
			{ eCreateError, L"create" },
			{ eCheckForFreeSpace, L"check-free-space" },
			{ eRetrieveFileInfo, L"retrieve-file-info" },
		};

		return compareTypeMapper;
	}
}
