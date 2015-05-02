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
#include "stdafx.h"
#include "TFakeFileSerializer.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

TFakeFileSerializer::TFakeFileSerializer(const TSmartPath& rPath) :
	m_pathFileSerializer(rPath)
{
}

TFakeFileSerializer::~TFakeFileSerializer()
{
}

TSmartPath TFakeFileSerializer::GetLocation() const
{
	return m_pathFileSerializer;
}

ISerializerContainerPtr TFakeFileSerializer::GetContainer(const TString& /*strContainerName*/)
{
	throw TCoreException(eErr_InvalidSerializer, m_pathFileSerializer.ToString(), __LINE__, __FUNCTIONW__);
}

void TFakeFileSerializer::Flush()
{
	throw TCoreException(eErr_InvalidSerializer, m_pathFileSerializer.ToString(), __LINE__, __FUNCTIONW__);
}

END_CHCORE_NAMESPACE
