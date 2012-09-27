#include "stdafx.h"
#include "../../../src/libicpf/gen_types.h"
#include "../../../src/libchcore/TDataBuffer.h"
#include "../../../src/libchcore/TCoreException.h"

///////////////////////////////////////////////////////////////////////////////
// TSimpleDataBuffer

TEST(TSimpleDataBuffer, GetBufferPtr)
{
	chcore::TSimpleDataBuffer tDataBuffer;
	EXPECT_EQ(tDataBuffer.GetBufferPtr(), (LPVOID)NULL);
}

TEST(TSimpleDataBuffer, ReleaseBuffer)
{
	chcore::TSimpleDataBuffer tDataBuffer;
	EXPECT_NO_FATAL_FAILURE(tDataBuffer.ReleaseBuffer());
}


TEST(TSimpleDataBuffer, GetSetDataSize)
{
	chcore::TSimpleDataBuffer tDataBuffer;
	EXPECT_EQ(tDataBuffer.GetDataSize(), 0);
	EXPECT_THROW(tDataBuffer.SetDataSize(4273), chcore::TCoreException);
	EXPECT_EQ(tDataBuffer.GetDataSize(), 0);
}


TEST(TSimpleDataBuffer, CutDataFromBuffer)
{
	chcore::TSimpleDataBuffer tDataBuffer;

	EXPECT_NO_FATAL_FAILURE(tDataBuffer.CutDataFromBuffer(7344));
	EXPECT_NO_FATAL_FAILURE(tDataBuffer.CutDataFromBuffer(0));
}

///////////////////////////////////////////////////////////////////////////////
// TSimpleDataBuffer
