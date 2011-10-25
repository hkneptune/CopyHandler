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
	memcmp(m_auiSizes, bsSizes.m_auiSizes, sizeof(TBufferSizes));
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

TDataBuffer::TDataBuffer() :
	m_pBuffer(NULL),
	m_uiRealSize(0)
{
}

TDataBuffer::~TDataBuffer()
{
	Delete();
}

const TBufferSizes& TDataBuffer::Create(const TBufferSizes& rbsSizes)
{
	// if trying to set 0-size buffer
	TBufferSizes bsSizes = rbsSizes;	// copy - not to mix in the def. param

	for(int iIndex = TBufferSizes::eBuffer_Default; iIndex < TBufferSizes::eBuffer_Last; ++iIndex)
	{
		TBufferSizes::EBufferType eType = (TBufferSizes::EBufferType)iIndex;
		if(bsSizes.GetSizeByType(eType) == 0)
			bsSizes.SetSizeByType(eType, DEFAULT_SIZE);
	}
	
	// max value from the all
	UINT uiLargest = 0;
	if(bsSizes.IsOnlyDefault())
		uiLargest = bsSizes.GetDefaultSize();
	else
	{
		for(int iIndex = TBufferSizes::eBuffer_Default; iIndex < TBufferSizes::eBuffer_Last; ++iIndex)
		{
			TBufferSizes::EBufferType eType = (TBufferSizes::EBufferType)iIndex;

			if(uiLargest < bsSizes.GetSizeByType(eType))
				uiLargest = bsSizes.GetSizeByType(eType);
		}
	}
	
	// modify buffer size to the next 64k boundary
	UINT uiRealSize = ROUNDTODS(uiLargest);
	if(m_uiRealSize == uiRealSize)
	{
		// real buffersize hasn't changed
		m_bsSizes = bsSizes;

		return m_bsSizes;
	}

	// try to allocate buffer
	LPVOID pBuffer = VirtualAlloc(NULL, uiRealSize, MEM_COMMIT, PAGE_READWRITE);
	if(pBuffer == NULL)
	{
		if(m_pBuffer == NULL)
		{
			// try safe buffesize
			pBuffer = VirtualAlloc(NULL, DEFAULT_SIZE, MEM_COMMIT, PAGE_READWRITE);
			if(pBuffer == NULL)
				return m_bsSizes;		// do not change anything
			
			// delete old buffer
			Delete();
			
			// store data
			m_pBuffer = static_cast<unsigned char*>(pBuffer);
			m_uiRealSize = DEFAULT_SIZE;
			m_bsSizes.SetOnlyDefault(bsSizes.IsOnlyDefault());
			m_bsSizes.SetDefaultSize(DEFAULT_SIZE);
			m_bsSizes.SetOneDiskSize(DEFAULT_SIZE);
			m_bsSizes.SetTwoDisksSize(DEFAULT_SIZE);
			m_bsSizes.SetCDSize(DEFAULT_SIZE);
			m_bsSizes.SetLANSize(DEFAULT_SIZE);
			
			return m_bsSizes;
		}
		else
		{
			// no new buffer could be created - leave the old one
			return m_bsSizes;
		}
	}
	else
	{
		// succeeded
		Delete();	// get rid of old buffer
		
		// store data
		m_pBuffer = static_cast<unsigned char*>(pBuffer);
		m_uiRealSize = uiRealSize;
		m_bsSizes = bsSizes;
		
		return m_bsSizes;
	}
}

void TDataBuffer::Delete()
{
	if(m_pBuffer != NULL)
	{
		VirtualFree(static_cast<LPVOID>(m_pBuffer), 0, MEM_RELEASE);
		m_pBuffer = NULL;
		m_uiRealSize = 0;
		m_bsSizes.Clear();
	}
}

void TDataBuffer::CutDataFromBuffer(UINT uiCount)
{
	if(uiCount >= m_uiRealSize || !m_pBuffer)
		return;	// nothing to do

	memmove(m_pBuffer, m_pBuffer + uiCount, m_uiRealSize - uiCount);
}

END_CHCORE_NAMESPACE
