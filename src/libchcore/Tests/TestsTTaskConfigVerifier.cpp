#include "stdafx.h"
#include "gtest/gtest.h"
#include "../TConfig.h"
#include "../log.h"
#include "../TTaskConfiguration.h"
#include "../TTaskConfigVerifier.h"

using namespace chcore;

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_FirstAlternateFilenameFormat_Valid)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config, L"First copy of %name");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config);

	EXPECT_STREQ(L"First copy of %name", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_FirstAlternateFilenameFormat_Invalid)
{
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config, L"First copy of %nme");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_First>(config);

	EXPECT_STREQ(L"%name - copy", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_NextAlternateFilenameFormat_Valid)
{
	log_file log;
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config, L"Subsequent copy of %name (%count)");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config);

	EXPECT_STREQ(L"Subsequent copy of %name (%count)", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_NextAlternateFilenameFormat_InvalidCount)
{
	log_file log;
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config, L"Subsequent copy of %name (%cout)");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config);

	EXPECT_STREQ(L"%name - copy (%count)", strValue.c_str());

}

TEST(TestsTTaskConfigVerifier, VerifyAndUpdate_NextAlternateFilenameFormat_InvalidName)
{
	log_file log;
	TConfig config;

	SetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config, L"Subsequent copy of %ame (%count)");

	TTaskConfigVerifier::VerifyAndUpdate(config, nullptr);

	TString strValue = GetTaskPropValue<eTO_AlternateFilenameFormatString_AfterFirst>(config);

	EXPECT_STREQ(L"%name - copy (%count)", strValue.c_str());
}
