// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
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
#ifndef __TFEEDBACKHANDLERWRAPPER_H__
#define __TFEEDBACKHANDLERWRAPPER_H__

#include "libchcore.h"
#include "IFeedbackHandler.h"

namespace chcore
{
	class TScopedRunningTimeTracker;

	class TFeedbackHandlerWrapper : public IFeedbackHandler
	{
	public:
		TFeedbackHandlerWrapper(const IFeedbackHandlerPtr& spFeedbackHandler, TScopedRunningTimeTracker& rTimeGuard);
		virtual ~TFeedbackHandlerWrapper();

		TFeedbackHandlerWrapper(const TFeedbackHandlerWrapper&) = delete;
		TFeedbackHandlerWrapper& operator=(const TFeedbackHandlerWrapper&) = delete;

		virtual TFeedbackResult FileError(const TString& strSrcPath, const TString& strDstPath, EFileError eFileError, unsigned long ulError) override;
		virtual TFeedbackResult FileAlreadyExists(const TFileInfo& spSrcFileInfo, const TFileInfo& spDstFileInfo) override;
		virtual TFeedbackResult NotEnoughSpace(const TString& strSrcPath, const TString& strDstPath, unsigned long long ullRequiredSize) override;
		virtual TFeedbackResult OperationFinished() override;
		virtual TFeedbackResult OperationError() override;

		virtual void RestoreDefaults() override;

		virtual void Store(const ISerializerContainerPtr& spContainer) const override;
		virtual void Load(const ISerializerContainerPtr& spContainer) override;

		virtual DWORD GetRetryInterval() const override;

	private:
		IFeedbackHandlerPtr m_spFeedbackHandler;
		TScopedRunningTimeTracker& m_rTimeGuard;
	};

	typedef std::shared_ptr<TFeedbackHandlerWrapper> TFeedbackHandlerWrapperPtr;
}

#endif
