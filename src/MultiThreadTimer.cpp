//
// Created by cxk_zjq on 25-5-30.
//

#include "MultiThreadTimer.h"

namespace cxk
{
MultiThreadTimer::MultiThreadTimer() noexcept
: m_isRunning(false), m_interval(0), repeatCount(-1)
{

}

MultiThreadTimer::~MultiThreadTimer()
{

}

void MultiThreadTimer::SetRepeatCount(std::size_t count)
{
    repeatCount = count;
}

void MultiThreadTimer::Stop()
{
    m_isRunning.store(false);
    if (m_thread.joinable()) {
        m_thread.join(); // 等待线程结束
    }
}

void MultiThreadTimer::Start(int interval, MultiThreadTimer::TimerCallback &callback, std::size_t repeat)
{
    if (m_isRunning.load()) {
        return; // 如果定时器已经在运行,则不再启动
    }
    m_interval = interval;
    m_callback = callback;
    repeatCount = repeat;
    m_isRunning.store(true);
    m_thread = std::thread([this]() {
        if (repeatCount < 0) {
            while (m_isRunning.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
                if (m_isRunning.load()) { // 双检查，避免在睡眠期间被停止
                    m_callback();
                } else {
                    return;
                }
            }
        } else if (repeatCount == 0) {
            return; // 不执行任何操作
        } else if (repeatCount > 0) {
            for (std::size_t i = 0; i < repeatCount; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
                if (m_isRunning.load()) {
                    m_callback();
                } else {
                    return;
                }
            }
        }
    });
}
template<typename F, typename... Args>
void MultiThreadTimer::Start(int interval, F &&f, Args &&... args)
{
    if (m_isRunning.load()) {
        return; // 如果定时器已经在运行,则不再启动
    }
    m_interval = interval;
    // 将回调函数和参数绑定
    m_callback = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
    m_isRunning.store(true);
    m_thread = std::thread([this]() {
        if (repeatCount < 0) {
            while (m_isRunning.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
                if (m_isRunning.load()) { // 双检查，避免在睡眠期间被停止
                    m_callback();
                } else {
                    return;
                }
            }
        } else if (repeatCount == 0) {
            return; // 不执行任何操作
        } else if (repeatCount > 0) {
            for (std::size_t i = 0; i < repeatCount; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(m_interval));
                if (m_isRunning.load()) {
                    m_callback();
                } else {
                    return;
                }
            }
        }
    });
}

template<typename F, typename... Args>
MultiThreadTimer::MultiThreadTimer(int interval, F &&f, Args &&... args, std::size_t repeat)
: m_isRunning(false), m_interval(interval), repeatCount(repeat)
{
    // 将回调函数和参数绑定
    m_callback = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
}
} // cxk