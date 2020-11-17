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
	namespace FeedbackErrorRuleEnum
	{
		enum EModifications
		{
			eMod_Added,
			eMod_UseMask,
			eMod_Mask,
			eMod_UseExcludeMask,
			eMod_ExcludeMask,
			eMod_UseErrorType,
			eMod_ErrorType,
			eMod_UseSystemErrorNo,
			eMod_SystemErrorNo,
			eMod_Result,

			eMod_Last
		};
	}

#pragma warning(push)
#pragma warning(disable: 4251)
	class LIBCHENGINE_API FeedbackErrorRule : public serializer::SerializableObject<FeedbackErrorRuleEnum::eMod_Last, FeedbackErrorRuleEnum::eMod_Added>
	{
	public:
		FeedbackErrorRule();
		FeedbackErrorRule(const FeedbackErrorRule& rSrc);
		FeedbackErrorRule& operator=(const FeedbackErrorRule& rSrc);

		bool operator==(const FeedbackErrorRule& rSrc) const;
		bool operator!=(const FeedbackErrorRule& rSrc) const;

		void SetData(const FeedbackErrorRule& rSrc);

		bool Matches(const string::TString& strSrcPath, const string::TString& strDstPath, EFileError eErrorType, unsigned long ulError, EFeedbackResult& eResult) const;

		void Store(const serializer::ISerializerContainerPtr& spContainer) const override;
		void Load(const serializer::ISerializerRowReaderPtr& spRowReader) override;
		static void InitColumns(serializer::IColumnsDefinition& rColumns);

		void StoreInConfig(TConfig& rConfig) const;
		void ReadFromConfig(const TConfig& rConfig);

		// comparison
		bool HaveSameCondition(const FeedbackErrorRule& rSrc) const;

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

		bool GetUseErrorType() const;
		void SetUseErrorType(bool bUseErrorType);
		EFileError GetErrorType() const;
		void SetErrorType(EFileError eErrorType);

		bool GetUseSystemErrorNo() const;
		void SetUseSystemErrorNo(bool bUseSystemErrorNo);
		unsigned int GetSystemErrorNo() const;
		void SetSystemErrorNo(unsigned int ulErrorNo);

		EFeedbackResult GetResult() const;
		void SetResult(EFeedbackResult eResult);

	private:
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackErrorRuleEnum::eMod_UseMask> m_bUseMask;
		serializer::TSharedModificationTracker<chcore::TStringPatternArray, Bitset, FeedbackErrorRuleEnum::eMod_Mask> m_spaMask;
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackErrorRuleEnum::eMod_UseExcludeMask> m_bUseExcludeMask;
		serializer::TSharedModificationTracker<chcore::TStringPatternArray, Bitset, FeedbackErrorRuleEnum::eMod_ExcludeMask> m_spaExcludeMask;

		serializer::TSharedModificationTracker<bool, Bitset, FeedbackErrorRuleEnum::eMod_UseErrorType> m_bUseErrorType;
		serializer::TSharedModificationTracker<EFileError, Bitset, FeedbackErrorRuleEnum::eMod_ErrorType> m_eErrorType;
		serializer::TSharedModificationTracker<bool, Bitset, FeedbackErrorRuleEnum::eMod_UseSystemErrorNo> m_bUseSystemErrorNo;
		serializer::TSharedModificationTracker<unsigned int, Bitset, FeedbackErrorRuleEnum::eMod_SystemErrorNo> m_ulSystemErrorNo;

		serializer::TSharedModificationTracker<EFeedbackResult, Bitset, FeedbackErrorRuleEnum::eMod_Result> m_eResult;
	};
#pragma warning(pop)
}
