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
#ifndef __TEXCEPTION_H__
#define __TEXCEPTION_H__

#include "libchcore.h"
#include "ErrorCodes.h"
#include "TBaseException.h"

// throws core exception object
#define THROW_CORE_EXCEPTION(error_code)\
	throw chcore::TCoreException(error_code, L"", __FILEW__, __LINE__, __FUNCTIONW__)

#define THROW_CORE_EXCEPTION_MSG(error_code, msg)\
	throw chcore::TCoreException(error_code, msg, __FILEW__, __LINE__, __FUNCTIONW__)

namespace chcore
{

	class LIBCHCORE_API TCoreException : public TBaseException
	{
	public:
		using TBaseException::TBaseException;
	};
}

#endif
