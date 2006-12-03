/************************************************************************
	Copy Handler 1.x - program for copying data in Microsoft Windows
						 systems.
	Copyright (C) 2001-2003 Ixen Gerthannes (ixen@interia.pl)

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
/* Code based on code written by Chris Maunder (Chris.Maunder@cbr.clw.csiro.au) */

#ifndef __TRAYICON_H__
#define __TRAYICON_H__

#include "shellapi.h"

class CTrayIcon
{
public:
// construction/destruction
	CTrayIcon();
	CTrayIcon(HWND hWnd, UINT uClbMsg, LPCTSTR szText, HICON hIcon, UINT uiID);
	~CTrayIcon();

//creation
	bool CreateIcon(HWND hWnd, UINT uClbMsg, LPCTSTR szText, HICON hIcon, UINT uiID);

// ToolTip text handleing
	bool SetTooltipText(LPCTSTR pszTip);
	void GetTooltipText(LPTSTR pszText) const;

// Icon handling
	bool SetIcon(HICON hIcon);
	bool SetStandardIcon(LPCTSTR lpIconName);
	HICON GetIcon() const;
	void HideIcon();
	bool ShowIcon();
	void RemoveIcon();
	void MoveToRight();

// Notifications
	bool SetNotificationWnd(HWND hWnd);
	HWND GetNotificationWnd() const;

// Attribs
public:
	bool m_bHidden;	// Has the icon been hidden?
	NOTIFYICONDATA m_tnd;
};

#endif