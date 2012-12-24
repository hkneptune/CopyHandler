// ============================================================================
//  Copyright (C) 2001-2011 by Jozef Starosczyk
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
/// @file  TBinarySerializer.h
/// @date  2011/06/20
/// @brief Contains declaration of TBinarySerializer class.
// ============================================================================
#ifndef __TBINARYSERIALIZER_H__
#define __TBINARYSERIALIZER_H__

#include "libchcore.h"
#include <boost\smart_ptr\make_shared.hpp>
#include <boost/mpl/set.hpp>
#include <stack>
#pragma warning(push)
#pragma warning(disable: 4996 4310 4244)
	#include <boost\archive\binary_iarchive.hpp>
	#include <boost\archive\binary_oarchive.hpp>
#pragma warning(pop)

BEGIN_CHCORE_NAMESPACE

class TSmartPath;
class TString;

class LIBCHCORE_API TReadBinarySerializer
{
public:
	typedef boost::mpl::set<bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, TString> NativeTypes;

public:
	TReadBinarySerializer();
	~TReadBinarySerializer();

	void Init(const TSmartPath& pathFile);
	void Close();

	void Load(bool& bValue);
	void Load(short& shValue);
	void Load(unsigned short& ushValue);
	void Load(int& iValue);
	void Load(unsigned int& uiValue);
	void Load(long& lValue);
	void Load(unsigned long& ulValue);
	void Load(long long& llValue);
	void Load(unsigned long long& ullValue);
	void Load(float& fValue);
	void Load(double& dValue);
	void Load(TString& strValue);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	boost::shared_ptr<std::ifstream> m_spInputStream;
	boost::shared_ptr<boost::archive::binary_iarchive> m_spArchive;
#pragma warning(pop)
};

class LIBCHCORE_API TWriteBinarySerializer
{
public:
	typedef boost::mpl::set<bool, short, unsigned short, int, unsigned int, long, unsigned long, long long, unsigned long long, float, double, TString, const wchar_t*> NativeTypes;

public:
	TWriteBinarySerializer();
	~TWriteBinarySerializer();

	void Init(const TSmartPath& pathFile);
	void Close();

	void Store(bool bValue);
	void Store(short shValue);
	void Store(unsigned short ushValue);
	void Store(int iValue);
	void Store(unsigned int uiValue);
	void Store(long lValue);
	void Store(unsigned long ulValue);
	void Store(long long llValue);
	void Store(unsigned long long ullValue);
	void Store(float fValue);
	void Store(double dValue);
	void Store(const TString& strValue);
	void Store(const wchar_t* strValue);

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	boost::shared_ptr<std::ofstream> m_spOutputStream;
	boost::shared_ptr<boost::archive::binary_oarchive> m_spArchive;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
