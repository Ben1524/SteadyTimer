//
// Created by cxk_zjq on 25-5-30.
//

#ifndef STEADYTIMER_TIMELINETIMER_H
#define STEADYTIMER_TIMELINETIMER_H
#include <functional>
#include <map>
#include "Singleton.h"
#include <mutex>
#include <queue>
#include <atomic>

namespace cxk
{

/*
 * \@brief 时间轴定时器类
 * 该类用于实现基于时间轴的定时器功能，支持设置回调函数和重复次数。
 */

class TimeLineTimer {
public:
    friend class TimerManager;

    using TimerCallback = std::function<void()>;
    explicit TimeLineTimer()noexcept;

    template<class Func,typename...Arg>
    explicit TimeLineTimer(std::size_t interval,Func&&f,Arg&&...arg,std::size_t repeat=-1); // 设置回调函数

    explicit TimeLineTimer(std::size_t interval,TimerCallback& callback,std::size_t repeat=-1); // 设置回调函数

    template<class Func,typename...Arg>
    void ResetTimer(int interval,Func&&f,Arg&&...arg,std::size_t repeat=-1); // 设置回调函数
    void SetRepeatCount(int count);
    void Trigger();
    static std::size_t GetCurrentTime();

    ~TimeLineTimer();

private:
    std::size_t repeatCount; // 重复次数
    std::size_t m_startTime,m_endTime;
    TimerCallback m_callback;
    std::size_t m_interval; // ms
};

template<class Func, typename... Arg>
TimeLineTimer::TimeLineTimer(std::size_t interval, Func &&f, Arg &&... arg, std::size_t repeat)
{
    m_interval = interval;
    m_callback = [f = std::forward<Func>(f), args = std::make_tuple(std::forward<Arg>(arg)...)]() mutable {
        std::apply(f, args); // 使用std::apply调用函数
    };
    repeatCount = repeat;
    m_startTime = GetCurrentTime();
    m_endTime = m_startTime + m_interval * repeatCount; // 计算结束时间
}


template<class Func, typename... Arg>
void TimeLineTimer::ResetTimer(int interval, Func &&f, Arg &&... arg, std::size_t repeat)
{
    m_interval = interval;
    m_callback = [f = std::forward<Func>(f), args = std::make_tuple(std::forward<Arg>(arg)...)]() mutable {
        std::apply(f, args); // 使用std::apply调用函数
    };
    repeatCount = repeat;
    m_startTime = GetCurrentTime();
    m_endTime = m_startTime + m_interval * repeatCount; // 计算结束时间

}


/**
 * \@brief 定时器管理类
 * 该类用于管理多个时间轴定时器实例，提供添加、更新和启动等功能。调度的间隔在这是设计
 */
class TimerManager {
    SINGLETON(TimerManager)
public:

    template<class F,typename...Args>
    void AddTimer(std::size_t interval,F&&f,Args&&...args, std::size_t repeat=-1);

    void AddTimer(std::size_t interval, TimeLineTimer::TimerCallback &callback, std::size_t repeat=-1);

    void Update(); // 更新定时器状态
    void Start(); // 启动定时器
    void Stop(); // 停止定时器
private:
    std::multimap<std::size_t , TimeLineTimer> m_timers; // 用于存储时间轴定时器,时间轴定时器按照开始时间排序
    std::queue<std::pair<std::size_t,TimeLineTimer>> m_task_queue; // 用于存储待定添加的定时器
    std::atomic<bool> m_isRunning{}; // 定时器是否正在运行
    std::mutex m_mutex_queue; // 保护task_queue

};

template<class F, typename... Args>
void TimerManager::AddTimer(std::size_t interval, F &&f, Args &&... args, std::size_t repeat)
{
    auto timer = TimeLineTimer(interval, std::forward<F>(f), std::forward<Args>(args)..., repeat);
    std::lock_guard<std::mutex> lock(m_mutex_queue); // 保护task_queue
    m_task_queue.emplace(TimeLineTimer::GetCurrentTime(), timer); // 将时间轴定时器放入task_queue
}

} // cxk

#endif //STEADYTIMER_TIMELINETIMER_H
