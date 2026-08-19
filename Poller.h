#pragma once

#include <vector>
#include <unordered_map>

#include "Timestamp.h"
#include "noncopyable.h"
//库中多路事件分发器核心Io复用

class Channel;
class EventLoop;

class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop *loop);
    virtual ~Poller() = default;

    //给所有IO复用保留统一接口
    virtual Timestamp poll(int timeoutMs, ChannelList *activeChannels)=0;
    virtual void updateChannel(Channel *channel)=0;
    virtual void removeChannel(Channel *channel)=0;

    //判断参数channel是否在当前poller当中
    bool hasChannel(Channel *channel) const;

    //派生类不好在基类中实现
    static Poller* newDefaultPoller(EventLoop *loop);

protected:
//MAP的key：sockfd， value：sockfd所属的channel类型
    using ChannelMap=std::unordered_map<int,Channel*>;
    ChannelMap channels_;

private:
    EventLoop *ownerLoop_;//定义poller所属的事件循环eventloop
};
