// ============================================================================
//  Copyright (C) 2001-2013 by Jozef Starosczyk
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
#ifndef __TTASKSERIALIZER_H__
#define __TTASKSERIALIZER_H__

#include "libchcore.h"
#include "ITaskSerializer.h"
#include "TString.h"
#include "TSQLiteDatabase.h"
#include "TPath.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TTaskSerializer : public ITaskSerializer
{
public:
	TTaskSerializer(const TSmartPath& pathDB);
	~TTaskSerializer();

	virtual TSmartPath GetPath() const;

	virtual void Setup();

private:
	TSmartPath m_pathDB;

#pragma warning(push)
#pragma warning(disable: 4251)
	sqlite::TSQLiteDatabasePtr m_spDatabase;
#pragma warning(pop)
};

typedef boost::shared_ptr<TTaskSerializer> TTaskSerializerPtr;

END_CHCORE_NAMESPACE

#endif
