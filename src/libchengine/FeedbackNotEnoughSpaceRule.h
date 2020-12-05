#pragma once

#include "libchengine.h"
#include "../libserializer/SerializableObject.h"
#include "../libchcore/TStringPatternArray.h"
#include "ECompareType.h"
#include "EFeedbackResult.h"
#include "../libserializer/SerializerDataTypes.h"
#include <bitset>
#include "../libserializer/TSharedModificationTracker.h"
#include "TFileInfo.h"
#include "TConfig.h"
#include "EFileError.h"

namespace chengine
{
	namespace FeedbackNotEnoughSpaceRuleEnum
	{
		enum EModifications
		{
			eMod_Added,
			eMod_UseMask,
			eMod_Mask,
			eMod_UseExcludeMask,
			eMod_ExcludeMask,
			eMod_Result,

			eMod_Last
		};
	}

#pragma warning(push)
#pragma warning(disable: 4251)
	class LIBCHENGINE_API FeedbackNotEnoughSpaceRule : public serializer::SerializableObject<FeedbackNotEnoughSpaceRuleEnum::eMod_Last, FeedbackNotEnoughSpaceRuleEnum::eMod_Added>
	{
	public:
		FeedbackNotEnoughSpaceRule();
		FeedbackNotEnoughSpaceRule(const FeedbackNotEnoughSpaceRule& rSrc);
		FeedbackNotEnoughSpaceRule& operator=(const FeedbackNotEnoughSpaceRule& rSrc);

		bool operator==(const FeedbackNotEnoughSpaceRule& rSrc) const;
		bool operator!=(const FeedbackNotEnoughSpaceRule& rSrc) const;

		void SetData(const FeedbackNotEnoughSpaceRule& rSrc);

		bool Matches(const string::TString& strDstPath, unsigned long long ullRequiredSize, EFeedbackResult& eResult) const;

		void Store(const serializer::ISerializerContainerPtr& spContainer) const override;
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader) override;
		static void InitColumns(serializer::IColumnsDefinition& rColumns);

		void StoreInConfig(TConfig& rConfig) const;
		void ReadFromConfig(const TConfig& rConfig);

		// comparison
		bool HaveSameCondition(const FeedbackNotEnoughSpaceRule& rSrc) const;

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

		const chcore::TStringPatternArray& GetIncludeMask() const { return m_spaMask; }
		const chcore::TStringPatternArray& GetExcludeMask() const { return m_spaExcludeMask; }

		EFeedbackResult GetResult() const;
		void SetResult(EFeedbackResult eResult);

	private:
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackNotEnoughSpaceRuleEnum::eMod_UseMask> m_bUseMask;
		serializer::TSharedModificationTracker<chcore::TStringPatternArray, Bitset, FeedbackNotEnoughSpaceRuleEnum::eMod_Mask> m_spaMask;
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackNotEnoughSpaceRuleEnum::eMod_UseExcludeMask> m_bUseExcludeMask;
		serializer::TSharedModificationTracker<chcore::TStringPatternArray, Bitset, FeedbackNotEnoughSpaceRuleEnum::eMod_ExcludeMask> m_spaExcludeMask;

		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, FeedbackNotEnoughSpaceRuleEnum::eMod_Result> m_eResult;
	};
#pragma warning(pop)
}
