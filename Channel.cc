#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

#include <sys/epoll.h>
const int Channel::kNoneEvent=0;
const int Channel::kReadEvent=EPOLLIN|EPOLLOUT;
const int Channel::kWriteEvent=EPOLLOUT;

Channel::Channel(EventLoop *loop,int fd)
    : loop_(loop),fd_(fd),events_(0),revents_(0),index_(-1),tied_(false)
{

}

Channel::~Channel()
{
}

void Channel::tie(const std::shared_ptr<void> &obj)
{
    tie_=obj;
    tied_=true;
}

/**
 * update 负责poller里更改fd相应事件epoll——ctl
*/
void Channel::update()
{
    // add code
    //loop->updateChannel(this);
}

//在所属的eventloop中删除当前的channel
void Channel::remove()
{
    //add code
    //loop->removeChannel(this);
}

void Channel::handleEvent(Timestamp receiveTime)
{
    if(tied_)
    {
        std::shared_ptr<void> guard=tie_.lock();
        if(guard)
        {
            handleEventWithGuard(receiveTime);
        }
    }
}

//根据poller通知的事件，由channel负责调用的回调操作
void Channel::handleEventWithGuard(Timestamp receiveTime)
{
    LOG_INFO("channel handleEvent revents: %d", revents_);
    
    if((revents_&EPOLLHUP)&&!(revents_&EPOLLIN))
    {
        if(closeCallback_)
        {
            errorCallback_();
        }
    }

    if(revents_&EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }

    if(revents_&(EPOLLIN|EPOLLPRI))
    {
        if(readCallback_)
        {
            readCallback_(receiveTime);
        }
    }

    if(revents_&EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
}