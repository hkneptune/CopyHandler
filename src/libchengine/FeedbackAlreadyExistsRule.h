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

namespace chengine
{
	namespace FeedbackAlreadyExistsRuleEnum
	{
		enum EModifications
		{
			eMod_Added,
			eMod_UseMask,
			eMod_Mask,
			eMod_UseExcludeMask,
			eMod_ExcludeMask,
			eMod_UseSizeCompare,
			eMod_SizeCompare,
			eMod_UseDateCompare,
			eMod_DateCompare,
			eMod_Result,

			eMod_Last
		};
	}

#pragma warning(push)
#pragma warning(disable: 4251)
	class LIBCHENGINE_API FeedbackAlreadyExistsRule : public serializer::SerializableObject<FeedbackAlreadyExistsRuleEnum::eMod_Last>
	{
	public:
		FeedbackAlreadyExistsRule();
		FeedbackAlreadyExistsRule(const FeedbackAlreadyExistsRule& rSrc);
		FeedbackAlreadyExistsRule& operator=(const FeedbackAlreadyExistsRule& rSrc);

		bool operator==(const FeedbackAlreadyExistsRule& rSrc) const;
		bool operator!=(const FeedbackAlreadyExistsRule& rSrc) const;

		void SetData(const FeedbackAlreadyExistsRule& rSrc);

		bool Matches(const TFileInfo& rSrcFile, const TFileInfo& rDstFile, EFeedbackResult& eResult) const;

		void Store(const serializer::ISerializerContainerPtr& spContainer) const override;
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader) override;
		static void InitColumns(serializer::IColumnsDefinition& rColumns);

		void StoreInConfig(TConfig& rConfig) const;
		void ReadFromConfig(const TConfig& rConfig);

		// comparison
		bool HaveSameCondition(const FeedbackAlreadyExistsRule& rSrc) const;

		// get/set
		// atrributes access
		bool GetUseMask() const;
		void SetUseMask(bool bUseMask);

		string::TString GetCombinedMask() const;
		void SetCombinedMask(const string::TString& strMask);

		bool GetUseExcludeMask() const;
		void SetUseExcludeMask(bool bUseExcludeMask);

		string::TString GetCombinedExcludeMask() const;
		void SetCombinedExcludeMask(const string::TString& strMask);

		bool GetUseDateCompare() const;
		void SetUseDateCompare(bool bUseDateCompare);

		ECompareType GetDateCompareType() const;
		void SetDateCompareType(ECompareType eCmpType);
		
		bool GetUseSizeCompare() const;
		void SetUseSizeCompare(bool bUseSizeCompare);

		ECompareType GetSizeCompareType() const;
		void SetSizeCompareType(ECompareType eCmpType);

		EFeedbackResult GetResult() const;
		void SetResult(EFeedbackResult eResult);

	private:
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_UseMask> m_bUseMask;
		serializer::TSharedModificationTracker<string::TStringPatternArray, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_Mask> m_spaMask;
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_UseExcludeMask> m_bUseExcludeMask;
		serializer::TSharedModificationTracker<string::TStringPatternArray, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_ExcludeMask> m_spaExcludeMask;

		serializer::TSharedModificationTracker<bool, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_UseDateCompare> m_bUseDateCompare;
		serializer::TSharedModificationTracker<ECompareType, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_DateCompare> m_cmpLastModified;
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_UseSizeCompare> m_bUseSizeCompare;
		serializer::TSharedModificationTracker<ECompareType, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_SizeCompare> m_cmpSize;

		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, FeedbackAlreadyExistsRuleEnum::eMod_Result> m_eResult;
	};
#pragma warning(pop)
}
