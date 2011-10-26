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
/// @file  TSubTaskBase.h
/// @date  2010/09/18
/// @brief Contains declarations of common elements of sub-operations.
// ============================================================================
#ifndef __TSUBTASKBASE_H__
#define __TSUBTASKBASE_H__

#include "../libchcore/FileInfo.h"

class TSubTaskContext;
class TBasePathDataContainer;

///////////////////////////////////////////////////////////////////////////
// TSubTaskBase

class TSubTaskBase
{
public:
	enum ESubOperationResult
	{
		eSubResult_Continue,
		eSubResult_KillRequest,
		eSubResult_Error,
		eSubResult_CancelRequest,
		eSubResult_PauseRequest
	};

public:
	TSubTaskBase(TSubTaskContext& rContext);
	virtual ~TSubTaskBase();

	virtual ESubOperationResult Exec() = 0;

	TSubTaskContext& GetContext() { return m_rContext; }
	const TSubTaskContext& GetContext() const { return m_rContext; }

protected:
	// some common operations
	chcore::TSmartPath CalculateDestinationPath(const chcore::TFileInfoPtr& spFileInfo, chcore::TSmartPath strPath, int iFlags) const;
	chcore::TSmartPath FindFreeSubstituteName(chcore::TSmartPath pathSrcPath, chcore::TSmartPath pathDstPath) const;

private:
	TSubTaskContext& m_rContext;
};

#endif
