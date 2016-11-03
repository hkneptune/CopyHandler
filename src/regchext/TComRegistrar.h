// ============================================================================
//  Copyright (C) 2001-2016 by Jozef Starosczyk
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
#ifndef __TCOMREGISTRAR_H__
#define __TCOMREGISTRAR_H__

#include <string>

class TComRegistrar
{
public:
	TComRegistrar();

	bool RegisterNative(const wchar_t* pszPath, const wchar_t* pszDir);
	bool UnregisterNative(const wchar_t* pszPath, const wchar_t* pszDir);
#ifdef _WIN64
	bool Register32bit(const wchar_t* pszPath, const wchar_t* pszDir);
	bool Unregister32bit(const wchar_t* pszPath, const wchar_t* pszDir);
#endif

private:
	void DetectRegsvrPaths();
	static bool Register(const wchar_t* pszPath, const wchar_t* pszDir, const wchar_t* pszRegsvrPath);
	static bool Unregister(const wchar_t* pszPath, const wchar_t* pszDir, const wchar_t* pszRegsvrPath);

private:
	std::wstring m_strNativeRegsvrPath;
#ifdef _WIN64
	std::wstring m_str32bitRegsvr;
#endif
};

#endif
