#include "EPollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>
#include <strings.h>

const int kNew=-1;//Channel 的成员index_—=-1  未添加到poller
const int kAdded=1;//已添加到poller
const int kDeleted=2;//从polle中删除

EPollPoller::EPollPoller(EventLoop *loop)
    :Poller(loop)
    ,epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    ,events_(kInitEventListSize) //VECTOR<EPOLL_EVENT>
{
    if(epollfd_<0){
        LOG_FATAL("epoll_create error:%d \n",errno);
    }
}

EPollPoller::~EPollPoller()
{
    ::close(epollfd_);
}

Timestamp  EPollPoller::poll(int timeoutMs,ChannelList *activeChannels) //epoll_wait
{
    LOG_DEBUG("func=%s => fd total count:%lu \n", __FUNCTION__, channels_.size());

    int numEvents=::epoll_wait(epollfd_, &*events_.begin(),static_cast<int>(events_.size()),timeoutMs);
    int saveErrno=errno;
    Timestamp now(Timestamp::now());

    if(numEvents>0)
    {
        LOG_DEBUG("%d events happened \n", numEvents);
        fillActiveChannels(numEvents,activeChannels);//填入list
        //扩容
        if(numEvents==events_.size())
        {
            events_.resize(events_.size()*2);
        }
    }else if (numEvents==0)
    {
        LOG_DEBUG("%s timeout! \n", __FUNCTION__);
    }else
    {
        if (saveErrno != EINTR)
        {
         errno = saveErrno;
         LOG_ERROR("EPollPoller::poll() err!");
        }
    }
    return now;
}

// channel update remove => EventLoop updateChannel removeChannel => Poller updateChannel removeChannel
/**
 *            EventLoop  =>   poller.poll
 *     ChannelList      Poller
 *                     ChannelMap  <fd, channel*>   epollfd
 */ 
void EPollPoller::updateChannel(Channel *channel) //epoll_ctl
{
    const int index=channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n",__FUNCTION__,channel->fd(),channel->events(), index);
    if(index==kNew||index==kDeleted)
    {
        if(index==kNew)
        {
            int fd=channel->fd();
            channels_[fd]=channel;
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD,channel);

    }else{
        //channel在poller注册过
        int fd=channel->fd();
        if(channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL,channel);
            channel->set_index(kDeleted);
        }
        else{
            update(EPOLL_CTL_MOD,channel);
        }
    }

}

void EPollPoller::removeChannel(Channel *channel) //epoll_ctl
{
    int fd=channel->fd();
    int index=channel->index();

    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, fd);
    
    channels_.erase(fd);
    if(index==kAdded)
    {
        update(EPOLL_CTL_DEL,channel);
    }

    channel->set_index(kNew);
}

//填写活跃链接
void EPollPoller::fillActiveChannels(int numEvents,ChannelList *activeChannels) const
{
     for(int i=0;i<numEvents;++i)
     {
        Channel *channel=static_cast<Channel*>(events_[i].data.ptr);
        channel->set_revents(events_[i].events);
        activeChannels->push_back(channel);
     }
}
    //更新通道 epoll_mod 
void EPollPoller::update(int operation , Channel *channel)
{
    epoll_event event;
    bzero(&event, sizeof event);
    int fd=channel->fd();
    event.events=channel->events();
    event.data.ptr=channel;
    int fd=channel->fd();

    if(::epoll_ctl(epollfd_,operation ,fd,&event)<0)
    {
        LOG_ERROR("epoll_ctl del error:%d\n",errno);

    }
    else
    {
        LOG_FATAL("epoll_ctl add/mod error:%d\n",errno);
    }
}