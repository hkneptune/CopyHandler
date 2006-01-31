/***************************************************************************
 *   Copyright (C) 2004 by Józef Starosczyk                                *
 *   copyhandler@o2.pl                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
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
/** \file str_help.cpp
 *  \brief Contain implementation of some string helper functions.
 */
#include "str_help.h"

BEGIN_ICPF_NAMESPACE

/** Checks if the character is a whitespace.
 * \param[in] ch - character to check
 * \return True if the character is a whitespace one, false otherwise.
 */
LIBICPF_API bool is_whitespace(char_t ch)
{
	return (ch >= 0x09 && ch <= 0x0d) || ch == 0x20;
}

END_ICPF_NAMESPACE
