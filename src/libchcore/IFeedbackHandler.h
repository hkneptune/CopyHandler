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
#ifndef __FEEDBACKHANDLERBASE_H__
#define __FEEDBACKHANDLERBASE_H__

#include "libchcore.h"
#include "EFeedbackResult.h"
#include "TString.h"
#include "TFileInfo.h"
#include "ISerializerRowReader.h"
#include "ISerializerRowData.h"
#include "EFileError.h"

BEGIN_CHCORE_NAMESPACE

class LIBCHCORE_API IFeedbackHandler
{
public:
	virtual ~IFeedbackHandler();

	// requests with some processing data
	virtual EFeedbackResult FileError(const TString& strSrcPath, const TString& strDstPath, EFileError eFileError, unsigned long ulError) = 0;
	virtual EFeedbackResult FileAlreadyExists(const TFileInfoPtr& spSrcFileInfo, const TFileInfoPtr& spDstFileInfo) = 0;
	virtual EFeedbackResult NotEnoughSpace(const TString& strSrcPath, const TString& strDstPath, unsigned long long ullRequiredSize) = 0;

	// no-data requests
	virtual EFeedbackResult OperationFinished() = 0;
	virtual EFeedbackResult OperationError() = 0;

	// reset permanent states
	virtual void RestoreDefaults() = 0;

	//serialization
	virtual void Store(const ISerializerContainerPtr& spContainer) const = 0;
	virtual void Load(const ISerializerContainerPtr& spContainer) = 0;
};

typedef boost::shared_ptr<IFeedbackHandler> IFeedbackHandlerPtr;

END_CHCORE_NAMESPACE

#endif
