//
// Created by cxk_zjq on 25-5-30.
//

#include "TimeLineTimer.h"
#include <clock.h>


namespace cxk
{
TimeLineTimer::TimeLineTimer() noexcept
: repeatCount(-1), m_startTime(0), m_endTime(0), m_interval(0)
{
    m_startTime = GetCurrentTime();
}


TimeLineTimer::TimeLineTimer(std::size_t interval, TimeLineTimer::TimerCallback &callback, std::size_t repeat)
: repeatCount(repeat), m_startTime(0), m_endTime(0), m_interval(interval), m_callback(callback)
{
    m_startTime = GetCurrentTime();
    if(repeatCount>0&&repeatCount>std::numeric_limits<std::size_t>::max()/m_interval)
    {
        repeatCount = std::numeric_limits<std::size_t>::max()/m_interval; // 防止溢出
    }
    m_endTime = m_startTime + m_interval * repeatCount; // 计算结束时间
}


void TimeLineTimer::SetRepeatCount(int count)
{

}

void TimeLineTimer::Trigger()
{
    if(m_callback== nullptr||repeatCount==0) // 没有设置回调函数,m_callback为空
    {
        return;
    }
    m_callback();
    m_startTime+=m_interval; // 更新下一次的开始时间
    repeatCount--;
}

std::size_t TimeLineTimer::GetCurrentTime()
{
    return FastSteadyClock::now().time_since_epoch().count() / 1000000; // 转换为毫秒
}

TimeLineTimer::~TimeLineTimer()
{

}

void TimerManager::AddTimer(std::size_t interval, TimeLineTimer::TimerCallback &callback, std::size_t repeat)
{
    TimeLineTimer timer(interval, callback, repeat);
    std::lock_guard<std::mutex> lock(m_mutex_queue);
    m_task_queue.emplace(TimeLineTimer::GetCurrentTime(), timer); // 将时间轴定时器放入task_queue
}

void TimerManager::Update()
{
    {
        std::lock_guard <std::mutex> lock(m_mutex_queue);
        if (m_timers.empty() && m_task_queue.empty()) // 如果定时器和队列为空,则直接返回
        {
            return;
        }
        if (!m_task_queue.empty()) {
            while (!m_task_queue.empty()) {
                m_timers.insert(m_task_queue.front());
                m_task_queue.pop();
            }
        }
        auto currentTime = TimeLineTimer::GetCurrentTime();
        for(auto it = m_timers.begin(); it != m_timers.end();)
        {
            if(it->second.m_startTime <= currentTime) // 如果当前时间大于等于定时器的开始时间
            {
                it->second.Trigger(); // 触发定时器
                if(it->second.repeatCount == 0) // 如果定时器已经结束,则删除
                {
                    it = m_timers.erase(it);
                }
                else // 否则更新开始时间
                {
                    it->second.m_startTime += it->second.m_interval; // 更新下一次的开始时间
                    ++it;
                }
            }
            else
            {
                ++it; // 如果当前时间小于定时器的开始时间,则继续下一个定时器
            }
        }
    }
}

void TimerManager::Start()
{
    m_isRunning.store(true);
    while(m_isRunning.load())
    {
        Update();
    }
}

void TimerManager::Stop()
{
    m_isRunning.store(false);
}
} // cxk