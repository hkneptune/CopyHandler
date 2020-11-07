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
	namespace FeedbackRuleEnum
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
	class LIBCHENGINE_API FeedbackRule : public serializer::SerializableObject<FeedbackRuleEnum::eMod_Last>
	{
	public:
		FeedbackRule();
		FeedbackRule(const FeedbackRule& rSrc);
		FeedbackRule& operator=(const FeedbackRule& rSrc);

		bool operator==(const FeedbackRule& rSrc) const;
		bool operator!=(const FeedbackRule& rSrc) const;

		void SetData(const FeedbackRule& rSrc);

		bool Matches(const TFileInfoPtr& spSrcFile, const TFileInfoPtr& spDstFile, EFeedbackResult& eResult) const;

		void Store(const serializer::ISerializerContainerPtr& spContainer) const override;
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader) override;
		static void InitColumns(serializer::IColumnsDefinition& rColumns);

		void StoreInConfig(TConfig& rConfig) const;
		void ReadFromConfig(const TConfig& rConfig);

		// get/set
		// atrributes access
		bool GetUseMask() const;
		void SetUseMask(bool bUseMask);

		string::TString GetCombinedMask() const;
		void SetCombinedMask(const string::TString& pMask);

		bool GetUseExcludeMask() const;
		void SetUseExcludeMask(bool bUseExcludeMask);

		string::TString GetCombinedExcludeMask() const;
		void SetCombinedExcludeMask(const string::TString& pMask);

		bool GetUseDateCompare() const;
		void SetUseDateCompare(bool bUseDateCompare);

		ECompareType GetDateCompareType() const;
		void SetDateCompareType(ECompareType eCmpType);
		
		bool GetUseSizeCompare() const;
		void SetUseSizeCompare(bool bUseSizeCompare);

		ECompareType GetSizeCompareType() const;
		void SetSizeCompareType(ECompareType eCmpType);

	private:

		// object identification

		serializer::TSharedModificationTracker<bool, Bitset, FeedbackRuleEnum::eMod_UseMask> m_bUseMask;
		serializer::TSharedModificationTracker<string::TStringPatternArray, Bitset, FeedbackRuleEnum::eMod_Mask> m_spaMask;
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackRuleEnum::eMod_UseExcludeMask> m_bUseExcludeMask;
		serializer::TSharedModificationTracker<string::TStringPatternArray, Bitset, FeedbackRuleEnum::eMod_ExcludeMask> m_spaExcludeMask;

		serializer::TSharedModificationTracker<bool, Bitset, FeedbackRuleEnum::eMod_UseDateCompare> m_bUseDateCompare;
		serializer::TSharedModificationTracker<ECompareType, Bitset, FeedbackRuleEnum::eMod_DateCompare> m_cmpLastModified;
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackRuleEnum::eMod_UseSizeCompare> m_bUseSizeCompare;
		serializer::TSharedModificationTracker<ECompareType, Bitset, FeedbackRuleEnum::eMod_SizeCompare> m_cmpSize;

		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, FeedbackRuleEnum::eMod_Result> m_eResult;
	};
#pragma warning(pop)
}
