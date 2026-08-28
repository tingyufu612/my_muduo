#include "TcpServer.h"
#include "Logger.h"
#include "TcpConnection.h"
#include "EventLoopThreadPool.h"

#include <strings.h>
#include <functional>

static EventLoop* CheckLoopNotNull(EventLoop *loop)
{
    if (loop == nullptr)
    {
        LOG_FATAL("%s:%s:%d mainLoop is null! \n", __FILE__, __FUNCTION__, __LINE__);
    }
    return loop;
}


TcpServer::TcpServer(EventLoop *loop,
                const InetAddress &listenAddr,
                const std::string &nameArg,
                Option option)
                    :loop_(CheckLoopNotNull(loop))
                    ,inPort_(listenAddr.toIpPort())
                    ,name_(nameArg)
                    ,acceptor_(new Acceptor(loop,listenAddr,option==kReusePort))
                    ,threadPool_(new EventLoopThreadPool(loop,name_))
                    , connectionCallback_()
                    , messageCallback_()
                    , nextConnId_(1)
                    , started_(0)
{
    //有新用户连接时，会执行acceptor 中的handelread ----- 会执行TcpServer::newConnection回调
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
}



void TcpServer::setThreadNum(int numThreads)//设置底层sub库
{
    threadPool_->setThreadNum(numThreads);
}

//loop.loop()
void TcpServer::start()
{
   if (started_++ == 0) // 防止一个TcpServer对象被start多次
    {
        threadPool_->start(threadInitCallback_); // 启动底层的loop线程池
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}


void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr)
{

}