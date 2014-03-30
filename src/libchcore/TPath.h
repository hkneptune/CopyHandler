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
#ifndef __TPATH_H__
#define __TPATH_H__

#include "libchcore.h"
#include "TConfig.h"

BEGIN_CHCORE_NAMESPACE

class TSmartPath;
class TPathContainer;

namespace details
{
	class TPath
	{
	public:
		TPath();
		TPath(const TPath& rSrc);
		~TPath();

		long AddRef() { return ++m_lRefCount; }
		long Release();
		bool IsShared() const { return m_lRefCount > 1; }

	protected:
		static TPath* New();
		TPath* Clone();
		static void Delete(TPath* pPath);

	protected:
#pragma warning(push)
#pragma warning(disable: 4251)
		TString m_strPath;
#pragma warning(pop)
		long m_lRefCount;

		friend class TSmartPath;
	};
}

class LIBCHCORE_API TSmartPath
{
protected:
	BOOST_STATIC_CONSTANT(bool, DefaultCaseSensitivity = false);

public:
	// Construction/destruction
	TSmartPath();
	TSmartPath(const TSmartPath& spPath);

	~TSmartPath();

	// operators
	TSmartPath& operator=(const TSmartPath& spPath);

	bool operator==(const TSmartPath& rPath) const;
	bool operator<(const TSmartPath& rPath) const;
	bool operator>(const TSmartPath& rPath) const;

	TSmartPath operator+(const TSmartPath& rPath) const;
	TSmartPath& operator+=(const TSmartPath& rPath);

	// from/to string conversions
	void FromString(const wchar_t* pszPath);
	void FromString(const TString& strPath);

	const wchar_t* ToString() const;
	TString ToWString() const;

	// other operations
	void Clear() throw();

	TSmartPath AppendCopy(const TSmartPath& pathToAppend, bool bEnsurePathSeparatorExists = true) const;
	TSmartPath& Append(const TSmartPath& pathToAppend, bool bEnsurePathSeparatorExists = true);

	void SplitPath(TPathContainer& vComponents) const;

	int Compare(const TSmartPath& rPath, bool bCaseSensitive = DefaultCaseSensitivity) const;
	bool IsChildOf(const TSmartPath& rPath, bool bCaseSensitive = DefaultCaseSensitivity) const;

	bool MakeRelativePath(const TSmartPath& rReferenceBasePath, bool bCaseSensitive = DefaultCaseSensitivity);
	bool MakeAbsolutePath(const TSmartPath& rReferenceBasePath);

	void AppendIfNotExists(const wchar_t* pszPostfix, bool bCaseSensitive = DefaultCaseSensitivity);
	void CutIfExists(const wchar_t* pszPostfix, bool bCaseSensitive = DefaultCaseSensitivity);

	bool IsNetworkPath() const;
	bool IsRelativePath() const;

	bool IsDrive() const;
	bool HasDrive() const;
	TSmartPath GetDrive() const;		// c: for c:\windows\test.txt
	wchar_t GetDriveLetter() const;		// 'c' for c:\windows\test.txt, null for non-drive based paths

	bool IsServerName() const;
	bool HasServerName() const;
	TSmartPath GetServerName() const;

	bool HasFileRoot() const;
	TSmartPath GetFileRoot() const;		// "c:\windows\" for "c:\windows\test.txt"

	bool HasFileDir() const;				// \windows\ for c:\windows\test.txt
	TSmartPath GetFileDir() const;			// \windows\ for c:\windows\test.txt

	bool HasFileTitle() const;				// test for c:\windows\test.txt
	TSmartPath GetFileTitle() const;		// test for c:\windows\test.txt

	bool HasExtension() const;				// txt for c:\windows\test.txt
	TSmartPath GetExtension() const;		// txt for c:\windows\test.txt

	bool HasFileName() const;				// test.txt for c:\windows\test.txt
	TSmartPath GetFileName() const;			// test.txt for c:\windows\test.txt
	void DeleteFileName();			// test.txt for c:\windows\test.txt

	bool EndsWithSeparator() const;
	void AppendSeparatorIfDoesNotExist();
	void StripSeparatorAtEnd();

	bool StartsWithSeparator() const;
	void PrependSeparatorIfDoesNotExist();
	void StripSeparatorAtFront();

	bool IsEmpty() const;
	size_t GetLength() const;

	// Serialization
	void Serialize(TReadBinarySerializer& rSerializer);
	void Serialize(TWriteBinarySerializer& rSerializer) const;

	void StoreInConfig(TConfig& rConfig, PCTSTR pszPropName) const;
	bool ReadFromConfig(const TConfig& rConfig, PCTSTR pszPropName);

protected:
	void PrepareToWrite();

	static bool IsSeparator(wchar_t wchSeparator);

protected:
	details::TPath* m_pPath;
};

LIBCHCORE_API TSmartPath PathFromString(const wchar_t* pszPath);
LIBCHCORE_API TSmartPath PathFromWString(const TString& strPath);

END_CHCORE_NAMESPACE

CONFIG_MEMBER_SERIALIZATION(TSmartPath)

#endif
