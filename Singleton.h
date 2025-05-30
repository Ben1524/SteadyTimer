//
// Created by cxk_zjq on 25-5-30.
//

#ifndef STEADYTIMER_SINGLETON_H
#define STEADYTIMER_SINGLETON_H


namespace cxk
{
template <typename T>
class Singleton
{
public:
    static T& GetInstance()
    {
        static T instance; // 静态局部变量,在程序结束时会被销毁
        return instance;
    }
private:
    Singleton()= default;
    Singleton(const Singleton&)= delete;
    Singleton&operator=(const Singleton&)= delete;
};

#define SINGLETON(classname) \
    friend class cxk::Singleton<classname>; \
    private:                \
    classname(const classname&)= delete; \
    classname&operator=(const classname&)= delete; \
    classname()= default;    \
    ~classname()=default;
}


#endif //STEADYTIMER_SINGLETON_H
