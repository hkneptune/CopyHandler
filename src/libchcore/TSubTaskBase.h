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

#include "libchcore.h"
#include "FileInfo.h"

BEGIN_CHCORE_NAMESPACE

class TSubTaskContext;

///////////////////////////////////////////////////////////////////////////
// TSubTaskBase

class LIBCHCORE_API TSubTaskBase
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
	TSmartPath CalculateDestinationPath(const TFileInfoPtr& spFileInfo, TSmartPath strPath, int iFlags) const;
	TSmartPath FindFreeSubstituteName(TSmartPath pathSrcPath, TSmartPath pathDstPath) const;

private:
	TSubTaskBase(const TSubTaskBase&);
	TSubTaskBase& operator=(const TSubTaskBase&);

private:
	TSubTaskContext& m_rContext;
};

END_CHCORE_NAMESPACE

#endif
