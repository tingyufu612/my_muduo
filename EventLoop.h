#pragma once

#include <functional>
#include <vector>
#include <atomic>
#include <memory>
#include <mutex>

#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

class Channel;
class Poller;



class EventLoop:noncopyable
{
public:
    using Functor =std::function<void()>;
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    Timestamp pollReturnTime() const {return pollReturnTime_;}

    // 在当前loop中执行cb
    void runInLoop(Functor cb);
    // 把cb放入队列中，唤醒loop所在的线程，执行cb
    void queueInLoop(Functor cb);

    // 用来唤醒loop所在的线程的
    void wakeup();

    // EventLoop的方法 =》 Poller的方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    // 判断EventLoop对象是否在自己的线程里面
    bool isInLoopThread() const { return threadId_ ==  CurrentThread::tid(); }
    

private:

    void handleRead(); // wake up
    void doPendingFunctors(); // 执行回调

    using ChannelList=std::vector<Channel*>;
    std::atomic_bool looping_;//原子操作 通过CAS实现
    std::atomic_bool quit_;//标识 退出loop
    
    const pid_t threadId_;//记录当前loop的id
    Timestamp pollReturnTime_;//poller返回事件的channels的时间
    std::unique_ptr<Poller> poller_;

    int wakeupFd_;//用linux系统中的eventfd 当mainLoop获取新用户的channel，通过轮询算法选择一个subloop，通过该成员你唤醒处理channel
    std::unique_ptr<Channel> wakeupChannel_;

    ChannelList activeChannels_;
    //Channel *currentActiveChannel_;

    std::atomic_bool callingPendingFunctors_;//当前loop是否从需要执行回调操作
    std::vector<Functor> pendingFunctors_;//存储需要执行的所有回调参数、
    std::mutex mutex_;//互斥锁 用来保护vector中线程安全操作


    
};