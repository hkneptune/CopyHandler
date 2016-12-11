#include "stdafx.h"
#include "gtest/gtest.h"
#include "../TConfig.h"
#include "../TTaskConfiguration.h"
#include "../TTaskConfigVerifier.h"

using namespace chengine;
using namespace string;

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_FirstAlternateFilenameFormat_Valid)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config, L"First copy of %name%ext");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config);

	EXPECT_STREQ(L"First copy of %name%ext", strValue.c_str());
}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_FirstAlternateFilenameFormat_Invalid)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config, L"First copy of %nme%ext");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config);

	EXPECT_STREQ(L"%name - copy%ext", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_FirstAlternateFilenameFormat_InvalidExt)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config, L"First copy of %name%et");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config);

	EXPECT_STREQ(L"%name - copy%ext", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_NextAlternateFilenameFormat_Valid)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config, L"Subsequent copy of %name (%count)%ext");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config);

	EXPECT_STREQ(L"Subsequent copy of %name (%count)%ext", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_NextAlternateFilenameFormat_InvalidCount)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config, L"Subsequent copy of %name (%cout)%ext");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config);

	EXPECT_STREQ(L"%name - copy (%count)%ext", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_NextAlternateFilenameFormat_InvalidExt)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config, L"Subsequent copy of %name (%count)%et");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config);

	EXPECT_STREQ(L"%name - copy (%count)%ext", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_NextAlternateFilenameFormat_InvalidName)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config, L"Subsequent copy of %ame (%count)%ext");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config);

	EXPECT_STREQ(L"%name - copy (%count)%ext", strValue.c_str());
}
