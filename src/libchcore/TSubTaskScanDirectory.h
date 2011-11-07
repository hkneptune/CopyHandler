// ============================================================================
//  Copyright (C) 2001-2009 by Jozef Starosczyk
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
/// @file  TSubTaskScanDirectory.h
/// @date  2010/09/18
/// @brief Contains declarations of classes responsible for directory scan sub-operation.
// ============================================================================
#ifndef __TSUBTASKSCANDIRECTORY_H__
#define __TSUBTASKSCANDIRECTORY_H__

#include "libchcore.h"
#include "TSubTaskBase.h"
#include "TPath.h"
#include "FileFilter.h"

BEGIN_CHCORE_NAMESPACE

///////////////////////////////////////////////////////////////////////////
// TSubTaskScanDirectories

class LIBCHCORE_API TSubTaskScanDirectories : public TSubTaskBase
{
public:
	TSubTaskScanDirectories(TSubTaskContext& rContext);
	virtual ~TSubTaskScanDirectories();

	virtual ESubOperationResult Exec();

private:
	int ScanDirectory(TSmartPath pathDirName, size_t stSrcIndex, bool bRecurse, bool bIncludeDirs, TFiltersArray& afFilters);
};

END_CHCORE_NAMESPACE

#endif