#include "EventLoopThread.h"
#include "Thread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), 
        const std::string &name)
        : loop_(nullptr)
        ,exiting_(false)
        ,thread_(std::bind(&EventLoopThread::threadFunc,this),name)
        ,mutex_()
        ,cond_()
        ,callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_=true;
    if(loop_!=nullptr)
    {
        loop_->quit();
        thread_.join();

    }
}

EventLoop* EventLoopThread::startLoop()
{
    thread_.start();//启动底层的新线程

    EventLoop* loop=nullptr;
    {
        //进行线程间通信
        std::unique_lock<std::mutex> lock(mutex_);
        while(loop_==nullptr)
        {
            cond_.wait(lock);

        }
        loop=loop_;
    }
    return loop;
}

//实现单独的新线程里面运行
void EventLoopThread::threadFunc()
{
    EventLoop loop;//创建独立的eventloop 和上面的线程一一对应
    if(callback_)
    {
        callback_(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_=&loop;
        cond_.notify_one();

    }
    loop.loop(); //执行了eventloop中的loop函数 进行poller中poll 阻塞 进行读写事件

    //进入以下代码证明服务器要关闭了
    std::unique_lock<std::mutex> lock(mutex_);
    loop_=nullptr;
}