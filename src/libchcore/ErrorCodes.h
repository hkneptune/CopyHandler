/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#ifndef __ERROR_CODES_H__
#define __ERROR_CODES_H__

#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

enum EGeneralErrors
{
	// general errors
	eErr_Success = 0,
	eErr_BoundsExceeded = 1,
	eErr_InvalidArgument = 2,
	eErr_UnhandledCase = 3,
	eErr_InternalProblem = 4,
	eErr_UseOfUninitializedObject = 5,

	// shared memory (500+)
	eErr_CannotOpenSharedMemory = 500,
	eErr_SharedMemoryNotOpen = 501,
	eErr_SharedMemoryInvalidFormat = 502,
	eErr_SharedMemoryAlreadyExists = 503,

	// threading (1000+)
	eErr_MutexTimedOut = 1000,

	// string errors (1500+)

	// Task definition errors (2000+)
	eErr_UnsupportedVersion = 2000,
	eErr_MissingXmlData = 2001,

	// Serialization errors (2500+)
	eErr_CannotReadArchive = 2500,
	eErr_SerializeLoadError = 2501,
	eErr_SerializeStoreError = 2502,
	eErr_ContainerObjectMismatch = 2503,
	eErr_NodeDoesNotExist = 2504,
	eErr_UnsupportedMultipleSubnodesLevels = 2505,
	eErr_CannotWriteArchive = 2506,
};

END_CHCORE_NAMESPACE

#endif
