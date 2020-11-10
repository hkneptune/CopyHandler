/***************************************************************************
*   Copyright (C) 2001-2015 by Józef Starosczyk                           *
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
#ifndef __EFILEERROR_H__
#define __EFILEERROR_H__

namespace chengine
{
	enum EFileError
	{
		eDeleteError,		///< Problem occurred when tried to delete the fs object
		eSeekError,			///< Problem occurred when tried to set file pointer
		eResizeError,		///< Problem occurred when tried to change size of the fs object
		eReadError,			///< Problem occurred when tried to read data from file
		eWriteError,		///< Problem occurred when tried to write data to a file
		eFinalizeError,		///< Problem occurred when tried to finalize file
		eFastMoveError,		///< Problem occurred when tried to perform fast move operation (that does not involve copying contents)
		eCreateError,		///< Problem occurred when tried to create the fs object
		eCheckForFreeSpace,	///< Problem occurred when tried to create the fs object
		eRetrieveFileInfo	///< Error while retrieving file information
	};
}

#endif
