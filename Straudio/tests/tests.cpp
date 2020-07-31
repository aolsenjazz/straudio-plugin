#define BOOST_TEST_MODULE Tests
#include <boost/test/included/unit_test.hpp>
#include <thread>
#include <chrono>

#include "pcm_gen.h"
#include "interval_executor.h"
#include "timeout_executor.h"

BOOST_AUTO_TEST_CASE(IntervalExecutor_RunWorkFunc_Success) {
	
}

//BOOST_AUTO_TEST_CASE(IntervalExecutor_RunWorkFunc_Success) {
//	std::shared_ptr<int> tracker = std::make_shared<int>(0);
//
//	auto runWhileFunc = [tracker]() {
//		return *tracker != 10;
//	};
//
//	auto workFunc = [tracker]() {
//		(*tracker)++;
//	};
//
//	IntervalExecutor exec(10, workFunc, runWhileFunc);
//	bool success = exec.start();
//
//	// if tests finish before executors, Boost freaks out. so we wait
//	std::this_thread::sleep_for (std::chrono::seconds(1));
//
//	BOOST_TEST(*tracker == 10);
//	BOOST_TEST(success == true);
//}
//
//BOOST_AUTO_TEST_CASE(IntervalExecutor_StartTwice_ReturnFalse) {
//	std::shared_ptr<int> tracker = std::make_shared<int>(0);
//
//	auto runWhileFunc = [tracker]() {
//		return *tracker != 10;
//	};
//
//	auto workFunc = [tracker]() {
//		(*tracker)++;
//	};
//
//	IntervalExecutor exec(10, workFunc, runWhileFunc);
//	exec.start();
//	bool shouldBeFalse = exec.start();
//
//	// if tests finish before executors, Boost freaks out. so we wait
//	std::this_thread::sleep_for (std::chrono::seconds(1));
//
//	BOOST_TEST(shouldBeFalse == false);
//}
//
//BOOST_AUTO_TEST_CASE(IntervalExecutor_InterruptKillsThread) {
//	std::shared_ptr<int> tracker = std::make_shared<int>(0);
//
//	auto runWhileFunc = [tracker]() {
//		return *tracker != 10;
//	};
//
//	auto workFunc = [tracker]() {
//		(*tracker)++;
//	};
//
//	IntervalExecutor exec(100, workFunc, runWhileFunc);
//	exec.start();
//	exec.stop();
//
//	// if tests finish before executors, Boost freaks out. so we wait
//	std::this_thread::sleep_for (std::chrono::seconds(1));
//
//	BOOST_TEST(*tracker == 0);
//}
//
//BOOST_AUTO_TEST_CASE(TimeoutExecutor_RunWorkFuncAfterDelay) {
//	std::shared_ptr<int> tracker = std::make_shared<int>(0);
//
//	auto workFunc = [tracker]() {
//		*tracker = 100;
//	};
//
//	TimeoutExecutor exec(1000, workFunc);
//	bool shouldBeTrue = exec.start();
//
//	BOOST_TEST(shouldBeTrue == true);
//
//	// if tests finish before executors, Boost freaks out. so we wait
//	std::this_thread::sleep_for (std::chrono::seconds(2));
//
//	BOOST_TEST(*tracker == 100);
//}
//
//BOOST_AUTO_TEST_CASE(TimeoutExecutor_SecondStartFails) {
//	std::shared_ptr<int> tracker = std::make_shared<int>(0);
//
//	auto workFunc = [tracker]() {
//		*tracker = 100;
//	};
//
//	TimeoutExecutor exec(100, workFunc);
//	exec.start();
//	bool shouldBeFalse = exec.start();
//
//	BOOST_TEST(shouldBeFalse == false);
//
//	// if tests finish before executors, Boost freaks out. so we wait
//	std::this_thread::sleep_for (std::chrono::seconds(1));
//
//}
//
//BOOST_AUTO_TEST_CASE(TimeoutExecutor_InterruptIsSuccessful) {
//	std::shared_ptr<int> tracker = std::make_shared<int>(0);
//
//	auto workFunc = [tracker]() {
//		*tracker = 100;
//	};
//
//	TimeoutExecutor exec(1000, workFunc);
//	exec.start();
//	exec.interrupt();
//
//	// if tests finish before executors, Boost freaks out. so we wait
//	std::this_thread::sleep_for (std::chrono::seconds(1));
//
//
//	BOOST_TEST(*tracker == 0);
//}
//
//BOOST_AUTO_TEST_CASE(TimeoutExecutor_ThreadOutlives_SuccessfulInterrupt) {
//	std::shared_ptr<int> tracker = std::make_shared<int>(0);
//
//	auto workFunc = [tracker]() {
//		*tracker = 100;
//	};
//
//	TimeoutExecutor *te = new TimeoutExecutor(1000, workFunc);
//	te->start();
//	te->interrupt();
//	delete te;
//
//	// if tests finish before executors, Boost freaks out. so we wait
//	std::this_thread::sleep_for (std::chrono::seconds(2));
//
//
//	BOOST_TEST(*tracker == 0);
//}
//
//BOOST_AUTO_TEST_CASE(TimeoutExecutor_ThreadOutlives_CallsFuncSuccessfully) {
//	std::shared_ptr<int> tracker = std::make_shared<int>(0);
//
//	auto workFunc = [tracker]() {
//		*tracker = 100;
//	};
//
//	TimeoutExecutor *te = new TimeoutExecutor(1000, workFunc);
//	te->start();
//	delete te;
//
//	// if tests finish before executors, Boost freaks out. so we wait
//	std::this_thread::sleep_for (std::chrono::seconds(2));
//
//
//	BOOST_TEST(*tracker == 100);
//}
