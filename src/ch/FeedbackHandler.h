/***************************************************************************
 *   Copyright (C) 2001-2008 by Jozef Starosczyk                           *
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
#ifndef __FEEDBACKHANDLER_H__
#define __FEEDBACKHANDLER_H__

#include "../libchcore/IFeedbackHandlerFactory.h"
#include "../libchcore/TFeedbackHandlerBase.h"
#include "../libchcore/EFeedbackResult.h"

class CFeedbackHandler : public chcore::TFeedbackHandlerBase
{
public:
	CFeedbackHandler();
	virtual ~CFeedbackHandler();

	virtual chcore::EFeedbackResult FileError(const chcore::TString& strSrcPath, const chcore::TString& strDstPath, chcore::EFileError eFileError, unsigned long ulError) override;
	virtual chcore::EFeedbackResult FileAlreadyExists(const chcore::TFileInfoPtr& spSrcFileInfo, const chcore::TFileInfoPtr& spDstFileInfo) override;
	virtual chcore::EFeedbackResult NotEnoughSpace(const chcore::TString& strSrcPath, const chcore::TString& strDstPath, unsigned long long ullRequiredSize) override;
	virtual chcore::EFeedbackResult OperationFinished() override;
	virtual chcore::EFeedbackResult OperationError() override;

protected:
	friend class CFeedbackHandlerFactory;
};

typedef boost::shared_ptr<CFeedbackHandler> CFeedbackHandlerPtr;

#endif
