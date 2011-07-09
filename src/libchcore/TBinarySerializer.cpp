// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
/// @file  TBinarySerializer.h
/// @date  2011/06/20
/// @brief Contains declaration of TBinarySerializer class.
// ============================================================================
#include "stdafx.h"
#include "TBinarySerializer.h"
#include <fstream>
#include <ios>

#include "TPath.h"
#include "ErrorCodes.h"

BEGIN_CHCORE_NAMESPACE

/////////////////////////////////////////////////////////////////////////////////////////
// class TReadBinarySerializer

TReadBinarySerializer::TReadBinarySerializer()
{
}

TReadBinarySerializer::~TReadBinarySerializer()
{
	try
	{
		Close();
	}
	catch(...)
	{
	}
}

void TReadBinarySerializer::Init(const TSmartPath& pathFile)
{
	Close();

	try
	{
		m_spInputStream = boost::make_shared<std::ifstream>(pathFile.ToString(), std::ios_base::in | std::ios_base::binary);
		m_spArchive = boost::make_shared<boost::archive::binary_iarchive>(std::tr1::ref(*m_spInputStream));
	}
	catch(std::exception& e)
	{
		m_spArchive.reset();
		m_spInputStream.reset();

		THROW_CORE_EXCEPTION_STD(eErr_CannotReadArchive, e);
	}
}

void TReadBinarySerializer::Close()
{
	// order is important - first close the archive, then input stream
	m_spArchive.reset();
	m_spInputStream.reset();
}

void TReadBinarySerializer::Load(bool& bValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & bValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(short& shValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & shValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(unsigned short& ushValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & ushValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(int& iValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & iValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(unsigned int& uiValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & uiValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(long& lValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & lValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(unsigned long& ulValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & ulValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(long long& llValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & llValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(unsigned long long& ullValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & ullValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(float& fValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & fValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(double& dValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & dValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

void TReadBinarySerializer::Load(TString& strValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		std::wstring wstrData;
		(*m_spArchive) & wstrData;
		strValue = wstrData.c_str();
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeLoadError, e);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// class TWriteBinarySerializer

TWriteBinarySerializer::TWriteBinarySerializer()
{
}

TWriteBinarySerializer::~TWriteBinarySerializer()
{
	try
	{
		Close();
	}
	catch(...)
	{
	}
}

void TWriteBinarySerializer::Init(const TSmartPath& pathFile)
{
	try
	{
		m_spOutputStream = boost::make_shared<std::ofstream>(pathFile.ToString(), std::ios_base::out | std::ios_base::binary);
		m_spArchive = boost::make_shared<boost::archive::binary_oarchive>(std::tr1::ref(*m_spOutputStream));
	}
	catch(std::exception&)
	{
		m_spArchive.reset();
		m_spOutputStream.reset();
		THROW_CORE_EXCEPTION(eErr_CannotWriteArchive);
	}
}

void TWriteBinarySerializer::Close()
{
	m_spArchive.reset();
	m_spOutputStream.reset();
}

void TWriteBinarySerializer::Store(bool bValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & bValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(short shValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & shValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(unsigned short ushValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & ushValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(int iValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & iValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(unsigned int uiValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & uiValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(long lValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & lValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(unsigned long ulValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & ulValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(long long llValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & llValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(unsigned long long ullValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & ullValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(float fValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & fValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(double dValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		(*m_spArchive) & dValue;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(const TString& strValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		std::wstring wstrData((const wchar_t*)strValue);
		(*m_spArchive) & wstrData;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

void TWriteBinarySerializer::Store(const wchar_t* strValue)
{
	if(!m_spArchive)
		THROW_CORE_EXCEPTION(eErr_UseOfUninitializedObject);

	try
	{
		std::wstring wstrData(strValue);
		(*m_spArchive) & wstrData;
	}
	catch (std::exception& e)
	{
		THROW_CORE_EXCEPTION_STD(eErr_SerializeStoreError, e);
	}
}

END_CHCORE_NAMESPACE
