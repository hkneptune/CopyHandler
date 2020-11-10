#pragma once

#include "libchengine.h"
#include "../libserializer/SerializableObject.h"
#include "../libstring/TStringPatternArray.h"
#include "ECompareType.h"
#include "EFeedbackResult.h"
#include "../libserializer/SerializerDataTypes.h"
#include <bitset>
#include "../libserializer/TSharedModificationTracker.h"
#include "TFileInfo.h"
#include "TConfig.h"
#include "EFileError.h"
#include "EOperationEvent.h"

namespace chengine
{
	namespace FeedbackOperationEventRuleEnum
	{
		enum EModifications
		{
			eMod_Added,
			eMod_UseOperationEvent,
			eMod_OperationEvent,
			eMod_Result,

			eMod_Last
		};
	}

#pragma warning(push)
#pragma warning(disable: 4251)
	class LIBCHENGINE_API FeedbackOperationEventRule : public serializer::SerializableObject<FeedbackOperationEventRuleEnum::eMod_Last>
	{
	public:
		FeedbackOperationEventRule();
		FeedbackOperationEventRule(const FeedbackOperationEventRule& rSrc);
		FeedbackOperationEventRule& operator=(const FeedbackOperationEventRule& rSrc);

		bool operator==(const FeedbackOperationEventRule& rSrc) const;
		bool operator!=(const FeedbackOperationEventRule& rSrc) const;

		void SetData(const FeedbackOperationEventRule& rSrc);

		bool Matches(EOperationEvent eEvent, EFeedbackResult& eResult) const;

		void Store(const serializer::ISerializerContainerPtr& spContainer) const override;
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader) override;
		static void InitColumns(serializer::IColumnsDefinition& rColumns);

		void StoreInConfig(TConfig& rConfig) const;
		void ReadFromConfig(const TConfig& rConfig);

		// comparison
		bool HaveSameCondition(const FeedbackOperationEventRule& rSrc) const;

		// attributes
		bool GetUseOperationEvent() const;
		void SetUseOperationEventType(bool bUseErrorType);
		EOperationEvent GetOperationEvent() const;
		void SetOperationEvent(EOperationEvent eErrorType);

		EFeedbackResult GetResult() const;
		void SetResult(EFeedbackResult eResult);

	private:
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackOperationEventRuleEnum::eMod_UseOperationEvent> m_bUseOperationEvent;
		serializer::TSharedModificationTracker<EOperationEvent, Bitset, FeedbackOperationEventRuleEnum::eMod_OperationEvent> m_eOperationEvent;

		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, FeedbackOperationEventRuleEnum::eMod_Result> m_eResult;
	};
#pragma warning(pop)
}
