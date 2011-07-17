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

#include "../libchcore/TPath.h"

/////////////////////////////////////////////////////////////////////////////
// TBasePathData
class TBasePathData
{
public:
	TBasePathData();
	TBasePathData(const TBasePathData& rEntry);

	void SetMove(bool bValue) { m_bMove=bValue; }
	bool GetMove() const { return m_bMove; }

	int GetDriveNumber() { return m_iDriveNumber; }
	void SetDriveNumber(int iDriveNumber) { m_iDriveNumber = iDriveNumber; }
	bool IsDriveNumberSet() const { return m_iDriveNumber != -2; }

	void SetDestinationPath(const chcore::TSmartPath& strPath);
	chcore::TSmartPath GetDestinationPath() const;
	bool IsDestinationPathSet() const { return !m_pathDst.IsEmpty(); }

	void Serialize(chcore::TReadBinarySerializer& rSerializer, bool bData);
	void Serialize(chcore::TWriteBinarySerializer& rSerializer, bool bData);

private:
	bool m_bMove;					// specifies if we can use MoveFile (if will be moved)

	int m_iDriveNumber;		// disk number (default -1 - none)

	chcore::TSmartPath m_pathDst;	// dest path
};

typedef boost::shared_ptr<TBasePathData> TBasePathDataPtr;

//////////////////////////////////////////////////////////////////////////
// TBasePathDataContainer

class TBasePathDataContainer
{
public:
	// constructors/destructor
	explicit TBasePathDataContainer(const chcore::TPathContainer& tBasePaths);
	~TBasePathDataContainer();

	// standard access to data
	TBasePathDataPtr GetAt(size_t iPos) const;

	void SetCount(size_t stCount);
	size_t GetCount() const;
	void Add(const TBasePathDataPtr& pEntry);
	void SetAt(size_t nIndex, const TBasePathDataPtr& pEntry);
	void RemoveAt(size_t nIndex, size_t nCount = 1);
	void Clear();

	// serialization
	void Serialize(chcore::TReadBinarySerializer& rSerializer, bool bData);
	void Serialize(chcore::TWriteBinarySerializer& rSerializer, bool bData);

private:
	TBasePathDataContainer(const TBasePathDataContainer& rSrc);
	TBasePathDataContainer& operator=(const TBasePathDataContainer& rSrc);

protected:
	std::vector<TBasePathDataPtr> m_vEntries;
	const chcore::TPathContainer& m_tBasePaths;

	mutable boost::shared_mutex m_lock;
};

#endif // __TBASEPATHDATA_H__
