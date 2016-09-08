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
#ifndef __THEME_SUPPORT__
#define __THEME_SUPPORT__

// definicja HTHEME - podobna do tej z UxTheme...h
#ifndef HTHEME
#define HTHEME HANDLE
#endif

typedef HTHEME(_stdcall *PFNOPENTHEMEDATA)(HWND hwnd, LPCWSTR pszClassList);
typedef HRESULT(_stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
typedef HRESULT(_stdcall *PFNDRAWTHEMEEDGE)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pDestRect, UINT uEdge, UINT uFlags, RECT* pContentRect);
typedef HRESULT(_stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pRect, const RECT* pClipRect);
typedef HRESULT(_stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hwnd, HDC hdc, RECT* prc);
typedef BOOL(_stdcall *PFNISAPPTHEMED)();

class CUxThemeSupport
{
public:
	CUxThemeSupport();
	~CUxThemeSupport();

	HTHEME OpenThemeData(HWND hwnd, LPCWSTR pszClassList);
	HRESULT CloseThemeData(HTHEME hTheme);

	bool IsThemeSupported() { return m_hThemesDll != nullptr; };
	BOOL IsAppThemed();

	HRESULT DrawThemeEdge(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT* pDestRect, UINT uEdge, UINT uFlags, RECT* pContentRect);

	HRESULT DrawThemeBackground(HTHEME hTheme, HDC hdc, int iPartId, int iStateId, const RECT *pRect, OPTIONAL const RECT *pClipRect);
	HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc, RECT* prc);

protected:

	HMODULE m_hThemesDll;
};

#endif
