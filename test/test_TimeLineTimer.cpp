//
// Created by cxk_zjq on 25-5-30.
//
#include <gtest/gtest.h>
#include "TimeLineTimer.h"
#include <atomic>
#include <thread>
#include <chrono>

using namespace cxk;

// 测试 TimeLineTimer 基础功能
TEST(TimeLineTimerTest, BasicFunctionality) {
    std::atomic<int> counter(0);
    auto callback = [&counter]() { counter++; };

    // 创建单次触发定时器
    TimeLineTimer timer(100, callback, 1);
    EXPECT_EQ(counter, 0);

    // 触发定时器
    timer.Trigger();
    EXPECT_EQ(counter, 1);

    // 再次触发（应无效果）
    timer.Trigger();
    EXPECT_EQ(counter, 1);
}

// 测试重复计数功能
TEST(TimeLineTimerTest, RepeatCount) {
    std::atomic<int> counter(0);
    auto callback = [&counter]() { counter++; };

    // 创建3次重复定时器
    TimeLineTimer timer(100, callback, 3);
    EXPECT_EQ(counter, 0);

    // 触发3次
    timer.Trigger();
    timer.Trigger();
    timer.Trigger();
    EXPECT_EQ(counter, 3);

    // 再次触发（应无效果）
    timer.Trigger();
    EXPECT_EQ(counter, 3);
}

// 测试多线程环境下的线程安全
TEST(TimerManagerTest, ThreadSafety) {
    TimerManager& manager = cxk::Singleton<cxk::TimerManager>::GetInstance();
    std::atomic<int> counter(0);
    auto callback = [&counter]() { counter++; };

    // 启动管理器线程
    std::thread managerThread([&]() {
        manager.Start();
    });

    // 从多个线程添加定时器
    constexpr int THREAD_COUNT = 10;
    std::thread threads[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; ++i) {
        threads[i] = std::thread([&, i]() {
            for (int j = 0; j < 100; ++j) {
                manager.AddTimer(100, callback, 1);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    // 等待所有添加线程完成
    for (auto& t : threads) {
        t.join();
    }

    // 等待足够时间让定时器触发
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 停止管理器
    manager.Stop();
    managerThread.join();

    // 验证计数器（由于定时器异步触发，实际计数可能小于预期）
    EXPECT_GT(counter, 0);
    std::cout << "Final counter value: " << counter << std::endl;
}

// 测试定时器重置功能
TEST(TimeLineTimerTest, ResetTimer) {
    std::atomic<int> counter(0);
    auto callback1 = [&counter]() { counter += 1; };
    auto callback2 = [&counter]() { counter += 2; };

    TimeLineTimer timer;
    timer.ResetTimer(100, callback1, 2);

    // 触发第一次
    timer.Trigger();
    EXPECT_EQ(counter, 1);

    // 重置定时器
    timer.ResetTimer(200, callback2, 1);

    // 触发重置后的定时器
    timer.Trigger();
    EXPECT_EQ(counter, 3);

    // 再次触发（应无效果）
    timer.Trigger();
    EXPECT_EQ(counter, 3);
}

// 测试边界条件：零间隔定时器
TEST(TimeLineTimerTest, ZeroIntervalTimer) {
    std::atomic<int> counter(0);
    auto callback = [&counter]() { counter++; };

    // 创建零间隔定时器
    TimeLineTimer timer(0, callback, 2);

    // 触发两次
    timer.Trigger();
    timer.Trigger();
    EXPECT_EQ(counter, 2);

    // 再次触发（应无效果）
    timer.Trigger();
    EXPECT_EQ(counter, 2);
}

// 测试边界条件：无限重复定时器
TEST(TimeLineTimerTest, InfiniteRepeatTimer) {
    std::atomic<int> counter(0);
    auto callback = [&counter]() { counter++; };

    // 创建无限重复定时器
    TimeLineTimer timer(100, callback, -1);

    // 触发10次
    for (int i = 0; i < 10; ++i) {
        timer.Trigger();
    }
    EXPECT_EQ(counter, 10);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}