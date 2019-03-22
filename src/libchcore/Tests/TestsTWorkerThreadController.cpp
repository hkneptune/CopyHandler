#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TWorkerThreadController.h"

using namespace chcore;

struct SampleThreadOwner
{
	SampleThreadOwner() : m_alCounter(0)
	{
	}

	static DWORD WINAPI ThreadRoutine(LPVOID lpThreadParameter)
	{
		SampleThreadOwner* pThis = (SampleThreadOwner*)lpThreadParameter;

		++pThis->m_alCounter;

		while (!pThis->m_controller.KillRequested(100))
		{
			++pThis->m_alCounter;
		}

		return 0;
	}

	std::atomic<long> m_alCounter;
	TWorkerThreadController m_controller;
};

TEST(TestsTWorkerThreadController, DefaultConstruction)
{
	TWorkerThreadController controller;
	EXPECT_TRUE(controller.GetKillThreadHandle() != INVALID_HANDLE_VALUE);
}

TEST(TestsTWorkerThreadController, StartStopThread_Manual)
{
	SampleThreadOwner owner;
	owner.m_controller.StartThread(&SampleThreadOwner::ThreadRoutine, &owner);
	owner.m_controller.SignalThreadToStop();
	owner.m_controller.WaitForThreadToExit(100);

	EXPECT_LE(1, owner.m_alCounter);
}

TEST(TestsTWorkerThreadController, StartStopThread_Auto)
{
	SampleThreadOwner owner;
	owner.m_controller.StartThread(&SampleThreadOwner::ThreadRoutine, &owner);
	owner.m_controller.StopThread(100);

	EXPECT_LE(1, owner.m_alCounter);
}
