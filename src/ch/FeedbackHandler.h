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

class CFeedbackHandler : public chcore::IFeedbackHandler
{
public:
	CFeedbackHandler();
	virtual ~CFeedbackHandler();

public:
	virtual ull_t RequestFeedback(ull_t ullFeedbackID, ptr_t pFeedbackParam);

protected:
	EFeedbackResult m_aeFeedbackTypeStatus[eFT_LastType];

	friend class CFeedbackHandlerFactory;
};

typedef boost::shared_ptr<CFeedbackHandler> CFeedbackHandlerPtr;

class CFeedbackHandlerFactory : public chcore::IFeedbackHandlerFactory
{
public:
	CFeedbackHandlerFactory();
	virtual ~CFeedbackHandlerFactory();

public:
	chcore::IFeedbackHandlerPtr Create();
};

typedef boost::shared_ptr<CFeedbackHandlerFactory> CFeedbackHandlerFactoryPtr;

#endif
