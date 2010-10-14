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

#include <boost/serialization/split_member.hpp>
#include "libchcore.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API TPath
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
	tstring_t m_strPath;
	long m_lRefCount;

	friend class TSmartPath;
};

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
	void FromString(const std::wstring& strPath);

	const wchar_t* ToString() const;
	std::wstring ToWString() const;

    // other operations
    void Clear() throw();

	bool Compare(const TSmartPath& rPath, bool bCaseSensitive = DefaultCaseSensitivity) const;
	bool IsChildOf(const TSmartPath& rPath, bool bCaseSensitive = DefaultCaseSensitivity) const;

	void MakeRelativePath(const TSmartPath& rReferenceBasePath, bool bCaseSensitive = DefaultCaseSensitivity);

	void AppendIfNotExists(const wchar_t* pszPostfix, bool bCaseSensitive = DefaultCaseSensitivity);
	void CutIfExists(const wchar_t* pszPostfix, bool bCaseSensitive = DefaultCaseSensitivity);

	void DeleteLastComponent();
	TSmartPath GetLastComponent();

	bool HasLengthExtension() const;

	bool HasDrive() const;
    TSmartPath GetDrive() const;      // c: for c:\windows\test.txt

	bool HasFileDir() const;        // \windows\ for c:\windows\test.txt
	TSmartPath GetFileDir() const;        // \windows\ for c:\windows\test.txt

	bool HasFileTitle() const;      // test for c:\windows\test.txt
	TSmartPath GetFileTitle() const;      // test for c:\windows\test.txt

	bool HasExtension() const;        // txt for c:\windows\test.txt
    TSmartPath GetExtension() const;        // txt for c:\windows\test.txt

	bool HasFileName() const;       // test.txt for c:\windows\test.txt
	TSmartPath GetFileName() const;       // test.txt for c:\windows\test.txt

	bool IsEmpty() const;

    // Serialization
	template<class Archive>
	void load(Archive& ar, unsigned int /*uiVersion*/)
	{
		PrepareToWrite();
		ar & m_pPath->m_strPath;
	}

	template<class Archive>
	void save(Archive& ar, unsigned int /*uiVersion*/) const
	{
		ar & m_pPath->m_strPath;
	}

	BOOST_SERIALIZATION_SPLIT_MEMBER();

protected:
	void PrepareToWrite();

protected:
	TPath* m_pPath;
};

LIBCHCORE_API TSmartPath PathFromString(const wchar_t* pszPath);
LIBCHCORE_API TSmartPath PathFromString(const std::wstring& strPath);

class LIBCHCORE_API TPathContainer
{
public:
	TPathContainer();
	TPathContainer(const TPathContainer& rSrcContainer);
	~TPathContainer();

	TPathContainer& operator=(const TPathContainer& rSrcContainer);

	void Add(const TSmartPath& spPath);
	
	const TSmartPath& GetAt(size_t stIndex) const;
	TSmartPath& GetAt(size_t stIndex);

	void SetAt(size_t stIndex, const TSmartPath& spPath);

	void DeleteAt(size_t stIndex);
	void Clear();

	size_t GetCount() const;
	bool IsEmpty() const;

private:
#pragma warning(push)
#pragma warning(disable: 4251)
	std::vector<TSmartPath> m_vPaths;
#pragma warning(pop)
};

END_CHCORE_NAMESPACE

#endif
