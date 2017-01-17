#include "stdafx.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../TThreadedQueueRunner.h"
#include "../TEvent.h"

using namespace chengine;

static void TestFunction(int& val)
{
	++val;
}

static void TestFunctionWithSleep(int& val)
{
	++val;
	Sleep(50);
}

TEST(TestsTThreadedQueueRunner, PushSingleTask)
{
	TThreadedQueueRunner<std::function<void()>> runner;

	int iValue = 0;
	runner.PushTask(std::function<void()>(std::bind(&TestFunction, std::ref(iValue))));
	runner.Stop();

	EXPECT_EQ(1, iValue);
}

TEST(TestsTThreadedQueueRunner, PushMultipleTasks)
{
	TThreadedQueueRunner<std::function<void()>> runner;

	int iValue = 0;
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.Stop();

	EXPECT_EQ(5, iValue);
}

TEST(TestsTThreadedQueueRunner, PushMultipleTasksWithPreemptiveStart)
{
	TThreadedQueueRunner<std::function<void()>> runner;
	runner.Start();

	int iValue = 0;
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.PushTask(std::function<void()>(std::bind(&TestFunctionWithSleep, std::ref(iValue))));
	runner.Stop();

	EXPECT_EQ(5, iValue);
}
