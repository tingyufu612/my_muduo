#pragma once

#include "Poller.h"

#include <vector>
#include <sys/epoll.h>

class Channel;
/**
 * epoll_create
 * epoll_ctl add/mod/del
 * epoll_wait
*/

class EPollPoller : public Poller
{
public:
 // epoll_create
    EPollPoller(EventLoop *loop);
    ~EPollPoller() override;
// 重写 基类Poller中的抽象方法
    Timestamp poll(int timeoutMs,ChannelList *activeChannels) override;//epoll_wait
    void updateChannel(Channel *channel) override;//epoll_ctl
    void removeChannel(Channel *channel) override;//epoll_ctl


private:
    static const int kInitEventListSize=16;
    //填写活跃链接
    void fillActiveChannels(int numEvents,ChannelList *activeChannels) const;
    //更新通道
    void update(int operation , Channel *channel);

    using EventList =std::vector<epoll_event>;

    int epollfd_;
    EventList events_;
};