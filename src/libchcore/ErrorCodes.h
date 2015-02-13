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
	eErr_InvalidData = 6,
	eErr_InvalidPointer = 7,

	// shared memory (500+)
	eErr_CannotOpenSharedMemory = 500,
	eErr_SharedMemoryNotOpen = 501,
	eErr_SharedMemoryInvalidFormat = 502,
	eErr_SharedMemoryAlreadyExists = 503,

	// threading (1000+)
	eErr_MutexTimedOut = 1000,
	eErr_CannotCreateEvent = 1001,
	eErr_ThreadAlreadyStarted = 1002,
	eErr_CannotResetEvent = 1003,
	eErr_CannotCreateThread = 1004,
	eErr_CannotChangeThreadPriority = 1005,
	eErr_CannotResumeThread = 1006,
	eErr_WaitingFailed = 1007,
	eErr_CannotSuspendThread = 1008,
	eErr_CannotSetEvent = 1009,

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
	eErr_InvalidSerializationData = 2507,
	eErr_CannotSetDatabaseOptions = 2508,
	eErr_InvalidSerializer = 2509,

	// Filesystem errors (3000+)
	eErr_FixedDriveWithoutDriveLetter = 3000,
	eErr_CannotGetFileInfo = 3001,
	eErr_CannotDeleteFile = 3002,

	// Task handling errors (4000+)
	eErr_MissingTaskSerializationPath = 4000,
	eErr_UndefinedOperation = 4001,

	// Memory allocation/deallocation (4500+)
	eErr_CannotAllocateMemory = 4500,

	// database errors (5000+)
	eErr_SQLiteCannotOpenDatabase = 5000,
	eErr_SQLiteFinalizeError = 5001,
	eErr_SQLitePrepareError = 5002,
	eErr_SQLiteStatementNotPrepared = 5003,
	eErr_SQLiteStepError = 5004,
	eErr_SQLiteBindError = 5005,
	eErr_SQLiteNoRowAvailable = 5006,
	eErr_SQLiteCannotBeginTransaction = 5007,
	eErr_SQLiteCannotRollbackTransaction = 5008,
	eErr_SQLiteCannotCommitTransaction = 5009,
	eErr_SQLiteReset = 5010,
};

END_CHCORE_NAMESPACE

#endif
