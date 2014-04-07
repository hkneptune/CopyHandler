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
#include "stdafx.h"
#include "DataBuffer.h"
#include "TBinarySerializer.h"
#include "SerializationHelpers.h"
#include "TCoreException.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

TBufferSizes::TBufferSizes() :
	m_bOnlyDefault(false)
{
	memset(m_auiSizes, 0, sizeof(m_auiSizes));
}

void TBufferSizes::SerializeLoad(TReadBinarySerializer& rSerializer)
{
	using Serializers::Serialize;

	for(int iIndex = 0; iIndex < eBuffer_Last; ++iIndex)
	{
		Serialize(rSerializer, m_auiSizes[iIndex]);
	}
	Serialize(rSerializer, m_bOnlyDefault);
}

void TBufferSizes::SerializeStore(TWriteBinarySerializer& rSerializer)
{
	using Serializers::Serialize;

	for(int iIndex = 0; iIndex < eBuffer_Last; ++iIndex)
	{
		Serialize(rSerializer, m_auiSizes[iIndex]);
	}

	Serialize(rSerializer, m_bOnlyDefault);
}

bool TBufferSizes::operator==(const TBufferSizes& bsSizes) const
{
	for(int iIndex = 0; iIndex < eBuffer_Last; ++iIndex)
	{
		if(m_auiSizes[iIndex] != bsSizes.m_auiSizes[iIndex])
			return false;
	}

	return true;
}

void TBufferSizes::Clear()
{
	memset(m_auiSizes, 0, sizeof(m_auiSizes));
	//m_bOnlyDefault = false;	// disabled because in the earlier code it wasn't reset too
}

UINT TBufferSizes::GetSizeByType(EBufferType eType) const
{
	if(eType < eBuffer_Default || eType > eBuffer_LAN)
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);

	return m_auiSizes[eType];
}

void TBufferSizes::SetSizeByType(EBufferType eType, UINT uiSize)
{
	if(eType < eBuffer_Default || eType > eBuffer_LAN)
		THROW_CORE_EXCEPTION(eErr_BoundsExceeded);
	
	m_auiSizes[eType] = uiSize;
}

UINT TBufferSizes::GetMaxSize() const
{
	if(m_bOnlyDefault)
		return m_auiSizes[eBuffer_Default];
	else
	{
		UINT uiMaxSize = 0;
		for(size_t stIndex = 0; stIndex < eBuffer_Last; ++stIndex)
		{
			if(m_auiSizes[stIndex] > uiMaxSize)
				uiMaxSize = m_auiSizes[stIndex];
		}
		return uiMaxSize;
	}
}

END_CHCORE_NAMESPACE
