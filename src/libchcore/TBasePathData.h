// ============================================================================
//  Copyright (C) 2001-2010 by Jozef Starosczyk
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
/// @file  TBasePathData.h
/// @date  2010/10/13
/// @brief Contains declarations of classes related to keeping additional path data.
// ============================================================================
#ifndef __TBASEPATHDATA_H__
#define __TBASEPATHDATA_H__

#include "libchcore.h"
#include "TPath.h"
#include "TModPathContainer.h"

BEGIN_CHCORE_NAMESPACE

/////////////////////////////////////////////////////////////////////////////
// TBasePathData
class LIBCHCORE_API TBasePathData
{
public:
	TBasePathData();
	TBasePathData(const TBasePathData& rEntry);

	bool GetSkipFurtherProcessing() const { return m_bSkipFurtherProcessing; }
	void SetSkipFurtherProcessing(bool bSkipFurtherProcessing) { m_bSkipFurtherProcessing = bSkipFurtherProcessing; }

	void SetDestinationPath(const TSmartPath& strPath);
	TSmartPath GetDestinationPath() const;
	bool IsDestinationPathSet() const { return !m_pathDst.IsEmpty(); }

	void Serialize(TReadBinarySerializer& rSerializer, bool bData);
	void Serialize(TWriteBinarySerializer& rSerializer, bool bData);

private:
	bool m_bSkipFurtherProcessing;	// specifies if the path should be (or not) processed further
	TSmartPath m_pathDst;	// dest path
};

typedef boost::shared_ptr<TBasePathData> TBasePathDataPtr;

//////////////////////////////////////////////////////////////////////////
// TBasePathDataContainer

class LIBCHCORE_API TBasePathDataContainer
{
public:
	// constructors/destructor
	TBasePathDataContainer();
	~TBasePathDataContainer();

	// standard access to data
	bool Exists(size_t stObjectID) const;
	TBasePathDataPtr GetExisting(size_t stObjectID) const;
	TBasePathDataPtr Get(size_t stObjectID);

	void Remove(size_t stObjectID);
	void Clear();

	// inner object read interface (to not create new inner objects when reading non-existent data)
	bool GetSkipFurtherProcessing(size_t stObjectID) const;
	TSmartPath GetDestinationPath(size_t stObjectID) const;
	bool IsDestinationPathSet(size_t stObjectID) const;

private:
	TBasePathDataContainer(const TBasePathDataContainer& rSrc);
	TBasePathDataContainer& operator=(const TBasePathDataContainer& rSrc);

protected:
#pragma warning(push)
#pragma warning(disable: 4251)
	typedef std::map<size_t, TBasePathDataPtr> MapEntries;
	MapEntries m_mapEntries;
	mutable boost::shared_mutex m_lock;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif // __TBASEPATHDATA_H__
