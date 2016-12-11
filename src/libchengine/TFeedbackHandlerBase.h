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
#ifndef __TFEEDBACKHANDLERBASE_H__
#define __TFEEDBACKHANDLERBASE_H__

#include "IFeedbackHandler.h"
#include <bitset>
#include "../libserializer/TSharedModificationTracker.h"

namespace chengine
{
	class LIBCHENGINE_API TFeedbackHandlerBase : public IFeedbackHandler
	{
	public:
		TFeedbackHandlerBase();
		TFeedbackHandlerBase(const TFeedbackHandlerBase&) = delete;
		virtual ~TFeedbackHandlerBase();

		TFeedbackHandlerBase& operator=(const TFeedbackHandlerBase&) = delete;

		// marking responses as permanent
		void SetFileErrorPermanentResponse(EFeedbackResult ePermanentResult);
		EFeedbackResult GetFileErrorPermanentResponse() const;
		bool HasFileErrorPermanentResponse(EFeedbackResult& rePermanentResult) const;

		void SetFileAlreadyExistsPermanentResponse(EFeedbackResult ePermanentResult);
		EFeedbackResult GetFileAlreadyExistsPermanentResponse() const;
		bool HasFileAlreadyExistsPermanentResponse(EFeedbackResult& rePermanentResult) const;

		void SetNotEnoughSpacePermanentResponse(EFeedbackResult ePermanentResult);
		EFeedbackResult GetNotEnoughSpacePermanentResponse() const;
		bool HasNotEnoughSpacePermanentResponse(EFeedbackResult& rePermanentResult) const;

		// resets the permanent status from all responses
		void RestoreDefaults() override;

		// serialization
		void Store(const serializer::ISerializerContainerPtr& spContainer) const override;
		static void InitColumns(const serializer::ISerializerContainerPtr& spContainer);
		void Load(const serializer::ISerializerContainerPtr& spContainer) override;

		DWORD GetRetryInterval() const override;

	private:
		enum EModifications
		{
			eMod_Added = 0,
			eMod_FileError,
			eMod_FileAlreadyExists,
			eMod_NotEnoughSpace,
			eMod_OperationFinished,
			eMod_OperationError,

			// last item
			eMod_Last
		};

#pragma warning(push)
#pragma warning(disable: 4251)
		mutable boost::shared_mutex m_lock;

		using Bitset = std::bitset<eMod_Last>;
		mutable Bitset m_setModifications;

		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, eMod_FileError> m_eFileError;
		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, eMod_FileAlreadyExists> m_eFileAlreadyExists;
		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, eMod_NotEnoughSpace> m_eNotEnoughSpace;
		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, eMod_OperationFinished> m_eOperationFinished;
		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, eMod_OperationError> m_eOperationError;
#pragma warning(pop)
	};

	typedef std::shared_ptr<TFeedbackHandlerBase> TFeedbackHandlerBasePtr;
}

#endif
