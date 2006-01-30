/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2004 Ixen Gerthannes (copyhandler@o2.pl)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*************************************************************************/
#ifndef __STATICEX_H__
#define __STATICEX_H__

// styles
#define SES_LINK			0x0001	/* link instead of common static */
#define SES_NOTIFY			0x0002	/* notifies parent about mouse messages */
#define SES_PATHELLIPSIS	0x0004
#define SES_ELLIPSIS		0x0008
#define SES_BOLD			0x0010
#define SES_LARGE			0x0020
#define SES_RALIGN			0x0040	/* incompatible with SES_LINK */
#define SES_WORDBREAK		0x0080

// messages
#define SEM_GETLINK		(WM_USER+16)	/* wParam - cnt of chars to copy, lParam - addr of text */

#define SEN_CLICKED		0x0001

bool RegisterStaticExControl(HINSTANCE hInstance);

struct STATICEXSETTINGS
{
	HFONT hFontNormal;
	HFONT hFontUnderline;

	HCURSOR hNormal;
	HCURSOR hLink;

	bool bDown;			// specifies if the mouse button has been pressed
	bool bActive;		// is the link hovered ?
	RECT rcText;		// current text position and size

	TCHAR *pszText;
	TCHAR *pszLink;
};

#endif