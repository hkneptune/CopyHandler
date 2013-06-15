/***************************************************************************
*   Copyright (C) 2001-2013 by Józef Starosczyk                           *
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
#include "TProgressCtrlEx.h"

TProgressCtrlEx::TProgressCtrlEx()
{
}

void TProgressCtrlEx::SetProgress(unsigned long long ullPos, unsigned long long ullMaxRange)
{
	unsigned int uiDivider = 0;
	while((ullMaxRange >> uiDivider) > (unsigned int)std::numeric_limits<int>::max())
		++uiDivider;

	ullPos >>= uiDivider;
	ullMaxRange >>= uiDivider;

	SetRange32(0, (int)ullMaxRange);
	SetPos((int)ullPos);
}
