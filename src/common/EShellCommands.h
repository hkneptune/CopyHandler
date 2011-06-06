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
/// @file  EShellCommands.h
/// @date  2011/05/06
/// @brief Contains enumeration of available shell commands IDs.
// ============================================================================
#ifndef __ESHELLCOMMANDS_H__
#define __ESHELLCOMMANDS_H__

enum EShellCommandID
{
   // standard context menu
   eShellCmd_Paste,
   eShellCmd_PasteSpecial,
   eShellCmd_Delete,

   // drag&drop context menu
   eShellCmd_CopyTo,
   eShellCmd_MoveTo,
   eShellCmd_CopyToMoveToSpecial,
};

#endif
