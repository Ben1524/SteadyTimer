//
// Created by cxk_zjq on 25-5-30.
//

#ifndef STEADYTIMER_MULTITHREADTIMER_H
#define STEADYTIMER_MULTITHREADTIMER_H

#include <thread>
#include <atomic>
#include <functional>

namespace cxk
{

class MultiThreadTimer {
public:
    using TimerCallback = std::function<void()>;
    explicit MultiThreadTimer() noexcept;
    MultiThreadTimer(const MultiThreadTimer&) = delete;
    template<typename F, typename... Args>
    explicit MultiThreadTimer(int interval, F&& f, Args&&... args,std::size_t repeat = -1);
    ~MultiThreadTimer();

    template<typename F, typename... Args>
    void Start(int interval, F&& f, Args&&... args);
    void Start(int interval, TimerCallback& callback, std::size_t repeat = -1);
    void SetRepeatCount(std::size_t count);
    void Stop();
private:
    std::thread m_thread;
    std::atomic<bool> m_isRunning;
    TimerCallback m_callback;
    int m_interval; // ms
    std::size_t repeatCount;
};

} // cxk

#endif //STEADYTIMER_MULTITHREADTIMER_H
